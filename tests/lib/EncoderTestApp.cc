#include <omnetpp.h>
#include <Messages/Flit.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class EncoderTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(EncoderTestApp);

void EncoderTestApp::initialize() {
    Flit* f1 = new Flit("flit1");
    take(f1);
    f1->setSource(Address2D(0, 0));
    f1->setTarget(Address2D(1, 1));
    f1->setGidOrFid(123);

    Flit* f2 = f1->dup();
    f2->setName("flit2");
    f2->setGidOrFid(321);

    send(f1, "out1");
    send(f2, "out1");

    Flit* f3 = new Flit("flit3");
    take(f3);
    f3->setSource(Address2D(1, 0));
    f3->setTarget(Address2D(2, 0));
    f3->setGidOrFid(234);

    Flit* f4 = f3->dup();
    f3->setName("flit4");
    f3->setGidOrFid(432);

    send(f3, "out2");
    send(f4, "out2");
}

void EncoderTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "GID " << f->getGidOrFid() << std::endl
       << "GEV " << f->getGev() << std::endl;
    delete f;
}

} //namespace

