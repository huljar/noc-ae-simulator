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

namespace HaecComm { namespace Clocking {

PacketQueueBase::PacketQueueBase()
    : awaitSendRequests(false)
	, syncFirstPacket(true)
	, maxLength(0)
    , cycleFree(true)
    , gotSendRequest(false)
    , queue(nullptr)
{
}

PacketQueueBase::~PacketQueueBase() {
    delete queue;
}

void PacketQueueBase::requestPacket() {
    gotSendRequest = true;
}

cPacket* PacketQueueBase::peek() {
    // front() returns nullptr if the queue is empty
    return queue->front();
}

void PacketQueueBase::initialize() {
    // subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

	awaitSendRequests = par("awaitSendRequests");
	syncFirstPacket = par("syncFirstPacket");
	maxLength = par("maxLength");

    queue = new cPacketQueue;

    qlenSignal = registerSignal("qlen");
    qfullSignal = registerSignal("qfull");
    pktdropSignal = registerSignal("pktdrop");
}

void PacketQueueBase::popQueueAndSend() {
    cPacket* packet = queue->pop();
    take(packet);
    send(packet, "out");
    emit(qfullSignal, false);
}

}} //namespace
