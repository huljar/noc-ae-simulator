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

#include "PacketQueue.h"

namespace HaecComm { namespace Clocking {

Define_Module(PacketQueue);

PacketQueue::PacketQueue()
	: cycleFree(true)
	, queue(nullptr)
{
}

PacketQueue::~PacketQueue() {
	delete queue;
}

void PacketQueue::initialize() {
	PacketQueueBase::initialize();

    queue = new cPacketQueue;

    qlenSignal = registerSignal("qlen");
    pktdropSignal = registerSignal("pktdrop");
}

void PacketQueue::handleMessage(cMessage* msg) {
	// Confirm that this is a packet
	if(!msg->isPacket()) {
		EV_WARN << "Received a message that is not a packet. Discarding it." << std::endl;
		delete msg;
		return;
	}

	cPacket* packet = static_cast<cPacket*>(msg); // No need for dynamic_cast or check_and_cast here

	if(isClocked) {
		if(!syncFirstPacket && cycleFree) {
			send(packet, "out");
			cycleFree = false;
		}
		else if(maxLength == 0 || queue->getLength() < maxLength) {
			queue->insert(packet);
		}
		else {
			emit(pktdropSignal, packet);
			EV_WARN << "Received a packet, but max queue length of " << maxLength
					<< " was already reached. Discarding it." << std::endl;
			delete packet;
		}
	}
	else {
		// Send packet through without waiting
		send(packet, "out");
	}
}

void PacketQueue::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		// Emit queue length signal
		emit(qlenSignal, queue->getLength());

		if(queue->isEmpty()) {
			cycleFree = true;
		}
		else {
			cPacket* packet = queue->pop();
			take(packet);
			send(packet, "out");
		}
	}
}

}} //namespace
