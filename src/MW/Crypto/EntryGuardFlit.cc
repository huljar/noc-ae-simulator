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

#include "EntryGuardFlit.h"
#include <Messages/Flit.h>

using namespace HaecComm::Buffers;
using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(EntryGuardFlit);

void EntryGuardFlit::initialize() {
    // Subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

	size_t busyCyclesEnc = static_cast<size_t>(par("busyCyclesEnc"));
	if(busyCyclesEnc < 1)
		throw cRuntimeError(this, "busyCyclesEnc must be greater than 0");

    size_t busyCyclesAuth = static_cast<size_t>(par("busyCyclesAuth"));
    if(busyCyclesAuth < 1)
        throw cRuntimeError(this, "busyCyclesAuth must be greater than 0");

    for(int i = 0; i < gateSize("encOut"); ++i)
        availableEncUnits.push(i);
    for(int i = 0; i < gateSize("authOut"); ++i)
	    availableAuthUnits.push(i);

	busyEncUnits = ShiftRegister<std::vector<int>>(busyCyclesEnc);
	busyAuthUnits = ShiftRegister<std::vector<int>>(busyCyclesAuth);

	// Retrieve pointers to the input queues
    cGate* appInGate = gate("appIn");
    if(!appInGate->isPathOK())
        throw cRuntimeError(this, "App input gate of the entry guard is not properly connected");

    cGate* netInGate = gate("netIn");
    if(!netInGate->isPathOK())
        throw cRuntimeError(this, "Net input gate of the entry guard is not properly connected");

    appInputQueue = check_and_cast<PacketQueueBase*>(appInGate->getPathStartGate()->getOwnerModule());
    netInputQueue = check_and_cast<PacketQueueBase*>(netInGate->getPathStartGate()->getOwnerModule());
}

void EntryGuardFlit::handleMessage(cMessage* msg) {
    if(availableEncUnits.empty() || availableAuthUnits.empty()) {
    	EV_WARN << "Received a message, but all encryption or authentication units are busy. Discarding it." << std::endl;
    	delete msg;
    	return;
    }

    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    int encIdx = availableEncUnits.front();
    int authIdx = availableAuthUnits.front();

    // TODO: flit names
    send(flit->dup(), "authOut", authIdx);
    send(flit, "encOut", encIdx);

    availableEncUnits.pop();
    busyEncUnits.back().push_back(encIdx);

    availableAuthUnits.pop();
    busyAuthUnits.back().push_back(authIdx);
}

void EntryGuardFlit::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		std::vector<int> finishedEncUnits = busyEncUnits.shift();
		for(auto it = finishedEncUnits.begin(); it != finishedEncUnits.end(); ++it)
			availableEncUnits.push(*it);

		std::vector<int> finishedAuthUnits = busyAuthUnits.shift();
        for(auto it = finishedAuthUnits.begin(); it != finishedAuthUnits.end(); ++it)
            availableAuthUnits.push(*it);

        if(availableEncUnits.size() >= 2 && availableAuthUnits.size() >= 2) {
            appInputQueue->requestPacket();
            netInputQueue->requestPacket();
        }
        else if(!availableEncUnits.empty() && !availableAuthUnits.empty()) {
	        // TODO: priorities of input queues (round robin?)
            if(netInputQueue->peek()) netInputQueue->requestPacket();
            else appInputQueue->requestPacket();
	    }
	}
}

}}} //namespace
