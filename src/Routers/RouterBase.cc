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

using namespace HaecComm::Buffers;
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

	// Preparations for the local port (-1) and the grid ports
	for(int i = -1; i < gateSize("port"); ++i) {
	    // Get in/out gates
        cGate* outGate = (i == -1 ? gate("local$o") : gate("port$o", i));
        cGate* inGate = (i == -1 ? gate("local$i") : gate("port$i", i));

        // Check gate connectivity (important for edge/corner nodes)
        // Skip if gate is not connected properly
        if(!(outGate->isPathOK() && inGate->isPathOK()))
            continue;

        // Subscribe to the "queue full" signal of the input queues of the connected routers/components
        cModule* connectedModule = outGate->getPathEndGate()->getOwnerModule();
        connectedModule->subscribe("qfull", this);

        EV_DEBUG << "Subscribed to " << connectedModule->getFullPath() << "'s \"qfull\" signal." << std::endl;

        // Fill the maps
        modulePortMap.emplace(connectedModule->getId(), i);
        portReadyMap.emplace(i, true);

        // Retrieve pointers to the input queues
        // This is necessary so we can request packets from a specific queue
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

	// Get information
	bool isSender = strcmp(flit->getArrivalGate()->getName(), "local$i") == 0;
	int sourcePort = (isSender ? -1 : flit->getArrivalGate()->getIndex());
	bool isReceiver = sourceDestinationCache.at(sourcePort) == -1;

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

	EV_DEBUG << "Routing flit: " << flit->getSource().str() << " -> " << flit->getTarget().str() << std::endl;

	// Route the flit using the cache
	int destPort = sourceDestinationCache.at(sourcePort);
	if(destPort == -1)
	    send(flit, "local$o");
	else
	    send(flit, "port$o", destPort);
}

void RouterBase::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
	    std::map<int, std::vector<int>> applicableInputs;

	    // Iterate over input queues
	    for(auto it = portQueueMap.begin(); it != portQueueMap.end(); ++it) {
	        // Peek at queue
	        cPacket* packet = it->second->peek();
	        // Check if queue was empty
	        if(!packet)
	            continue;

	        // Confirm that this is a flit
	        Flit* flit = dynamic_cast<Flit*>(it->second->peek());
            if(!flit) {
                EV_WARN << "Input queue " << it->first << " has a message that is not a flit. Discarding it." << std::endl;
                it->second->requestDrop();
                continue;
            }

            // Get destination port (implemented by subclasses)
            int destPort = computeDestinationPort(flit);

            // Check if the destination port's receiving queue ready
            if(portReadyMap.at(destPort)) {
                // Insert into map
                applicableInputs[destPort].push_back(it->first);
            }
	    }

	    // Iterate over output ports that have one or more packets to be sent
	    for(auto it = applicableInputs.begin(); it != applicableInputs.end(); ++it) {
	        // Choose a random queue that may send through this port now
	        size_t pIdx = static_cast<size_t>(intrand(static_cast<long>(it->second.size())));

	        // Insert source/destination port pair into cache
	        sourceDestinationCache[it->second[pIdx]] = it->first;

	        // Request a packet from this queue
	        portQueueMap.at(it->second[pIdx])->requestPacket();
	    }
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
