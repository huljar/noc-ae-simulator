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

#include "MiddlewareBase.h"

namespace HaecComm {

MiddlewareBase::MiddlewareBase()
	: isClocked(false)
	, locallyClocked(false)
	, queueLength(0)
	, incoming(nullptr)
	, parent(nullptr)
{
}

MiddlewareBase::~MiddlewareBase() {
    delete incoming;
}

void MiddlewareBase::initialize() {
	isClocked = getAncestorPar("isClocked");
	if(isClocked) {
		// subscribe to clock signal
		getSimulation()->getSystemModule()->subscribe("clock", this);
	}

    locallyClocked = isClocked ? par("locallyClocked") : false;
    queueLength = par("queueLength");

    incoming = new cPacketQueue;

    parent = dynamic_cast<HaecModule*>(getParentModule());
    if(!parent)
    	throw cRuntimeError(this, "Middleware parent module must be of type HaecModule");
}

void MiddlewareBase::handleMessage(cMessage* msg) {
	// Confirm that this is a packet
	if(!msg->isPacket()) {
		EV_WARN << "Received a message that is not a packet. Discarding it." << std::endl;
		delete msg;
		return;
	}

	cPacket* packet = static_cast<cPacket*>(msg); // No need for dynamic_cast or check_and_cast here

    if (locallyClocked) {
        // enqueue until signal
        if(queueLength && incoming->getLength() >= queueLength) { // queue is size restricted
            dropAndDelete(packet);
            EV << "Received message, but queue is full" << std::endl;
        }
        incoming->insert(packet);
    } else {
        handleMessageInternal(packet);
    }
}

void MiddlewareBase::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        // this is a tick
        if(incoming->isEmpty()) {
            handleCycle(nullptr);
        }
        else {
            handleCycle(incoming->pop());
        }
    }
}

cMessage* MiddlewareBase::createMessage(const char* name) {
    cMessage *msg = new cMessage(name);
    take(msg);
    return msg;
}

} /* namespace HaecComm */
