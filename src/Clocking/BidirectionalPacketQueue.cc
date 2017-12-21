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

#include "BidirectionalPacketQueue.h"

namespace HaecComm { namespace Clocking {

Define_Module(BidirectionalPacketQueue);

BidirectionalPacketQueue::BidirectionalPacketQueue()
	: cycleFreeLeftToRight(true)
	, cycleFreeRightToLeft(true)
	, queueLeftToRight(nullptr)
	, queueRightToLeft(nullptr)
{
}

BidirectionalPacketQueue::~BidirectionalPacketQueue() {
	delete queueLeftToRight;
	delete queueRightToLeft;
}

void BidirectionalPacketQueue::initialize() {
    PacketQueueBase::initialize();

    queueLeftToRight = new cPacketQueue;
    queueRightToLeft = new cPacketQueue;

    qlenltrSignal = registerSignal("qlenltr");
    qlenrtlSignal = registerSignal("qlenrtl");
    pktdropltrSignal = registerSignal("pktdropltr");
    pktdroprtlSignal = registerSignal("pktdroprtl");
}

void BidirectionalPacketQueue::handleMessage(cMessage* msg) {
	// Confirm that this is a packet
	if(!msg->isPacket()) {
		EV_WARN << "Received a message that is not a packet. Discarding it." << std::endl;
		delete msg;
		return;
	}

	cPacket* packet = static_cast<cPacket*>(msg); // No need for dynamic_cast or check_and_cast here

	if(strcmp(packet->getArrivalGate()->getName(), "left$i") == 0) {
		if(isClocked) {
			if(!syncFirstPacket && cycleFreeLeftToRight) {
				send(packet, "right$o");
				cycleFreeLeftToRight = false;
			}
			else if(maxLength == 0 || queueLeftToRight->getLength() < maxLength) {
				queueLeftToRight->insert(packet);
			}
			else {
				emit(pktdropltrSignal, packet);
				EV_WARN << "Received a packet on left gate, but max queue length of "
						<< maxLength << " was already reached. Discarding it." << std::endl;
				delete packet;
			}
		}
		else {
			send(packet, "right$o");
		}
	}
	else { // arrival gate: "right$i"
		if(isClocked) {
			if(!syncFirstPacket && cycleFreeRightToLeft) {
				send(packet, "left$o");
				cycleFreeRightToLeft = false;
			}
			else if(maxLength == 0 || queueRightToLeft->getLength() < maxLength) {
				queueRightToLeft->insert(packet);
			}
			else {
				emit(pktdroprtlSignal, packet);
				EV_WARN << "Received a packet on right gate, but max queue length of "
						<< maxLength << " was already reached. Discarding it." << std::endl;
				delete packet;
			}
		}
		else {
			send(packet, "left$o");
		}
	}
}

void BidirectionalPacketQueue::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		// Emit queue length signals
		emit(qlenltrSignal, queueLeftToRight->getLength());
		emit(qlenrtlSignal, queueRightToLeft->getLength());

		if(queueLeftToRight->isEmpty()) {
			cycleFreeLeftToRight = true;
		}
		else {
			cPacket* packet = queueLeftToRight->pop();
			take(packet);
			send(packet, "right$o");
		}

		if(queueRightToLeft->isEmpty()) {
			cycleFreeRightToLeft = true;
		}
		else {
			cPacket* packet = queueRightToLeft->pop();
			take(packet);
			send(packet, "left$o");
		}
	}
}

}} //namespace