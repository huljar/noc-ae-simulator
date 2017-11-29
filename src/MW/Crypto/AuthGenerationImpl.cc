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

#include "AuthGenerationImpl.h"
#include <Messages/Flit_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(AuthGenerationImpl);

AuthGenerationImpl::AuthGenerationImpl()
	: generationSize(1)
{
}

AuthGenerationImpl::~AuthGenerationImpl() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
		delete it->second;
}

void AuthGenerationImpl::initialize() {
    AuthBase::initialize();

    generationSize = par("generationSize");
    if(generationSize < 1)
    	throw cRuntimeError(this, "Generation size must be greater than 0, but received %i", generationSize);
}

void AuthGenerationImpl::handleMessage(cMessage* msg) {
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
		// Duplicate the first flit (because MAC flit has the same headers)
		Flit* macFlit = static_cast<Flit*>(generation->get(0))->dup();

		// Set mode flag that this is a MAC flit
		macFlit->setMode(MODE_MAC);

		// TODO: do actual MAC computation

		// Send out data flits first, then MAC
		for(int i = 0; i < generationSize; ++i) {
			// Removing flits from the array in ascending order works
			// because cArray doesn't move the following elements down
			Flit* dataFlit = static_cast<Flit*>(generation->remove(i));
			take(dataFlit);
			send(dataFlit, "out");
		}
		send(macFlit, "out");

		// Clean up flit cache
		delete generation;
		flitCache.erase(target);
	}
}

}}} //namespace
