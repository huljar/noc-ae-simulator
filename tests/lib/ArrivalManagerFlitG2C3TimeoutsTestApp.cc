#include <omnetpp.h>
#include <Core/Clock.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Core;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class ArrivalManagerFlitG2C3TimeoutsTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    Clock* clock;
};

Define_Module(ArrivalManagerFlitG2C3TimeoutsTestApp);

void ArrivalManagerFlitG2C3TimeoutsTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    clock = dynamic_cast<Clock*>(getSimulation()->getSystemModule()->getSubmodule("clock"));
    ASSERT(clock != nullptr);

    // Test single flit
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 1, NC_G2C3);
    take(f1);

    send(f1, "netOut");

    // Test two flits with different GEVs
    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(2, 2), Address2D(0, 0), MODE_MAC, 123, 2, NC_G2C3);
    take(f2);

    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(2, 2), Address2D(0, 0), MODE_DATA, 123, 5, NC_G2C3);
    take(f3);

    sendDelayed(f2, SimTime(2, SIMTIME_NS), "netOut");
    sendDelayed(f3, SimTime(8, SIMTIME_NS), "netOut");

    // Test two modified pairs
    Flit* f4 = MessageFactory::createFlit("flit4", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 222, 1, NC_G2C3);
    take(f4);
    f4->setModified(true);

    Flit* f5 = MessageFactory::createFlit("flit5", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 222, 1, NC_G2C3);
    take(f5);

    Flit* f6 = MessageFactory::createFlit("flit6", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 222, 42, NC_G2C3);
    take(f6);

    Flit* f7 = MessageFactory::createFlit("flit7", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 222, 42, NC_G2C3);
    take(f7);
    f7->setModified(true);

    sendDelayed(f4, SimTime(10, SIMTIME_NS), "netOut");
    sendDelayed(f5, SimTime(12, SIMTIME_NS), "netOut");
    sendDelayed(f6, SimTime(16, SIMTIME_NS), "netOut");
    sendDelayed(f7, SimTime(22, SIMTIME_NS), "netOut");

    // Test loss with all GEVs known
    Flit* f8 = MessageFactory::createFlit("flit8", Address2D(1, 1), Address2D(0, 0), MODE_DATA, 42, 12, NC_G2C3);
    take(f8);

    Flit* f9 = MessageFactory::createFlit("flit9", Address2D(1, 1), Address2D(0, 0), MODE_MAC, 42, 12, NC_G2C3);
    take(f9);

    Flit* f10 = MessageFactory::createFlit("flit10", Address2D(1, 1), Address2D(0, 0), MODE_MAC, 42, 25, NC_G2C3);
    take(f10);

    Flit* f11 = MessageFactory::createFlit("flit11", Address2D(1, 1), Address2D(0, 0), MODE_MAC, 42, 28, NC_G2C3);
    take(f11);

    sendDelayed(f8, SimTime(18, SIMTIME_NS), "netOut");
    sendDelayed(f9, SimTime(20, SIMTIME_NS), "netOut");
    sendDelayed(f10, SimTime(28, SIMTIME_NS), "netOut");
    sendDelayed(f11, SimTime(34, SIMTIME_NS), "netOut");
}

void ArrivalManagerFlitG2C3TimeoutsTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    // Error if anything arrives targeting the network
    if(strcmp(f->getArrivalGate()->getName(), "retBufIn") == 0)
        throw cRuntimeError(this, "Received flit going to the retransmission buffer!");

    if(strcmp(f->getArrivalGate()->getName(), "appIn") == 0) {
        EV << clock->getCurrentCycle() << " Got flit " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "GID " << f->getGidOrFid() << std::endl
           << "GEV " << f->getGev() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;
    }
    else if(strcmp(f->getArrivalGate()->getName(), "arqIn") == 0) {
        EV << clock->getCurrentCycle() << " Got ARQ " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "GID " << f->getGidOrFid() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
           << "ARQ modes ";
        for(auto it = f->getNcArqs().begin(); it != f->getNcArqs().end(); ++it)
            EV << "(" << it->first << "," << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(it->second) << ")";
        EV << std::endl;
    }
    else
        throw cRuntimeError(this, "Unexpected arrival gate: %s", f->getArrivalGate()->getName());

    delete f;
}

void ArrivalManagerFlitG2C3TimeoutsTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {

    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

