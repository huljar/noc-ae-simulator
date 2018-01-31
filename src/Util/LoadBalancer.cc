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

#include "LoadBalancer.h"

using namespace HaecComm::Clocking;

namespace HaecComm { namespace Util {

Define_Module(LoadBalancer);

void LoadBalancer::initialize() {
    // subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

	size_t busyCycles = static_cast<size_t>(par("busyCycles"));
	if(busyCycles < 1)
		throw cRuntimeError(this, "busyCycles must be greater than 0");

	for(int i = 0; i < gateSize("out"); ++i)
		availableUnits.push(i);
	busyUnits = ShiftRegister<std::vector<int>>(busyCycles);

	// Retrieve pointer to the input queue
    cGate* inGate = gate("in");
    if(!inGate->isPathOK())
        throw cRuntimeError(this, "Input gate of the load balancer is not properly connected");

    inputQueue = check_and_cast<PacketQueueBase*>(inGate->getPathStartGate()->getOwnerModule());
}

void LoadBalancer::handleMessage(cMessage* msg) {
    if(availableUnits.empty()) {
    	EV_WARN << "Received a message, but all units are busy. Discarding it." << std::endl;
    	delete msg;
    	return;
    }

    int idx = availableUnits.front();
    send(msg, "out", idx);

    availableUnits.pop();
    busyUnits.back().push_back(idx);
}

void LoadBalancer::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		std::vector<int> finishedUnits = busyUnits.shift();
		for(auto it = finishedUnits.begin(); it != finishedUnits.end(); ++it)
			availableUnits.push(*it);
	}

	if(!availableUnits.empty())
	    inputQueue->requestPacket();
}

}} //namespace
