#include <omnetpp.h>
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class EncoderGenTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(EncoderGenTestApp);

void EncoderGenTestApp::initialize() {
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 123);
    take(f1);

    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 321);
    take(f2);

    send(f1, "out1");
    send(f2, "out1");

    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(1, 0), Address2D(2, 0), MODE_DATA, 234);
    take(f3);

    Flit* f4 = MessageFactory::createFlit("flit4", Address2D(1, 0), Address2D(2, 0), MODE_DATA, 432);
    take(f4);

    send(f3, "out2");
    send(f4, "out2");

    Flit* mac12 = MessageFactory::createFlit("mac12", Address2D(0, 0), Address2D(1, 1), MODE_MAC, 5);
    Flit* mac34 = MessageFactory::createFlit("mac34", Address2D(1, 0), Address2D(2, 0), MODE_MAC, 5);

    sendDelayed(mac12, SimTime(10, SIMTIME_NS), "out1");
    sendDelayed(mac34, SimTime(10, SIMTIME_NS), "out2");
}

void EncoderGenTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "GID " << f->getGidOrFid() << std::endl
       << "GEV " << f->getGev() << std::endl;
    delete f;
}

} //namespace

