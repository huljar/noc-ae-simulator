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

#include "AuthHalfFlitImpl.h"
#include <Messages/Flit_m.h>
#include <Messages/FlitLarge_m.h>
#include <Messages/FlitSmall_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(AuthHalfFlitImpl);

void AuthHalfFlitImpl::initialize() {
	MiddlewareBase::initialize();
}

void AuthHalfFlitImpl::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit1 = dynamic_cast<Flit*>(msg);
	if(!flit1) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	flit1->setMode(MODE_DATA_MAC);
	Flit* flit2 = flit1->dup();

	// TODO: actual MAC computations

	if(FlitLarge* firstFlit = dynamic_cast<FlitLarge*>(flit1)) {
		ASSERT(firstFlit->getPayloadArraySize() == 16);
		FlitLarge* secondFlit = static_cast<FlitLarge*>(flit2);

		// Split into two 64bit data flits, compute 64bit MACs
		for(unsigned int i = 0; i < 8; ++i)
			secondFlit->setPayload(i, firstFlit->getPayload(i + 8));

	}
	else if(FlitSmall* firstFlit = dynamic_cast<FlitSmall*>(flit1)) {
		ASSERT(firstFlit->getPayloadArraySize() == 8);
		FlitSmall* secondFlit = static_cast<FlitSmall*>(flit2);

		// Split into two 32bit data flits, compute 32bit MACs
		for(unsigned int i = 0; i < 4; ++i)
			secondFlit->setPayload(i, firstFlit->getPayload(i + 4));
	}
	else {
		throw cRuntimeError(this, "Received an unknown subclass of Flit");
	}

	// Send out both flits
	send(flit1, "out");
	send(flit2, "out");
}

}}} //namespace
