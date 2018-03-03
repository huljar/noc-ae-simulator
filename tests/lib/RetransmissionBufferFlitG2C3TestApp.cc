#include <omnetpp.h>
#include <Messages/Flit.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class RetransmissionBufferFlitG2C3TestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    int count;
};

Define_Module(RetransmissionBufferFlitG2C3TestApp);

void RetransmissionBufferFlitG2C3TestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    count = 1;

    // Send 2 data/mac combinations into the buffer
    Flit* f1 = new Flit("data1");
    take(f1);
    f1->setSource(Address2D(3, 3));
    f1->setTarget(Address2D(0, 0));
    f1->setGidOrFid(123);
    f1->setGev(2);
    f1->setMode(MODE_DATA);
    f1->setNcMode(NC_G2C3);

    Flit* f2 = f1->dup();
    take(f2);
    f2->setName("mac1");
    f2->setMode(MODE_MAC);

    Flit* f3 = f1->dup();
    take(f3);
    f3->setName("data2");
    f3->setGev(19);

    Flit* f4 = f3->dup();
    take(f4);
    f4->setName("mac2");
    f4->setMode(MODE_MAC);

    send(f1, "dataOut");
    send(f2, "dataOut");
    send(f3, "dataOut");
    send(f4, "dataOut");
}

void RetransmissionBufferFlitG2C3TestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    EV << count++ << " Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "GID " << f->getGidOrFid() << std::endl
       << "GEV " << f->getGev() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;

    delete f;
}

void RetransmissionBufferFlitG2C3TestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Send ARQ for specific flits (TELL_MISSING)
            Flit* arq1 = new Flit("arq1");
            take(arq1);
            arq1->setSource(Address2D(0, 0));
            arq1->setTarget(Address2D(3, 3));
            arq1->setGidOrFid(123);
            arq1->setMode(MODE_ARQ_TELL_MISSING);
            arq1->setNcMode(NC_G2C3);
            arq1->setNcArqs(GevArqMap{{2, ARQ_MAC}, {19, ARQ_DATA_MAC}});

            send(arq1, "arqOut");
        }
        else if(l == 3) {
            // Send ARQ for specific flits (TELL_MISSING) but for flits partially not in the buffer
            Flit* arq2 = new Flit("arq2");
            take(arq2);
            arq2->setSource(Address2D(0, 0));
            arq2->setTarget(Address2D(3, 3));
            arq2->setGidOrFid(123);
            arq2->setMode(MODE_ARQ_TELL_MISSING);
            arq2->setNcMode(NC_G2C3);
            arq2->setNcArqs(GevArqMap{{2, ARQ_MAC}, {19, ARQ_DATA_MAC}, {12, ARQ_DATA}});

            send(arq2, "arqOut");
        }
        else if(l == 8) {
            // Send ARQ from other source (should no get an answer)
            Flit* arq2 = new Flit("arq22");
            take(arq2);
            arq2->setSource(Address2D(1, 0));
            arq2->setTarget(Address2D(3, 3));
            arq2->setGidOrFid(123);
            arq2->setMode(MODE_ARQ_TELL_MISSING);
            arq2->setNcMode(NC_G2C3);
            arq2->setNcArqs(GevArqMap{{2, ARQ_MAC}, {19, ARQ_DATA_MAC}, {12, ARQ_DATA}});

            send(arq2, "arqOut");
        }
        else if(l == 10) {
            // Send ARQ for unknown flits (TELL_RECEIVED), there should not be an answer (12/MODE_MAC not in buffer)
            Flit* arq3 = new Flit("arq3");
            take(arq3);
            arq3->setSource(Address2D(0, 0));
            arq3->setTarget(Address2D(3, 3));
            arq3->setGidOrFid(123);
            arq3->setMode(MODE_ARQ_TELL_RECEIVED);
            arq3->setNcMode(NC_G2C3);
            arq3->setNcArqs(GevArqMap{{2, ARQ_MAC}, {19, ARQ_DATA_MAC}, {12, ARQ_DATA}});

            send(arq3, "arqOut");
        }
        else if(l == 15) {
            // Send ARQ for unknown flits (TELL_RECEIVED), there should not be an answer (12/MODE_DATA not in buffer)
            Flit* arq3 = new Flit("arq33");
            take(arq3);
            arq3->setSource(Address2D(0, 0));
            arq3->setTarget(Address2D(3, 3));
            arq3->setGidOrFid(123);
            arq3->setMode(MODE_ARQ_TELL_RECEIVED);
            arq3->setNcMode(NC_G2C3);
            arq3->setNcArqs(GevArqMap{{2, ARQ_DATA_MAC}, {12, ARQ_MAC}});

            send(arq3, "arqOut");
        }
        else if(l == 20) {
            // Send ARQ for unknown flits (TELL_RECEIVED)
            Flit* arq4 = new Flit("arq4");
            take(arq4);
            arq4->setSource(Address2D(0, 0));
            arq4->setTarget(Address2D(3, 3));
            arq4->setGidOrFid(123);
            arq4->setMode(MODE_ARQ_TELL_RECEIVED);
            arq4->setNcMode(NC_G2C3);
            arq4->setNcArqs(GevArqMap{{2, ARQ_MAC}, {12, ARQ_DATA_MAC}});

            send(arq4, "arqOut");
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

