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

#ifndef __HAECCOMM_ROUTER_H_
#define __HAECCOMM_ROUTER_H_

#include <omnetpp.h>
#include <Core/HaecModule.h>
#include <vector>

using namespace omnetpp;

namespace HaecComm {

/**
 * TODO - Generated class
 */
class Router : public HaecModule {
public:
	Router();
	virtual ~Router();
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details);

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

private:
    cPacketQueue* localSendQueue;
    cPacketQueue* localReceiveQueue;
    std::vector<cPacketQueue*> portSendQueues;
    std::vector<cPacketQueue*> portReceiveQueues;
};

} //namespace

#endif
