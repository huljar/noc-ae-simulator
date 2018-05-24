#include <omnetpp.h>
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class AMFlitG2C3TestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
};

Define_Module(AMFlitG2C3TestApp);

void AMFlitG2C3TestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);

    // Test unmodified data/mac pair
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 123, NC_G2C3);
    take(f1);
    f1->setOriginalIdsArraySize(2);
    f1->setOriginalIds(0, 34);
    f1->setOriginalIds(1, 35);

    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 123, NC_G2C3);
    take(f2);

    send(f1, "netOut");
    send(f2, "netOut");
}

void AMFlitG2C3TestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    // Error if anything arrives targeting the network
    if(strcmp(f->getArrivalGate()->getName(), "retBufIn") == 0)
        throw cRuntimeError(this, "Received flit going to the retransmission buffer!");

    if(strcmp(f->getArrivalGate()->getName(), "appIn") == 0) {
        EV << "Got flit " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "FID " << f->getGidOrFid() << std::endl
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
            Flit* f3 = MessageFactory::createFlit("flit3-new", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 321, NC_G2C3);
            take(f3);
            f3->setOriginalIdsArraySize(2);
            f3->setOriginalIds(0, 34);
            f3->setOriginalIds(1, 35);

            Flit* f4 = MessageFactory::createFlit("flit4-new", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 321, NC_G2C3);
            take(f4);

            sendDelayed(f3, SimTime(2, SIMTIME_NS), "netOut");
            sendDelayed(f4, SimTime(2, SIMTIME_NS), "netOut");
        }
    }
    else
        throw cRuntimeError(this, "Unexpected arrival gate: %s", f->getArrivalGate()->getName());

    delete f;
}

void AMFlitG2C3TestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Test modified data/mac pair
            Flit* f3 = MessageFactory::createFlit("flit3", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 321, NC_G2C3);
            take(f3);
            f3->setModified(true);
            f3->setOriginalIdsArraySize(2);
            f3->setOriginalIds(0, 34);
            f3->setOriginalIds(1, 35);

            Flit* f4 = MessageFactory::createFlit("flit4", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 321, NC_G2C3);
            take(f4);
            f4->setModified(true);

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

