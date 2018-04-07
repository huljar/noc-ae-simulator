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

#include "EntryGuardSplit.h"
#include <Messages/Flit.h>
#include <sstream>

using namespace HaecComm::Buffers;
using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(EntryGuardSplit);

void EntryGuardSplit::initialize() {
    // Subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

	busyCyclesEnc = par("busyCyclesEnc");
	if(busyCyclesEnc < 1)
		throw cRuntimeError(this, "busyCyclesEnc must be greater than 0");

    busyCyclesAuth = par("busyCyclesAuth");
    if(busyCyclesAuth < 1)
        throw cRuntimeError(this, "busyCyclesAuth must be greater than 0");

    for(int i = 0; i < gateSize("encOut"); ++i)
        availableEncUnits.push(i);
    for(int i = 0; i < gateSize("authOut"); ++i)
	    availableAuthUnits.push(i);

	busyEncUnits = ShiftRegister<std::vector<int>>(static_cast<size_t>(busyCyclesEnc));
	busyAuthUnits = ShiftRegister<std::vector<int>>(static_cast<size_t>(busyCyclesAuth));

	// Retrieve pointers to the input queues
    cGate* appInGate = gate("appIn");
    if(!appInGate->isPathOK())
        throw cRuntimeError(this, "App input gate of the entry guard is not properly connected");

    cGate* exitInGate = gate("exitIn");
    if(!exitInGate->isPathOK())
        throw cRuntimeError(this, "Exit guard input gate of the entry guard is not properly connected");

    cGate* netInGate = gate("netIn");
    if(!netInGate->isPathOK())
        throw cRuntimeError(this, "Net input gate of the entry guard is not properly connected");

    appInputQueue = check_and_cast<PacketQueueBase*>(appInGate->getPathStartGate()->getOwnerModule());
    exitInputQueue = check_and_cast<PacketQueueBase*>(exitInGate->getPathStartGate()->getOwnerModule());
    netInputQueue = check_and_cast<PacketQueueBase*>(netInGate->getPathStartGate()->getOwnerModule());

    // Register busy signals and corresponding statistic for all enc/auth units
    cProperty* encStatTemplate = getProperties()->get("statisticTemplate", "encBusy");
    cProperty* authStatTemplate = getProperties()->get("statisticTemplate", "authBusy");

    for(int i = 0; i < gateSize("encOut"); ++i) {
        // Build signal name
        std::ostringstream signalStatName;
        signalStatName << "enc" << i << "Busy";
        simsignal_t signal = registerSignal(signalStatName.str().c_str());
        encBusySignals.emplace(i, signal);

        getEnvir()->addResultRecorders(this, signal, signalStatName.str().c_str(), encStatTemplate);
    }

    for(int i = 0; i < gateSize("authOut"); ++i) {
        // Build signal name
        std::ostringstream signalStatName;
        signalStatName << "auth" << i << "Busy";
        simsignal_t signal = registerSignal(signalStatName.str().c_str());
        authBusySignals.emplace(i, signal);

        getEnvir()->addResultRecorders(this, signal, signalStatName.str().c_str(), authStatTemplate);
    }
}

void EntryGuardSplit::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    ASSERT(flit->getStatus() == STATUS_NONE);

    // Perform actions depending on arrival gate
    if(strcmp(flit->getArrivalGate()->getName(), "appIn") == 0) {
        // Flit arrived from the PE, send to encryption
        flit->setStatus(STATUS_ENCRYPTING);

        // Ensure that a unit is available
        if(availableEncUnits.empty()) {
            EV_WARN << "Received a message, but all encryption units are busy. Discarding it." << std::endl;
            delete flit;
            return;
        }
        int encIdx = availableEncUnits.top();

        // Set name
        std::ostringstream flitName;
        flitName << flit->getName() << "-enc";
        flit->setName(flitName.str().c_str());

        // Send flit
        send(flit, "encOut", encIdx);

        // Adjust unit status
        availableEncUnits.pop();
        busyEncUnits.back().push_back(encIdx);

        // Emit busy signal
        emit(encBusySignals.at(encIdx), true);
    }
    else if(strcmp(flit->getArrivalGate()->getName(), "exitIn") == 0) {
        // Flit arrived from the exit guard (encrypted), send to authentication
        flit->setStatus(STATUS_AUTHENTICATING);

        // Ensure that a unit is available
        if(availableAuthUnits.empty()) {
            EV_WARN << "Received a message, but all authentication units are busy. Discarding it." << std::endl;
            delete flit;
            return;
        }
        int authIdx = availableAuthUnits.top();

        // Set name
        // TODO: better names
        std::ostringstream flitName;
        flitName << flit->getName() << "-mac";
        flit->setName(flitName.str().c_str());

        // Send flit
        send(flit, "authOut", authIdx);

        // Adjust unit status
        availableAuthUnits.pop();
        busyAuthUnits.back().push_back(authIdx);

        // Emit busy signal
        emit(authBusySignals.at(authIdx), true);
    }
    else { // arrivalGate == "netIn"
        // Flit arrived from the network, send to both decryption and verification
        // Ensure that units are available
        if(availableEncUnits.empty() || availableAuthUnits.empty()) {
            EV_WARN << "Received a message, but either all encryption or all authentication units are busy. Discarding it." << std::endl;
            delete flit;
            return;
        }
        int encIdx = availableEncUnits.top();
        int authIdx = availableAuthUnits.top();

        // Create verification flit
        Flit* mac = flit->dup();

        // Set status
        flit->setStatus(STATUS_DECRYPTING);
        mac->setStatus(STATUS_VERIFYING);

        // Set names
        std::ostringstream dataName;
        dataName << flit->getName() << "-dec";
        flit->setName(dataName.str().c_str());

        std::ostringstream macName;
        macName << mac->getName() << "-ver";
        mac->setName(macName.str().c_str());

        // Send flits
        send(flit, "encOut", encIdx);
        send(mac, "authOut", authIdx);

        // Adjust unit status
        availableEncUnits.pop();
        busyEncUnits.back().push_back(encIdx);
        availableAuthUnits.pop();
        busyAuthUnits.back().push_back(authIdx);

        // Emit busy signals
        emit(encBusySignals.at(encIdx), true);
        emit(authBusySignals.at(authIdx), true);
    }
}

void EntryGuardSplit::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
	    // Shift busy units and emit free signals for finished units
		std::vector<int> finishedEncUnits = busyEncUnits.shift();
		for(auto it = finishedEncUnits.begin(); it != finishedEncUnits.end(); ++it) {
			availableEncUnits.push(*it);
			emit(encBusySignals.at(*it), false);
		}

		std::vector<int> finishedAuthUnits = busyAuthUnits.shift();
        for(auto it = finishedAuthUnits.begin(); it != finishedAuthUnits.end(); ++it) {
            availableAuthUnits.push(*it);
            emit(authBusySignals.at(*it), false);
        }

        // Count the free units affected by packet requests
        size_t availEnc = availableEncUnits.size();
        size_t availAuth = availableAuthUnits.size();

        // Highest priority: authenticate an already encrypted flit
        if(exitInputQueue->peek() && availAuth > 0) {
            --availAuth;
            exitInputQueue->requestPacket();
        }

        // Next priority: decrypt/verify an arriving flit
        if(netInputQueue->peek() && availEnc > 0 && availAuth > 0) {
            --availEnc;
            --availAuth;
            netInputQueue->requestPacket();
        }

        // Next priority: encrypt a new departing flit
        if(appInputQueue->peek() && availEnc > 0) {
            --availEnc;
            appInputQueue->requestPacket();
	    }
	}
}

}}} //namespace
