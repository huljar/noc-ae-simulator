#include <omnetpp.h>
#include <Core/Clock.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Core;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class AMSplitG2C3RemovePlannedARQTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    Clock* clock;
};

Define_Module(AMSplitG2C3RemovePlannedARQTestApp);

void AMSplitG2C3RemovePlannedARQTestApp::initialize() {
    clock = dynamic_cast<Clock*>(getSimulation()->getSystemModule()->getSubmodule("clock"));
    ASSERT(clock != nullptr);

    // Send parts of a generation
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(1, 0), Address2D(0, 0), MODE_SPLIT_NC, 123, 42, NC_G2C3, {42, 42});
    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(1, 0), Address2D(0, 0), MODE_SPLIT_NC, 123, 43, NC_G2C3, {42, 42});

    send(f1, "netOut");
    sendDelayed(f2, SimTime(6, SIMTIME_NS), "netOut");

    // Send rest of generation while ARQ is planned
    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(1, 0), Address2D(0, 0), MODE_SPLIT_NC, 123, 44, NC_G2C3, {42, 42});

    sendDelayed(f3, SimTime(16, SIMTIME_NS), "netOut");
}

void AMSplitG2C3RemovePlannedARQTestApp::handleMessage(cMessage* msg) {
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

} //namespace

