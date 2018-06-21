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
#include <Core/Clock.h>
#include <map>

using namespace omnetpp;

namespace HaecComm { namespace Buffers {

/**
 * TODO - Generated class
 */
class PacketQueueBase : public cSimpleModule, public cListener {
public:
	PacketQueueBase();
	virtual ~PacketQueueBase();

	virtual void requestPacket();
	virtual cPacket* peek();
	virtual void requestDrop();

	virtual int getLength();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    void popQueueAndSend();
    void popQueueAndDiscard();

    bool awaitSendRequests;
    bool syncFirstPacket;
    int maxLength;
    bool softLimit;

    bool cycleFree;
    cPacketQueue* queue;
    std::map<cPacket*, unsigned long> enqueueTimes;
    Core::Clock* clock;

    simsignal_t flitSentSignal;
    simsignal_t queueLengthSignal;
    simsignal_t queueFullSignal;
    simsignal_t flitDropSignal;
    simsignal_t timeInQueueSignal;
};

}} //namespace

#endif
