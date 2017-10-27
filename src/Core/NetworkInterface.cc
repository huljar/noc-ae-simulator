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

#include "NetworkInterface.h"

namespace HaecComm {

Define_Module(NetworkInterface);

NetworkInterface::NetworkInterface()
	: appSendQueue(nullptr)
	, appReceiveQueue(nullptr)
	, routerSendQueue(nullptr)
	, routerReceiveQueue(nullptr)
{
}

NetworkInterface::~NetworkInterface() {
	delete appSendQueue;
	delete appReceiveQueue;
	delete routerSendQueue;
	delete routerReceiveQueue;
}

void NetworkInterface::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	// Check that this is the clock signal
	if(signalID == registerSignal("clock")) {
		// Only handle the signal if this is a clocked class
		if(isClocked) {
			// If there is a packet in the app send queue, send it to the app
			processQueue(appSendQueue, "appOut");
			// If there is a packet in the app receive queue, pass it to the middleware (app->router pipeline)
			processQueue(appReceiveQueue, "mwEntry", 0);
			// If there is a packet in the router send queue, send it to the router
			processQueue(routerSendQueue, "routerOut");
			// If there is a packet in the router receive queue, pass it to the middleware (router->app pipeline)
			processQueue(routerReceiveQueue, "mwEntry", 1);
		}
		else
			EV_WARN << "Received a clock signal in an unclocked module. Discarding signal." << std::endl;
	}
}

void NetworkInterface::initialize() {
    HaecModule::initialize();

    appSendQueue = new cPacketQueue;
    appReceiveQueue = new cPacketQueue;
    routerSendQueue = new cPacketQueue;
    routerReceiveQueue = new cPacketQueue;
}

void NetworkInterface::handleMessage(cMessage *msg) {
	// Confirm that this is a packet
	if(!msg->isPacket()) {
		EV_WARN << "Received a message that is not a packet. Discarding it." << std::endl;
		delete msg;
		return;
	}

	cPacket* packet = static_cast<cPacket*>(msg); // No need for dynamic_cast or check_and_cast here

	if(packet->getArrivalGate() == gate("mwExit", 0)) {
		// Handle packet arriving from middleware (app->router pipeline)
		// If we are clocked, insert into router send queue, else send immediately
		if(isClocked) routerSendQueue->insert(packet);
		else send(packet, "routerOut");
	}
	else if(packet->getArrivalGate() == gate("mwExit", 1)) {
		// Handle packet arriving from middleware (router->app pipeline)
		if(isClocked) appSendQueue->insert(packet);
		else send(packet, "appOut");
	}
	else if(packet->getArrivalGate() == gate("routerIn")) {
		// Handle packet arriving from network
		if(isClocked) routerReceiveQueue->insert(packet);
		else send(packet, "mwEntry", 1);
	}
	else if(packet->getArrivalGate() == gate("appIn")) {
		// Handle packet arriving from app
		if(isClocked) appReceiveQueue->insert(packet);
		else send(packet, "mwEntry", 0);
	}
	else {
		EV_WARN << "Received a packet from unexcepted gate \"" << packet->getArrivalGate()->getFullName() << "\"" << std::endl;
	}
}

} //namespace
