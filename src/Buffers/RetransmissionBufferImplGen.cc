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

#include "RetransmissionBufferImplGen.h"
#include <Util/Constants.h>

using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace Buffers {

Define_Module(RetransmissionBufferImplGen);

RetransmissionBufferImplGen::RetransmissionBufferImplGen() {
}

RetransmissionBufferImplGen::~RetransmissionBufferImplGen() {
    for(auto it = macCache.begin(); it != macCache.end(); ++it)
        delete it->second;
}

void RetransmissionBufferImplGen::handleDataMessage(Flit* flit) {
    ASSERT(flit->getNcMode() != NC_UNCODED);

    // Get parameters
    IdTargetKey key = std::make_pair(flit->getGidOrFid(), flit->getTarget());
    Mode mode = static_cast<Mode>(flit->getMode());

    // Duplicate flit so we can store the original
    Flit* copy = flit->dup();

    // Add original to cache (MAC goes to special cache)
    if(mode == MODE_MAC) {
        if(!macCache.emplace(key, flit).second)
            throw cRuntimeError(this, "Unable to cache MAC in the retransmission buffer!");
    }
    else {
        uint16_t gev = flit->getGev();
        if(!ncFlitCache[key][gev].emplace(mode, flit).second)
            throw cRuntimeError(this, "Unable to cache flit in the retransmission buffer!");
    }
    ncFlitQueue.push(flit);

    // Check if cache size was exceeded
    while(ncFlitQueue.size() > static_cast<size_t>(bufSize)) {
        Messages::Flit* front = ncFlitQueue.front();
        if(front->getMode() == MODE_MAC) {
            macCache.erase(std::make_pair(front->getGidOrFid(), front->getTarget()));
        }
        else {
            ncRemoveFromCache(front);
        }
        ncFlitQueue.pop();
    }

    // Send out the duplicated flit
    send(copy, "dataOut");
}

void RetransmissionBufferImplGen::handleArqMessage(Flit* flit) {
    // Get parameters
    IdTargetKey key = std::make_pair(flit->getGidOrFid(), flit->getSource()); // ARQ source is retransmission target
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());

    // Queue to store the flits that shall be sent
    std::queue<Flit*> sendQueue;

    // Assure that we are dealing with a network coded environment
    ASSERT(ncMode != NC_UNCODED);

    // Check if this ID/Target is still in the cache
    GenCache::iterator outerIter = ncFlitCache.find(key);
    if(outerIter == ncFlitCache.end()) {
        EV << "Failed to answer ARQ - ID/Target not in buffer any more" << std::endl;
        delete flit;
        return;
    }
    GevCache& gevCache = outerIter->second;
    const GevArqMap& gevArqModes = flit->getNcArqs();
    bool genMac = flit->getNcArqGenMac();

    // Check what kind of ARQ we have
    if(mode == MODE_ARQ_TELL_RECEIVED) {
        // Count how many flits we have to find
        unsigned int numMissing = countMissingFlits(gevArqModes, !genMac);

        if(!retrieveMissingFlits(gevCache, gevArqModes, sendQueue)) {
            EV << "Failed to answer ARQ - One of the missing flits is not in buffer any more" << std::endl;
            delete flit;
            return;
        }

        // Get the generation MAC if it was not received
        if(!genMac && !retrieveGenerationMac(key, sendQueue)) {
            EV << "Failed to answer ARQ - Generation MAC not in buffer any more" << std::endl;
            delete flit;
            return;
        }

        // Check if we found enough flits
        if(sendQueue.size() < static_cast<size_t>(numMissing)) {
            EV << "Failed to answer ARQ - One of the missing GEVs (or the MAC) is not in buffer any more" << std::endl;
            delete flit;
            return;
        }
    }
    else if(mode == MODE_ARQ_TELL_MISSING) {
        // Iterate over specified missing flits
        for(auto it = gevArqModes.begin(); it != gevArqModes.end(); ++it) {
            GevCache::iterator innerIter = gevCache.find(it->first);
            if(innerIter == gevCache.end()) {
                EV << "Failed to answer ARQ - Specified GEV not in buffer any more" << std::endl;
                delete flit;
                return;
            }
            ModeCache& modeCache = innerIter->second;

            if(!retrieveSpecifiedFlits(modeCache, it->second, sendQueue)) {
                EV << "Failed to answer ARQ - One of the requested flits is not in buffer any more" << std::endl;
                delete flit;
                return;
            }
        }

        // Get the generation MAC if requested
        if(genMac && !retrieveGenerationMac(key, sendQueue)) {
            EV << "Failed to answer ARQ - Generation MAC not in buffer any more" << std::endl;
            delete flit;
            return;
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected ARQ mode: %s", cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode));
    }

    // Send out copies of the retrieved flits
    while(!sendQueue.empty()) {
        send(sendQueue.front()->dup(), "arqOut");
        sendQueue.pop();
    }

    // Delete ARQ flit
    delete flit;
}

bool RetransmissionBufferImplGen::retrieveSpecifiedFlits(const ModeCache& cache, ArqMode mode, FlitQueue& outQueue) {
    // Retrieve the specified missing flits
    // Try to find data flit
    ModeCache::const_iterator iter = cache.find(MODE_DATA);
    if(iter == cache.end())
        return false;

    outQueue.push(iter->second);

    return true;
}

bool RetransmissionBufferImplGen::retrieveMissingFlits(const GevCache& cache, const GevArqMap& modes, FlitQueue& outQueue) {
    // Iterate over all the GEVs we have for this ID/Target
    for(auto it = cache.begin(); it != cache.end(); ++it) {
        const ModeCache& modeCache = it->second;

        // Find the GEV in the ARQ
        GevArqMap::const_iterator arqIter = modes.find(it->first);

        // Check if the data flit of this GEV has been received
        if(arqIter == modes.end()) {
            // Try to find the data flit
            ModeCache::const_iterator modeIter = modeCache.find(MODE_DATA);
            if(modeIter == modeCache.end())
                return false;

            outQueue.push(modeIter->second);
        }
    }

    return true;
}

bool RetransmissionBufferImplGen::retrieveGenerationMac(const IdTargetKey& key, FlitQueue& outQueue) {
    MacCache::const_iterator iter = macCache.find(key);
    if(iter == macCache.end())
        return false;

    outQueue.push(iter->second);
    return true;
}

unsigned int RetransmissionBufferImplGen::countMissingFlits(const GevArqMap& modes, bool genMacRequested) {
    // There is one flit per GEV, so we just subtract the number of ARQ mode entries and add the generation MAC if requested
    return static_cast<unsigned int>(numCombinations) - modes.size() + (genMacRequested ? 1 : 0);
}

}} //namespace
