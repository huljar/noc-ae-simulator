#include <omnetpp.h>
#include <Core/Clock.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Core;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class AMFlitG2C3RemovePlannedARQTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    Clock* clock;
};

Define_Module(AMFlitG2C3RemovePlannedARQTestApp);

void AMFlitG2C3RemovePlannedARQTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    clock = dynamic_cast<Clock*>(getSimulation()->getSystemModule()->getSubmodule("clock"));
    ASSERT(clock != nullptr);

    // Send parts of a generation
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(1, 0), Address2D(0, 0), MODE_MAC, 123, 42, NC_G2C3);
    take(f1);

    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(1, 0), Address2D(0, 0), MODE_MAC, 123, 43, NC_G2C3);
    take(f2);

    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(1, 0), Address2D(0, 0), MODE_DATA, 123, 43, NC_G2C3);
    take(f3);
    f3->setOriginalIdsArraySize(2);
    f3->setOriginalIds(0, 222);
    f3->setOriginalIds(1, 333);

    Flit* f4 = MessageFactory::createFlit("flit4", Address2D(1, 0), Address2D(0, 0), MODE_DATA, 123, 44, NC_G2C3);
    take(f4);
    f4->setOriginalIdsArraySize(2);
    f4->setOriginalIds(0, 222);
    f4->setOriginalIds(1, 333);

    send(f1, "netOut");
    sendDelayed(f2, SimTime(2, SIMTIME_NS), "netOut");
    sendDelayed(f3, SimTime(8, SIMTIME_NS), "netOut");
    sendDelayed(f4, SimTime(10, SIMTIME_NS), "netOut");

    // Send rest of generation while ARQ is planned
    Flit* f5 = MessageFactory::createFlit("flit5", Address2D(1, 0), Address2D(0, 0), MODE_MAC, 123, 44, NC_G2C3);
    take(f5);
    f5->setOriginalIdsArraySize(2);
    f5->setOriginalIds(0, 222);
    f5->setOriginalIds(1, 333);

    Flit* f6 = MessageFactory::createFlit("flit6", Address2D(1, 0), Address2D(0, 0), MODE_DATA, 123, 42, NC_G2C3);
    take(f6);
    f6->setOriginalIdsArraySize(2);
    f6->setOriginalIds(0, 222);
    f6->setOriginalIds(1, 333);

    sendDelayed(f5, SimTime(20, SIMTIME_NS), "netOut");
    sendDelayed(f6, SimTime(22, SIMTIME_NS), "netOut");
}

void AMFlitG2C3RemovePlannedARQTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    // Error if anything arrives targeting the network
    if(strcmp(f->getArrivalGate()->getName(), "retBufIn") == 0)
        throw cRuntimeError(this, "Received flit going to the retransmission buffer!");

    if(strcmp(f->getArrivalGate()->getName(), "appIn") == 0) {
        EV << clock->getCurrentCycle() << " Got flit " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "FID " << f->getGidOrFid() << std::endl
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

void AMFlitG2C3RemovePlannedARQTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {

    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

