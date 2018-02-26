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

#include "EncoderImpl.h"
#include <Messages/Flit.h>
#include <sstream>
#include <utility>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace NetworkCoding {

Define_Module(EncoderImpl);

EncoderImpl::EncoderImpl()
	: gidCounter(0)
{
}

EncoderImpl::~EncoderImpl() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
	    for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
	        delete *jt;
}

void EncoderImpl::initialize() {
    NetworkCodingBase::initialize();
}

void EncoderImpl::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	// Get parameters
	Mode mode = static_cast<Mode>(flit->getMode());

	if(mode == MODE_DATA) {
		// Insert flit into cache (indexed by target address)
		Address2D target = flit->getTarget();
		FlitVector& generation = flitCache[target];
		generation.push_back(flit);

		EV_DEBUG << "Caching flit " << flit->getName() << " for encoding (destination: " << flit->getTarget() << ")" << std::endl;
		EV_DEBUG << "We now have " << generation.size() << "flit" << (generation.size() != 1 ? "s" : "") << " cached for " << flit->getTarget() << std::endl;

		// Check if we have enough flits to create a generation
		if(generation.size() == static_cast<size_t>(generationSize)) {
			// TODO: do actual network coding

			// Logging
		    EV << "Creating new generation (ID: " << gidCounter << ", " << flit->getSource() << "->" << flit->getTarget() << ") from flit IDs ";
		    for(int i = 0; i < generationSize - 1; ++i)
		        EV << generation[i]->getGidOrFid() << "+";
		    EV << generation[generationSize-1]->getGidOrFid() << std::endl;

		    // right now we just copy the first flit a few times because
            // there is no payload yet
			for(int i = 0; i < numCombinations; ++i) {
				Flit* combination = generation[0]->dup();

                // Set original IDs vector
                combination->setOriginalIdsArraySize(generationSize);
                for(int j = 0; j < generationSize; ++j) {
                    combination->setOriginalIds(j, generation[j]->getGidOrFid());
                }

				// Set network coding metadata
				combination->setGidOrFid(gidCounter);
				combination->setGev(static_cast<uint16_t>(i));

				if(generationSize == 2 && numCombinations == 3)
					combination->setNcMode(NC_G2C3);
				else if(generationSize == 2 && numCombinations == 4)
					combination->setNcMode(NC_G2C4);
				else
					throw cRuntimeError(this, "Cannot set NC mode to NC_G%uC%u", generationSize, numCombinations);

				// Set name
	            std::ostringstream packetName;
	            packetName << "nc-" << gidCounter << "-" << i << "-s" << combination->getSource().str()
	                       << "-t" << combination->getTarget().str();
	            combination->setName(packetName.str().c_str());

				// Send the encoded flit
				send(combination, "out");
			}

			// Clean up uncoded flits
			for(auto it = generation.begin(); it != generation.end(); ++it)
			    delete *it;
			generation.clear();

			// Increment GID counter
			++gidCounter;
		}
	}
	else if(mode == MODE_MAC) {
		flit->setGidOrFid(gidCounter - 1); // -1 because the counter contains the ID of the next generation
		send(flit, "out");
	}
	else if(mode == MODE_SPLIT_1) {
		// TODO: network coding for half-flits
		send(flit, "out");
	}
	else {
		send(flit, "out");
	}
}

}}} //namespace
