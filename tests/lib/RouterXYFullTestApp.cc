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
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Buffers;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class RouterXYFullTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
};

Define_Module(RouterXYFullTestApp);

void RouterXYFullTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
}

void RouterXYFullTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "FID " << f->getGidOrFid() << std::endl;
    delete f;
}

void RouterXYFullTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 1) {
            Flit* f1 = MessageFactory::createFlit("toLocalFromNorth", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 1);
            take(f1);
            send(f1, "port$o", 0);

            Flit* f2 = MessageFactory::createFlit("toNorthFromEast", Address2D(2, 2), Address2D(1, 0), MODE_SPLIT_1, 2);
            take(f2);
            send(f2, "port$o", 1);

            Flit* f3 = MessageFactory::createFlit("toWestFromSouth", Address2D(1, 2), Address2D(0, 1), MODE_SPLIT_2, 2);
            take(f3);
            send(f3, "port$o", 2);

            Flit* f4 = MessageFactory::createFlit("toEastFromWest", Address2D(0, 2), Address2D(2, 1), MODE_DATA, 10);
            take(f4);
            send(f4, "port$o", 3);

            Flit* f5 = MessageFactory::createFlit("toSouthFromLocal", Address2D(1, 1), Address2D(1, 2), MODE_MAC, 10);
            take(f5);
            send(f5, "local$o");
        }
        else if(l == 4) {
            Flit* f6 = MessageFactory::createFlit("toEastFromWest2", Address2D(0, 2), Address2D(2, 1), MODE_SPLIT_1, 18);
            take(f6);
            send(f6, "port$o", 3);

            Flit* f7 = MessageFactory::createFlit("toEastFromWest3", Address2D(0, 2), Address2D(2, 1), MODE_SPLIT_2, 18);
            take(f7);
            send(f7, "port$o", 3);
        }
        else if(l == 8) {
            EV << "Router queue lengths" << std::endl
               << "rl: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.router.localInputQueue"))->getLength() << std::endl
               << "r0: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.router.nodeInputQueue[0]"))->getLength() << std::endl
               << "r1: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.router.nodeInputQueue[1]"))->getLength() << std::endl
               << "r2: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.router.nodeInputQueue[2]"))->getLength() << std::endl
               << "r3: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.router.nodeInputQueue[3]"))->getLength() << std::endl
               << "ql: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.ql"))->getLength() << std::endl
               << "q0: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.q0"))->getLength() << std::endl
               << "q1: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.q1"))->getLength() << std::endl
               << "q2: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.q2"))->getLength() << std::endl
               << "q3: " << check_and_cast<PacketQueueBase*>(getSimulation()->getModuleByPath("<root>.q3"))->getLength() << std::endl;
        }
    }
}

} //namespace

