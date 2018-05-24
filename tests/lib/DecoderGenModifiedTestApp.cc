#include <omnetpp.h>
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class DecoderGenModifiedTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(DecoderGenModifiedTestApp);

void DecoderGenModifiedTestApp::initialize() {
    // Second combination modified
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 123, 5, NC_G2C3, {222, 333});
    take(f1);

    Flit* f2 = f1->dup();
    f2->setName("flit2");
    f2->setGev(8);
    f2->setModified(true);

    send(f1, "out");
    send(f2, "out");

    // Some MAC passing through
    Flit* mac12 = MessageFactory::createFlit("mac12", Address2D(0, 0), Address2D(1, 1), MODE_MAC, 123, 0, NC_G2C3);
    sendDelayed(mac12, SimTime(2, SIMTIME_NS), "out");

    // First combination modified
    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 124, 5, NC_G2C3, {444, 555});
    take(f3);

    Flit* f4 = f3->dup();
    f4->setName("flit4");
    f4->setGev(8);

    f3->setBitError(true);

    sendDelayed(f3, SimTime(10, SIMTIME_NS), "out");
    sendDelayed(f4, SimTime(14, SIMTIME_NS), "out");

    // No combination modified
    Flit* f5 = MessageFactory::createFlit("flit5", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 125, 5, NC_G2C4, {666, 777});
    take(f5);

    Flit* f6 = f5->dup();
    f6->setName("flit6");
    f6->setGev(8);

    sendDelayed(f5, SimTime(18, SIMTIME_NS), "out");
    sendDelayed(f6, SimTime(20, SIMTIME_NS), "out");
}

void DecoderGenModifiedTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "FID " << f->getGidOrFid() << std::endl
       << "Modified " << std::boolalpha << f->isModified() << std::endl
       << "BitError " << f->hasBitError() << std::noboolalpha << std::endl;
    if(f->getOriginalIdsArraySize() > 0)
        EV << "OrigID0 " << f->getOriginalIds(0) << std::endl;
    else
        EV << "No OrigID0" << std::endl;
    delete f;
}

} //namespace

