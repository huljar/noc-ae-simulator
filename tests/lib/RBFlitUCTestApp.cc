//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <omnetpp.h>
#include <Messages/Flit.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class RBFlitUCTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    int count;
};

Define_Module(RBFlitUCTestApp);

void RBFlitUCTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    count = 1;

    // Send 2 data/mac flit pairs into the buffer
    Flit* f1 = MessageFactory::createFlit("data1", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123);
    take(f1);

    Flit* f2 = MessageFactory::createFlit("mac1", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123);
    take(f2);

    send(f1, "dataOut");
    send(f2, "dataOut");

    Flit* f3 = MessageFactory::createFlit("data2", Address2D(3, 3), Address2D(1, 3), MODE_DATA, 234);
    take(f3);

    Flit* f4 = MessageFactory::createFlit("mac2", Address2D(3, 3), Address2D(1, 3), MODE_MAC, 234);
    take(f4);

    send(f3, "dataOut");
    send(f4, "dataOut");
}

void RBFlitUCTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    EV << count++ << " Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "FID " << f->getGidOrFid() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;

    delete f;
}

void RBFlitUCTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Send ARQ for FID 123, both flits
            Flit* arq1 = MessageFactory::createFlit("arq1", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123);
            take(arq1);
            arq1->setUcArqs(ARQ_DATA_MAC);

            // Send ARQ from other received, there should not be an answer
            Flit* arq2 = MessageFactory::createFlit("arq2", Address2D(1, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123);
            take(arq2);
            arq2->setUcArqs(ARQ_DATA_MAC);

            // Send ARQ for wrong ID, there should not be an answer
            Flit* arq3 = MessageFactory::createFlit("arq3", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 124);
            take(arq3);
            arq3->setUcArqs(ARQ_DATA_MAC);

            send(arq1, "arqOut");
            send(arq2, "arqOut");
            send(arq3, "arqOut");
        }
        else if(l == 3) {
            // Add another data/mac pair to the buffer
            Flit* f1 = MessageFactory::createFlit("data3", Address2D(3, 3), Address2D(2, 2), MODE_DATA, 345);
            take(f1);

            Flit* f2 = MessageFactory::createFlit("mac3", Address2D(3, 3), Address2D(2, 2), MODE_MAC, 345);
            take(f2);

            send(f1, "dataOut");
            send(f2, "dataOut");
        }
        else if(l == 4) {
            // Send another ARQ for first pair, there should not be an answer
            Flit* arq1 = MessageFactory::createFlit("arq1", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123);
            take(arq1);
            arq1->setUcArqs(ARQ_DATA_MAC);

            send(arq1, "arqOut");
        }
        else if(l == 5) {
            // Send ARQ for first MAC, this should still be in buffer
            Flit* arq1 = MessageFactory::createFlit("arq1", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123);
            take(arq1);
            arq1->setUcArqs(ARQ_MAC);

            send(arq1, "arqOut");
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

