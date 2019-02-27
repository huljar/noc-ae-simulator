//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <omnetpp.h>
#include <Buffers/PacketQueueBase.h>

using namespace omnetpp;
using namespace HaecComm::Buffers;

namespace HaecCommTest {

class AlwaysFullPacketQueue : public PacketQueueBase {
protected:
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
};

Define_Module(AlwaysFullPacketQueue);

void AlwaysFullPacketQueue::handleMessage(cMessage* msg) {
    throw cRuntimeError(this, "Always full queue received a message!");
}

void AlwaysFullPacketQueue::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    PacketQueueBase::receiveSignal(source, signalID, l, details);

    if(signalID == registerSignal("clock") && l == 0) {
        EV_DEBUG << "Emitting queueFull signal (true)" << std::endl;
        emit(queueFullSignal, true);
    }
}

} //namespace
