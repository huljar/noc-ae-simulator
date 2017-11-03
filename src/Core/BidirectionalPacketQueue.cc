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

namespace HaecComm {

Define_Module(BidirectionalPacketQueue);

BidirectionalPacketQueue::BidirectionalPacketQueue()
	: maxLength(0)
	, queueLeftToRight(nullptr)
	, queueRightToLeft(nullptr)
{
}

BidirectionalPacketQueue::~BidirectionalPacketQueue() {
	delete queueLeftToRight;
	delete queueRightToLeft;
}

void BidirectionalPacketQueue::initialize() {
    if(getAncestorPar("isClocked")) {
		// subscribe to clock signal
		getSimulation()->getSystemModule()->subscribe("clock", this);
	}
    maxLength = par("maxLength");
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
		if(getAncestorPar("isClocked")) {
			if(queueLeftToRight->getLength() < maxLength) {
				queueLeftToRight->insert(packet);
			}
			else {
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
		if(getAncestorPar("isClocked")) {
			if(queueRightToLeft->getLength() < maxLength) {
				queueRightToLeft->insert(packet);
			}
			else {
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
		if(queueLeftToRight->getLength() > 0) send(queueLeftToRight->pop(), "right$o");
		if(queueRightToLeft->getLength() > 0) send(queueRightToLeft->pop(), "left$o");
	}
	else
		EV_WARN << "Received unexpected signal with ID " << signalID << ", expected clock signal" << std::endl;
}

} //namespace
