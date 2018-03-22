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

#include "PacketQueueBase.h"
#include <Messages/Flit.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace Buffers {

PacketQueueBase::PacketQueueBase()
    : awaitSendRequests(false)
	, syncFirstPacket(true)
	, maxLength(0)
    , cycleFree(true)
    , queue(nullptr)
{
}

PacketQueueBase::~PacketQueueBase() {
    delete queue;
}

void PacketQueueBase::requestPacket() {
    if(awaitSendRequests && !queue->isEmpty()) {
    	popQueueAndSend();
    }
}

cPacket* PacketQueueBase::peek() {
    // front() returns nullptr if the queue is empty
    return queue->front();
}

void PacketQueueBase::requestDrop() {
    if(awaitSendRequests && !queue->isEmpty()) {
        popQueueAndDiscard();
    }
}

int PacketQueueBase::getLength() {
    return queue->getLength();
}

void PacketQueueBase::initialize() {
    // subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

	awaitSendRequests = par("awaitSendRequests");
	syncFirstPacket = par("syncFirstPacket");
	maxLength = par("maxLength");
	if(maxLength < 0)
		throw cRuntimeError(this, "Queue max length must be greater or equal to 0, but received %i", maxLength);

    queue = new cPacketQueue;

    queueLengthSignal = registerSignal("queueLength");
    queueFullSignal = registerSignal("queueFull");
    flitDropSignal = registerSignal("flitDrop");
}

void PacketQueueBase::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = check_and_cast<Flit*>(msg);

    if(!awaitSendRequests && !syncFirstPacket && cycleFree) {
        // If we don't wait for requests, don't sync the first packet
        // and did not already send a packet this cycle, we can send it
        // immediately.
        send(flit, "out");
        cycleFree = false;
    }
    else if(maxLength == 0) {
        // Otherwise, if we don't have a queue length restriction, we can
        // freely insert the packet.
        queue->insert(flit);
    }
    else if(queue->getLength() < maxLength) {
        // Otherwise, if there is a length restriction, but we did not reach
        // it yet, we can also insert the packet.
        queue->insert(flit);

        // If the queue is full now, we emit the appropriate signal.
        if(queue->getLength() == maxLength)
        	emit(queueFullSignal, true);
    }
    else {
        // If we receive a packet while the queue is full, we must discard it.
        emit(flitDropSignal, flit);
        EV_WARN << "Received a packet, but max queue length of " << maxLength
                << " was already reached. Discarding it." << std::endl;
        delete flit;
    }
}

void PacketQueueBase::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		// Emit queue length signal
		emit(queueLengthSignal, queue->getLength());

		if(queue->isEmpty()) {
			cycleFree = true;
		}
		else if(!awaitSendRequests) {
			popQueueAndSend();
		}
	}
}

void PacketQueueBase::popQueueAndSend() {
    cPacket* packet = queue->pop();
    take(packet);
    send(packet, "out");
    if(queue->getLength() == maxLength - 1)
    	emit(queueFullSignal, false);
}

void PacketQueueBase::popQueueAndDiscard() {
    cPacket* packet = queue->pop();
    take(packet);
    delete(packet);
    if(queue->getLength() == maxLength - 1)
        emit(queueFullSignal, false);
}

}} //namespace
