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

#include "AuthGenImpl.h"

#include <Messages/Flit.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(AuthGenImpl);

AuthGenImpl::AuthGenImpl()
	: generationSize(1)
{
}

AuthGenImpl::~AuthGenImpl() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
		delete *it;
}

void AuthGenImpl::initialize() {
    MiddlewareBase::initialize();

    generationSize = getAncestorPar("generationSize");
    if(generationSize < 1)
    	throw cRuntimeError(this, "Generation size must be greater than 0, but received %i", generationSize);
}

void AuthGenImpl::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	// Insert flit into cache
	if(!flitCache.empty())
	    ASSERT(flit->getTarget() == flitCache.front()->getTarget());
	flitCache.push_back(flit);

	if(flitCache.size() == static_cast<size_t>(generationSize)) {
		// Use the first flit as MAC (because MAC flit has the same headers)
		Flit* macFlit = flitCache.front();

		// Set mode flag that this is a MAC flit
		macFlit->setMode(MODE_MAC);

		// Send out MAC flit
		send(macFlit, "out");

		// Clean up flit cache (except first flit, which became the MAC)
		for(auto it = ++flitCache.begin(); it != flitCache.end(); ++it)
		    delete *it;
		flitCache.clear();
	}
}

}}} //namespace
