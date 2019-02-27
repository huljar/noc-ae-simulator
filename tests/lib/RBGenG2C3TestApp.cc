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
#include <Util/Constants.h>

using namespace omnetpp;
using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecCommTest {

class RBGenG2C3TestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    int count;
};

Define_Module(RBGenG2C3TestApp);

void RBGenG2C3TestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    count = 1;

    // Send one complete, two incomplete generations into the buffer
    Flit* f1 = MessageFactory::createFlit("data11", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 1, NC_G2C3);
    Flit* f2 = MessageFactory::createFlit("data12", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 2, NC_G2C3);
    Flit* f3 = MessageFactory::createFlit("data13", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 3, NC_G2C3);
    Flit* f4 = MessageFactory::createFlit("mac1", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, Constants::GEN_MAC_GEV, NC_G2C3);

    send(f1, "dataOut");
    send(f2, "dataOut");
    send(f3, "dataOut");
    send(f4, "dataOut");

    Flit* f5 = MessageFactory::createFlit("data21", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 124, 1, NC_G2C3);
    Flit* f6 = MessageFactory::createFlit("data22", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 124, 2, NC_G2C3);

    send(f5, "dataOut");
    send(f6, "dataOut");

    // Using MAC GEV for data flit just to test
    Flit* f7 = MessageFactory::createFlit("data31", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 125, 1, NC_G2C3);
    Flit* f8 = MessageFactory::createFlit("mac3", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 125, Constants::GEN_MAC_GEV, NC_G2C3);

    send(f7, "dataOut");
    send(f8, "dataOut");
}

void RBGenG2C3TestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    EV << count++ << " Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "GID " << f->getGidOrFid() << std::endl
       << "GEV " << f->getGev() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;

    delete f;
}

void RBGenG2C3TestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        if(l == 10) {
            // Send ARQ for specific flits (TELL_MISSING) without MAC (9+10)
            Flit* arq1 = MessageFactory::createFlit("arq1", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123, 0, NC_G2C3);
            take(arq1);
            arq1->setNcArqs(GevArqMap{{2, ARQ_DATA}, {3, ARQ_DATA}});

            send(arq1, "arqOut");
        }
        else if(l == 13) {
            // Send ARQ for specific flits (TELL_MISSING) with MAC (11+12)
            Flit* arq1 = MessageFactory::createFlit("arq1", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123, 0, NC_G2C3);
            take(arq1);
            arq1->setNcArqs(GevArqMap{{1, ARQ_DATA}});
            arq1->setNcArqGenMac(true);

            send(arq1, "arqOut");
        }
        else if(l == 14) {
            // Send ARQ for specific flits (TELL_MISSING) but for flits partially not in the buffer
            Flit* arq2 = MessageFactory::createFlit("arq2", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 124, 0, NC_G2C3);
            take(arq2);
            arq2->setNcArqs(GevArqMap{{2, ARQ_DATA}, {3, ARQ_DATA}});

            send(arq2, "arqOut");
        }
        else if(l == 15) {
            // Send ARQ for specific flits (TELL_MISSING) but for flits partially not in the buffer (MAC not in buffer)
            Flit* arq2 = MessageFactory::createFlit("arq2", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 124, 0, NC_G2C3);
            take(arq2);
            arq2->setNcArqs(GevArqMap{{1, ARQ_DATA}});
            arq2->setNcArqGenMac(true);

            send(arq2, "arqOut");
        }
        else if(l == 18) {
            // Send ARQ from other source (should not get an answer)
            Flit* arq2 = MessageFactory::createFlit("arq22", Address2D(1, 0), Address2D(3, 3), MODE_ARQ_TELL_MISSING, 123, 0, NC_G2C3);
            take(arq2);
            arq2->setNcArqs(GevArqMap{{2, ARQ_DATA}});
            arq2->setNcArqGenMac(true);

            send(arq2, "arqOut");
        }
        else if(l == 20) {
            // Send ARQ for unknown flits (TELL_RECEIVED), there should not be an answer (MAC not in buffer)
            Flit* arq3 = MessageFactory::createFlit("arq3", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 124, 0, NC_G2C3);
            take(arq3);
            arq3->setNcArqs(GevArqMap{{1, ARQ_DATA}, {3, ARQ_DATA}});

            send(arq3, "arqOut");
        }
        else if(l == 25) {
            // Send ARQ for unknown flits (TELL_RECEIVED) (13+14)
            Flit* arq3 = MessageFactory::createFlit("arq33", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 125, 0, NC_G2C3);
            take(arq3);
            arq3->setNcArqs(GevArqMap{{2, ARQ_DATA}, {12, ARQ_DATA}});

            send(arq3, "arqOut");
        }
        else if(l == 28) {
            // Send ARQ for unknown flits (TELL_RECEIVED), there should not be an answer (not enough data flits in buffer)
            Flit* arq4 = MessageFactory::createFlit("arq4", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 125, 0, NC_G2C3);
            take(arq4);
            arq4->setNcArqs(GevArqMap{{1, ARQ_DATA}, {2, ARQ_DATA}});

            send(arq4, "arqOut");
        }
        else if(l == 30) {
            // Send ARQ for unknown flits (TELL_RECEIVED), there should not be an answer (not enough data flits in buffer)
            Flit* arq4 = MessageFactory::createFlit("arq4", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 125, 0, NC_G2C3);
            take(arq4);
            arq4->setNcArqs(GevArqMap{{1, ARQ_DATA}, {2, ARQ_DATA}});
            arq4->setNcArqGenMac(true);

            send(arq4, "arqOut");
        }
        else if(l == 32) {
            // Send ARQ for unknown flits (TELL_RECEIVED) (15)
            Flit* arq4 = MessageFactory::createFlit("arq4", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 124, 0, NC_G2C3);
            take(arq4);
            arq4->setNcArqs(GevArqMap{{0, ARQ_DATA}, {2, ARQ_DATA}});
            arq4->setNcArqGenMac(true);

            send(arq4, "arqOut");
        }
        else if(l == 35) {
            // Send ARQ for unknown flits (TELL_RECEIVED) (16-18)
            Flit* arq4 = MessageFactory::createFlit("arq4", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 123, 0, NC_G2C3);
            take(arq4);
            arq4->setNcArqs(GevArqMap());
            arq4->setNcArqGenMac(true);

            send(arq4, "arqOut");
        }
        else if(l == 38) {
            // Send ARQ for unknown flits (TELL_RECEIVED) (19-22)
            Flit* arq4 = MessageFactory::createFlit("arq4", Address2D(0, 0), Address2D(3, 3), MODE_ARQ_TELL_RECEIVED, 123, 0, NC_G2C3);
            take(arq4);
            arq4->setNcArqs(GevArqMap());

            send(arq4, "arqOut");
        }
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

