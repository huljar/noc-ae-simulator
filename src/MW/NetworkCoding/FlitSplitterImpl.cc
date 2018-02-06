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

#include "FlitSplitterImpl.h"
#include <Messages/Flit_m.h>
#include <Messages/FlitLarge_m.h>
#include <Messages/FlitSmall_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace NetworkCoding {

Define_Module(FlitSplitterImpl);

void FlitSplitterImpl::initialize() {
    MiddlewareBase::initialize();
}

void FlitSplitterImpl::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	if(flit->getMode() == MODE_DATA) {
		flit->setMode(MODE_DATA_MAC);
		Flit* split = flit->dup();
		// TODO: set a sensible ID for the new flit (use omnet internal ID again?)

		if(FlitLarge* firstFlit = dynamic_cast<FlitLarge*>(flit)) {
			ASSERT(firstFlit->getPayloadArraySize() == 16);
			FlitLarge* secondFlit = static_cast<FlitLarge*>(split);

			// Split into two 64bit data flits
			for(unsigned int i = 0; i < 8; ++i) {
				secondFlit->setPayload(i, firstFlit->getPayload(i + 8));
				firstFlit->setPayload(i + 8, 0);
				secondFlit->setPayload(i + 8, 0);
			}

		}
		else if(FlitSmall* firstFlit = dynamic_cast<FlitSmall*>(flit)) {
			ASSERT(firstFlit->getPayloadArraySize() == 8);
			FlitSmall* secondFlit = static_cast<FlitSmall*>(split);

			// Split into two 32bit data flits
			for(unsigned int i = 0; i < 4; ++i) {
				secondFlit->setPayload(i, firstFlit->getPayload(i + 4));
				firstFlit->setPayload(i + 4, 0);
				secondFlit->setPayload(i + 4, 0);
			}
		}
		else {
			throw cRuntimeError(this, "Received an unknown subclass of Flit");
		}
	}
	else {
		EV_WARN << "Received an unexpected type of flit. Mode: " << flit->getMode() << ". Discarding it." << std::endl;
		delete flit;
	}
}

}}} //namespace
