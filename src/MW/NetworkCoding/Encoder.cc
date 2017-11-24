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

#include "Encoder.h"
#include <Messages/Flit_m.h>
#include <utility>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace NetworkCoding {

Define_Module(Encoder);

Encoder::Encoder()
	: gidCounter(0)
{
}

Encoder::~Encoder() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
		delete it->second;
}

void Encoder::initialize() {
    NetworkCodingBase::initialize();
}

void Encoder::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

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
			combination->setGid(gidCounter);
			combination->setGev(42); // TODO: set to something meaningful when NC is implemented
			send(combination, "out");
		}
		delete generation;
		flitCache.erase(target);
		++gidCounter;
	}
}

}}} //namespace
