#include <omnetpp.h>
#include <Buffers/PacketQueueBase.h>
#include <Messages/Flit.h>

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
            Flit* f1 = new Flit("toLocalFromNorth");
            take(f1);
            f1->setSource(Address2D(0, 0));
            f1->setTarget(Address2D(1, 1));
            send(f1, "port$o", 0);

            Flit* f2 = new Flit("toNorthFromEast");
            take(f2);
            f2->setSource(Address2D(2, 2));
            f2->setTarget(Address2D(1, 0));
            send(f2, "port$o", 1);

            Flit* f3 = new Flit("toWestFromSouth");
            take(f3);
            f3->setSource(Address2D(1, 2));
            f3->setTarget(Address2D(0, 1));
            send(f3, "port$o", 2);

            Flit* f4 = new Flit("toEastFromWest");
            take(f4);
            f4->setSource(Address2D(0, 2));
            f4->setTarget(Address2D(2, 1));
            send(f4, "port$o", 3);

            Flit* f5 = new Flit("toSouthFromLocal");
            take(f5);
            f5->setSource(Address2D(1, 1));
            f5->setTarget(Address2D(1, 2));
            send(f5, "local$o");
        }
        else if(l == 4) {
            Flit* f6 = new Flit("toEastFromWest2");
            take(f6);
            f6->setSource(Address2D(0, 2));
            f6->setTarget(Address2D(2, 1));
            send(f6, "port$o", 3);

            Flit* f7 = new Flit("toEastFromWest3");
            take(f7);
            f7->setSource(Address2D(0, 2));
            f7->setTarget(Address2D(2, 1));
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

