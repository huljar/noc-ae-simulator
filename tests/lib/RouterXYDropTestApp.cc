#include <omnetpp.h>
#include <Buffers/PacketQueueBase.h>
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>
#include <sstream>

using namespace omnetpp;
using namespace HaecComm::Buffers;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class RouterXYDropTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    int count[2];
};

Define_Module(RouterXYDropTestApp);

void RouterXYDropTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    count[0] = 0;
    count[1] = 0;

    // Send 10 flits to each router
    for(int i = 0; i < 10; ++i) {
        std::ostringstream s1;
        s1 << "1flit" << i;
        Flit* f1 = MessageFactory::createFlit(s1.str().c_str(), Address2D(3, 0), Address2D(0, 3), MODE_SPLIT_1, i);
        send(f1, "local1$o");

        std::ostringstream s2;
        s2 << "2flit" << i;
        Flit* f2 = MessageFactory::createFlit(s2.str().c_str(), Address2D(2, 2), Address2D(1, 0), MODE_SPLIT_2, i+100);
        send(f2, "port2$o", 2);
    }
}

void RouterXYDropTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    int srcRouter = (strcmp(msg->getArrivalGate()->getBaseName(), "local1") == 0 || strcmp(msg->getArrivalGate()->getBaseName(), "port1") == 0
                    ? 1
                    : 2);

    EV << "Got flit from router " << srcRouter << " " << f->getName() << std::endl
       << "Modified " << std::boolalpha << f->isModified() << std::noboolalpha << std::endl;
    delete f;

    ++count[srcRouter];
    EV << "Received a total of " << count[srcRouter] << " flits from router " << srcRouter << std::endl;
}

void RouterXYDropTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {

    }
}

} //namespace

