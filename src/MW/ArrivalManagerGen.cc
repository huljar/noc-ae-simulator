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

#include "ArrivalManagerGen.h"
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>
#include <algorithm>
#include <cmath>
#include <sstream>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(ArrivalManagerGen);

ArrivalManagerGen::ArrivalManagerGen() {
}

ArrivalManagerGen::~ArrivalManagerGen() {
    for(auto it = arqTimers.begin(); it != arqTimers.end(); ++it)
        cancelAndDelete(it->second);

    for(auto it = receivedDataCache.begin(); it != receivedDataCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;
    for(auto it = receivedMacCache.begin(); it != receivedMacCache.end(); ++it)
        delete it->second;
    for(auto it = decryptedDataCache.begin(); it != decryptedDataCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete *jt;
    for(auto it = computedMacCache.begin(); it != computedMacCache.end(); ++it)
        delete it->second;

    for(auto it = plannedArqs.begin(); it != plannedArqs.end(); ++it)
        delete it->second;
}

void ArrivalManagerGen::initialize() {
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

    int genSize = par("generationSize");
    if(genSize < 1)
        throw cRuntimeError(this, "Generation size must be greater than 0, but received %i", genSize);
    generationSize = static_cast<unsigned short>(genSize);

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

void ArrivalManagerGen::handleMessage(cMessage* msg) {
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

void ArrivalManagerGen::handleNetMessage(Flit* flit) {
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

    // Assert that we are in a network coded environment
    ASSERT(ncMode != NC_UNCODED);

    // Check if this is a data or MAC flit
    if(mode == MODE_DATA) {
        // Get parameters
        uint16_t gev = flit->getGev();
        GevCache& gevCache = receivedDataCache[key];
        GevSet& requested = dataRequestedViaArq[key];

        // Check if the data cache already contains a flit with this GEV
        GevCache::iterator oldFlit = gevCache.find(gev);
        if(oldFlit != gevCache.end()) {
            // Check if this flit was requested via an ARQ
            if(requested.count(gev)) {
                // Delete the old flit
                EV_DEBUG << "Received a data flit from " << source << " with GID " << id << " and GEV " << gev
                         << " via an ARQ answer. Overwriting the old flit." << std::endl;

                delete oldFlit->second;
                gevCache.erase(oldFlit);

                // Remove all combinations containing this GEV from the set of decoded combinations
                std::vector<GevSet>& decoded = decodedGevs[key];
                auto decodedEnd = std::remove_if(decoded.begin(), decoded.end(), [gev](const GevSet& s) { return s.count(gev); });
                decoded.erase(decodedEnd, decoded.end());
            }
            else {
                // Discard this flit
                EV << "Received a data flit from " << source << " with GID " << id << " and GEV " << gev
                   << ", but we already have a data flit cached" << std::endl;
                delete flit;
                return;
            }
        }
        // else: check if there are already enough data flits for the used NC mode
        else if((ncMode == NC_G2C3 && gevCache.size() >= 3) || (ncMode == NC_G2C4 && gevCache.size() >= 4)) {
            EV << "Received a data flit from " << source << " with GID " << id << " and GEV " << gev
               << ", but we already have all the data flits from this generation" << std::endl;
            delete flit;
            return;
        }

        // We can safely cache the flit now
        EV_DEBUG << "Caching data flit \"" << flit->getName() << "\" from " << source << " with ID " << id << " and GEV " << gev << std::endl;
        gevCache.emplace(gev, flit);

        // Unflag this GEV as requested via an ARQ (in case it was)
        requested.erase(gev);

        // In case this flit is already in a planned ARQ, remove it from there
        tryRemoveDataFromPlannedArq(key, GevArqMap{{gev, ARQ_DATA}});

        // Check if we can cancel the ARQ timer
        if(checkCompleteGenerationReceived(key, flit->getNumCombinations())) {
            // Cancel ARQ timer
            cancelArqTimer(key);
        }
        // If there is no planned ARQ, start/update the ARQ timer
        else if(!checkArqPlanned(key)) {
            setArqTimer(key, ncMode);
        }

        // Try to start decoding, decryption and authentication (if we have enough combinations to decode and are not currently decoding)
        tryStartDecodeDecryptAndAuth(key);
    }
    else if(mode == MODE_MAC) {
        // Check if the MAC cache already contains a flit
        FlitCache::iterator oldFlit = receivedMacCache.find(key);
        if(oldFlit != receivedMacCache.end()) {
            // Check if this flit was requested via an ARQ
            if(macRequestedViaArq.count(key)) {
                // Delete the old flit
                EV_DEBUG << "Received a MAC flit from " << source << " with GID " << id
                         << " via an ARQ answer. Overwriting the old flit." << std::endl;

                delete oldFlit->second;
                receivedMacCache.erase(oldFlit);

                // Remove all combinations from the set of decoded combinations (except the last if it is still being verified)
                std::vector<GevSet>& decoded = decodedGevs[key];
                if(currentlyComputingMac.count(key)) {
                    ASSERT(!decoded.empty());
                    decoded.front() = decoded.back();
                    decoded.erase(decoded.begin() + 1, decoded.end());
                }
                else {
                    decoded.clear();
                }
            }
            else {
                // Discard this flit
                EV << "Received a MAC flit from " << source << " with ID " << id
                   << ", but we already have a MAC flit cached" << std::endl;
                delete flit;
                return;
            }
        }

        // We can safely cache the flit now
        EV_DEBUG << "Caching MAC flit \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
        receivedMacCache.emplace(key, flit);

        // Unflag this MAC as requested via an ARQ (in case it was)
        macRequestedViaArq.erase(key);

        // In case this flit is already in a planned ARQ, remove it from there
        tryRemoveMacFromPlannedArq(key);

        // Check if we can cancel the ARQ timer
        if(checkCompleteGenerationReceived(key, flit->getNumCombinations())) {
            // Cancel ARQ timer
            cancelArqTimer(key);
        }
        // If there is no planned ARQ, start/update the ARQ timer
        else if(!checkArqPlanned(key)) {
            setArqTimer(key, ncMode);
        }

        // Try to verify the flit (in case the computed MAC is already there)
        tryVerification(key);
    }
    else {
        throw cRuntimeError(this, "Received flit with unexpected mode %u from %s (ID: %u)", mode, source.str().c_str(), id);
    }
}

void ArrivalManagerGen::handleCryptoMessage(Flit* flit) {
    // Get parameters
    uint32_t id = flit->getOriginalIds(0); // The data flits have their old FIDs restored, but we need the GID (MAC flit also has the GID here)
    Address2D source = flit->getSource();
    IdSourceKey key = std::make_pair(id, source);
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());

    // Assert that the flits are not encoded any more
    ASSERT(ncMode == NC_UNCODED);

    // Check if this ID is already finished
    const IdSet& finishedIdSet = finishedIds[source].first;
    if(finishedIdSet.count(id)) {
        EV_DEBUG << "Received a decrypted/authenticated flit for finished GID " << id << std::endl;
        delete flit;
        return;
    }

    // Check flit mode
    if(mode == MODE_DATA) {
        // This is a decrypted flit arriving from a crypto unit
        // Check if we have to discard this flit (because corruption was detected on MAC verification)
        if(discardDecrypting[key] > 0) {
            EV_DEBUG << "Discarding corrupted decrypted data flit \"" << flit->getName()
                     << "\" (source: " << source << ", ID: " << id << ")" << std::endl;
            --discardDecrypting[key];
            delete flit;
            return;
        }

        // Assert that the decrypted flit cache does not contain a full generation already
        ASSERT(decryptedDataCache[key].size() < generationSize);

        // Insert decrypted flit into the cache
        EV_DEBUG << "Caching decrypted data flit \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
        decryptedDataCache[key].push_back(flit);

        // Try to send out the decrypted flits (if MAC was successfully verified)
        trySendToApp(key);
    }
    else if(mode == MODE_MAC) {
        // This is a computed MAC arriving from a crypto unit
        // Assert that the computed MAC cache does not contain a flit
        ASSERT(!computedMacCache.count(key));

        // We don't have this flit yet, cache it
        EV_DEBUG << "Caching computed MAC flit \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
        computedMacCache.emplace(key, flit);

        // Unset the flag that a MAC is currently being computed
        currentlyComputingMac.erase(key);

        // Try to verify the flit (in case the received MAC is already there)
        tryVerification(key);
    }
    else {
        throw cRuntimeError(this, "Crypto unit sent flit with unexpected mode %u (Source: %s, ID: %u)", mode, source.str().c_str(), id);
    }
}

void ArrivalManagerGen::handleArqTimer(ArqTimer* timer) {
    // TODO: clear all requestedViaArq flags
    // TODO: don't fail immediately on final timeout if a MAC is currently being computed and we have a MAC received

    // Get parameters
    uint32_t id = timer->getGidOrFid();
    Address2D source = timer->getSource();
    IdSourceKey key = std::make_pair(id, source);
    NcMode ncMode = static_cast<NcMode>(timer->getNcMode());

    EV_DEBUG << "ARQ timeout triggered for source " << key.second << ", ID " << key.first << std::endl;

    // Clear all requested via ARQ flags for this transmission unit
    dataRequestedViaArq.erase(key);
    macRequestedViaArq.erase(key);

    // Check if we have reached the ARQ limit
    if(issuedArqs[key] >= static_cast<unsigned int>(arqLimit)) {
        // If there is no verification going on, we have failed completely: clean up everything and discard flits from this ID
        if(!checkVerificationOngoing(key)) {
            EV_DEBUG << "ARQ limit reached for source " << key.second << ", ID " << key.first << std::endl;
            cleanUp(key);
        }

        // Otherwise, we let the current verification finish. All combinations of currently available coded data flits will be tried.
        // If none of them work, cleanup will be initiated.
        return;
    }

    // Get parameters
    const GevCache& receivedData = receivedDataCache[key];

    // We always use TELL_RECEIVED because there is only one flit per GEV
    Mode flitMode = MODE_ARQ_TELL_RECEIVED;
    GevArqMap arqModes;

    // Iterate over received data flits and insert them
    for(auto it = receivedData.begin(); it != receivedData.end(); ++it) {
        arqModes.emplace(it->first, ARQ_DATA);
    }

    // Check if we have received the MAC
    bool macReceived = receivedMacCache.count(key);

    // Issue the ARQ
    issueArq(key, flitMode, arqModes, macReceived, ncMode);
}

bool ArrivalManagerGen::tryStartDecodeDecryptAndAuth(const IdSourceKey& key) {
    // This currently only works with generation size 2
    if(generationSize != 2)
        throw cRuntimeError(this, "This module currently only works with generation size 2! We received %u.", generationSize);

    // Check that we are not currently working on this generation
    if(currentlyComputingMac.count(key))
        return false;

    // Get the GEVs that we have received
    GevCache& received = receivedDataCache[key];

    // Get the GEV combinations that were already decoded
    std::vector<GevSet>& decoded = decodedGevs[key];

    // Set of GEVs that we will send out now (filled throughout this function
    GevSet toSend;

    // Is it possible to decode with previously unused GEVs only?
    GevSet usedGevs;
    for(auto it = decoded.begin(); it != decoded.end(); ++it)
        usedGevs.insert(it->begin(), it->end());

    if(received.size() >= usedGevs.size() + generationSize) {
        // Use only previously unused GEVs
        for(auto it = received.begin(); it != received.end(); ++it) {
            if(!usedGevs.count(it->first)) {
                toSend.insert(it->first);
                if(toSend.size() == generationSize)
                    break;
            }
        }
        ASSERT(toSend.size() == generationSize);
    }
    else if(received.size() >= generationSize) {
        // Try to find a new combination of GEVs
        for(auto it = received.begin(); it != received.end(); ++it) {
            bool breakOuter = false;
            auto jt = it;
            for(++jt; jt != received.end(); ++jt) {
                // Examine this set
                GevSet candidate{it->first, jt->first};
                if(std::find(decoded.begin(), decoded.end(), candidate) == decoded.end()) {
                    // Decoding this set has not been tried
                    toSend = candidate;
                    breakOuter = true;
                    break;
                }
            }
            if(breakOuter) {
                break;
            }
        }
    }

    // Check if we have found a set of GEVs to send
    if(toSend.size() == generationSize) {
        // Set parameters
        currentlyComputingMac.insert(key);
        decoded.push_back(toSend);

        // Retrieve the requested flits from the cache and send copies out
        // for decoding, decryption, and authentication
        // We use copies here so that we don't have to remove the flits from the cache,
        // which are still used to check if the flits have already arrived
        EV_DEBUG << "Starting flit decoding for GEVs ";
        for(auto it = toSend.begin(); it != toSend.end(); ++it)
            EV_DEBUG << *it << " ";
        EV_DEBUG << "(source: " << key.second << ", ID: " << key.first << ")" << std::endl;

        for(auto it = toSend.begin(); it != toSend.end(); ++it) {
            Flit* copy = received.at(*it)->dup();
            send(copy, "decoderOut");
        }

        return true;
    }

    return false;
}

void ArrivalManagerGen::tryVerification(const IdSourceKey& key) {
    // Check if both computed and received MAC are present
    FlitCache::iterator recvMac = receivedMacCache.find(key);
    FlitCache::iterator compMac = computedMacCache.find(key);

    if(recvMac != receivedMacCache.end() && compMac != computedMacCache.end()) {
        // Verify their equality
        bool equal = !recvMac->second->isModified() && !compMac->second->isModified() &&
                     !recvMac->second->hasBitError() && !compMac->second->hasBitError();

        // Examine verification result
        if(equal) {
            // Verification was successful, insert into success cache
            EV_DEBUG << "Successfully verified MAC \"" << recvMac->second->getName() << "\" (source: " << key.second
                     << ", ID: " << key.first << ")" << std::endl;
            verified.insert(key);

            // Try to send out the decrypted flits
            trySendToApp(key);
        }
        else {
            // Verification was not successful
            EV_DEBUG << "MAC verification failed for \"" << recvMac->second->getName() << "\" (source: " << key.second
                     << ", ID: " << key.first << ")" << std::endl;

            // Get parameters
            GevSet& requestedData = dataRequestedViaArq[key];

            // Check if we have an ARQ left
            if(issuedArqs[key] < static_cast<unsigned int>(arqLimit)) {
                // Issue ARQ for the last set of combinations that was sent to the decoder and for the MAC
                // If there was already an ARQ sent for one of them, don't send another one
                const std::vector<GevSet>& decoded = decodedGevs[key];
                ASSERT(!decoded.empty() && decoded.back().size() == generationSize);

                GevArqMap arqModes;
                for(auto it = decoded.back().begin(); it != decoded.back().end(); ++it) {
                    if(!requestedData.count(*it))
                        arqModes.emplace(*it, ARQ_DATA);
                }
                bool requestMac = !macRequestedViaArq.count(key);
                ASSERT(!arqModes.empty() || requestMac);

                // Issue ARQ
                issueArq(key, MODE_ARQ_TELL_MISSING, arqModes, requestMac, static_cast<NcMode>(recvMac->second->getNcMode()));

                // Mark GEVs and MAC as requested via ARQ to ensure we can receive the retransmission
                for(auto it = arqModes.begin(); it != arqModes.end(); ++it)
                    requestedData.insert(it->first);
                if(requestMac)
                    macRequestedViaArq.insert(key);
            }

            // Clear the decrypted data flits or, in case it some have not arrived yet,
            // mark that they will be discarded on arrival
            unsigned short decryptedDeleted = deleteFromCache(decryptedDataCache, key);
            ASSERT(decryptedDeleted <= generationSize);
            discardDecrypting[key] += generationSize - decryptedDeleted;

            // Also clear the computed MAC to ensure that we don't compare against it any more
            deleteFromCache(computedMacCache, key);

            // Try to send a different combination of GEVs to the decoder
            bool anotherTry = tryStartDecodeDecryptAndAuth(key);

            // If we couldn't start another try and there is no ARQ ongoing, we have failed completely
            if(!anotherTry && requestedData.empty() && !macRequestedViaArq.count(key)) {
                // We have failed completely, clean up everything and discard flits from this ID
                EV_DEBUG << "No more decoder combinations available and ARQ limit reached for source " << key.second << ", ID " << key.first << std::endl;
                cleanUp(key);
                return;
            }
        }
    }
}

void ArrivalManagerGen::trySendToApp(const IdSourceKey& key) {
    // Check if enough decrypted data flits are present and the MAC was successfully verified
    const FlitVector& decData = decryptedDataCache[key];
    if(verified.count(key) && decData.size() >= generationSize) {
        // Send out copies of the decrypted data flits
        // We use copies here to avoid potential conflicts with cleanup
        EV_DEBUG << "Sending out decrypted data flits: source " << key.second << ", ID " << key.first << std::endl;

        for(size_t i = 0; i < generationSize; ++i) {
            Flit* copy = decData[i]->dup();
            send(copy, "appOut");
        }

        // This generation is done, initiate cleanup
        cleanUp(key);
    }
}

void ArrivalManagerGen::issueArq(const IdSourceKey& key, Mode mode, const GevArqMap& arqModes, bool macArqMode, NcMode ncMode) {
    // Assert that we have not reached the maximum number of ARQs
    ASSERT(issuedArqs[key] < static_cast<unsigned int>(arqLimit));

    // Check if there already is a planned ARQ for this ID/source
    FlitCache::iterator plannedIter = plannedArqs.find(key);
    if(plannedIter == plannedArqs.end()) {
        // Create a new ARQ and insert it into the planned ARQ map
        EV_DEBUG << "Initiating planned ARQ for source " << key.second << ", ID " << key.first << ", mode "
                 << cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode) << " " << arqModes << (macArqMode ? " with MAC" : " without MAC") << std::endl;
        plannedArqs.emplace(key, generateArq(key, mode, arqModes, macArqMode, ncMode));
    }
    else {
        // Merge the planned ARQ with the new ARQ arguments
        EV_DEBUG << "Merging planned ARQ for source " << key.second << ", ID " << key.first << " with new mode "
                 << cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode) << " " << arqModes << std::endl;
        plannedIter->second->mergeNcArqModesGen(mode, arqModes, macArqMode);
    }

    trySendPlannedArq(key, !lastArqWaitForOngoingVerifications);
}

void ArrivalManagerGen::tryRemoveDataFromPlannedArq(const IdSourceKey& key, const GevArqMap& arqModes) {
    // Check if there is an ARQ planned
    FlitCache::iterator plannedIter = plannedArqs.find(key);
    if(plannedIter == plannedArqs.end())
        return;

    // Remove specified modes from the ARQ
    EV_DEBUG << "Removing " << arqModes << " from planned ARQ for source " << key.second << ", ID " << key.first << std::endl;
    plannedIter->second->removeFromNcArqGen(arqModes, false);

    // Check if the ARQ is empty now; if yes, delete it
    if(plannedIter->second->getNcArqs().empty() && !plannedIter->second->getNcArqGenMac()) {
        EV_DEBUG << "Canceling planned ARQ for source " << key.second << ", ID " << key.first << std::endl;
        delete plannedIter->second;
        plannedArqs.erase(plannedIter);
    }
}

void ArrivalManagerGen::tryRemoveMacFromPlannedArq(const IdSourceKey& key) {
    // Check if there is an ARQ planned
    FlitCache::iterator plannedIter = plannedArqs.find(key);
    if(plannedIter == plannedArqs.end())
        return;

    // Remove MAC from the ARQ
    EV_DEBUG << "Removing MAC from planned ARQ for source " << key.second << ", ID " << key.first << std::endl;
    plannedIter->second->removeFromNcArqGen(GevArqMap(), true);

    // Check if the ARQ is empty now; if yes, delete it
    if(plannedIter->second->getNcArqs().empty() && !plannedIter->second->getNcArqGenMac()) {
        EV_DEBUG << "Canceling planned ARQ for source " << key.second << ", ID " << key.first << std::endl;
        delete plannedIter->second;
        plannedArqs.erase(plannedIter);
    }
}

void ArrivalManagerGen::trySendPlannedArq(const IdSourceKey& key, bool forceImmediate) {
    // Check if there is an ARQ planned
    FlitCache::iterator plannedIter = plannedArqs.find(key);
    if(plannedIter == plannedArqs.end())
        return;

    // Don't send the ARQ if the generation is successfully verified
    if(verified.count(key))
        return;

    // Check if we can send out the ARQ now (forced, more than one ARQ remaining, or no verifications ongoing)
    if(forceImmediate || issuedArqs[key] < static_cast<unsigned int>(arqLimit) - 1 || !checkVerificationOngoing(key)) {
        // Get ARQ
        Flit* arq = plannedIter->second;

        // Send ARQ
        //emit(pktgenerateSignal, flit->getGidOrFid());
        EV << "Sending ARQ \"" << arq->getName() << "\" from " << arq->getSource()
           << " to " << arq->getTarget() << " (ID: " << arq->getGidOrFid() << ") (mode "
           << cEnum::get("HaecComm::Messages::Mode")->getStringFor(arq->getMode()) << " "
           << arq->getNcArqs() << ")" << (arq->getNcArqGenMac() ? " with MAC" : " without MAC") << std::endl;
        send(arq, "arqOut");

        // Remove from planned ARQ map
        plannedArqs.erase(plannedIter);

        // Increment ARQ counter
        ++issuedArqs[key];

        // Set the ARQ timer
        setArqTimer(key, static_cast<NcMode>(arq->getNcMode()), true);
    }
}

void ArrivalManagerGen::cleanUp(const IdSourceKey& key) {
    // Clean up all information related to this ID/address
    EV_DEBUG << "Fully cleaning up: source " << key.second << ", ID " << key.first << std::endl;

    // Clear data/MAC caches
    deleteFromCache(receivedDataCache, key);
    deleteFromCache(receivedMacCache, key);
    deleteFromCache(decryptedDataCache, key);
    deleteFromCache(computedMacCache, key);

    // Clear decoded GEVs
    decodedGevs.erase(key);

    // Clear currently computing flag
    currentlyComputingMac.erase(key);

    // Clear verification result
    verified.erase(key);

    // Clear number of issued ARQs
    issuedArqs.erase(key);

    // Clear requested via ARQ trackers
    dataRequestedViaArq.erase(key);
    macRequestedViaArq.erase(key);

    // Clear corrupted decryption counter
    discardDecrypting.erase(key);

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
    FlitCache::iterator arqIter = plannedArqs.find(key);
    if(arqIter != plannedArqs.end()) {
        delete arqIter->second;
        plannedArqs.erase(arqIter);
    }
}

bool ArrivalManagerGen::deleteFromCache(FlitCache& cache, const IdSourceKey& key) {
    FlitCache::iterator element = cache.find(key);
    if(element != cache.end()) {
        delete element->second;
        cache.erase(element);
        return true;
    }
    return false;
}

bool ArrivalManagerGen::deleteFromCache(GenCache& cache, const IdSourceKey& key) {
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

bool ArrivalManagerGen::deleteFromCache(GenCache& cache, const IdSourceKey& key, uint16_t gev) {
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

unsigned short ArrivalManagerGen::deleteFromCache(DecryptedCache& cache, const IdSourceKey& key) {
    unsigned short ret = 0;
    DecryptedCache::iterator vecIter = cache.find(key);
    if(vecIter != cache.end()) {
        FlitVector& vec = vecIter->second;
        ret = vec.size();
        for(auto it = vec.begin(); it != vec.end(); ++it)
            delete *it;
        cache.erase(vecIter);
    }
    return ret;
}

Flit* ArrivalManagerGen::generateArq(const IdSourceKey& key, Mode mode, const GevArqMap& arqModes, bool requestMac, NcMode ncMode) {
    Address2D self(nodeX, nodeY);

    // Build packet name
    std::ostringstream packetName;
    packetName << "arq-" << key.first << "-s" << self << "-t" << key.second;

    // Create the flit
    Flit* arq = MessageFactory::createFlit(packetName.str().c_str(), self, key.second, mode, key.first, 0, ncMode);
    take(arq);

    // Set ARQ payload
    arq->setNcArqs(arqModes);
    arq->setNcArqGenMac(requestMac);

    // Return ARQ
    return arq;
}

void ArrivalManagerGen::setArqTimer(const IdSourceKey& key, NcMode ncMode, bool useAnswerTime, bool setToMax) {
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

void ArrivalManagerGen::cancelArqTimer(const IdSourceKey& key) {
    // Check if there is a timer for this ID/source
    TimerCache::iterator timerIter = arqTimers.find(key);
    if(timerIter != arqTimers.end()) {
        // Cancel the timer in case it is scheduled
        cancelEvent(timerIter->second);
    }
}

bool ArrivalManagerGen::checkCompleteGenerationReceived(const IdSourceKey& key, unsigned short numCombinations) {
    // Check if we have received the appropriate number of combinations and the generation MAC
    // Subtract any pending flits requested via ARQ
    GevCache receivedData = receivedDataCache[key];
    const GevSet& pendingData = dataRequestedViaArq[key];

    for(auto it = pendingData.begin(); it != pendingData.end(); ++it)
        receivedData.erase(*it);

    return receivedData.size() >= numCombinations && receivedMacCache.count(key) && !macRequestedViaArq.count(key);
}

bool ArrivalManagerGen::checkVerificationOngoing(const IdSourceKey& key) const {
    // Check if we are currently verifying a flit
    // This is the case when the currentlyComputingMac flag is set and
    // there is a received MAC
    return currentlyComputingMac.count(key) && receivedMacCache.count(key);
}

bool ArrivalManagerGen::checkArqPlanned(const IdSourceKey& key) const {
    return plannedArqs.count(key);
}

}} //namespace
