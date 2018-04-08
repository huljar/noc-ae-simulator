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

#include "DecoderImplGen.h"

#include <Messages/Flit.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace NetworkCoding {

Define_Module(DecoderImplGen);

DecoderImplGen::DecoderImplGen() {
}

DecoderImplGen::~DecoderImplGen() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete *jt;
}

void DecoderImplGen::initialize() {
    NetworkCodingBase::initialize();
}

void DecoderImplGen::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

    // Get parameters
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());

    ASSERT(mode == MODE_DATA || mode == MODE_MAC);
    ASSERT(ncMode != NC_UNCODED);

    // Get parameters
    uint32_t gid = flit->getGidOrFid();
    const Address2D& source = flit->getSource();
    const Address2D& target = flit->getTarget();
    IdSourceKey key = std::make_pair(gid, source);

    if(mode == MODE_DATA) {
        // Insert flit into cache (indexed by source address and generation ID)
        FlitVector& combinations = flitCache[key];
        combinations.push_back(flit);

        EV_DEBUG << "Caching flit " << flit->getName() << " for decoding (GID: " << gid << ", source: " << source << ")" << std::endl;
        EV_DEBUG << "We now have " << combinations.size() << " flit" << (combinations.size() != 1 ? "s" : "") << " cached for " << gid << "/" << source << std::endl;

        // Check if we have enough combinations to decode the generation
        if(combinations.size() == static_cast<size_t>(generationSize)) {
            decodeAndSendGeneration(combinations, gid, source, target);

            // Clean up encoded flits
            for(auto it = combinations.begin(); it != combinations.end(); ++it)
                delete *it;
            flitCache.erase(key);
        }
    }
    else if(mode == MODE_MAC) {
        EV_DEBUG << "Passing MAC of GID " << gid << " from " << source << " on" << std::endl;
        flit->setNcMode(NC_UNCODED);
        send(flit, "out");
    }
}

}}} //namespace