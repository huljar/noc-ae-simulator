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

#include "Router.h"

namespace HaecComm {

Define_Module(Router);

Router::Router()
	: localSendQueue(nullptr)
	, localReceiveQueue(nullptr)
{
}

Router::~Router() {
	delete localSendQueue;
	delete localReceiveQueue;
	for(auto it = portSendQueues.begin(); it != portSendQueues.end(); ++it)
		delete *it;
	for(auto it = portReceiveQueues.begin(); it != portReceiveQueues.end(); ++it)
		delete *it;
}

void Router::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	// Check that this is the clock signal
	if(signalID == registerSignal("clock")) {
		// Only handle the signal if this is a clocked class
		if(isClocked) {
			// If there is a packet in the local send queue, send it to the local node
			processQueue(localSendQueue, "localOut");
			// If there is a packet in the local receive queue, pass it to the middleware (local->network pipeline)
			processQueue(localReceiveQueue, "mwEntry", 0);

			// If there is a packet in any of the port send queues, send them to the network
			for(size_t i = 0; i < portSendQueues.size(); ++i)
				processQueue(portSendQueues[i], "port$o", static_cast<int>(i));

			// If there is a packet in any of the  port receive queues, pass them to the middleware (network->network/local pipeline)
			// TODO: round robin here?
			for(size_t i = 0; i < portReceiveQueues.size(); ++i)
				processQueue(portReceiveQueues[i], "mwEntry", 1);
		}
		else
			EV_WARN << "Received a clock signal in an unclocked module. Discarding signal." << std::endl;
	}
}

void Router::initialize() {
    HaecModule::initialize();

    localSendQueue = new cPacketQueue;
    localReceiveQueue = new cPacketQueue;
    int portCount = gateSize("port");
    for(int i = 0; i < portCount; ++i)
    	portSendQueues.push_back(new cPacketQueue);
}

void Router::handleMessage(cMessage *msg) {
	// Confirm that this is a packet
	if(!msg->isPacket()) {
		EV_WARN << "Received a message that is not a packet. Discarding it." << std::endl;
		delete msg;
		return;
	}

	cPacket* packet = static_cast<cPacket*>(msg); // No need for dynamic_cast or check_and_cast here

	// TODO: check packet tag and extract correct port/queue index
	if(packet->getArrivalGate() == gate("mwExit", 0)) {
		// Handle packet arriving from middleware (local->network pipeline)
		// If we are clocked, insert into port send queue, else send immediately
		if(isClocked) portSendQueues[0]->insert(packet);
		else send(packet, "port$o", 0);
	}
	else if(packet->getArrivalGate() == gate("mwExit", 1)) {
		// Handle packet arriving from middleware (network->network/local pipeline)
		if(isClocked) portSendQueues[0]->insert(packet);
		else send(packet, "port$o", 0);
	}
	else if(packet->getArrivalGate() == gate("routerIn")) {
		// Handle packet arriving from network
		if(isClocked) routerReceiveQueue->insert(packet);
		else send(packet, "mwEntry", 1);
	}
	else if(packet->getArrivalGate() == gate("appIn")) {
		// Handle packet arriving from local node
		if(isClocked) appReceiveQueue->insert(packet);
		else send(packet, "mwEntry", 0);
	}
	else {
		EV_WARN << "Received a packet from unexcepted gate \"" << packet->getArrivalGate()->getFullName() << "\"" << std::endl;
	}
}

} //namespace
