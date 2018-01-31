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

#ifndef __HAECCOMM_LOADBALANCER_H_
#define __HAECCOMM_LOADBALANCER_H_

#include <omnetpp.h>
#include <Clocking/PacketQueueBase.h>
#include <Util/ShiftRegister.h>
#include <queue>
#include <vector>

using namespace omnetpp;

namespace HaecComm { namespace Util {

/**
 * TODO - implement load balancing for unclocked simulations
 */
class LoadBalancer : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

private:
    std::queue<int> availableUnits;
    ShiftRegister<std::vector<int>> busyUnits;

    Clocking::PacketQueueBase* inputQueue;
};

}} //namespace

#endif
