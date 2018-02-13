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
		delete it->second;
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

	if(flit->getMode() == MODE_DATA) {
		// Insert flit into cache (indexed by target address)
		Address2D target = flit->getTarget();
		cArray*& generation = flitCache[target];
		if(!generation)
			generation = new cArray;
		generation->add(flit);

		if(generation->size() == generationSize) {
			// TODO: do actual network coding

			// right now we just copy the first flit a few times because
			// there is no payload yet
			for(int i = 0; i < numCombinations; ++i) {
				Flit* combination = static_cast<Flit*>(generation->get(0))->dup();

                // Set original IDs vector
                combination->setOriginalIdsArraySize(generationSize);
                for(int j = 0; j < generationSize; ++j) {
                    combination->setOriginalIds(j, static_cast<Flit*>(generation->get(j))->getGidOrFid());
                }

				// Set network coding metadata
				combination->setGidOrFid(gidCounter);
				combination->setGev(static_cast<uint16_t>(i));

				// Set name
	            std::ostringstream packetName;
	            packetName << "nc-" << gidCounter << "-" << i << "-s" << combination->getSource().str()
	                       << "-t" << combination->getTarget().str();
	            combination->setName(packetName.str().c_str());

				// Send the encoded flit
				send(combination, "out");
			}
			delete generation;
			flitCache.erase(target);
			++gidCounter;
		}
	}
	else if(flit->getMode() == MODE_MAC) {
		flit->setGidOrFid(gidCounter - 1); // -1 because the counter contains the ID of the next generation
		send(flit, "out");
	}
	else if(flit->getMode() == MODE_SPLIT_1) {
		// TODO: network coding for half-flits
		send(flit, "out");
	}
	else {
		send(flit, "out");
	}
}

}}} //namespace
