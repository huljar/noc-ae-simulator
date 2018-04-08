#include <omnetpp.h>
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class RBFlitG2C4TestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    int count;
};

Define_Module(RBFlitG2C4TestApp);

void RBFlitG2C4TestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    count = 1;

    // Send 2 data/mac combinations into the buffer
    Flit* f1 = MessageFactory::createFlit("data1", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 2, NC_G2C4);
    Flit* f2 = MessageFactory::createFlit("mac1", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 2, NC_G2C4);
    Flit* f3 = MessageFactory::createFlit("data2", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 19, NC_G2C4);
    Flit* f4 = MessageFactory::createFlit("mac2", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 19, NC_G2C4);
    Flit* f5 = MessageFactory::createFlit("data3", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 33, NC_G2C4);
    Flit* f6 = MessageFactory::createFlit("mac4", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 35, NC_G2C4);

    send(f1, "dataOut");
    send(f2, "dataOut");
    send(f3, "dataOut");
    send(f4, "dataOut");
    send(f5, "dataOut");
    send(f6, "dataOut");
}

void RBFlitG2C4TestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    EV << count++ << " Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "GID " << f->getGidOrFid() << std::endl
       << "GEV " << f->getGev() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;

    delete f;
}

void RBFlitG2C4TestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Send ARQ for specific flits (TELL_MISSING)
            Flit* arq1 = MessageFactory::createFlit("arq1", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123, 0, NC_G2C4);
            take(arq1);
            arq1->setNcArqs(GevArqMap{{2, ARQ_MAC}, {19, ARQ_DATA_MAC}, {33, ARQ_DATA}, {35, ARQ_MAC}});

            send(arq1, "arqOut");
        }
        else if(l == 3) {
            // Send ARQ for specific flits (TELL_MISSING) but for flits partially not in the buffer
            Flit* arq2 = MessageFactory::createFlit("arq2", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123, 0, NC_G2C4);
            take(arq2);
            arq2->setNcArqs(GevArqMap{{2, ARQ_MAC}, {19, ARQ_DATA_MAC}, {35, ARQ_DATA}});

            send(arq2, "arqOut");
        }
        else if(l == 8) {
            // Send ARQ from other source (should no get an answer)
            Flit* arq2 = MessageFactory::createFlit("arq22", Address2D(1, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123, 0, NC_G2C4);
            take(arq2);
            arq2->setNcArqs(GevArqMap{{2, ARQ_MAC}, {19, ARQ_DATA_MAC}, {33, ARQ_DATA}});

            send(arq2, "arqOut");
        }
        else if(l == 10) {
            // Send ARQ for unknown flits (TELL_RECEIVED), there should not be an answer (33/ARQ_MAC not in buffer)
            Flit* arq3 = MessageFactory::createFlit("arq3", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 123, 0, NC_G2C4);
            take(arq3);
            arq3->setNcArqs(GevArqMap{{2, ARQ_MAC}, {19, ARQ_DATA_MAC}, {33, ARQ_DATA}, {35, ARQ_DATA}});

            send(arq3, "arqOut");
        }
        else if(l == 15) {
            // Send ARQ for unknown flits (TELL_RECEIVED)
            Flit* arq3 = MessageFactory::createFlit("arq33", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 123, 0, NC_G2C4);
            take(arq3);
            arq3->setNcArqs(GevArqMap{{2, ARQ_DATA}, {33, ARQ_MAC}, {35, ARQ_DATA_MAC}});

            send(arq3, "arqOut");
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

