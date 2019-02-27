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

class RBSplitG2C3TestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    int count;
};

Define_Module(RBSplitG2C3TestApp);

void RBSplitG2C3TestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    count = 1;

    // Send several splits into the buffer
    Flit* f1 = MessageFactory::createFlit("split11", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 123, 2, NC_G2C3);
    Flit* f2 = MessageFactory::createFlit("split12", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 123, 3, NC_G2C3);
    Flit* f3 = MessageFactory::createFlit("split13", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 123, 4, NC_G2C3);
    Flit* f4 = MessageFactory::createFlit("split21", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 124, 1, NC_G2C3);

    send(f1, "dataOut");
    send(f2, "dataOut");
    send(f3, "dataOut");
    send(f4, "dataOut");
}

void RBSplitG2C3TestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    EV << count++ << " Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "GID " << f->getGidOrFid() << std::endl
       << "GEV " << f->getGev() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;

    delete f;
}

void RBSplitG2C3TestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 2) {
            // Send ARQ for specific flits (TELL_MISSING)
            Flit* arq1 = MessageFactory::createFlit("arq1", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123, 0, NC_G2C3);
            take(arq1);
            arq1->setNcArqs(GevArqMap{{2, ARQ_SPLIT_NC}, {4, ARQ_SPLIT_NC}});

            send(arq1, "arqOut");
        }
        else if(l == 3) {
            // Send ARQ for specific flits (TELL_MISSING) but for flits partially not in the buffer
            Flit* arq2 = MessageFactory::createFlit("arq2", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 124, 0, NC_G2C3);
            take(arq2);
            arq2->setNcArqs(GevArqMap{{1, ARQ_SPLIT_NC}, {2, ARQ_SPLIT_NC}});

            send(arq2, "arqOut");
        }
        else if(l == 8) {
            // Send ARQ from other source (should no get an answer)
            Flit* arq2 = MessageFactory::createFlit("arq22", Address2D(1, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123, 0, NC_G2C3);
            take(arq2);
            arq2->setNcArqs(GevArqMap{{2, ARQ_SPLIT_NC}});

            send(arq2, "arqOut");
        }
        else if(l == 10) {
            // Send ARQ for unknown flits (TELL_RECEIVED), there should not be an answer (not enough flits found)
            Flit* arq3 = MessageFactory::createFlit("arq3", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 124, 0, NC_G2C3);
            take(arq3);
            arq3->setNcArqs(GevArqMap{{1, ARQ_SPLIT_NC}});

            send(arq3, "arqOut");
        }
        else if(l == 15) {
            // Send ARQ for unknown flits (TELL_RECEIVED)
            Flit* arq3 = MessageFactory::createFlit("arq33", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 123, 0, NC_G2C3);
            take(arq3);
            arq3->setNcArqs(GevArqMap());

            send(arq3, "arqOut");
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

