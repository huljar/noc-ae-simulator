#include <omnetpp.h>
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class AMSplitG2C3TestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
};

Define_Module(AMSplitG2C3TestApp);

void AMSplitG2C3TestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);

    // Test unmodified split
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 123, 123, NC_G2C3);
    take(f1);

    send(f1, "netOut");
}

void AMSplitG2C3TestApp::handleMessage(cMessage* msg) {
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
            // Answer to ARQ with unmodified split
            Flit* f2 = MessageFactory::createFlit("flit2-new", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 123, 321, NC_G2C3);
            take(f2);

            sendDelayed(f2, SimTime(2, SIMTIME_NS), "netOut");
        }
    }
    else
        throw cRuntimeError(this, "Unexpected arrival gate: %s", f->getArrivalGate()->getName());

    delete f;
}

void AMSplitG2C3TestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Test modified split
            Flit* f2 = MessageFactory::createFlit("flit2", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 123, 321, NC_G2C3);
            take(f2);
            f2->setModified(true);

            send(f2, "netOut");
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

