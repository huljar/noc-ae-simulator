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

#include "Delay.h"

using namespace HaecComm::Util;

namespace HaecComm { namespace MW {

Define_Module(Delay);

Delay::Delay()
	: waitCycles(0)
{
}

Delay::~Delay() {
	for(auto it = shiftRegister.begin(); it != shiftRegister.end(); ++it)
		delete *it;
}

void Delay::initialize() {
	MiddlewareBase::initialize();

	waitCycles = par("waitCycles");
	if(waitCycles < 0)
		throw cRuntimeError(this, "waitCycles must be 0 or greater, but is %i", waitCycles);

    // subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

    // set up shift register
    shiftRegister = ShiftRegister<cArray*>(waitCycles < 1 ? 1 : waitCycles);
}

void Delay::handleMessage(cMessage* msg) {
	// this is an incoming message subject to be delayed
    // check if waitCycles is 0
    if(waitCycles == 0) {
        // send message back out immediately
        send(msg, "out");
    }
    else {
        // insert the message at the back of the shift register
        shiftRegister.back()->add(msg);
    }
}

void Delay::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		// Create new element to insert at the back of the shift register
		cArray* back = new cArray;
		take(back);

		// Shift the register
		cArray* front = shiftRegister.shift(back);
		if(front) {
			// Go over all messages that were at the front and send them
			for(cArray::Iterator it(*front); !it.end(); ++it) {
				// make the array release ownership of the object and cast it to message
				// no need for dynamic_cast because we only insert messages
				cMessage* msg = static_cast<cMessage*>(front->remove(*it));
				take(msg);
				send(msg, "out");
			}
			delete front;
		}
	}
}



}} //namespace
