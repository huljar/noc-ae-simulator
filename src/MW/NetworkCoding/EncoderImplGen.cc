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

#include "EncoderImplGen.h"
#include <Util/Constants.h>

#include <Messages/Flit.h>
#include <sstream>
#include <utility>

using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace MW { namespace NetworkCoding {

Define_Module(EncoderImplGen);

EncoderImplGen::EncoderImplGen() {
}

EncoderImplGen::~EncoderImplGen() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
	    for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
	        delete *jt;
}

void EncoderImplGen::initialize() {
    NetworkCodingBase::initialize();
}

void EncoderImplGen::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	// Get parameters
    const Address2D& source = flit->getSource();
    const Address2D& target = flit->getTarget();
    Mode mode = static_cast<Mode>(flit->getMode());
    NcMode ncMode = static_cast<NcMode>(flit->getNcMode());
    FlitVector& generation = flitCache[target];

    ASSERT(mode == MODE_DATA || mode == MODE_MAC);
    ASSERT(ncMode == NC_UNCODED);

    if(mode == MODE_DATA) {
        // Insert flit into cache (indexed by target address)
        generation.push_back(flit);

        EV_DEBUG << "Caching flit " << flit->getName() << " for encoding (destination: " << target << ")" << std::endl;
        EV_DEBUG << "We now have " << generation.size() << " flit" << (generation.size() != 1 ? "s" : "") << " cached for " << target << std::endl;

        // Check if we have enough flits to create a generation
        if(generation.size() == static_cast<size_t>(generationSize)) {
            lastUsedGids[target].push(encodeAndSendGeneration(generation, source, target));

            // Clean up uncoded flits
            for(auto it = generation.begin(); it != generation.end(); ++it)
                delete *it;
            generation.clear();
        }
    }
    else if(mode == MODE_MAC) {
        // Assign GID of the oldest generation sent to this target that does not yet have a MAC
        std::queue<uint32_t>& gidQueue = lastUsedGids.at(target);
        ASSERT(!gidQueue.empty());

        uint32_t gid = gidQueue.front();
        gidQueue.pop();

        EV_DEBUG << "Assigning GID " << gid << " to the MAC for target " << target << std::endl;

        flit->setGidOrFid(gid);
        flit->setGev(Constants::GEN_MAC_GEV);

        if(generationSize == 2 && numCombinations == 3)
            flit->setNcMode(NC_G2C3);
        else if(generationSize == 2 && numCombinations == 4)
            flit->setNcMode(NC_G2C4);
        else
            throw cRuntimeError(this, "Cannot set NC mode to NC_G%uC%u", generationSize, numCombinations);

        send(flit, "out");
    }
}

}}} //namespace
