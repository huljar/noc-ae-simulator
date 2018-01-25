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

#ifndef __HAECCOMM_PACKETQUEUEBASE_H_
#define __HAECCOMM_PACKETQUEUEBASE_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace HaecComm { namespace Clocking {

/**
 * TODO - Generated class
 */
class PacketQueueBase : public cSimpleModule, public cListener {
public:
	PacketQueueBase();
	virtual ~PacketQueueBase();

	virtual void requestPacket();
	virtual cPacket* peek();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override = 0;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override = 0;

    void popQueueAndSend();

    bool awaitSendRequests;
    bool syncFirstPacket;
    int maxLength;

    bool cycleFree;
    bool gotSendRequest;
    cPacketQueue* queue;

    simsignal_t qlenSignal;
    simsignal_t qfullSignal;
    simsignal_t pktdropSignal;
};

}} //namespace

#endif
