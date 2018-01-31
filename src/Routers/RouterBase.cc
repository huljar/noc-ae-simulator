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

#include "RouterBase.h"
#include <Messages/Flit_m.h>

using namespace HaecComm::Clocking;
using namespace HaecComm::Messages;

namespace HaecComm { namespace Routers {

RouterBase::RouterBase()
	: gridColumns(1)
	, nodeId(0)
{
}

RouterBase::~RouterBase() {
}

void RouterBase::initialize() {
	gridColumns = getAncestorPar("columns");
	nodeId = getAncestorPar("id");

	nodeX = nodeId % gridColumns;
	nodeY = nodeId / gridColumns;

	pktsendSignal = registerSignal("pktsend");
	pktreceiveSignal = registerSignal("pktreceive");
	pktrouteSignal = registerSignal("pktroute");

    // subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

	// subscribe to the "queue full" signal of the input queues
	// of the connected routers
	for(int i = 0; i < gateSize("port"); ++i) {
        cGate* outGate = gate("port$o", i);
        if(!outGate->isPathOK())
            throw cRuntimeError(this, "Output gate of the router is not properly connected");

        cModule* connectedModule = outGate->getPathEndGate()->getOwnerModule();
        connectedModule->subscribe("qfull", this);

        EV_DEBUG << "Subscribed to \"" << connectedModule->getFullPath() << "\"'s qfull signal." << std::endl;

        modulePortMap.emplace(connectedModule->getId(), i);
        portReadyMap.emplace(i, true);

        // Retrieve pointers to the input queues
        // This is necessary so we can request packets from a specific queue
        cGate* inGate = gate("port$i", i);
        if(!inGate->isPathOK())
        	throw cRuntimeError(this, "Input gate of the router is not properly connected");

        PacketQueueBase* inputQueue = check_and_cast<PacketQueueBase*>(inGate->getPathStartGate()->getOwnerModule());
        portQueueMap.emplace(i, inputQueue);
	}
}

void RouterBase::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	// Get node information
	int targetX = flit->getTarget().x();
	int targetY = flit->getTarget().y();

	bool isSender = strcmp(flit->getArrivalGate()->getName(), "local$i") == 0;
	bool isReceiver = targetX == nodeX && targetY == nodeY;

	// Emit signals
	if(isSender && !isReceiver)
		emit(pktsendSignal, flit);
	if(isReceiver && !isSender)
		emit(pktreceiveSignal, flit);
	if(!isSender && !isReceiver)
		emit(pktrouteSignal, flit);

	// Increase hop count if we are not the sender node
	if(!isSender) {
		flit->setHopCount(flit->getHopCount() + 1);
	}

	EV << "Routing flit: " << flit->getSource().str() << " -> " << flit->getTarget().str() << std::endl;

	// Route the flit
	if(targetX != nodeX) {
		// Move in X direction
		send(flit, "port$o", targetX < nodeX ? 3 : 1); // implicit knowledge
	}
	else if(targetY != nodeY) {
		// Move in Y direction
		send(flit, "port$o", targetY < nodeY ? 0 : 2);
	}
	else {
		// This node is the destination
		send(flit, "local$o");
	}
}

void RouterBase::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {

	}
}

void RouterBase::receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) {
    // If we receive a "queue is full" signal, set the ready state for the port that connects to
    // the source module to false. If the queue is not full any more, set ready state to true.
    if(signalID == registerSignal("qfull")) {
        portReadyMap.at(modulePortMap.at(source->getId())) = !b;
    }
}

}} //namespace
