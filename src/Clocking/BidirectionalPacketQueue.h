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

#ifndef __HAECCOMM_BIDIRECTIONALPACKETQUEUE_H_
#define __HAECCOMM_BIDIRECTIONALPACKETQUEUE_H_

#include <omnetpp.h>
#include <Clocking/PacketQueueBase.h>

using namespace omnetpp;

namespace HaecComm { namespace Clocking {

/**
 * \brief Bidirectional packet queue class for synchronizing packets with the global clock
 *
 * The bidirectional packet queue can queue up packets in both directions. On each clock
 * tick, one packet is sent out in both directions from their respective queues (if there
 * is a packet enqueued). If the simulation is not clocked, the packets are redirected
 * immediately, and no queueing is performed.
 */
class BidirectionalPacketQueue : public PacketQueueBase {
public:
	BidirectionalPacketQueue();
	virtual ~BidirectionalPacketQueue();

protected:
	virtual void initialize() override;
	virtual void handleMessage(cMessage* msg) override;
	virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

	bool cycleFreeLeftToRight;
	bool cycleFreeRightToLeft;
	cPacketQueue* queueLeftToRight;
	cPacketQueue* queueRightToLeft;

	simsignal_t qlenltrSignal;
	simsignal_t qlenrtlSignal;
	simsignal_t pktdropltrSignal;
	simsignal_t pktdroprtlSignal;
};

}} //namespace

#endif
