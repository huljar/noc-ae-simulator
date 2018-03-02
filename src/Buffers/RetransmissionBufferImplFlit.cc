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

#include "RetransmissionBufferImplFlit.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace Buffers {

Define_Module(RetransmissionBufferImplFlit);

RetransmissionBufferImplFlit::RetransmissionBufferImplFlit() {
}

RetransmissionBufferImplFlit::~RetransmissionBufferImplFlit() {
}

void RetransmissionBufferImplFlit::handleArqMessage(Flit* flit) {
    // Get parameters
    IdTargetKey key = std::make_pair(flit->getGidOrFid(), flit->getSource()); // ARQ source is retransmission target
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());

    // Queue to store the flits that shall be sent
    std::queue<Flit*> sendQueue;

    // Check network coding
    if(ncMode == NC_UNCODED) {
        // Check if this ID/Target is still in the cache
        FlitCache::iterator outerIter = ucFlitCache.find(key);
        if(outerIter == ucFlitCache.end()) {
            EV << "Failed to answer ARQ - ID/Target not in buffer any more" << std::endl;
            delete flit;
            return;
        }

        // Confirm that we are using ARQ_MISSING mode
        if(mode == MODE_ARQ_TELL_MISSING) {
            ModeCache& modeCache = outerIter->second;
            ArqMode arqMode = static_cast<ArqMode>(flit->getUcArqs());

            if(!retrieveSpecifiedFlits(modeCache, arqMode, sendQueue)) {
                delete flit;
                return;
            }
        }
        else {
            throw cRuntimeError(this, "Unexpected ARQ mode: %s", cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode));
        }
    }
    else { // ncMode != uncoded
        // Check if this ID/Target is still in the cache
        GenCache::iterator outerIter = ncFlitCache.find(key);
        if(outerIter == ncFlitCache.end()) {
            EV << "Failed to answer ARQ - ID/Target not in buffer any more" << std::endl;
            delete flit;
            return;
        }
        GevCache& gevCache = outerIter->second;
        const GevArqMap& gevArqModes = flit->getNcArqs();

        // Check what kind of ARQ we have
        if(mode == MODE_ARQ_TELL_RECEIVED) {
            // Count how many flits we have to find
            unsigned int numMissing = countMissingFlits(gevArqModes);

            if(!retrieveMissingFlits(gevCache, gevArqModes, sendQueue)) {
                EV << "Failed to answer ARQ - One of the missing flits is not in buffer any more" << std::endl;
                delete flit;
                return;
            }

            // Check if we found enough flits
            if(sendQueue.size() < static_cast<size_t>(numMissing)) {
                EV << "Failed to answer ARQ - One of the missing GEVs is not in buffer any more" << std::endl;
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
        }
        else {
            throw cRuntimeError(this, "Unexpected ARQ mode: %s", cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode));
        }
    }

    // Send out copies of the retrieved flits
    while(!sendQueue.empty()) {
        send(sendQueue.front()->dup(), "arqOut");
        sendQueue.pop();
    }

    // Delete ARQ flit
    delete flit;
}

bool RetransmissionBufferImplFlit::retrieveSpecifiedFlits(const ModeCache& cache, ArqMode mode, FlitQueue& outQueue) {
    // Retrieve the specified missing flits
    if(mode == ARQ_DATA || mode == ARQ_DATA_MAC) {
        // Try to find data flit
        ModeCache::const_iterator iter = cache.find(MODE_DATA);
        if(iter == cache.end())
            return false;

        outQueue.push(iter->second);
    }
    if(mode == ARQ_MAC || mode == ARQ_DATA_MAC) {
        // Try to find MAC flit
        ModeCache::const_iterator iter = cache.find(MODE_MAC);
        if(iter == cache.end())
            return false;

        outQueue.push(iter->second);
    }

    return true;
}

bool RetransmissionBufferImplFlit::retrieveMissingFlits(const GevCache& cache, const GevArqMap& modes, FlitQueue& outQueue) {
    // Iterate over all the GEVs we have for this ID/Target
    for(auto it = cache.begin(); it != cache.end(); ++it) {
        const ModeCache& modeCache = it->second;

        // Find the GEV in the ARQ
        GevArqMap::const_iterator arqIter = modes.find(it->first);

        // Check if the data flit of this GEV has been received
        if(arqIter == modes.end() || arqIter->second == ARQ_MAC) {
            // Try to find the data flit
            ModeCache::const_iterator modeIter = modeCache.find(MODE_DATA);
            if(modeIter == modeCache.end())
                return false;

            outQueue.push(modeIter->second);
        }

        // Check if the MAC flit of this GEV has been received
        if(arqIter == modes.end() || arqIter->second == ARQ_DATA) {
            // Try to find the MAC flit
            ModeCache::const_iterator modeIter = modeCache.find(MODE_MAC);
            if(modeIter == modeCache.end())
                return false;

            outQueue.push(modeIter->second);
        }
    }

    return true;
}

unsigned int RetransmissionBufferImplFlit::countMissingFlits(const GevArqMap& modes) {
    // The maximum number of missing flits is data+MAC for each combination
    unsigned int num = 2 * static_cast<unsigned int>(numCombinations);

    // Subtract the amount of flits that the ARQ sender already has
    for(auto it = modes.begin(); it != modes.end(); ++it) {
        if(it->second == ARQ_DATA || it->second == ARQ_MAC) --num;
        else if(it->second == ARQ_DATA_MAC) num -= 2;
        else throw cRuntimeError(this, "Unexpected mode in ARQ: %s", cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(it->second));
    }

    return num;
}

}} //namespace
