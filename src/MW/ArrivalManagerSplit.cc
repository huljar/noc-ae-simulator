//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "ArrivalManagerSplit.h"
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>
#include <cmath>
#include <sstream>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(ArrivalManagerSplit);

ArrivalManagerSplit::ArrivalManagerSplit() {
}

ArrivalManagerSplit::~ArrivalManagerSplit() {
    for(auto it = arqTimers.begin(); it != arqTimers.end(); ++it)
        cancelAndDelete(it->second);

    for(auto it = ucReceivedSplitsCache.begin(); it != ucReceivedSplitsCache.end(); ++it) {
        delete it->second.first;
        delete it->second.second;
    }
    for(auto it = ucDecryptedSplitsCache.begin(); it != ucDecryptedSplitsCache.end(); ++it) {
        delete it->second.first;
        delete it->second.second;
    }
    for(auto it = ucComputedMacsCache.begin(); it != ucComputedMacsCache.end(); ++it) {
        delete it->second.first;
        delete it->second.second;
    }

    for(auto it = ucPlannedArqs.begin(); it != ucPlannedArqs.end(); ++it)
        delete it->second;

    for(auto it = ncReceivedSplitsCache.begin(); it != ncReceivedSplitsCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;
    for(auto it = ncDecryptedSplitsCache.begin(); it != ncDecryptedSplitsCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;
    for(auto it = ncComputedMacsCache.begin(); it != ncComputedMacsCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;

    for(auto it = ncPlannedArqs.begin(); it != ncPlannedArqs.end(); ++it)
        delete it->second;
}

void ArrivalManagerSplit::initialize() {
    arqLimit = par("arqLimit");
    if(arqLimit < 1)
        throw cRuntimeError(this, "arqLimit must be greater than 0");

    arqIssueTimeout = par("arqIssueTimeout");
    if(arqIssueTimeout < 1)
        throw cRuntimeError(this, "arqIssueTimeout must be greater than 0");

    arqAnswerTimeoutBase = par("arqAnswerTimeoutBase");
    if(arqAnswerTimeoutBase < 1)
        throw cRuntimeError(this, "arqAnswerTimeoutBase must be greater than 0");

    lastArqWaitForOngoingVerifications = par("lastArqWaitForOngoingVerifications");

    finishedIdsTracked = par("finishedIdsTracked");
    if(finishedIdsTracked < 0)
        throw cRuntimeError(this, "finishedIdsTracked must be greater than or equal to 0");

    gridColumns = getAncestorPar("columns");
    nodeId = getAncestorPar("id");

    nodeX = nodeId % gridColumns;
    nodeY = nodeId / gridColumns;

    // Compute ARQ answer timeouts
    int gridRows = getAncestorPar("rows");
    for(int y = 0; y < gridRows; ++y) {
        for(int x = 0; x < gridColumns; ++x) {
            if(!(x == nodeX && y == nodeY)) {
                arqAnswerTimeouts[Address2D(x, y)] = arqAnswerTimeoutBase + 2 * (std::abs(x - nodeX) + std::abs(y - nodeY));
            }
        }
    }
}

void ArrivalManagerSplit::handleMessage(cMessage* msg) {
    if(msg->isSelfMessage()) {
        ArqTimer* timer = check_and_cast<ArqTimer*>(msg);
        handleArqTimer(timer);
    }
    else {
        // Confirm that this is a flit
        Flit* flit = dynamic_cast<Flit*>(msg);
        if(!flit) {
            EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
            delete msg;
            return;
        }

        // Handle flits arriving from the network
        if(strcmp(flit->getArrivalGate()->getName(), "netIn") == 0) {
            handleNetMessage(flit);
        }
        else { // arrivalGate == cryptoIn
            handleCryptoMessage(flit);
        }
    }
}

void ArrivalManagerSplit::handleNetMessage(Flit* flit) {
    // Get parameters
    uint32_t id = flit->getGidOrFid();
    Address2D source = flit->getSource();

    // Check if this is a finished ID (might be a repeat attack or redundant transmission)
    if(finishedIds[source].first.count(id)) {
        EV << "Received a flit from " << source << " with ID " << id << ", but the ID is already finished." << std::endl;
        delete flit;
        return;
    }

    // We now know that this flit's ID from this source is not finished
    // Get parameters
    IdSourceKey key = std::make_pair(id, source);
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());

    // Determine if we are network coding or not
    if(ncMode == NC_UNCODED) {
        // Check if we already have something from this ID/source
        SplitCache::iterator splitIter = ucReceivedSplitsCache.find(key);
        if(splitIter != ucReceivedSplitsCache.end()) {
            // Check if we already have this mode; if not, insert it
            if(mode == MODE_SPLIT_1 && !splitIter->second.first) {
                EV_DEBUG << "Caching first split \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
                splitIter->second.first = flit;
            }
            else if(mode == MODE_SPLIT_2 && !splitIter->second.second) {
                EV_DEBUG << "Caching second split \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
                splitIter->second.second = flit;
            }
            else {
                EV << "Received a split from " << source << " with ID " << id << ", but we either have it already or the mode is unknown ("
                   << cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode) << ")" << std::endl;
                delete flit;
                return;
            }
        }
        else {
            // We don't have anything from this ID/source yet
            if(mode == MODE_SPLIT_1) {
                EV_DEBUG << "Caching first split \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
                splitIter = ucReceivedSplitsCache.emplace(key, std::make_pair(flit, nullptr)).first;
            }
            else if(mode == MODE_SPLIT_2) {
                EV_DEBUG << "Caching second split \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
                splitIter = ucReceivedSplitsCache.emplace(key, std::make_pair(nullptr, flit)).first;
            }
            else {
                EV << "Received a split from " << source << " with ID " << id << ", but the mode is unknown ("
                   << cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode) << ")" << std::endl;
                delete flit;
                return;
            }
        }

        // In case this flit is already in a planned ARQ, remove it from there
        ucTryRemoveFromPlannedArq(key, (mode == MODE_SPLIT_1 ? ARQ_SPLIT_1 : ARQ_SPLIT_2));

        // Check if both splits have arrived now
        if(splitIter->second.first && splitIter->second.second) {
            // Cancel ARQ timer
            cancelArqTimer(key);
        }
        // If there is no planned ARQ, start/update the ARQ timer
        else if(!ucCheckArqPlanned(key)) {
            setArqTimer(key, ncMode);
        }

        // Start decryption and authentication for the new split
        ucStartDecryptAndAuth(key, mode);
    }
    else { // ncMode != uncoded
        // Get parameters
        uint16_t gev = flit->getGev();
        GevCache& gevCache = ncReceivedSplitsCache[key];

        // Assert that this is a network coded split
        ASSERT(mode == MODE_SPLIT_NC);

        // Check if the split cache already contains a split with this GEV
        if(gevCache.count(gev)) {
            // We already have a split with this GEV cached, we don't need this one
            EV << "Received a split from " << source << " with GID " << id << " and GEV " << gev
               << ", but we already have a split cached" << std::endl;
            delete flit;
            return;
        }

        // Check if there are already enough splits for the used NC mode
        if((ncMode == NC_G2C3 && gevCache.size() >= 3) || (ncMode == NC_G2C4 && gevCache.size() >= 4)) {
            EV << "Received a split from " << source << " with GID " << id << " and GEV " << gev
               << ", but we already have all the splits from this generation" << std::endl;
            delete flit;
            return;
        }

        // We can safely cache the flit now
        EV_DEBUG << "Caching split \"" << flit->getName() << "\" from " << source << " with ID " << id << " and GEV " << gev << std::endl;
        gevCache.emplace(gev, flit);

        // In case this flit is already in a planned ARQ, remove it from there
        ncTryRemoveFromPlannedArq(key, GevArqMap{{gev, ARQ_SPLIT_NC}});

        // Check if we can cancel the ARQ timer
        if(ncCheckCompleteGenerationReceived(key, flit->getNumCombinations())) {
            // Cancel ARQ timer
            cancelArqTimer(key);
        }
        // If there is no planned ARQ, start/update the ARQ timer
        else if(!ncCheckArqPlanned(key)) {
            setArqTimer(key, ncMode);
        }

        // Send the split to decryption and authentication
        ncStartDecryptAndAuth(key, gev);
    }
}

void ArrivalManagerSplit::handleCryptoMessage(Flit* flit) {
    // Get parameters
    uint32_t id = flit->getGidOrFid();
    Address2D source = flit->getSource();
    IdSourceKey key = std::make_pair(id, source);
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());
    Status status = static_cast<Status>(flit->getStatus());

    // Check network coding mode
    if(ncMode == NC_UNCODED) {
        // Check if this ID is already finished
        const IdSet& finishedIdSet = finishedIds[source].first;
        if(finishedIdSet.count(id)) {
            EV_DEBUG << "Received a decrypted/authenticated flit for finished FID " << id << std::endl;
            delete flit;
            return;
        }

        // Check flit status
        if(status == STATUS_DECRYPTING) {
            flit->setStatus(STATUS_NONE);

            // This is a decrypted flit arriving from a crypto unit
            if(mode == MODE_SPLIT_1) {
                // Check if we have to discard this flit (because corruption was detected on MAC verification)
                if(ucDiscardDecrypting[key].first > 0) {
                    EV_DEBUG << "Discarding corrupted decrypted first split \"" << flit->getName()
                             << "\" (source: " << source << ", ID: " << id << ")" << std::endl;
                    --ucDiscardDecrypting[key].first;
                    delete flit;
                    return;
                }

                // Assert that there is no decrypted split yet
                SplitPair& splits = ucDecryptedSplitsCache[key];
                ASSERT(!splits.first);

                // Insert decrypted split into the cache
                EV_DEBUG << "Caching decrypted first split \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
                splits.first = flit;
            }
            else if(mode == MODE_SPLIT_2) {
                // Check if we have to discard this flit (because corruption was detected on MAC verification)
                if(ucDiscardDecrypting[key].second > 0) {
                    EV_DEBUG << "Discarding corrupted decrypted second split \"" << flit->getName()
                             << "\" (source: " << source << ", ID: " << id << ")" << std::endl;
                    --ucDiscardDecrypting[key].second;
                    delete flit;
                    return;
                }

                // Assert that there is no decrypted split yet
                SplitPair& splits = ucDecryptedSplitsCache[key];
                ASSERT(!splits.second);

                // Insert decrypted split into the cache
                EV_DEBUG << "Caching decrypted second split \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
                splits.second = flit;
            }
            else {
                throw cRuntimeError(this, "Crypto unit sent flit with unexpected mode %u (Source: %s, ID: %u)", mode, source.str().c_str(), id);
            }

            // Try to send out the decrypted splits
            ucTrySendToApp(key);
        }
        else if(status == STATUS_VERIFYING) {
            flit->setStatus(STATUS_NONE);

            // This is a computed MAC arriving from a crypto unit
            if(mode == MODE_SPLIT_1) {
                // Assert that there is no computed MAC yet
                SplitPair& splits = ucComputedMacsCache[key];
                ASSERT(!splits.first);

                // Insert computed MAC into the cache
                EV_DEBUG << "Caching computed MAC for first split \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
                splits.first = flit;
            }
            else if(mode == MODE_SPLIT_2) {
                // Assert that there is no computed MAC yet
                SplitPair& splits = ucComputedMacsCache[key];
                ASSERT(!splits.second);

                // Insert computed MAC into the cache
                EV_DEBUG << "Caching computed MAC for second split \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
                splits.second = flit;
            }
            else {
                throw cRuntimeError(this, "Crypto unit sent flit with unexpected mode %u (Source: %s, ID: %u)", mode, source.str().c_str(), id);
            }

            // Try to verify the split
            ucTryVerification(key, mode);
        }
        else {
            throw cRuntimeError(this, "Crypto unit sent flit with unexpected status %u (Source: %s, ID: %u)", status, source.str().c_str(), id);
        }
    }
    else { // ncMode != uncoded
        // Get parameters
        uint16_t gev = flit->getGev();

        // Check if this generation is already finished
        const IdSet& finishedIdSet = finishedIds[source].first;
        if(finishedIdSet.count(id)) {
            EV_DEBUG << "Received a decrypted/authenticated flit for finished GID " << id << " (GEV " << gev << ")" << std::endl;
            delete flit;
            return;
        }

        // Check flit status
        if(status == STATUS_DECRYPTING) {
            flit->setStatus(STATUS_NONE);

            // This is a decrypted split arriving from a crypto unit
            // Get parameters
            GevCache& gevCache = ncDecryptedSplitsCache[key];

            // Assert that the decrypted splits cache does not contain a flit with this GEV
            ASSERT(!gevCache.count(gev));

            // Check if we have to discard this flit (because corruption was detected on MAC verification)
            if(ncDiscardDecrypting[key][gev] > 0) {
                EV_DEBUG << "Discarding corrupted decrypted split \"" << flit->getName()
                         << "\" (source: " << source << ", ID: " << id << ", GEV: " << gev << ")" << std::endl;
                --ncDiscardDecrypting[key][gev];
                delete flit;
                return;
            }

            // We can safely cache the flit now
            EV_DEBUG << "Caching decrypted split \"" << flit->getName() << "\" from " << source << " with ID " << id << " and GEV " << gev << std::endl;
            gevCache.emplace(gev, flit);

            // Try to send out the decrypted flit
            ncTrySendToApp(key, gev);
        }
        else if(status == STATUS_VERIFYING) {
            flit->setStatus(STATUS_NONE);

            // This is a computed MAC arriving from a crypto unit
            // Get parameters
            GevCache& gevCache = ncComputedMacsCache[key];

            // Assert that the decrypted data cache does not contain a flit with this GEV
            ASSERT(!gevCache.count(gev));

            // We can safely cache the flit now
            EV_DEBUG << "Caching computed MAC for split \"" << flit->getName() << "\" from " << source << " with ID " << id << " and GEV " << gev << std::endl;
            gevCache.emplace(gev, flit);

            // Try to verify the flit (in case the received MAC is already there)
            ncTryVerification(key, gev);
        }
        else {
            throw cRuntimeError(this, "Crypto unit sent flit with unexpected status %u (Source: %s, ID: %u)", status, source.str().c_str(), id);
        }
    }
}

void ArrivalManagerSplit::handleArqTimer(ArqTimer* timer) {
    // Get parameters
    uint32_t id = timer->getGidOrFid();
    Address2D source = timer->getSource();
    IdSourceKey key = std::make_pair(id, source);
    NcMode ncMode = static_cast<NcMode>(timer->getNcMode());

    EV_DEBUG << "ARQ timeout triggered for source " << key.second << ", ID " << key.first << std::endl;

    // Check if we have reached the ARQ limit
    if(issuedArqs[key] >= static_cast<unsigned int>(arqLimit)) {
        // We have failed completely, clean up everything and discard flits from this ID
        EV_DEBUG << "ARQ limit reached for source " << key.second << ", ID " << key.first << std::endl;
        if(ncMode == NC_UNCODED)
            ucCleanUp(key);
        else
            ncCleanUp(key);
        return;
    }

    // Check network coding
    if(ncMode == NC_UNCODED) {
        ArqMode arqMode;

        // Check what is missing
        SplitPair& splits = ucReceivedSplitsCache.at(key);
        if(!splits.first && !splits.second) {
            // Both splits are missing
            arqMode = ARQ_SPLITS_BOTH;
        }
        else if(!splits.first && splits.second) {
            // Only first split is missing
            arqMode = ARQ_SPLIT_1;
        }
        else if(splits.first && !splits.second) {
            // Only second split is missing
            arqMode = ARQ_SPLIT_2;
        }
        else {
            // No flits are missing, this should never happen
            throw cRuntimeError(this, "ARQ timeout triggered for source %s, ID %u, but no flits are missing", key.second.str().c_str(), key.first);
        }

        // Issue the ARQ
        ucIssueArq(key, arqMode);
    }
    else { // ncMode != uncoded
        // Get parameters
        const GevCache& receivedSplits = ncReceivedSplitsCache[key];

        // We always use TELL_RECEIVED because there are no separate data/mac flits
        Mode flitMode = MODE_ARQ_TELL_RECEIVED;
        GevArqMap arqModes;

        // Iterate over received splits and insert them
        for(auto it = receivedSplits.begin(); it != receivedSplits.end(); ++it) {
            arqModes.emplace(it->first, ARQ_SPLIT_NC);
        }

        // Issue the ARQ
        ncIssueArq(key, flitMode, arqModes, ncMode);
    }
}

void ArrivalManagerSplit::ucStartDecryptAndAuth(const IdSourceKey& key, Mode mode) {
    ASSERT(mode == MODE_SPLIT_1 || mode == MODE_SPLIT_2);

    // Retrieve the requested split from the cache and send a copy out
    // for decryption and authentication
    // We use a copy here so that we don't have to remove the split from the cache,
    // which is still used to check if the split has already arrived
    if(mode == MODE_SPLIT_1) {
        EV_DEBUG << "Starting first split decryption for \"" << ucReceivedSplitsCache.at(key).first->getName()
                 << "\" (source: " << key.second << ", ID: " << key.first << ")" << std::endl;
        Flit* copy = ucReceivedSplitsCache.at(key).first->dup();
        send(copy, "cryptoOut");
    }
    else {
        EV_DEBUG << "Starting second split decryption for \"" << ucReceivedSplitsCache.at(key).second->getName()
                 << "\" (source: " << key.second << ", ID: " << key.first << ")" << std::endl;
        Flit* copy = ucReceivedSplitsCache.at(key).second->dup();
        send(copy, "cryptoOut");
    }
}

void ArrivalManagerSplit::ucTryVerification(const IdSourceKey& key, Mode mode) {
    ASSERT(mode == MODE_SPLIT_1 || mode == MODE_SPLIT_2);

    // Get computed MAC and received split
    Flit* recvSplit = (mode == MODE_SPLIT_1 ? ucReceivedSplitsCache.at(key).first : ucReceivedSplitsCache.at(key).second);
    Flit* compMac = (mode == MODE_SPLIT_1 ? ucComputedMacsCache.at(key).first : ucComputedMacsCache.at(key).second);

    // Verify their equality
    bool equal = !recvSplit->isModified() && !compMac->isModified() &&
                 !recvSplit->hasBitError() && !compMac->hasBitError();

    // Examine verification result
    if(equal) {
        // Verification was successful, insert into success cache
        EV_DEBUG << "Successfully verified MAC for \"" << recvSplit->getName() << "\" (source: " << key.second
                 << ", ID: " << key.first << ")" << std::endl;
        (mode == MODE_SPLIT_1 ? ucVerified[key].first : ucVerified[key].second) = true;

        // Try to send out the decrypted splits
        ucTrySendToApp(key);

        // Check if there is a planned ARQ waiting for ongoing verifications to finish
        ucTrySendPlannedArq(key);
    }
    else {
        // Verification was not successful
        EV_DEBUG << "MAC verification failed for \"" << recvSplit->getName() << "\" (source: " << key.second
                 << ", ID: " << key.first << ")" << std::endl;

        // Check if we have reached the ARQ limit
        if(issuedArqs[key] >= static_cast<unsigned int>(arqLimit)) {
            // We have failed completely, clean up everything and discard flits from this ID
            EV_DEBUG << "ARQ limit reached for source " << key.second << ", ID " << key.first << std::endl;
            ucCleanUp(key);
            return;
        }

        // Issue ARQ
        ucIssueArq(key, (mode == MODE_SPLIT_1 ? ARQ_SPLIT_1 : ARQ_SPLIT_2));

        // Clear received cache to ensure we can receive the retransmission
        ucDeleteFromCache(ucReceivedSplitsCache, key, mode);

        // Also clear the decrypted split or, in case it has not arrived yet,
        // mark that it will be discarded on arrival
        if(!ucDeleteFromCache(ucDecryptedSplitsCache, key, mode))
            ++(mode == MODE_SPLIT_1 ? ucDiscardDecrypting[key].first : ucDiscardDecrypting[key].second);

        // Also clear the computed MAC to ensure that we don't compare the next
        // received split against the wrong MAC
        ucDeleteFromCache(ucComputedMacsCache, key, mode);
    }
}

void ArrivalManagerSplit::ucTrySendToApp(const IdSourceKey& key) {
    // Check if both decrypted splits are present and both MACs were successfully verified
    SplitCache::iterator decSplitIter = ucDecryptedSplitsCache.find(key);
    PairVerCache::iterator compMacIter = ucVerified.find(key);
    if(decSplitIter != ucDecryptedSplitsCache.end() && compMacIter != ucVerified.end()) {
        const SplitPair& decSplits = decSplitIter->second;
        const std::pair<bool, bool>& verifications = compMacIter->second;

        if(decSplits.first && decSplits.second && verifications.first && verifications.second) {
            // Send out copies of the decrypted data splits
            // We use copies here to avoid potential conflicts with cleanup
            EV_DEBUG << "Sending out decrypted data splits: source " << key.second << ", ID " << key.first << std::endl;
            send(decSplits.first->dup(), "appOut");
            send(decSplits.second->dup(), "appOut");

            // This split pair is done, initiate cleanup
            ucCleanUp(key);
        }
    }
}

void ArrivalManagerSplit::ucIssueArq(const IdSourceKey& key, ArqMode arqMode) {
    // Assert that we have not reached the maximum number of ARQs
    ASSERT(issuedArqs[key] < static_cast<unsigned int>(arqLimit));

    // Check if there already is a planned ARQ for this ID/source
    FlitCache::iterator plannedIter = ucPlannedArqs.find(key);
    if(plannedIter == ucPlannedArqs.end()) {
        // Create a new ARQ and insert it into the planned ARQ map
        EV_DEBUG << "Initiating planned ARQ for source " << key.second << ", ID " << key.first << ", mode "
                 << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(arqMode) << std::endl;
        ucPlannedArqs.emplace(key, generateArq(key, MODE_ARQ_TELL_MISSING, arqMode));
    }
    else {
        // Merge the planned ARQ with the new ARQ argument
        EV_DEBUG << "Merging planned ARQ for source " << key.second << ", ID " << key.first << " with new mode "
                 << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(arqMode) << std::endl;
        Flit* arq = plannedIter->second;
        ArqMode currentMode = static_cast<ArqMode>(arq->getUcArqs());

        if(arqMode == ARQ_SPLITS_BOTH || (arqMode == ARQ_SPLIT_1 && currentMode == ARQ_SPLIT_2)
                                      || (arqMode == ARQ_SPLIT_2 && currentMode == ARQ_SPLIT_1))
            arq->setUcArqs(ARQ_SPLITS_BOTH);
    }

    ucTrySendPlannedArq(key, !lastArqWaitForOngoingVerifications);
}

void ArrivalManagerSplit::ucTryRemoveFromPlannedArq(const IdSourceKey& key, ArqMode arqMode) {
    // Check if there is an ARQ planned
    FlitCache::iterator plannedIter = ucPlannedArqs.find(key);
    if(plannedIter == ucPlannedArqs.end())
        return;

    // Remove specified mode from the ARQ
    EV_DEBUG << "Removing " << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(arqMode) << " from planned ARQ for source "
             << key.second << ", ID " << key.first << std::endl;
    Flit* arq = plannedIter->second;
    ArqMode currentMode = static_cast<ArqMode>(arq->getUcArqs());

    if(arqMode == ARQ_SPLIT_1 && (currentMode == ARQ_SPLITS_BOTH || currentMode == ARQ_SPLIT_2))
        arq->setUcArqs(ARQ_SPLIT_2);
    else if(arqMode == ARQ_SPLIT_2 && (currentMode == ARQ_SPLITS_BOTH || currentMode == ARQ_SPLIT_1))
        arq->setUcArqs(ARQ_SPLIT_1);
    else {
        // The ARQ is now empty and can be deleted
        EV_DEBUG << "Canceling planned ARQ for source " << key.second << ", ID " << key.first << std::endl;
        delete arq;
        ucPlannedArqs.erase(plannedIter);
    }
}

void ArrivalManagerSplit::ucTrySendPlannedArq(const IdSourceKey& key, bool forceImmediate) {
    // Check if there is an ARQ planned
    FlitCache::iterator plannedIter = ucPlannedArqs.find(key);
    if(plannedIter == ucPlannedArqs.end())
        return;

    // Check if we can send out the ARQ now (forced, more than one ARQ remaining, or no verifications ongoing)
    if(forceImmediate || issuedArqs[key] < static_cast<unsigned int>(arqLimit) - 1 || !ucCheckVerificationOngoing(key)) {
        // Get ARQ
        Flit* arq = plannedIter->second;

        // Send ARQ
        //emit(pktgenerateSignal, flit->getGidOrFid());
        EV << "Sending ARQ \"" << arq->getName() << "\" from " << arq->getSource()
           << " to " << arq->getTarget() << " (ID: " << arq->getGidOrFid() << ") (mode "
           << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(arq->getUcArqs()) << ")" << std::endl;
        send(arq, "arqOut");

        // Remove from planned ARQ map
        ucPlannedArqs.erase(plannedIter);

        // Increment ARQ counter
        ++issuedArqs[key];

        // Set the ARQ timer
        setArqTimer(key, NC_UNCODED, true);
    }
}

void ArrivalManagerSplit::ucCleanUp(const IdSourceKey& key) {
    // Clean up all information related to this ID/address
    EV_DEBUG << "Fully cleaning up: source " << key.second << ", ID " << key.first << std::endl;

    // Clear split caches
    ucDeleteFromCache(ucReceivedSplitsCache, key);
    ucDeleteFromCache(ucDecryptedSplitsCache, key);
    ucDeleteFromCache(ucComputedMacsCache, key);

    // Clear verification result
    ucVerified.erase(key);

    // Clear number of issued ARQs
    issuedArqs.erase(key);

    // Clear corrupted decryption counter
    ucDiscardDecrypting.erase(key);

    // Insert ID into finished ID set
    IdSet& finishedSet = finishedIds[key.second].first;
    IdQueue& finishedQueue = finishedIds[key.second].second;

    finishedSet.insert(key.first);
    finishedQueue.push(key.first);

    // Check if the finished ID set size grew too large
    while(finishedSet.size() > static_cast<size_t>(finishedIdsTracked)) {
        finishedSet.erase(finishedQueue.front());
        finishedQueue.pop();
    }

    // Cancel and delete any remaining ARQ timers
    TimerCache::iterator timerIter = arqTimers.find(key);
    if(timerIter != arqTimers.end()) {
        cancelAndDelete(timerIter->second);
        arqTimers.erase(timerIter);
    }

    // Delete any planned ARQs
    FlitCache::iterator arqIter = ucPlannedArqs.find(key);
    if(arqIter != ucPlannedArqs.end()) {
        delete arqIter->second;
        ucPlannedArqs.erase(arqIter);
    }
}

bool ArrivalManagerSplit::ucDeleteFromCache(SplitCache& cache, const IdSourceKey& key) {
    SplitCache::iterator element = cache.find(key);
    if(element != cache.end()) {
        delete element->second.first;
        delete element->second.second;
        cache.erase(element);
        return true;
    }

    return false;
}

bool ArrivalManagerSplit::ucDeleteFromCache(SplitCache& cache, const IdSourceKey& key, Mode mode) {
    ASSERT(mode == MODE_SPLIT_1 || mode == MODE_SPLIT_2);

    SplitCache::iterator element = cache.find(key);
    if(element != cache.end()) {
        if(mode == MODE_SPLIT_1) {
            if(element->second.first) {
                delete element->second.first;
                element->second.first = nullptr;
                return true;
            }
        }
        else {
            if(element->second.second) {
                delete element->second.second;
                element->second.second = nullptr;
                return true;
            }
        }
    }

    return false;
}

void ArrivalManagerSplit::ncStartDecryptAndAuth(const IdSourceKey& key, uint16_t gev) {
    // Retrieve the requested split from the cache and send a copy out
    // for decryption and authentication
    // We use a copy here so that we don't have to remove the split from the cache,
    // which is still used to check if the split has already arrived
    EV_DEBUG << "Starting split decryption for \"" << ncReceivedSplitsCache.at(key).at(gev)->getName()
             << "\" (source: " << key.second << ", ID: " << key.first << ", GEV: " << gev << ")" << std::endl;
    Flit* copy = ncReceivedSplitsCache.at(key).at(gev)->dup();
    send(copy, "cryptoOut");
}

void ArrivalManagerSplit::ncTryVerification(const IdSourceKey& key, uint16_t gev) {
    // Get parameters
    Flit* recvSplit = ncReceivedSplitsCache.at(key).at(gev);
    Flit* compMac = ncComputedMacsCache.at(key).at(gev);

    // Verify their equality
    bool equal = !recvSplit->isModified() && !compMac->isModified() &&
                 !recvSplit->hasBitError() && !compMac->hasBitError();

    // Examine verification result
    if(equal) {
        // Verification was successful, insert into success cache
        EV_DEBUG << "Successfully verified MAC for \"" << recvSplit->getName() << "\" (source: " << key.second
                 << ", ID: " << key.first << ", GEV: " << gev << ")" << std::endl;
        ncVerified[key].insert(gev);

        // Try to send out the decrypted split
        ncTrySendToApp(key, gev);

        // Check if there is a planned ARQ waiting for ongoing verifications to finish
        ncTrySendPlannedArq(key);
    }
    else {
        // Verification was not successful
        EV_DEBUG << "MAC verification failed for \"" << recvSplit->getName() << "\" (source: " << key.second
                 << ", ID: " << key.first << ", GEV: " << gev << ")" << std::endl;

        // Check if we have reached the ARQ limit
        if(issuedArqs[key] >= static_cast<unsigned int>(arqLimit)) {
            // We have failed completely, clean up everything and discard flits from this ID
            EV_DEBUG << "ARQ limit reached for source " << key.second << ", ID " << key.first << std::endl;
            ncCleanUp(key);
            return;
        }

        // Send out ARQ
        ncIssueArq(key, MODE_ARQ_TELL_MISSING, GevArqMap{{gev, ARQ_SPLIT_NC}}, static_cast<NcMode>(recvSplit->getNcMode()));

        // Clear received cache to ensure we can receive the retransmission
        ncDeleteFromCache(ncReceivedSplitsCache, key, gev);

        // Also clear the decrypted split or, in case it has not arrived yet,
        // mark that it will be discarded on arrival
        if(!ncDeleteFromCache(ncDecryptedSplitsCache, key, gev))
            ++ncDiscardDecrypting[key][gev];

        // Also clear the computed MAC to ensure that we don't compare the next
        // received split against the wrong MAC
        ncDeleteFromCache(ncComputedMacsCache, key, gev);
    }
}

void ArrivalManagerSplit::ncTrySendToApp(const IdSourceKey& key, uint16_t gev) {
    // Get parameters
    GevCache& decSplitsCache = ncDecryptedSplitsCache[key];

    // Check if decrypted split is present and MAC was successfully verified
    GevCache::iterator decSplit = decSplitsCache.find(gev);
    if(decSplit != decSplitsCache.end() && ncVerified[key].count(gev)) {
        // Send out a copy of the decrypted split
        // We use a copy here to avoid potential conflicts with cleanup
        EV_DEBUG << "Sending out decrypted split: source " << key.second << ", ID " << key.first << ", GEV " << gev << std::endl;
        Flit* copy = decSplit->second->dup();
        send(copy, "appOut");

        // Insert this GEV into the dispatched GEV set for this generation
        ncDispatchedGevs[key].insert(gev);

        // Check if this generation is done
        ncCheckGenerationDone(key, decSplit->second->getGenSize());
    }
}

void ArrivalManagerSplit::ncIssueArq(const IdSourceKey& key, Mode mode, const GevArqMap& arqModes, Messages::NcMode ncMode) {
    // Assert that we have not reached the maximum number of ARQs
    ASSERT(issuedArqs[key] < static_cast<unsigned int>(arqLimit));

    // Check if there already is a planned ARQ for this ID/source
    FlitCache::iterator plannedIter = ncPlannedArqs.find(key);
    if(plannedIter == ncPlannedArqs.end()) {
        // Create a new ARQ and insert it into the planned ARQ map
        EV_DEBUG << "Initiating planned ARQ for source " << key.second << ", ID " << key.first << ", mode "
                 << cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode) << " " << arqModes << std::endl;
        ncPlannedArqs.emplace(key, generateArq(key, mode, arqModes, ncMode));
    }
    else {
        // Merge the planned ARQ with the new ARQ arguments
        EV_DEBUG << "Merging planned ARQ for source " << key.second << ", ID " << key.first << " with new mode "
                 << cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode) << " " << arqModes << std::endl;
        plannedIter->second->mergeNcArqModesFlit(mode, arqModes);
    }

    ncTrySendPlannedArq(key, !lastArqWaitForOngoingVerifications);
}

void ArrivalManagerSplit::ncTryRemoveFromPlannedArq(const IdSourceKey& key, const GevArqMap& arqModes) {
    // Check if there is an ARQ planned
    FlitCache::iterator plannedIter = ncPlannedArqs.find(key);
    if(plannedIter == ncPlannedArqs.end())
        return;

    // Remove specified modes from the ARQ
    EV_DEBUG << "Removing " << arqModes << " from planned ARQ for source " << key.second << ", ID " << key.first << std::endl;
    plannedIter->second->removeFromNcArqSplit(arqModes);

    // Check if the ARQ is empty now; if yes, delete it
    if(plannedIter->second->getNcArqs().empty()) {
        EV_DEBUG << "Canceling planned ARQ for source " << key.second << ", ID " << key.first << std::endl;
        delete plannedIter->second;
        ncPlannedArqs.erase(plannedIter);
    }
}

void ArrivalManagerSplit::ncTrySendPlannedArq(const IdSourceKey& key, bool forceImmediate) {
    // Check if there is an ARQ planned
    FlitCache::iterator plannedIter = ncPlannedArqs.find(key);
    if(plannedIter == ncPlannedArqs.end())
        return;

    // Check if we can send out the ARQ now (forced, more than one ARQ remaining, or no verifications ongoing)
    if(forceImmediate || issuedArqs[key] < static_cast<unsigned int>(arqLimit) - 1 || !ncCheckVerificationOngoing(key)) {
        // Get ARQ
        Flit* arq = plannedIter->second;

        // Send ARQ
        //emit(pktgenerateSignal, flit->getGidOrFid());
        EV << "Sending ARQ \"" << arq->getName() << "\" from " << arq->getSource()
           << " to " << arq->getTarget() << " (ID: " << arq->getGidOrFid() << ") (mode "
           << cEnum::get("HaecComm::Messages::Mode")->getStringFor(arq->getMode()) << " "
           << arq->getNcArqs() << ")" << std::endl;
        send(arq, "arqOut");

        // Remove from planned ARQ map
        ncPlannedArqs.erase(plannedIter);

        // Increment ARQ counter
        ++issuedArqs[key];

        // Set the ARQ timer
        setArqTimer(key, static_cast<NcMode>(arq->getNcMode()), true);
    }
}

void ArrivalManagerSplit::ncCheckGenerationDone(const IdSourceKey& key, unsigned short generationSize) {
    // Check if we have sent enough data flits to the app in order
    // to decode this generation
    GevSet& dispatchedGevs = ncDispatchedGevs[key];
    if(dispatchedGevs.size() >= generationSize) {
        // Clean up whole generation
        ncCleanUp(key);
    }
}

void ArrivalManagerSplit::ncCleanUp(const IdSourceKey& key) {
    // Clean up all information related to this ID/address
    EV_DEBUG << "Fully cleaning up: source " << key.second << ", ID " << key.first << std::endl;

    // Clear data/MAC caches
    ncDeleteFromCache(ncReceivedSplitsCache, key);
    ncDeleteFromCache(ncDecryptedSplitsCache, key);
    ncDeleteFromCache(ncComputedMacsCache, key);

    // Clear verification results
    ncVerified.erase(key);

    // Clear number of issued ARQs
    issuedArqs.erase(key);

    // Clear dispatched GEV set
    ncDispatchedGevs.erase(key);

    // Clear corrupted decryption counter
    ncDiscardDecrypting.erase(key);

    // Insert ID into finished ID set
    IdSet& finishedSet = finishedIds[key.second].first;
    IdQueue& finishedQueue = finishedIds[key.second].second;

    finishedSet.insert(key.first);
    finishedQueue.push(key.first);

    // Check if the finished ID set size grew too large
    while(finishedSet.size() > static_cast<size_t>(finishedIdsTracked)) {
        finishedSet.erase(finishedQueue.front());
        finishedQueue.pop();
    }

    // Cancel and delete any remaining ARQ timers
    TimerCache::iterator timerIter = arqTimers.find(key);
    if(timerIter != arqTimers.end()) {
        cancelAndDelete(timerIter->second);
        arqTimers.erase(timerIter);
    }

    // Delete any planned ARQs
    FlitCache::iterator arqIter = ncPlannedArqs.find(key);
    if(arqIter != ncPlannedArqs.end()) {
        delete arqIter->second;
        ncPlannedArqs.erase(arqIter);
    }
}

bool ArrivalManagerSplit::ncDeleteFromCache(GenCache& cache, const IdSourceKey& key) {
    GenCache::iterator actualCacheIter = cache.find(key);
    if(actualCacheIter != cache.end()) {
        bool ret = false;
        GevCache& actualCache = actualCacheIter->second;
        for(auto it = actualCache.begin(); it != actualCache.end(); ++it) {
            delete it->second;
            ret = true;
        }
        cache.erase(actualCacheIter);
        return ret;
    }
    return false;
}

bool ArrivalManagerSplit::ncDeleteFromCache(GenCache& cache, const IdSourceKey& key, uint16_t gev) {
    GenCache::iterator outerIter = cache.find(key);
    if(outerIter != cache.end()) {
        GevCache::iterator innerIter = outerIter->second.find(gev);
        if(innerIter != outerIter->second.end()) {
            delete innerIter->second;
            outerIter->second.erase(innerIter);
            return true;
        }
    }
    return false;
}

Flit* ArrivalManagerSplit::generateArq(const IdSourceKey& key, Mode mode, ArqMode arqMode) {
    Address2D self(nodeX, nodeY);

    // Build packet name
    std::ostringstream packetName;
    packetName << "arq-" << key.first << "-s" << self << "-t" << key.second;

    // Create the flit
    Flit* arq = MessageFactory::createFlit(packetName.str().c_str(), self, key.second, mode, key.first);
    take(arq);

    // Set ARQ payload
    arq->setUcArqs(arqMode);

    // Return ARQ
    return arq;
}

Flit* ArrivalManagerSplit::generateArq(const IdSourceKey& key, Mode mode, const GevArqMap& arqModes, NcMode ncMode) {
    Address2D self(nodeX, nodeY);

    // Build packet name
    std::ostringstream packetName;
    packetName << "arq-" << key.first << "-s" << self << "-t" << key.second;

    // Create the flit
    Flit* arq = MessageFactory::createFlit(packetName.str().c_str(), self, key.second, mode, key.first, 0, ncMode);
    take(arq);

    // Set ARQ payload
    arq->setNcArqs(arqModes);

    // Return ARQ
    return arq;
}

void ArrivalManagerSplit::setArqTimer(const IdSourceKey& key, NcMode ncMode, bool useAnswerTime, bool setToMax) {
    // Check if there is already a timer for this ID/source
    TimerCache::iterator timerIter = arqTimers.find(key);
    if(timerIter == arqTimers.end()) {
        // Create a new timer
        std::ostringstream timerName;
        timerName << "arqTimer-" << key.first << "-s" << key.second;
        timerIter = arqTimers.emplace(key, MessageFactory::createArqTimer(timerName.str().c_str(), key.first, key.second, ncMode)).first;
    }

    // Get the timer
    ArqTimer* timer = timerIter->second;

    // Compute the desired arrival time
    simtime_t cycle = getAncestorPar("clockPeriod");
    simtime_t offset = (useAnswerTime
        ? arqAnswerTimeouts.at(key.second) * cycle
        : arqIssueTimeout * cycle
    );
    simtime_t zeroHour = simTime() + offset;

    // Check if the timer is already scheduled (i.e. active and counting down)
    if(timer->isScheduled()) {
        // Check if we use the maximum of desired and scheduled time
        if(setToMax) {
            // Get arrival time
            simtime_t oldZeroHour = timer->getArrivalTime();

            // Compute maximum
            if(oldZeroHour > zeroHour)
                zeroHour = oldZeroHour;
        }

        // Cancel the timer
        cancelEvent(timer);
    }

    // Set the new timer
    scheduleAt(zeroHour, timer);
}

void ArrivalManagerSplit::cancelArqTimer(const IdSourceKey& key) {
    // Check if there is a timer for this ID/source
    TimerCache::iterator timerIter = arqTimers.find(key);
    if(timerIter != arqTimers.end()) {
        // Cancel the timer in case it is scheduled
        cancelEvent(timerIter->second);
    }
}

bool ArrivalManagerSplit::ucCheckVerificationOngoing(const IdSourceKey& key) const {
    // Check if we are currently verifying a split
    // This is the case when we have received a split, but
    // there is no computed MAC for it yet
    SplitCache::const_iterator receivedSplits = ucReceivedSplitsCache.find(key);
    SplitCache::const_iterator computedMacs = ucComputedMacsCache.find(key);

    // If the received cache is empty, there are no verifications ongoing
    if(receivedSplits == ucReceivedSplitsCache.end())
        return false;

    // Check for the received splits if there is a corresponding computed MAC
    if(receivedSplits->second.first && (computedMacs == ucComputedMacsCache.end() || !computedMacs->second.first))
        return true;
    if(receivedSplits->second.second && (computedMacs == ucComputedMacsCache.end() || !computedMacs->second.second))
        return true;

    // We have found a MAC for all received splits
    return false;
}

bool ArrivalManagerSplit::ucCheckArqPlanned(const IdSourceKey& key) const {
    return ucPlannedArqs.count(key);
}

bool ArrivalManagerSplit::ncCheckCompleteGenerationReceived(const IdSourceKey& key, unsigned short numCombinations) {
    // Check if we have received the appropriate number of combinations
    const GevCache& receivedSplits = ncReceivedSplitsCache[key];

    return receivedSplits.size() >= numCombinations;
}

bool ArrivalManagerSplit::ncCheckVerificationOngoing(const IdSourceKey& key) const {
    // Check if we are currently verifying a split
    // This is the case when we have received a split, but
    // there is no computed MAC yet (for each GEV)
    GenCache::const_iterator receivedSplits = ncReceivedSplitsCache.find(key);
    GenCache::const_iterator computedMacs = ncComputedMacsCache.find(key);

    // If one of the received caches is empty, there are no verifications ongoing
    if(receivedSplits == ncReceivedSplitsCache.end())
        return false;

    // Iterate over all GEVs for which we have received a split
    for(auto it = receivedSplits->second.begin(); it != receivedSplits->second.end(); ++it) {
        // Check if there is a computed MAC for this split
        if(computedMacs == ncComputedMacsCache.end() || !computedMacs->second.count(it->first)) {
            // No computed MAC found
            return true;
        }
    }

    // We have found a computed MAC for all applicable GEVs
    return false;
}

bool ArrivalManagerSplit::ncCheckArqPlanned(const IdSourceKey& key) const {
    return ncPlannedArqs.count(key);
}

}} //namespace