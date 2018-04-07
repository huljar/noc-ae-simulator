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

#include "SplitMerger.h"
#include <Messages/Flit.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(SplitMerger);

SplitMerger::SplitMerger() {
}

SplitMerger::~SplitMerger() {
	for(auto it = splitCache.begin(); it != splitCache.end(); ++it) {
	    delete it->second.first;
	    delete it->second.second;
	}
}

void SplitMerger::initialize() {
    MiddlewareBase::initialize();
}

void SplitMerger::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

    // Get parameters
    Mode mode = static_cast<Mode>(flit->getMode());

    ASSERT(mode == MODE_SPLIT_1 || mode == MODE_SPLIT_2);

    // Get parameters
    const Address2D& source = flit->getSource();

    // Insert split into cache (indexed by source address and generation ID)
    SplitPair& cache = splitCache[source];

    if(mode == MODE_SPLIT_1) {
        if(cache.first)
            throw cRuntimeError(this, "Received a first split for source %s, but we already have one", source.str().c_str());
        cache.first = flit;
    }
    else if(mode == MODE_SPLIT_2) {
        if(cache.second)
            throw cRuntimeError(this, "Received a second split for source %s, but we already have one", source.str().c_str());
        cache.second = flit;
    }

    // Check if we have both splits and can merge
    if(cache.first && cache.second) {
        ASSERT(cache.first->getGidOrFid() == cache.second->getGidOrFid());

        Flit* merge = cache.first;
        merge->setMode(MODE_DATA);

        send(merge, "out");

        delete cache.second;
        splitCache.erase(source);
    }
}

}} //namespace
