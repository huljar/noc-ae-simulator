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

#ifndef __HAECCOMM_ENTRYGUARDFLIT_H_
#define __HAECCOMM_ENTRYGUARDFLIT_H_

#include <omnetpp.h>
#include <Buffers/PacketQueueBase.h>
#include <Util/ShiftRegister.h>
#include <map>
#include <queue>
#include <vector>

using namespace omnetpp;

namespace HaecComm { namespace MW { namespace Crypto {

/**
 * Module class that is used to manage the crypto modules in the individual authentication protocol
 */
class EntryGuardFlit : public cSimpleModule, public cListener {
public:
    typedef std::priority_queue<int, std::vector<int>, std::greater<int>> AvailableQueue;
    typedef Util::ShiftRegister<std::vector<int>> BusyRegister;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    int busyCyclesEnc;
    int busyCyclesAuth;

    AvailableQueue availableEncUnits;
    BusyRegister busyEncUnits;
    AvailableQueue availableAuthUnits;
    BusyRegister busyAuthUnits;

    Buffers::PacketQueueBase* appInputQueue;
    Buffers::PacketQueueBase* exitInputQueue;
    Buffers::PacketQueueBase* decInputQueue;
    Buffers::PacketQueueBase* verInputQueue;

    std::map<int, simsignal_t> encBusySignals;
    std::map<int, simsignal_t> authBusySignals;
};

}}} //namespace

#endif
