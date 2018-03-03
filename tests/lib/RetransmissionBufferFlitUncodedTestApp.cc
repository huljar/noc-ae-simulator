#include <omnetpp.h>
#include <Messages/Flit.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class RetransmissionBufferFlitUncodedTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    int count;
};

Define_Module(RetransmissionBufferFlitUncodedTestApp);

void RetransmissionBufferFlitUncodedTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    count = 1;

    // Send 2 data/mac flit pairs into the buffer
    Flit* f1 = new Flit("data1");
    take(f1);
    f1->setSource(Address2D(3, 3));
    f1->setTarget(Address2D(0, 0));
    f1->setGidOrFid(123);
    f1->setMode(MODE_DATA);

    Flit* f2 = f1->dup();
    take(f2);
    f2->setName("mac1");
    f2->setMode(MODE_MAC);

    send(f1, "dataOut");
    send(f2, "dataOut");

    Flit* f3 = new Flit("data2");
    take(f3);
    f3->setSource(Address2D(3, 3));
    f3->setTarget(Address2D(1, 3));
    f3->setGidOrFid(234);
    f3->setMode(MODE_DATA);

    Flit* f4 = f3->dup();
    take(f4);
    f4->setName("mac2");
    f4->setMode(MODE_MAC);

    send(f3, "dataOut");
    send(f4, "dataOut");
}

void RetransmissionBufferFlitUncodedTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    EV << count++ << " Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "FID " << f->getGidOrFid() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;

    delete f;
}

void RetransmissionBufferFlitUncodedTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Send ARQ for FID 123, both flits
            Flit* arq1 = new Flit("arq1");
            take(arq1);
            arq1->setSource(Address2D(0, 0));
            arq1->setTarget(Address2D(3, 3));
            arq1->setGidOrFid(123);
            arq1->setMode(MODE_ARQ_TELL_MISSING);
            arq1->setUcArqs(ARQ_DATA_MAC);

            // Send ARQ from other received, there should not be an answer
            Flit* arq2 = arq1->dup();
            take(arq2);
            arq2->setName("arq2");
            arq2->setSource(Address2D(1, 0));

            // Send ARQ for wrong ID, there should not be an answer
            Flit* arq3 = arq1->dup();
            take(arq3);
            arq3->setName("arq3");
            arq3->setGidOrFid(124);

            send(arq1, "arqOut");
            send(arq2, "arqOut");
            send(arq3, "arqOut");
        }
        else if(l == 3) {
            // Add another data/mac pair to the buffer
            Flit* f1 = new Flit("data3");
            take(f1);
            f1->setSource(Address2D(3, 3));
            f1->setTarget(Address2D(2, 2));
            f1->setGidOrFid(345);
            f1->setMode(MODE_DATA);

            Flit* f2 = f1->dup();
            take(f2);
            f2->setName("mac3");
            f2->setMode(MODE_MAC);

            send(f1, "dataOut");
            send(f2, "dataOut");
        }
        else if(l == 4) {
            // Send another ARQ for first pair, there should not be an answer
            Flit* arq1 = new Flit("arq1");
            take(arq1);
            arq1->setSource(Address2D(0, 0));
            arq1->setTarget(Address2D(3, 3));
            arq1->setGidOrFid(123);
            arq1->setMode(MODE_ARQ_TELL_MISSING);
            arq1->setUcArqs(ARQ_DATA_MAC);

            send(arq1, "arqOut");
        }
        else if(l == 5) {
            // Send ARQ for first MAC, this should still be in buffer
            Flit* arq1 = new Flit("arq1");
            take(arq1);
            arq1->setSource(Address2D(0, 0));
            arq1->setTarget(Address2D(3, 3));
            arq1->setGidOrFid(123);
            arq1->setMode(MODE_ARQ_TELL_MISSING);
            arq1->setUcArqs(ARQ_MAC);

            send(arq1, "arqOut");
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

