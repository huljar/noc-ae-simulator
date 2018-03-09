#include <omnetpp.h>
#include <Messages/Flit.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class ArrivalManagerFlitUncodedTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
};

Define_Module(ArrivalManagerFlitUncodedTestApp);

void ArrivalManagerFlitUncodedTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);

    // Test unmodified data/mac pair
    Flit* f1 = new Flit("flit1");
    take(f1);
    f1->setSource(Address2D(3, 3));
    f1->setTarget(Address2D(0, 0));
    f1->setGidOrFid(123);
    f1->setMode(MODE_DATA);

    Flit* f2 = f1->dup();
    take(f2);
    f2->setName("flit2");
    f2->setMode(MODE_MAC);

    send(f1, "netOut");
    send(f2, "netOut");
}

void ArrivalManagerFlitUncodedTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    // Error if anything arrives targeting the network
    if(strcmp(f->getArrivalGate()->getName(), "retBufIn") == 0)
        throw cRuntimeError(this, "Received flit going to the retransmission buffer!");

    if(strcmp(f->getArrivalGate()->getName(), "appIn") == 0) {
        EV << "Got flit " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "FID " << f->getGidOrFid() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;
    }
    else if(strcmp(f->getArrivalGate()->getName(), "arqIn") == 0) {
        EV << "Got ARQ " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "FID " << f->getGidOrFid() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
           << "ARQ mode " << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(f->getUcArqs()) << std::endl;

        if(f->getGidOrFid() == 666) {
            // Answer to ARQ with unmodified flits
            Flit* f3 = new Flit("flit3-new");
            take(f3);
            f3->setSource(Address2D(3, 3));
            f3->setTarget(Address2D(0, 0));
            f3->setGidOrFid(666);
            f3->setMode(MODE_DATA);

            Flit* f4 = f3->dup();
            take(f4);
            f4->setName("flit4-new");
            f4->setMode(MODE_MAC);

            sendDelayed(f3, SimTime(2, SIMTIME_NS), "netOut");
            sendDelayed(f4, SimTime(2, SIMTIME_NS), "netOut");
        }
    }
    else
        throw cRuntimeError(this, "Unexpected arrival gate: %s", f->getArrivalGate()->getName());

    delete f;
}

void ArrivalManagerFlitUncodedTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Test modified data/mac pair
            Flit* f3 = new Flit("flit3");
            take(f3);
            f3->setSource(Address2D(3, 3));
            f3->setTarget(Address2D(0, 0));
            f3->setGidOrFid(666);
            f3->setMode(MODE_DATA);

            Flit* f4 = f3->dup();
            take(f4);
            f4->setName("flit4");
            f4->setMode(MODE_MAC);
            f4->setModified(true);

            send(f3, "netOut");
            send(f4, "netOut");
        }
        else if(l == 3) {
            // Test data/mac pair with bit error
            Flit* f5 = new Flit("flit5");
            take(f5);
            f5->setSource(Address2D(3, 3));
            f5->setTarget(Address2D(0, 0));
            f5->setGidOrFid(777);
            f5->setMode(MODE_DATA);

            Flit* f6 = f5->dup();
            take(f6);
            f6->setName("flit6");
            f6->setMode(MODE_MAC);

            f5->setBitError(true);

            send(f5, "netOut");
            send(f6, "netOut");
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

