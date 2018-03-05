#include <omnetpp.h>
#include <Messages/Flit.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class ArrivalManagerFlitG2C3TestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
};

Define_Module(ArrivalManagerFlitG2C3TestApp);

void ArrivalManagerFlitG2C3TestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);

    // Test unmodified data/mac pair
    Flit* f1 = new Flit("flit1");
    take(f1);
    f1->setSource(Address2D(3, 3));
    f1->setTarget(Address2D(0, 0));
    f1->setGidOrFid(123);
    f1->setGev(123);
    f1->setMode(MODE_DATA);
    f1->setNcMode(NC_G2C3);

    Flit* f2 = f1->dup();
    take(f2);
    f2->setName("flit2");
    f2->setMode(MODE_MAC);

    send(f1, "netOut");
    send(f2, "netOut");
}

void ArrivalManagerFlitG2C3TestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    // Error if anything arrives targeting the network
    if(strcmp(f->getArrivalGate()->getName(), "retBufIn") == 0)
        throw cRuntimeError(this, "Received flit going to the retransmission buffer!");

    if(strcmp(f->getArrivalGate()->getName(), "appIn") == 0) {
        EV << "Got flit " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "GID " << f->getGidOrFid() << std::endl
           << "GEV " << f->getGev() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
           << "NC mode " << cEnum::get("HaecComm::Messages::NcMode")->getStringFor(f->getNcMode()) << std::endl;
    }
    else if(strcmp(f->getArrivalGate()->getName(), "arqIn") == 0) {
        EV << "Got ARQ " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "GID " << f->getGidOrFid() << std::endl
           << "GEV " << f->getGev() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
           << "NC mode " << cEnum::get("HaecComm::Messages::NcMode")->getStringFor(f->getNcMode()) << std::endl
           << "ARQ modes ";
        for(auto it = f->getNcArqs().begin(); it != f->getNcArqs().end(); ++it)
            EV << "(" << it->first << "," << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(it->second) << ")";
        EV << std::endl;

        if(f->getGidOrFid() == 123 && f->getNcArqs().count(321)) {
            // Answer to ARQ with unmodified flits
            Flit* f3 = new Flit("flit3-new");
            take(f3);
            f3->setSource(Address2D(3, 3));
            f3->setTarget(Address2D(0, 0));
            f3->setGidOrFid(123);
            f3->setGev(321);
            f3->setMode(MODE_DATA);
            f3->setNcMode(NC_G2C3);

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

void ArrivalManagerFlitG2C3TestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Test modified data/mac pair
            Flit* f3 = new Flit("flit3");
            take(f3);
            f3->setSource(Address2D(3, 3));
            f3->setTarget(Address2D(0, 0));
            f3->setGidOrFid(123);
            f3->setGev(321);
            f3->setMode(MODE_DATA);
            f3->setNcMode(NC_G2C3);
            f3->setModified(true);

            Flit* f4 = f3->dup();
            take(f4);
            f4->setName("flit4");
            f4->setMode(MODE_MAC);

            send(f3, "netOut");
            send(f4, "netOut");
        }
        else if(l == 3) {

        }
        else if(l == 5) {
            // TODO: test losses
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

