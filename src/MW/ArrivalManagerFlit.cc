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
#include <sstream>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(ArrivalManagerFlit);

ArrivalManagerFlit::ArrivalManagerFlit() {
}

ArrivalManagerFlit::~ArrivalManagerFlit() {
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
}

void ArrivalManagerFlit::initialize() {
    arqLimit = par("arqLimit");
    if(arqLimit < 1)
        throw cRuntimeError(this, "arqLimit must be greater than 0");

    arqIssueTimeout = par("arqIssueTimeout");
    if(arqIssueTimeout < 1)
        throw cRuntimeError(this, "arqIssueTimeout must be greater than 0");

    arqResendTimeout = par("arqResendTimeout");
    if(arqResendTimeout < 1)
        throw cRuntimeError(this, "arqResendTimeout must be greater than 0");

    outOfOrderIdGracePeriod = par("outOfOrderIdGracePeriod");
    if(outOfOrderIdGracePeriod < 0)
        throw cRuntimeError(this, "outOfOrderIdGracePeriod must be greater than or equal to 0");

    gridColumns = getAncestorPar("columns");
    nodeId = getAncestorPar("id");

    nodeX = nodeId % gridColumns;
    nodeY = nodeId / gridColumns;
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

// There is a possibility for flit handling errors in the case
// where decryption takes significantly longer than authentication.
// To be precise, if flit authentication + ARQ round trip is faster than
// decryption, then the wrong decrypted data flit will be sent to the app.
// However, this case should not occur in practice because decryption and
// authentication should be very similar in speed. Still, one should keep
// this in mind when setting NED parameters.
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
    NC ncMode = static_cast<NC>(flit->getNcMode());

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
            ucReceivedDataCache.emplace(key, flit);

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
            ucReceivedMacCache.emplace(key, flit);

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
            gevCache.emplace(gev, flit);

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
            gevCache.emplace(gev, flit);

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
    NC ncMode = static_cast<NC>(flit->getNcMode());

    // Check network coding mode
    if(ncMode == NC_UNCODED) {
        // Check flit mode
        if(mode == MODE_DATA) {
            // This is a decrypted flit arriving from a crypto unit
            // Assert that the decrypted flit cache does not contain a flit
            ASSERT(!ucDecryptedDataCache.count(key));

            // Insert decrypted flit into the cache
            ucDecryptedDataCache.emplace(key, flit);

            // Try to send out the decrypted flit
            ucTrySendToApp(key);
        }
        else if(mode == MODE_MAC) {
            // This is a computed MAC arriving from a crypto unit
            // Assert that the computed MAC cache does not contain a flit
            ASSERT(!ucComputedMacCache.count(key));

            // We don't have this flit yet, cache it
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
            EV << "Received a decrypted/authenticated flit from an already finished generation" << std::endl;
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

            // We can safely cache the flit now
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
}

void ArrivalManagerFlit::ucStartDecryptAndAuth(const IdSourceKey& key) {
    // Clear any previously decrypted flits
    // This is to prevent accidentally sending the wrong flit to the app
    ucDeleteFromCache(ucDecryptedDataCache, key);

    // Retrieve the requested flit from the cache and send a copy out
    // for decryption and authentication
    // We use a copy here so that we don't have to remove the flit from the cache,
    // which is still used to check if the flit has already arrived
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
            ucVerified.insert(key);

            // Try to send out the decrypted flit
            ucTrySendToApp(key);
        }
        else {
            // Verification was not successful
            // Check if we have reached the ARQ limit
            if(issuedArqs[key] >= arqLimit) {
                // We have failed completely, clean up everything and discard flits from this ID
                ucCleanUp(key);
                return;
            }

            // Send out ARQ
            generateArq(key, MODE_DATA); // TODO: do this right

            // Clear received cache to ensure we can receive the retransmission
            ucDeleteFromCache(ucReceivedDataCache, key);
            ucDeleteFromCache(ucReceivedMacCache, key);

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
        Flit* copy = decData->second->dup();
        send(copy, "appOut");

        // This data/MAC pair is done, initiate cleanup
        ucCleanUp(key);
    }
}

void ArrivalManagerFlit::ucCleanUp(const IdSourceKey& key) {
    // Clean up all information related to this ID/address
    // Clear data/MAC caches
    ucDeleteFromCache(ucReceivedDataCache, key);
    ucDeleteFromCache(ucReceivedMacCache, key);
    ucDeleteFromCache(ucDecryptedDataCache, key);
    ucDeleteFromCache(ucComputedMacCache, key);

    // Clear verification result
    ucVerified.erase(key);

    // Clear number of issued ARQs
    issuedArqs.erase(key);

    // Remove ID from active ID set
    activeIds.at(key.second).erase(key.first);

    // TODO: check if this ID needs to be inserted into the finished ID set for the grace period
}

void ArrivalManagerFlit::ucDeleteFromCache(FlitCache& cache, const IdSourceKey& key) {
    FlitCache::iterator element = cache.find(key);
    if(element != cache.end()) {
        delete element->second;
        cache.erase(element);
    }
}

void ArrivalManagerFlit::ncStartDecryptAndAuth(const IdSourceKey& key, uint16_t gev) {
    // Clear any previously decrypted flits
    // This is to prevent accidentally sending the wrong flit to the app
    ncDeleteFromCache(ncDecryptedDataCache, key, gev);

    // Retrieve the requested flit from the cache and send a copy out
    // for decryption and authentication
    // We use a copy here so that we don't have to remove the flit from the cache,
    // which is still used to check if the flit has already arrived
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
            ncVerified[key].insert(gev);

            // Try to send out the decrypted flit
            ncTrySendToApp(key, gev);
        }
        else {
            // Verification was not successful
            // Check if we have reached the ARQ limit
            if(issuedArqs[key] >= arqLimit) {
                // We have failed completely, clean up everything and discard flits from this ID
                ncCleanUp(key);
                return;
            }

            // TODO: send out ARQ now or later? depending on ARQ limit
            // Send out ARQ
            //generateArq(key, MODE_DATA); // TODO: do this right

            // Clear received cache to ensure we can receive the retransmission
            //ncDeleteFromCache(ncReceivedDataCache, key);
            //ncDeleteFromCache(ncReceivedMacCache, key);

            // Also clear the computed MAC to ensure that we don't compare the next
            // received MAC against the wrong computed one
            //ncDeleteFromCache(ncComputedMacCache, key);
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
        Flit* copy = decData->second->dup();
        send(copy, "appOut");

        // Insert this GEV into the dispatched GEV set for this generation
        ncDispatchedGevs[key].insert(gev);

        // Check if this generation is done
        ncCheckGenerationDone(key);
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

    // Remove ID from active ID set
    activeIds.at(key.second).erase(key.first);
}

void ArrivalManagerFlit::ncDeleteFromCache(GenCache& cache, const IdSourceKey& key) {
    GenCache::iterator actualCacheIter = cache.find(key);
    if(actualCacheIter != cache.end()) {
        GevCache& actualCache = actualCacheIter->second;
        for(auto it = actualCache.begin(); it != actualCache.end(); ++it)
            delete it->second;
        cache.erase(actualCacheIter);
    }
}

void ArrivalManagerFlit::ncDeleteFromCache(GenCache& cache, const IdSourceKey& key, uint16_t gev) {
    GenCache::iterator outerIter = cache.find(key);
    if(outerIter != cache.end()) {
        GevCache::iterator innerIter = outerIter->second.find(gev);
        if(innerIter != outerIter->second.end()) {
            delete innerIter->second;
            outerIter->second.erase(innerIter);
        }
    }
}

void ArrivalManagerFlit::generateArq(const IdSourceKey& key, Messages::Mode mode) {
    // TODO: more detailed ARQ payload (HAVE + available (GEVs/)modes or DON'T HAVE + required (GEVs/)modes)
    Address2D source(nodeX, nodeY);

    // Build packet name
    std::ostringstream packetName;
    packetName << "arq-" << key.first << "-s" << source << "-t" << key.second;

    // Create the flit
    Flit* arq = new Flit(packetName.str().c_str());
    take(arq);

    // Set header fields
    arq->setSource(source);
    arq->setTarget(key.second);
    arq->setGidOrFid(key.first);

    //emit(pktgenerateSignal, flit->getGidOrFid());
    EV << "Sending ARQ \"" << arq->getName() << "\" from " << arq->getSource()
       << " to " << arq->getTarget() << " (ID: " << arq->getGidOrFid() << ")" << std::endl;
    send(arq, "out");

    // Increment ARQ counter
    ++issuedArqs[key];
}

}} //namespace
