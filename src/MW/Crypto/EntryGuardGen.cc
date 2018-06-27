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

#include "EntryGuardGen.h"
#include <Messages/Flit.h>
#include <sstream>

using namespace HaecComm::Buffers;
using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(EntryGuardGen);

void EntryGuardGen::initialize() {
    // Subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

	busyCyclesEnc = par("busyCyclesEnc");
	if(busyCyclesEnc < 1)
		throw cRuntimeError(this, "busyCyclesEnc must be greater than 0");

    busyCyclesAuth = par("busyCyclesAuth");
    if(busyCyclesAuth < 1)
        throw cRuntimeError(this, "busyCyclesAuth must be greater than 0");

    generationSize = getAncestorPar("generationSize");
    if(generationSize < 1)
        throw cRuntimeError(this, "Generation size must be greater than 0, but received %i", generationSize);

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

void EntryGuardGen::handleMessage(cMessage* msg) {
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

        // Check if there is already an authentication unit serving this target
        Messages::Address2D target = flit->getTarget();
        AuthUnitGensMap::iterator unitIter = authUnitGensDeparting.find(target);
        int authIdx;

        if(unitIter == authUnitGensDeparting.end()) {
            // Ensure that a new unit is available
            if(availableAuthUnits.empty()) {
                EV_WARN << "Received a message, but all authentication units are busy. Discarding it." << std::endl;
                delete flit;
                return;
            }

            authIdx = availableAuthUnits.top();
            availableAuthUnits.pop();

            // Register this unit for the target
            authUnitGensDeparting.emplace(target, std::make_pair(authIdx, 1));

            // Emit busy signal (unit is considered busy while waiting for more flits)
            emit(authBusySignals.at(authIdx), true);
        }
        else {
            // Keep using this unit
            authIdx = unitIter->second.first;

            // Increment the number of flits this unit has received
            ++unitIter->second.second;

            // Check if the complete generation has arrived at the unit
            if(unitIter->second.second == generationSize) {
                // Adjust unit status (now the unit is actually busy)
                authUnitGensDeparting.erase(unitIter);
                busyAuthUnits.back().push_back(authIdx);
            }
        }

        // Set name
        std::ostringstream flitName;
        flitName << flit->getName() << "-mac";
        flit->setName(flitName.str().c_str());

        // Send flit
        send(flit, "authOut", authIdx);
    }
    else { // arrivalGate == "netIn"
        // Flit arrived from the network, send to both decryption and verification
        // Ensure that a decryption unit are available
        if(availableEncUnits.empty()) {
            EV_WARN << "Received a message, but all decryption units are busy. Discarding it." << std::endl;
            delete flit;
            return;
        }
        int encIdx = availableEncUnits.top();

        // Check if there is already an authentication unit serving this source
        Messages::Address2D source = flit->getSource();
        AuthUnitGensMap::iterator unitIter = authUnitGensArriving.find(source);
        int authIdx;

        if(unitIter == authUnitGensArriving.end()) {
            // Ensure that a new unit is available
            if(availableAuthUnits.empty()) {
                EV_WARN << "Received a message, but all authentication units are busy. Discarding it." << std::endl;
                delete flit;
                return;
            }

            authIdx = availableAuthUnits.top();
            availableAuthUnits.pop();

            // Register this unit for the source
            authUnitGensArriving.emplace(source, std::make_pair(authIdx, 1));

            // Emit busy signal (unit is considered busy while waiting for more flits)
            emit(authBusySignals.at(authIdx), true);
        }
        else {
            // Keep using this unit
            authIdx = unitIter->second.first;

            // Increment the number of flits this unit has received
            ++unitIter->second.second;

            // Check if the complete generation has arrived at the unit
            if(unitIter->second.second == generationSize) {
                // Adjust unit status (now the unit is actually busy)
                authUnitGensArriving.erase(unitIter);
                busyAuthUnits.back().push_back(authIdx);
            }
        }

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

        // Adjust decryption unit status
        availableEncUnits.pop();
        busyEncUnits.back().push_back(encIdx);

        // Emit decryption unit busy signal
        emit(encBusySignals.at(encIdx), true);
    }
}

void EntryGuardGen::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
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
        Flit* exitInputFlit = check_and_cast_nullable<Flit*>(exitInputQueue->peek());
        if(exitInputFlit) {
            if(authUnitGensDeparting.count(exitInputFlit->getTarget())) {
                exitInputQueue->requestPacket();
            }
            else if(availAuth > 0) {
                --availAuth;
                exitInputQueue->requestPacket();
            }
        }

        // Next priority: encrypt a new departing flit
        if(appInputQueue->peek() && availEnc > 0) {
            --availEnc;
            appInputQueue->requestPacket();
        }

        // Next priority: decrypt/verify an arriving flit
        Flit* netInputFlit = check_and_cast_nullable<Flit*>(netInputQueue->peek());
        if(netInputFlit && availEnc > 0) {
            if(authUnitGensArriving.count(netInputFlit->getSource())) {
                --availEnc;
                netInputQueue->requestPacket();
            }
            else if(availAuth > 0) {
                --availEnc;
                --availAuth;
                netInputQueue->requestPacket();
            }
        }
	}
}

}}} //namespace
