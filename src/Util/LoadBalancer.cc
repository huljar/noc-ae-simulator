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

namespace HaecComm { namespace Util {

Define_Module(LoadBalancer);

void LoadBalancer::initialize() {
	// subscribe to the busy signals of the connected modules
	for(int i = 0; i < gateSize("out"); ++i) {
		cModule* connectedUnit = gate("out", i)->getNextGate()->getOwnerModule();
		connectedUnit->subscribe("busy", this);
		connectedModules.insert(std::make_pair(connectedUnit, std::make_pair(false, true)));
		moduleQueue.push(connectedUnit);
	}
}

void LoadBalancer::handleMessage(cMessage* msg) {
    // TODO - Generated method body
}

void LoadBalancer::receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) {
	if(signalID == registerSignal("busy")) {
		cModule* sender = check_and_cast<cModule*>(source);
		auto element = connectedModules.find(sender);
		if(element != connectedModules.end()) {
			element->second.first = b;
			if(!b && !element->second.second) {
				moduleQueue.push(sender);
				element->second.second = true;
			}
		}
		else {
			EV_WARN << "Received busy signal from unexpected source: " << sender->getFullName() << std::endl;
		}
	}
}

}} //namespace
