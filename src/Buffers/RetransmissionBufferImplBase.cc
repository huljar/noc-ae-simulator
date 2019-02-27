//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "RetransmissionBufferImplBase.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace Buffers {

RetransmissionBufferImplBase::RetransmissionBufferImplBase()
    : bufSize(10)
    , numCombinations(3)
{
}

RetransmissionBufferImplBase::~RetransmissionBufferImplBase() {
    for(auto it = ucFlitCache.begin(); it != ucFlitCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete jt->second;
    for(auto it = ncFlitCache.begin(); it != ncFlitCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            for(auto kt = jt->second.begin(); kt != jt->second.end(); ++kt)
                delete kt->second;
}

void RetransmissionBufferImplBase::initialize() {
    bufSize = par("bufSize");
    if(bufSize < 0)
        throw cRuntimeError(this, "Retransmission buffer size must be greater or equal to 0, but received %i", bufSize);

    networkCoding = getAncestorPar("networkCoding");

    if(networkCoding) {
        numCombinations = getAncestorPar("numCombinations");
        if(numCombinations < 2)
            throw cRuntimeError(this, "Number of combinations must be greater than 2, but received %i", numCombinations);
    }
}

void RetransmissionBufferImplBase::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    // Check if this is an ARQ or not
    if(flit->isArq()) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "arqIn") == 0);

        // Check if the ARQ is modified
        // In practice, the RT cannot know if the flit was modified as long as the structure is intact. However, it would
        // send wrong retransmissions then. We don't distinguish between these cases here.
        if(flit->isModified() || flit->hasBitError()) {
            EV << "Received a corrupted ARQ \"" << flit->getName() << "\" at retransmission buffer. Discarding it." << std::endl;
            delete flit;
            return;
        }

        handleArqMessage(flit);
    }
    else {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "dataIn") == 0);
        handleDataMessage(flit);
    }
}

void RetransmissionBufferImplBase::handleDataMessage(Flit* flit) {
    // Get parameters
    IdTargetKey key = std::make_pair(flit->getGidOrFid(), flit->getTarget());
    Mode mode = static_cast<Mode>(flit->getMode());

    // Duplicate flit so we can store the original
    Flit* copy = flit->dup();

    // Add original to cache
    if(flit->getNcMode() == NC_UNCODED) {
        // Uncoded flit
        if(!ucFlitCache[key].emplace(mode, flit).second)
            throw cRuntimeError(this, "Unable to cache flit in the retransmission buffer!");
        ucFlitQueue.push(flit);
    }
    else {
        // Network coded flit
        uint16_t gev = flit->getGev();
        if(!ncFlitCache[key][gev].emplace(mode, flit).second)
            throw cRuntimeError(this, "Unable to cache flit in the retransmission buffer!");
        ncFlitQueue.push(flit);
    }

    // Check if cache size was exceeded
    while(ucFlitQueue.size() > static_cast<size_t>(bufSize)) {
        ucRemoveFromCache(ucFlitQueue.front());
        ucFlitQueue.pop();
    }
    while(ncFlitQueue.size() > static_cast<size_t>(bufSize)) {
        ncRemoveFromCache(ncFlitQueue.front());
        ncFlitQueue.pop();
    }

    // Send out the duplicated flit
    send(copy, "dataOut");
}

void RetransmissionBufferImplBase::ucRemoveFromCache(Flit* flit) {
    // Get parameters
    IdTargetKey key = std::make_pair(flit->getGidOrFid(), flit->getTarget());
    Mode mode = static_cast<Mode>(flit->getMode());
    ModeCache& modeCache = ucFlitCache.at(key);

    // Delete flit
    delete modeCache.at(mode);
    modeCache.erase(mode);

    // Remove container entry if it became empty
    if(modeCache.empty())
        ucFlitCache.erase(key);
}

void RetransmissionBufferImplBase::ncRemoveFromCache(Flit* flit) {
    // Get parameters
    IdTargetKey key = std::make_pair(flit->getGidOrFid(), flit->getTarget());
    uint16_t gev = flit->getGev();
    Mode mode = static_cast<Mode>(flit->getMode());

    GevCache& gevCache = ncFlitCache.at(key);
    ModeCache& modeCache = gevCache.at(gev);

    // Delete flit
    delete modeCache.at(mode);
    modeCache.erase(mode);

    // Remove container entries if they became empty
    if(modeCache.empty())
        gevCache.erase(gev);
    if(gevCache.empty())
        ncFlitCache.erase(key);
}

}} //namespace
