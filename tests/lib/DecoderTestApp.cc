#include <omnetpp.h>
#include <Messages/Flit.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class DecoderTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(DecoderTestApp);

void DecoderTestApp::initialize() {
    Flit* f1 = new Flit("flit1");
    take(f1);
    f1->setSource(Address2D(0, 0));
    f1->setTarget(Address2D(1, 1));
    f1->setGidOrFid(123);
    f1->setGev(5);
    f1->setOriginalIdsArraySize(2);
    f1->setOriginalIds(0, 222);
    f1->setOriginalIds(1, 333);
    f1->setNcMode(NC_G2C3);

    Flit* f2 = f1->dup();
    f2->setName("flit2");
    f2->setGev(8);

    send(f1, "out");
    send(f2, "out");
}

void DecoderTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "FID " << f->getGidOrFid() << std::endl;
    delete f;
}

} //namespace

