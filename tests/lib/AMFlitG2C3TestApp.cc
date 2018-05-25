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
};

Define_Module(AMFlitG2C3TestApp);

void AMFlitG2C3TestApp::initialize() {
    // Test unmodified data/mac pair
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 123, NC_G2C3, {34, 35});
    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 123, NC_G2C3);

    send(f1, "netOut");
    send(f2, "netOut");

    // Test modified data/mac pair
    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 321, NC_G2C3, {34, 35});
    f3->setModified(true);

    Flit* f4 = MessageFactory::createFlit("flit4", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 321, NC_G2C3);
    f4->setModified(true);

    sendDelayed(f3, SimTime(4, SIMTIME_NS), "netOut");
    sendDelayed(f4, SimTime(4, SIMTIME_NS), "netOut");

    // Test removing flit from decryption candidate
    Flit* f5 = MessageFactory::createFlit("flit5", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 12, 34, NC_G2C3, {1, 2});
    Flit* f6 = MessageFactory::createFlit("flit6", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 12, 34, NC_G2C3);
    f6->setModified(true);

    sendDelayed(f6, SimTime(40, SIMTIME_NS), "netOut");
    sendDelayed(f5, SimTime(48, SIMTIME_NS), "netOut");

    Flit* f7 = MessageFactory::createFlit("flit7", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 12, 56, NC_G2C3, {1, 2});
    Flit* f8 = MessageFactory::createFlit("flit8", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 12, 56, NC_G2C3);
    Flit* f9 = MessageFactory::createFlit("flit9", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 12, 78, NC_G2C3, {1, 2});
    Flit* f10 = MessageFactory::createFlit("flit10", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 12, 78, NC_G2C3);

    sendDelayed(f7, SimTime(64, SIMTIME_NS), "netOut");
    sendDelayed(f8, SimTime(66, SIMTIME_NS), "netOut");
    sendDelayed(f9, SimTime(68, SIMTIME_NS), "netOut");
    sendDelayed(f10, SimTime(70, SIMTIME_NS), "netOut");

    // Test removing both flits from decryption candidate (G2C4)
    Flit* f11 = MessageFactory::createFlit("flit11", Address2D(3, 1), Address2D(0, 0), MODE_DATA, 14, 56, NC_G2C4, {3, 4});
    Flit* f12 = MessageFactory::createFlit("flit12", Address2D(3, 1), Address2D(0, 0), MODE_DATA, 14, 57, NC_G2C4, {3, 4});
    Flit* f13 = MessageFactory::createFlit("flit13", Address2D(3, 1), Address2D(0, 0), MODE_MAC, 14, 56, NC_G2C4);
    Flit* f14 = MessageFactory::createFlit("flit14", Address2D(3, 1), Address2D(0, 0), MODE_MAC, 14, 57, NC_G2C4);

    f11->setModified(true);
    f14->setBitError(true);

    Flit* f15 = MessageFactory::createFlit("flit15", Address2D(3, 1), Address2D(0, 0), MODE_DATA, 14, 58, NC_G2C4, {3, 4});
    Flit* f16 = MessageFactory::createFlit("flit16", Address2D(3, 1), Address2D(0, 0), MODE_DATA, 14, 59, NC_G2C4, {3, 4});
    Flit* f17 = MessageFactory::createFlit("flit17", Address2D(3, 1), Address2D(0, 0), MODE_MAC, 14, 59, NC_G2C4);
    Flit* f18 = MessageFactory::createFlit("flit18", Address2D(3, 1), Address2D(0, 0), MODE_MAC, 14, 58, NC_G2C4);

    sendDelayed(f11, SimTime(80, SIMTIME_NS), "netOut");
    sendDelayed(f12, SimTime(82, SIMTIME_NS), "netOut");
    sendDelayed(f13, SimTime(84, SIMTIME_NS), "netOut");
    sendDelayed(f14, SimTime(86, SIMTIME_NS), "netOut");
    sendDelayed(f15, SimTime(88, SIMTIME_NS), "netOut");
    sendDelayed(f16, SimTime(90, SIMTIME_NS), "netOut");
    sendDelayed(f17, SimTime(92, SIMTIME_NS), "netOut");
    sendDelayed(f18, SimTime(94, SIMTIME_NS), "netOut");
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
            Flit* f3 = MessageFactory::createFlit("flit3-new", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 321, NC_G2C3, {34, 35});
            take(f3);

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

} //namespace

