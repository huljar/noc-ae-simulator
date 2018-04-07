#include <omnetpp.h>
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class EncoderSplitTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(EncoderSplitTestApp);

void EncoderSplitTestApp::initialize() {
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 123);
    take(f1);

    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(1, 0), Address2D(2, 0), MODE_DATA, 321);
    take(f2);

    send(f1, "out1");
    send(f2, "out2");
}

void EncoderSplitTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "GID " << f->getGidOrFid() << std::endl
       << "GEV " << f->getGev() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
       << "OrigIDs " << f->getOriginalIds(0) << " " << f->getOriginalIds(1) << std::endl;
    delete f;
}

} //namespace

