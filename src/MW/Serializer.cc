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

#include "Serializer.h"

namespace HaecComm { namespace MW {

Define_Module(Serializer);

Serializer::Serializer()
	: cycleFree(true)
{
}

Serializer::~Serializer() {
	while(!queue.empty()) {
		delete queue.front();
		queue.pop();
	}
}

void Serializer::initialize() {
    MiddlewareBase::initialize();

	if(getAncestorPar("isClocked")) {
		// subscribe to clock signal
		getSimulation()->getSystemModule()->subscribe("clock", this);
	}
}

void Serializer::handleMessage(cMessage* msg) {
	if(cycleFree) {
		send(msg, "out");
		cycleFree = false;
	}
	else {
		queue.push(msg);
	}
}

void Serializer::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		if(queue.empty()) {
			cycleFree = true;
		}
		else {
			send(queue.front(), "out");
			queue.pop();
		}
	}
}

}} //namespace
