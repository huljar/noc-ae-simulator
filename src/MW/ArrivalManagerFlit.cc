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

#include "ArrivalManagerFlit.h"
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>
#include <cmath>
#include <sstream>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(ArrivalManagerFlit);

ArrivalManagerFlit::ArrivalManagerFlit() {
}

ArrivalManagerFlit::~ArrivalManagerFlit() {
    for(auto it = arqTimers.begin(); it != arqTimers.end(); ++it)
        cancelAndDelete(it->second);

    for(auto it = ucReceivedDataCache.begin(); it != ucReceivedDataCache.end(); ++it)
        delete it->second;
    for(auto it = ucReceivedMacCache.begin(); it != ucReceivedMacCache.end(); ++it)
        delete it->second;
    for(auto it = ucDecryptedDataCache.begin(); it != ucDecryptedDataCache.end(); ++it)
        delete it->second;
    for(auto it = ucComputedMacCache.begin(); it != ucComputedMacCache.end(); ++it)
        delete it->second;

    for(auto it = ncReceivedDataCache.begin(); it != ncReceivedDataCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;
    for(auto it = ncReceivedMacCache.begin(); it != ncReceivedMacCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;
    for(auto it = ncDecryptedDataCache.begin(); it != ncDecryptedDataCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;
    for(auto it = ncComputedMacCache.begin(); it != ncComputedMacCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;

    for(auto it = ncPlannedArqs.begin(); it != ncPlannedArqs.end(); ++it)
        delete it->second;
}

void ArrivalManagerFlit::initialize() {
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

    outOfOrderIdGracePeriod = par("outOfOrderIdGracePeriod");
    if(outOfOrderIdGracePeriod < 0)
        throw cRuntimeError(this, "outOfOrderIdGracePeriod must be greater than or equal to 0");

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

void ArrivalManagerFlit::handleMessage(cMessage* msg) {
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

void ArrivalManagerFlit::handleNetMessage(Flit* flit) {
    // Get parameters
    uint32_t id = flit->getGidOrFid();
    Address2D source = flit->getSource();

    // Check if this is an active or new ID (might be a repeat attack)
    IdSet& activeIdSet = activeIds[source];
    if(!highestKnownIds.count(source)) {
        // This is the first flit to arrive from this source
        // Set current ID as new highest known ID
        highestKnownIds.emplace(source, id);

        // Insert as active ID
        activeIdSet.insert(id);
    }
    else {
        // Check if this flit belongs to an active ID (or is the new highest ID)
        uint32_t& highestId = highestKnownIds[source];
        if(id > highestId) {
            // Set current ID as new highest known ID
            highestId = id;

            // Insert as active ID
            activeIdSet.insert(id);
        }
        //else if(id >= highestId - outOfOrderIdGracePeriod) {
            // TODO: Allow grace period for out-of-order arrivals (lower ID may arrive later than higher ID)
            // This ID is still allowed into the active ID set
            //activeIdSet.insert(id); // TODO: do not allow finished IDs here?
        //}
        else {
            // Check if this ID in the active ID set
            if(!activeIdSet.count(id)) {
                // This is either a retransmission of an old, finished ID or a repeat attack
                // Discard the flit and return
                EV << "Received a flit from " << source << " with ID " << id
                   << ", but ID is inactive (highest known ID: " << highestId << ")" << std::endl;
                delete flit;
                return;
            }
            // else: this flit's ID is active, continue without special action
        }
    }

    // We now know that this flit's ID from this source is active
    // Get parameters
    IdSourceKey key = std::make_pair(id, source);
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());

    // Determine if we are network coding or not
    if(ncMode == NC_UNCODED) {
        // Check if this is a data or MAC flit
        if(mode == MODE_DATA) {
            // Check if the data cache already contains a flit
            if(ucReceivedDataCache.count(key)) {
                // We already have a data flit cached, we don't need this one
                EV << "Received a data flit from " << source << " with ID " << id
                   << ", but we already have a data flit cached" << std::endl;
                delete flit;
                return;
            }

            // We don't have this flit yet, cache it
            EV_DEBUG << "Caching data flit \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
            ucReceivedDataCache.emplace(key, flit);

            // Check if the MAC flit has already arrived
            if(ucReceivedMacCache.count(key)) {
                // Cancel ARQ timer
                cancelArqTimer(key);
            }
            else {
                // Start/update ARQ timer
                setArqTimer(key, ncMode);
            }

            // We only need the data flit to start decryption and authentication,
            // so start it now
            ucStartDecryptAndAuth(key);
        }
        else if(mode == MODE_MAC) {
            // Check if the MAC cache already contains a flit
            if(ucReceivedMacCache.count(key)) {
                // We already have a MAC flit cached, we don't need this one
                EV << "Received a MAC flit from " << source << " with ID " << id
                   << ", but we already have a MAC flit cached" << std::endl;
                delete flit;
                return;
            }

            // We don't have this flit yet, cache it
            EV_DEBUG << "Caching MAC flit \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
            ucReceivedMacCache.emplace(key, flit);

            // Check if the data flit has already arrived
            if(ucReceivedDataCache.count(key)) {
                // Cancel ARQ timer
                cancelArqTimer(key);
            }
            else {
                // Start/update ARQ timer
                setArqTimer(key, ncMode);
            }

            // Try to verify the flit (in case the computed MAC is already there)
            ucTryVerification(key);
        }
        else {
            throw cRuntimeError(this, "Received flit with unexpected mode %u from %s (ID: %u)", mode, source.str().c_str(), id);
        }
    }
    else { // ncMode != uncoded
        // Get parameters
        uint16_t gev = flit->getGev();

        // Check if this is a data or MAC flit
        if(mode == MODE_DATA) {
            // Get parameters
            GevCache& gevCache = ncReceivedDataCache[key];

            // Check if the data cache already contains a flit with this GEV
            if(gevCache.count(gev)) {
                // We already have a data flit with this GEV cached, we don't need this one
                EV << "Received a data flit from " << source << " with GID " << id << " and GEV " << gev
                   << ", but we already have a data flit cached" << std::endl;
                delete flit;
                return;
            }

            // Check if there are already enough data flits for the used NC mode
            if((ncMode == NC_G2C3 && gevCache.size() >= 3) || (ncMode == NC_G2C4 && gevCache.size() >= 4)) {
                EV << "Received a data flit from " << source << " with GID " << id << " and GEV " << gev
                   << ", but we already have all the data flits from this generation" << std::endl;
                delete flit;
                return;
            }

            // We can safely cache the flit now
            EV_DEBUG << "Caching data flit \"" << flit->getName() << "\" from " << source << " with ID " << id << " and GEV " << gev << std::endl;
            gevCache.emplace(gev, flit);

            // Check if we can cancel the ARQ timer
            if(ncCheckCompleteGenerationReceived(key, flit->getNumCombinations())) {
                // Cancel ARQ timer
                cancelArqTimer(key);
            }
            else {
                // Start/update ARQ timer
                setArqTimer(key, ncMode);
            }

            // We only need the data flit to start decryption and authentication,
            // so start it now
            ncStartDecryptAndAuth(key, gev);
        }
        else if(mode == MODE_MAC) {
            // Get parameters
            GevCache& gevCache = ncReceivedMacCache[key];

            // Check if the MAC cache already contains a flit with this GEV
            if(gevCache.count(gev)) {
                // We already have a MAC flit with this GEV cached, we don't need this one
                EV << "Received a MAC flit from " << source << " with GID " << id << " and GEV " << gev
                   << ", but we already have a MAC flit cached" << std::endl;
                delete flit;
                return;
            }

            // Check if there are already enough MAC flits for the used NC mode
            if((ncMode == NC_G2C3 && gevCache.size() >= 3) || (ncMode == NC_G2C4 && gevCache.size() >= 4)) {
                EV << "Received a MAC flit from " << source << " with GID " << id << " and GEV " << gev
                   << ", but we already have all the MAC flits from this generation" << std::endl;
                delete flit;
                return;
            }

            // We can safely cache the flit now
            EV_DEBUG << "Caching MAC flit \"" << flit->getName() << "\" from " << source << " with ID " << id << " and GEV " << gev << std::endl;
            gevCache.emplace(gev, flit);

            // Check if we can cancel the ARQ timer
            if(ncCheckCompleteGenerationReceived(key, flit->getNumCombinations())) {
                // Cancel ARQ timer
                cancelArqTimer(key);
            }
            else {
                // Start/update ARQ timer
                setArqTimer(key, ncMode);
            }

            // Try to verify the flit (in case the computed MAC is already there)
            ncTryVerification(key, gev);
        }
        else {
            throw cRuntimeError(this, "Received flit with unexpected mode %u from %s (ID: %u)", mode, source.str().c_str(), id);
        }
    }
}

void ArrivalManagerFlit::handleCryptoMessage(Flit* flit) {
    // Get parameters
    uint32_t id = flit->getGidOrFid();
    Address2D source = flit->getSource();
    IdSourceKey key = std::make_pair(id, source);
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());

    // Check network coding mode
    if(ncMode == NC_UNCODED) {
        // Check if this ID is already finished
        IdSet& activeIdSet = activeIds[source];
        if(!activeIdSet.count(id)) {
            EV_DEBUG << "Received a decrypted/authenticated flit for finished FID " << id << std::endl;
            delete flit;
            return;
        }

        // Check flit mode
        if(mode == MODE_DATA) {
            // This is a decrypted flit arriving from a crypto unit
            // Assert that the decrypted flit cache does not contain a flit
            ASSERT(!ucDecryptedDataCache.count(key));

            // Check if we have to discard this flit (because corruption was detected on MAC verification)
            if(ucDiscardDecrypting[key] > 0) {
                EV_DEBUG << "Discarding corrupted decrypted data flit \"" << flit->getName()
                         << "\" (source: " << source << ", ID: " << id << ")" << std::endl;
                --ucDiscardDecrypting[key];
                delete flit;
                return;
            }

            // Insert decrypted flit into the cache
            EV_DEBUG << "Caching decrypted data flit \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
            ucDecryptedDataCache.emplace(key, flit);

            // Try to send out the decrypted flit
            ucTrySendToApp(key);
        }
        else if(mode == MODE_MAC) {
            // This is a computed MAC arriving from a crypto unit
            // Assert that the computed MAC cache does not contain a flit
            ASSERT(!ucComputedMacCache.count(key));

            // We don't have this flit yet, cache it
            EV_DEBUG << "Caching computed MAC flit \"" << flit->getName() << "\" from " << source << " with ID " << id << std::endl;
            ucComputedMacCache.emplace(key, flit);

            // Try to verify the flit (in case the received MAC is already there)
            ucTryVerification(key);
        }
        else {
            throw cRuntimeError(this, "Crypto unit sent flit with unexpected mode %u (Source: %s, ID: %u)", mode, source.str().c_str(), id);
        }
    }
    else { // ncMode != uncoded
        // Get parameters
        uint16_t gev = flit->getGev();

        // Check if this generation is already finished
        IdSet& activeIdSet = activeIds[source];
        if(!activeIdSet.count(id)) {
            EV_DEBUG << "Received a decrypted/authenticated flit for finished GID " << id << " (GEV " << gev << ")" << std::endl;
            delete flit;
            return;
        }

        // Check flit mode
        if(mode == MODE_DATA) {
            // This is a decrypted flit arriving from a crypto unit
            // Get parameters
            GevCache& gevCache = ncDecryptedDataCache[key];

            // Assert that the decrypted data cache does not contain a flit with this GEV
            ASSERT(!gevCache.count(gev));

            // Check if we have to discard this flit (because corruption was detected on MAC verification)
            if(ncDiscardDecrypting[key][gev] > 0) {
                EV_DEBUG << "Discarding corrupted decrypted data flit \"" << flit->getName()
                         << "\" (source: " << source << ", ID: " << id << ", GEV: " << gev << ")" << std::endl;
                --ncDiscardDecrypting[key][gev];
                delete flit;
                return;
            }

            // We can safely cache the flit now
            EV_DEBUG << "Caching decrypted data flit \"" << flit->getName() << "\" from " << source << " with ID " << id << " and GEV " << gev << std::endl;
            gevCache.emplace(gev, flit);

            // Try to send out the decrypted flit
            ncTrySendToApp(key, gev);
        }
        else if(mode == MODE_MAC) {
            // This is a computed MAC arriving from a crypto unit
            // Get parameters
            GevCache& gevCache = ncComputedMacCache[key];

            // Assert that the decrypted data cache does not contain a flit with this GEV
            ASSERT(!gevCache.count(gev));

            // We can safely cache the flit now
            EV_DEBUG << "Caching computed MAC flit \"" << flit->getName() << "\" from " << source << " with ID " << id << " and GEV " << gev << std::endl;
            gevCache.emplace(gev, flit);

            // Try to verify the flit (in case the received MAC is already there)
            ncTryVerification(key, gev);
        }
        else {
            throw cRuntimeError(this, "Crypto unit sent flit with unexpected mode %u (Source: %s, ID: %u, GEV: %u)", mode, source.str().c_str(), id, gev);
        }
    }
}

void ArrivalManagerFlit::handleArqTimer(ArqTimer* timer) {
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
        if(!ucReceivedDataCache.count(key) && !ucReceivedMacCache.count(key)) {
            // Both flits are missing
            arqMode = ARQ_DATA_MAC;
        }
        else if(ucReceivedDataCache.count(key) && !ucReceivedMacCache.count(key)) {
            // Only MAC is missing
            arqMode = ARQ_MAC;
        }
        else if(!ucReceivedDataCache.count(key) && ucReceivedMacCache.count(key)) {
            // Only data is missing
            arqMode = ARQ_DATA;
        }
        else {
            // No flits are missing, this should never happen
            throw cRuntimeError(this, "ARQ timeout triggered for source %s, ID %u, but no flits are missing", key.second.str().c_str(), key.first);
        }

        // Issue the ARQ
        ucIssueArq(key, MODE_ARQ_TELL_MISSING, arqMode);
    }
    else { // ncMode != uncoded
        // Get parameters
        const GevCache& receivedData = ncReceivedDataCache[key];
        const GevCache& receivedMacs = ncReceivedMacCache[key];

        Mode flitMode;
        GevArqMap arqModes;

        // Check whether we'll TELL_MISSING or TELL_RECEIVED (based on how many GEVs we know)
        std::set<uint16_t> gevs;
        for(auto it = receivedData.begin(); it != receivedData.end(); ++it)
            gevs.insert(it->first);
        for(auto it = receivedMacs.begin(); it != receivedMacs.end(); ++it)
            gevs.insert(it->first);

        if(gevs.size() >= timer->getNumCombinations()) {
            // Use TELL_MISSING
            flitMode = MODE_ARQ_TELL_MISSING;

            // Iterate over GEVs, insert missing modes
            for(auto it = gevs.begin(); it != gevs.end(); ++it) {
                if(!receivedData.count(*it))
                    arqModes.emplace(*it, ARQ_DATA);
                else if(!receivedMacs.count(*it))
                    arqModes.emplace(*it, ARQ_MAC);
            }
        }
        else {
            // Use TELL_RECEIVED
            flitMode = MODE_ARQ_TELL_RECEIVED;

            // Iterate over GEVs, insert received modes
            for(auto it = gevs.begin(); it != gevs.end(); ++it) {
                if(receivedData.count(*it) && receivedMacs.count(*it))
                    arqModes.emplace(*it, ARQ_DATA_MAC);
                else if(receivedData.count(*it))
                    arqModes.emplace(*it, ARQ_DATA);
                else
                    arqModes.emplace(*it, ARQ_MAC);
            }
        }

        // Issue the ARQ
        ncIssueArq(key, flitMode, arqModes, ncMode);
    }
}

void ArrivalManagerFlit::ucStartDecryptAndAuth(const IdSourceKey& key) {
    // Retrieve the requested flit from the cache and send a copy out
    // for decryption and authentication
    // We use a copy here so that we don't have to remove the flit from the cache,
    // which is still used to check if the flit has already arrived
    EV_DEBUG << "Starting flit decryption for \"" << ucReceivedDataCache.at(key)->getName()
             << "\" (source: " << key.second << ", ID: " << key.first << ")" << std::endl;
    Flit* copy = ucReceivedDataCache.at(key)->dup();
    send(copy, "cryptoOut");
}

void ArrivalManagerFlit::ucTryVerification(const IdSourceKey& key) {
    // Check if both computed and received MAC are present
    FlitCache::iterator recvMac = ucReceivedMacCache.find(key);
    FlitCache::iterator compMac = ucComputedMacCache.find(key);

    if(recvMac != ucReceivedMacCache.end() && compMac != ucComputedMacCache.end()) {
        // Verify their equality
        bool equal = !recvMac->second->isModified() && !compMac->second->isModified() &&
                     !recvMac->second->hasBitError() && !compMac->second->hasBitError();

        // Examine verification result
        if(equal) {
            // Verification was successful, insert into success cache
            EV_DEBUG << "Successfully verified MAC \"" << recvMac->second->getName() << "\" (source: " << key.second
                     << ", ID: " << key.first << ")" << std::endl;
            ucVerified.insert(key);

            // Try to send out the decrypted flit
            ucTrySendToApp(key);
        }
        else {
            // Verification was not successful
            EV_DEBUG << "MAC verification failed for \"" << recvMac->second->getName() << "\" (source: " << key.second
                     << ", ID: " << key.first << ")" << std::endl;

            // Check if we have reached the ARQ limit
            if(issuedArqs[key] >= static_cast<unsigned int>(arqLimit)) {
                // We have failed completely, clean up everything and discard flits from this ID
                EV_DEBUG << "ARQ limit reached for source " << key.second << ", ID " << key.first << std::endl;
                ucCleanUp(key);
                return;
            }

            // Issue ARQ
            ucIssueArq(key, MODE_ARQ_TELL_MISSING, ARQ_DATA_MAC);

            // Clear received cache to ensure we can receive the retransmission
            ucDeleteFromCache(ucReceivedDataCache, key);
            ucDeleteFromCache(ucReceivedMacCache, key);

            // Also clear the decrypted data flit or, in case it has not arrived yet,
            // mark that it will be discarded on arrival
            if(!ucDeleteFromCache(ucDecryptedDataCache, key))
                ++ucDiscardDecrypting[key];

            // Also clear the computed MAC to ensure that we don't compare the next
            // received MAC against the wrong computed one
            ucDeleteFromCache(ucComputedMacCache, key);
        }
    }
}

void ArrivalManagerFlit::ucTrySendToApp(const IdSourceKey& key) {
    // Check if decrypted data flit is present and MAC was successfully verified
    FlitCache::iterator decData = ucDecryptedDataCache.find(key);
    if(decData != ucDecryptedDataCache.end() && ucVerified.count(key)) {
        // Send out a copy of the decrypted data flit
        // We use a copy here to avoid potential conflicts with cleanup
        EV_DEBUG << "Sending out decrypted data flit: source " << key.second << ", ID " << key.first << std::endl;
        Flit* copy = decData->second->dup();
        send(copy, "appOut");

        // This data/MAC pair is done, initiate cleanup
        ucCleanUp(key);
    }
}

void ArrivalManagerFlit::ucIssueArq(const IdSourceKey& key, Mode mode, ArqMode arqMode) {
    // Assert that we have not reached the maximum number of ARQs
    ASSERT(issuedArqs[key] < static_cast<unsigned int>(arqLimit));

    // Create ARQ
    Flit* arq = generateArq(key, mode, arqMode);

    // Send ARQ
    EV << "Sending ARQ \"" << arq->getName() << "\" from " << arq->getSource()
       << " to " << arq->getTarget() << " (ID: " << arq->getGidOrFid() << ")" << std::endl;
    send(arq, "arqOut");

    // Increment ARQ counter
    ++issuedArqs[key];

    // Set the ARQ timer
    setArqTimer(key, NC_UNCODED, true);
}

void ArrivalManagerFlit::ucCleanUp(const IdSourceKey& key) {
    // Clean up all information related to this ID/address
    EV_DEBUG << "Fully cleaning up: source " << key.second << ", ID " << key.first << std::endl;

    // Clear data/MAC caches
    ucDeleteFromCache(ucReceivedDataCache, key);
    ucDeleteFromCache(ucReceivedMacCache, key);
    ucDeleteFromCache(ucDecryptedDataCache, key);
    ucDeleteFromCache(ucComputedMacCache, key);

    // Clear verification result
    ucVerified.erase(key);

    // Clear number of issued ARQs
    issuedArqs.erase(key);

    // Clear corrupted decryption counter
    ucDiscardDecrypting.erase(key);

    // Remove ID from active ID set
    activeIds.at(key.second).erase(key.first);

    // TODO: check if this ID needs to be inserted into the finished ID set for the grace period

    // Cancel and delete any remaining ARQ timers
    TimerCache::iterator timerIter = arqTimers.find(key);
    if(timerIter != arqTimers.end()) {
        cancelAndDelete(timerIter->second);
        arqTimers.erase(timerIter);
    }
}

bool ArrivalManagerFlit::ucDeleteFromCache(FlitCache& cache, const IdSourceKey& key) {
    FlitCache::iterator element = cache.find(key);
    if(element != cache.end()) {
        delete element->second;
        cache.erase(element);
        return true;
    }
    return false;
}

void ArrivalManagerFlit::ncStartDecryptAndAuth(const IdSourceKey& key, uint16_t gev) {
    // Retrieve the requested flit from the cache and send a copy out
    // for decryption and authentication
    // We use a copy here so that we don't have to remove the flit from the cache,
    // which is still used to check if the flit has already arrived
    EV_DEBUG << "Starting flit decryption for \"" << ncReceivedDataCache.at(key).at(gev)->getName()
             << "\" (source: " << key.second << ", ID: " << key.first << ", GEV: " << gev << ")" << std::endl;
    Flit* copy = ncReceivedDataCache.at(key).at(gev)->dup();
    send(copy, "cryptoOut");
}

void ArrivalManagerFlit::ncTryVerification(const IdSourceKey& key, uint16_t gev) {
    // Get parameters
    GevCache& recvMacCache = ncReceivedMacCache[key];
    GevCache& compMacCache = ncComputedMacCache[key];

    // Check if both computed and received MAC are present
    GevCache::iterator recvMac = recvMacCache.find(gev);
    GevCache::iterator compMac = compMacCache.find(gev);

    if(recvMac != recvMacCache.end() && compMac != compMacCache.end()) {
        // Verify their equality
        bool equal = !recvMac->second->isModified() && !compMac->second->isModified() &&
                     !recvMac->second->hasBitError() && !compMac->second->hasBitError();

        // Examine verification result
        if(equal) {
            // Verification was successful, insert into success cache
            EV_DEBUG << "Successfully verified MAC \"" << recvMac->second->getName() << "\" (source: " << key.second
                     << ", ID: " << key.first << ", GEV: " << gev << ")" << std::endl;
            ncVerified[key].insert(gev);

            // Try to send out the decrypted flit
            ncTrySendToApp(key, gev);

            // Check if there is a planned ARQ waiting for ongoing verifications to finish
            ncTrySendPlannedArq(key);
        }
        else {
            // Verification was not successful
            EV_DEBUG << "MAC verification failed for \"" << recvMac->second->getName() << "\" (source: " << key.second
                     << ", ID: " << key.first << ", GEV: " << gev << ")" << std::endl;

            // Check if we have reached the ARQ limit
            if(issuedArqs[key] >= static_cast<unsigned int>(arqLimit)) {
                // We have failed completely, clean up everything and discard flits from this ID
                EV_DEBUG << "ARQ limit reached for source " << key.second << ", ID " << key.first << std::endl;
                ncCleanUp(key);
                return;
            }

            // Send out ARQ
            ncIssueArq(key, MODE_ARQ_TELL_MISSING, GevArqMap{{gev, ARQ_DATA_MAC}}, static_cast<NcMode>(recvMac->second->getNcMode()));

            // Clear received cache to ensure we can receive the retransmission
            ncDeleteFromCache(ncReceivedDataCache, key, gev);
            ncDeleteFromCache(ncReceivedMacCache, key, gev);

            // Also clear the decrypted data flit or, in case it has not arrived yet,
            // mark that it will be discarded on arrival
            if(!ncDeleteFromCache(ncDecryptedDataCache, key, gev))
                ++ncDiscardDecrypting[key][gev];

            // Also clear the computed MAC to ensure that we don't compare the next
            // received MAC against the wrong computed one
            ncDeleteFromCache(ncComputedMacCache, key, gev);
        }
    }
}

void ArrivalManagerFlit::ncTrySendToApp(const IdSourceKey& key, uint16_t gev) {
    // Get parameters
    GevCache& decDataCache = ncDecryptedDataCache[key];

    // Check if decrypted data flit is present and MAC was successfully verified
    GevCache::iterator decData = decDataCache.find(gev);
    if(decData != decDataCache.end() && ncVerified[key].count(gev)) {
        // Send out a copy of the decrypted data flit
        // We use a copy here to avoid potential conflicts with cleanup
        EV_DEBUG << "Sending out decrypted data flit: source " << key.second << ", ID " << key.first << ", GEV " << gev << std::endl;
        Flit* copy = decData->second->dup();
        send(copy, "appOut");

        // Insert this GEV into the dispatched GEV set for this generation
        ncDispatchedGevs[key].insert(gev);

        // Check if this generation is done
        ncCheckGenerationDone(key, decData->second->getGenSize());
    }
}

void ArrivalManagerFlit::ncIssueArq(const IdSourceKey& key, Mode mode, const GevArqMap& arqModes, Messages::NcMode ncMode) {
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

void ArrivalManagerFlit::ncTrySendPlannedArq(const IdSourceKey& key, bool forceImmediate) {
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
           << " to " << arq->getTarget() << " (ID: " << arq->getGidOrFid() << ")" << std::endl;
        send(arq, "arqOut");

        // Remove from planned ARQ map
        ncPlannedArqs.erase(plannedIter);

        // Increment ARQ counter
        ++issuedArqs[key];

        // Set the ARQ timer
        setArqTimer(key, static_cast<NcMode>(arq->getNcMode()), true);
    }
}

void ArrivalManagerFlit::ncCheckGenerationDone(const IdSourceKey& key, unsigned short generationSize) {
    // Check if we have sent enough data flits to the app in order
    // to decode this generation
    GevSet& dispatchedGevs = ncDispatchedGevs[key];
    if(dispatchedGevs.size() >= generationSize) {
        // Clean up whole generation
        ncCleanUp(key);

        // TODO: check if this ID needs to be inserted into the finished ID set for the grace period
    }
}

void ArrivalManagerFlit::ncCleanUp(const IdSourceKey& key) {
    // Clean up all information related to this ID/address
    EV_DEBUG << "Fully cleaning up: source " << key.second << ", ID " << key.first << std::endl;

    // Clear data/MAC caches
    ncDeleteFromCache(ncReceivedDataCache, key);
    ncDeleteFromCache(ncReceivedMacCache, key);
    ncDeleteFromCache(ncDecryptedDataCache, key);
    ncDeleteFromCache(ncComputedMacCache, key);

    // Clear verification results
    ncVerified.erase(key);

    // Clear number of issued ARQs
    issuedArqs.erase(key);

    // Clear dispatched GEV set
    ncDispatchedGevs.erase(key);

    // Clear corrupted decryption counter
    ncDiscardDecrypting.erase(key);

    // Remove ID from active ID set
    activeIds.at(key.second).erase(key.first);

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

bool ArrivalManagerFlit::ncDeleteFromCache(GenCache& cache, const IdSourceKey& key) {
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

bool ArrivalManagerFlit::ncDeleteFromCache(GenCache& cache, const IdSourceKey& key, uint16_t gev) {
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

Flit* ArrivalManagerFlit::generateArq(const IdSourceKey& key, Mode mode, ArqMode arqMode) {
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

Flit* ArrivalManagerFlit::generateArq(const IdSourceKey& key, Mode mode, const GevArqMap& arqModes, NcMode ncMode) {
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

void ArrivalManagerFlit::setArqTimer(const IdSourceKey& key, NcMode ncMode, bool useAnswerTime, bool setToMax) {
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

void ArrivalManagerFlit::cancelArqTimer(const IdSourceKey& key) {
    // Check if there is a timer for this ID/source
    TimerCache::iterator timerIter = arqTimers.find(key);
    if(timerIter != arqTimers.end()) {
        // Cancel the timer in case it is scheduled
        cancelEvent(timerIter->second);
    }
}

bool ArrivalManagerFlit::ncCheckCompleteGenerationReceived(const IdSourceKey& key, unsigned short numCombinations) {
    // Check if we have received the appropriate number of combinations (both data and MAC)
    const GevCache& receivedData = ncReceivedDataCache[key];
    const GevCache& receivedMacs = ncReceivedMacCache[key];

    return receivedData.size() == numCombinations && receivedMacs.size() == numCombinations;
}

bool ArrivalManagerFlit::ncCheckVerificationOngoing(const IdSourceKey& key) const {
    // Check if we are currently verifying a flit
    // This is the case when we have received both data and mac flit, but
    // there is no computed MAC yet (for each GEV)
    GenCache::const_iterator receivedData = ncReceivedDataCache.find(key);
    GenCache::const_iterator receivedMacs = ncReceivedMacCache.find(key);
    GenCache::const_iterator computedMacs = ncComputedMacCache.find(key);

    // If one of the received caches is empty, there are no verifications ongoing
    if(receivedData == ncReceivedDataCache.end() || receivedMacs == ncReceivedMacCache.end())
        return false;

    // Iterate over all GEVs for which we have received both data and mac flit
    for(auto it = receivedData->second.begin(); it != receivedData->second.end(); ++it) {
        if(receivedMacs->second.count(it->first)) {
            // We have both data and mac for this GEV; check if there is a computed MAC
            if(computedMacs == ncComputedMacCache.end() || !computedMacs->second.count(it->first)) {
                // No computed MAC found
                return true;
            }
        }
    }

    // We have found a computed MAC for all applicable GEVs
    return false;
}

}} //namespace
