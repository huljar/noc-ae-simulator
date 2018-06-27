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
#include <sstream>

using namespace HaecComm::Buffers;
using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(EntryGuardFlit);

void EntryGuardFlit::initialize() {
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

    cGate* decInGate = gate("decIn");
    if(!decInGate->isPathOK())
        throw cRuntimeError(this, "Dec input gate of the entry guard is not properly connected");

    cGate* verInGate = gate("verIn");
    if(!verInGate->isPathOK())
        throw cRuntimeError(this, "Ver input gate of the entry guard is not properly connected");

    appInputQueue = check_and_cast<PacketQueueBase*>(appInGate->getPathStartGate()->getOwnerModule());
    exitInputQueue = check_and_cast<PacketQueueBase*>(exitInGate->getPathStartGate()->getOwnerModule());
    decInputQueue = check_and_cast<PacketQueueBase*>(decInGate->getPathStartGate()->getOwnerModule());
    verInputQueue = check_and_cast<PacketQueueBase*>(verInGate->getPathStartGate()->getOwnerModule());

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

void EntryGuardFlit::handleMessage(cMessage* msg) {
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
    else if(strcmp(flit->getArrivalGate()->getName(), "decIn") == 0) {
        // Flit arrived from the decoder, send to decryption
        flit->setStatus(STATUS_DECRYPTING);

        // Ensure that a unit is available
        if(availableEncUnits.empty()) {
            EV_WARN << "Received a message, but all decryption units are busy. Discarding it." << std::endl;
            delete flit;
            return;
        }
        int encIdx = availableEncUnits.top();

        // Set name
        std::ostringstream flitName;
        flitName << flit->getName() << "-dec";
        flit->setName(flitName.str().c_str());

        // Send flit
        send(flit, "encOut", encIdx);

        // Adjust unit status
        availableEncUnits.pop();
        busyEncUnits.back().push_back(encIdx);

        // Emit busy signal
        emit(encBusySignals.at(encIdx), true);
    }
    else { // arrivalGate == "verIn"
        // Flit arrived from the network, send to verification
        flit->setStatus(STATUS_VERIFYING);

        // Ensure that a unit is available
        if(availableAuthUnits.empty()) {
            EV_WARN << "Received a message, but all verification units are busy. Discarding it." << std::endl;
            delete flit;
            return;
        }
        int authIdx = availableAuthUnits.top();

        // Set name
        std::ostringstream flitName;
        flitName << flit->getName() << "-ver";
        flit->setName(flitName.str().c_str());

        // Send flit
        send(flit, "authOut", authIdx);

        // Adjust unit status
        availableAuthUnits.pop();
        busyAuthUnits.back().push_back(authIdx);

        // Emit busy signal
        emit(authBusySignals.at(authIdx), true);
    }
}

void EntryGuardFlit::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
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

        // Highest authentication priority: authenticate an already encrypted flit
        if(exitInputQueue->peek() && availAuth > 0) {
            --availAuth;
            exitInputQueue->requestPacket();
        }

        // Next authentication priority: verify an arriving flit from the network
        if(verInputQueue->peek() && availAuth > 0) {
            --availAuth;
            verInputQueue->requestPacket();
        }

        // Highest encryption priority: encrypt a new departing flit
        if(appInputQueue->peek() && availEnc > 0) {
            --availEnc;
            appInputQueue->requestPacket();
        }

        // Next priority: decrypt an arriving (decoded) flit from the network
        if(decInputQueue->peek() && availEnc > 0) {
            --availEnc;
            decInputQueue->requestPacket();
        }
	}
}

}}} //namespace
