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

#include "EncoderImplFlit.h"

#include <Messages/Flit.h>
#include <sstream>
#include <utility>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace NetworkCoding {

Define_Module(EncoderImplFlit);

EncoderImplFlit::EncoderImplFlit() {
}

EncoderImplFlit::~EncoderImplFlit() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
	    for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
	        delete *jt;
}

void EncoderImplFlit::initialize() {
    NetworkCodingBase::initialize();
}

void EncoderImplFlit::handleMessage(cMessage* msg) {
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

    ASSERT(mode == MODE_DATA);
    ASSERT(ncMode == NC_UNCODED);

    // Insert flit into cache (indexed by target address)
    generation.push_back(flit);

    EV_DEBUG << "Caching flit " << flit->getName() << " for encoding (destination: " << target << ")" << std::endl;
    EV_DEBUG << "We now have " << generation.size() << " flit" << (generation.size() != 1 ? "s" : "") << " cached for " << target << std::endl;

    // Check if we have enough flits to create a generation
    if(generation.size() == static_cast<size_t>(generationSize)) {
        encodeAndSendGeneration(generation, source, target);

        // Clean up uncoded flits
        for(auto it = generation.begin(); it != generation.end(); ++it)
            delete *it;
        generation.clear();
    }
}

}}} //namespace
