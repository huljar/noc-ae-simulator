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

#include "App.h"

namespace HaecComm {

Define_Module(App);

App::App()
	: sendQueue(nullptr)
	, receiveQueue(nullptr)
{
}

App::~App() {
	delete sendQueue;
	delete receiveQueue;
}

void App::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    // Check that this is the clock signal
    if(signalID == registerSignal("clock")) {
        // Only handle the signal if this is a clocked class
        if(isClocked) {
        	// If there is a packet in the send queue, send it to the network
        	processQueue(sendQueue, "networkOut");
        	// If there is a packet in the receive queue, pass it to the middleware (receiver pipeline)
        	processQueue(receiveQueue, "mwEntry", 1);
        }
        else
            EV_WARN << "Received a clock signal in an unclocked module. Discarding signal." << std::endl;
    }
}

void App::initialize() {
    HaecModule::initialize();

    sendQueue = new cPacketQueue;
    receiveQueue = new cPacketQueue;
}

void App::handleMessage(cMessage *msg) {
	// Confirm that this is a packet
	if(!msg->isPacket()) {
		EV_WARN << "Received a message that is not a packet. Discarding it." << std::endl;
		delete msg;
		return;
	}

	cPacket* packet = static_cast<cPacket*>(msg); // No need for dynamic_cast or check_and_cast here

	if(packet->getArrivalGate() == gate("mwExit", 0)) {
		// Handle packet arriving from middleware (sender pipeline)
		// If we are clocked, insert into sender queue, else send immediately
		if(isClocked)
			sendQueue->insert(packet);
		else
			send(packet, "networkOut");
	}
	else if(packet->getArrivalGate() == gate("networkIn")) {
		// Handle packet arriving from network
		// If we are clocked, insert this into receiver queue, else send to middleware (receiver pipeline)
		if(isClocked)
			receiveQueue->insert(packet);
		else
			send(packet, "mwEntry", 1);
	}
	else {
		EV_WARN << "Received a packet from unexcepted gate \"" << packet->getArrivalGate()->getFullName() << "\"" << std::endl;
	}
}

} //namespace
