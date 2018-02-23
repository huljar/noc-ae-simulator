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
    // TODO: delete any remaining flits in the maps
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
        else {
            // Check if this ID in the active ID set
            if(!activeIdSet.count(id)) {
                // Allow grace period for out-of-order arrivals (lower ID may arrive later than higher ID)
                if(id >= highestId - outOfOrderIdGracePeriod) {
                    // This ID is still allowed into the active ID set
                    activeIdSet.insert(id); // TODO: do not allow finished IDs here?
                }
                else {
                    // This is either a retransmission of an old, finished ID or a repeat attack
                    // Discard the flit and return
                    EV << "Received a flit from " << source << " with ID " << id
                       << ", but ID is inactive (highest known ID: " << highestId << ")" << std::endl;
                    delete flit;
                    return;
                }
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
    else { // mode != uncoded
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
            gevCache.emplace(flit->getGev(), flit);

            // TODO: how to proceed?
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
            gevCache.emplace(flit->getGev(), flit);

            // TODO: how to proceed?
        }
        else {
            throw cRuntimeError(this, "Received flit with unexpected mode %u from %s (ID: %u)", mode, source.str().c_str(), id);
        }
    }
}

void ArrivalManagerFlit::handleCryptoMessage(Flit* flit) {
}

void ArrivalManagerFlit::handleArqTimer(ArqTimer* timer) {
}

void ArrivalManagerFlit::ucStartDecryptAndAuth(const IdSourceKey& key) {
    // Retrieve the requested flit from the cache and send a copy out
    // for decryption and authentication
    Flit* copy = ucReceivedDataCache.at(key)->dup();
    send(copy, "cryptoOut");
}

void ArrivalManagerFlit::ucTryVerification(const IdSourceKey& key) {
    // Check if both computed and received MAC are present
    FlitCache::iterator recvMac = ucReceivedMacCache.find(key);
    FlitCache::iterator compMac = ucComputedMacCache.find(key);

    if(recvMac != ucReceivedMacCache.end() && compMac != ucComputedMacCache.end()) {
        // Verify their equality
        // TODO: how to pseudo-implement this? dirty bit?
        bool equal = true;

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
        }
    }
}

void ArrivalManagerFlit::ucTrySendToApp(const IdSourceKey& key) {
}

void ArrivalManagerFlit::ucCleanUp(const IdSourceKey& key) {
    // Clean up all information related to this ID/address
}

void ArrivalManagerFlit::ncCleanUp(const IdSourceKey& key) {
    // Clean up all information related to this ID/address
}

void ArrivalManagerFlit::generateArq(const IdSourceKey& key, Messages::Mode mode) {
    // TODO: more detailed ARQ payload (HAVE + available (GEVs/)modes or DON'T HAVE + required (GEVs/)modes)
    static const Address2D source(nodeX, nodeY);

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
