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

#include "RetransmissionBufferImplSplit.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace Buffers {

Define_Module(RetransmissionBufferImplSplit);

RetransmissionBufferImplSplit::RetransmissionBufferImplSplit() {
}

RetransmissionBufferImplSplit::~RetransmissionBufferImplSplit() {
}

void RetransmissionBufferImplSplit::handleArqMessage(Flit* flit) {
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

            if(!retrieveSpecifiedFlits(modeCache, arqMode, sendQueue, false)) {
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
                ASSERT(modeCache.size() == 1);

                if(!retrieveSpecifiedFlits(modeCache, it->second, sendQueue, true)) {
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

bool RetransmissionBufferImplSplit::retrieveSpecifiedFlits(const ModeCache& cache, ArqMode mode, FlitQueue& outQueue, bool networkCoded) {
    // Retrieve the specified missing flits
    if(!networkCoded) {
        // No network coding
        if(mode == ARQ_SPLIT_1 || mode == ARQ_SPLITS_BOTH) {
            // Try to find first split
            ModeCache::const_iterator iter = cache.find(MODE_SPLIT_1);
            if(iter == cache.end())
                return false;

            outQueue.push(iter->second);
        }
        if(mode == ARQ_SPLIT_2 || mode == ARQ_SPLITS_BOTH) {
            // Try to find second split
            ModeCache::const_iterator iter = cache.find(MODE_SPLIT_2);
            if(iter == cache.end())
                return false;

            outQueue.push(iter->second);
        }
    }
    else {
        // Network coding
        ASSERT(mode == ARQ_SPLIT_NC);

        // Try to find the split (should be the only one in the mode cache)
        ModeCache::const_iterator iter = cache.find(MODE_SPLIT_NC);
        if(iter == cache.end())
            return false;

        outQueue.push(iter->second);
    }

    return true;
}

bool RetransmissionBufferImplSplit::retrieveMissingFlits(const GevCache& cache, const GevArqMap& modes, FlitQueue& outQueue) {
    // Iterate over all the GEVs we have for this ID/Target
    for(auto it = cache.begin(); it != cache.end(); ++it) {
        const ModeCache& modeCache = it->second;

        // Find the GEV in the ARQ
        GevArqMap::const_iterator arqIter = modes.find(it->first);

        // Check if the split of this GEV has been received
        if(arqIter == modes.end()) {
            // Try to find the split
            ModeCache::const_iterator modeIter = modeCache.find(MODE_SPLIT_NC);
            if(modeIter == modeCache.end()) // This should never happen, if the split was deleted, the map should not exist any more
                throw cRuntimeError(this, "Unexpected situation during flit search: A mode map without a flit was found.");

            outQueue.push(modeIter->second);
        }
    }

    return true;
}

unsigned int RetransmissionBufferImplSplit::countMissingFlits(const GevArqMap& modes) {
    // There is one flit per GEV, so we just subtract the number of ARQ mode entries
    return static_cast<unsigned int>(numCombinations) - modes.size();
}

}} //namespace
