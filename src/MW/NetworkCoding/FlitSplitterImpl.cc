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
#include <Messages/Flit.h>

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
		Flit* split = flit->dup();
        flit->setMode(MODE_SPLIT_1);
        split->setMode(MODE_SPLIT_2);
		// TODO: set a sensible ID for the new flit (use omnet internal ID again?)

        // Split payload
        unsigned int payloadHalfSize = flit->getPayloadArraySize() / 2;
        for(unsigned int i = 0; i < payloadHalfSize; ++i) {
            split->setPayload(i, flit->getPayload(payloadHalfSize + i));
            flit->setPayload(payloadHalfSize + i, 0);
            split->setPayload(payloadHalfSize + i, 0);
        }
	}
	else {
		EV_WARN << "Received an unexpected type of flit. Mode: " << flit->getMode() << ". Discarding it." << std::endl;
		delete flit;
	}
}

}}} //namespace
