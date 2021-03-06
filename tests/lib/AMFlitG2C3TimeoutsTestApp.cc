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
#include <Core/Clock.h>
#include <Messages/MessageFactory.h>

using namespace omnetpp;
using namespace HaecComm::Core;
using namespace HaecComm::Messages;

namespace HaecCommTest {

class AMFlitG2C3TimeoutsTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    Clock* clock;
};

Define_Module(AMFlitG2C3TimeoutsTestApp);

void AMFlitG2C3TimeoutsTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
    clock = dynamic_cast<Clock*>(getSimulation()->getSystemModule()->getSubmodule("clock"));
    ASSERT(clock != nullptr);

    // Test single flit
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 1, NC_G2C3, {34, 35});
    send(f1, "netOut");

    // Test two flits with different GEVs
    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(2, 2), Address2D(0, 0), MODE_MAC, 123, 2, NC_G2C3, {34, 35});
    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(2, 2), Address2D(0, 0), MODE_DATA, 123, 5, NC_G2C3, {34, 35});

    sendDelayed(f2, SimTime(2, SIMTIME_NS), "netOut");
    sendDelayed(f3, SimTime(8, SIMTIME_NS), "netOut");

    // Test two modified pairs
    Flit* f4 = MessageFactory::createFlit("flit4", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 222, 1, NC_G2C3, {34, 35});
    f4->setModified(true);

    Flit* f5 = MessageFactory::createFlit("flit5", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 222, 1, NC_G2C3, {34, 35});
    Flit* f6 = MessageFactory::createFlit("flit6", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 222, 42, NC_G2C3, {34, 35});

    Flit* f7 = MessageFactory::createFlit("flit7", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 222, 42, NC_G2C3, {34, 35});
    f7->setModified(true);

    sendDelayed(f4, SimTime(10, SIMTIME_NS), "netOut");
    sendDelayed(f5, SimTime(12, SIMTIME_NS), "netOut");
    sendDelayed(f6, SimTime(16, SIMTIME_NS), "netOut");
    sendDelayed(f7, SimTime(22, SIMTIME_NS), "netOut");

    // Test loss with all GEVs known
    Flit* f8 = MessageFactory::createFlit("flit8", Address2D(1, 1), Address2D(0, 0), MODE_DATA, 42, 12, NC_G2C3, {34, 35});
    Flit* f9 = MessageFactory::createFlit("flit9", Address2D(1, 1), Address2D(0, 0), MODE_MAC, 42, 12, NC_G2C3, {34, 35});
    Flit* f10 = MessageFactory::createFlit("flit10", Address2D(1, 1), Address2D(0, 0), MODE_MAC, 42, 25, NC_G2C3, {34, 35});
    Flit* f11 = MessageFactory::createFlit("flit11", Address2D(1, 1), Address2D(0, 0), MODE_MAC, 42, 28, NC_G2C3, {34, 35});

    sendDelayed(f8, SimTime(18, SIMTIME_NS), "netOut");
    sendDelayed(f9, SimTime(20, SIMTIME_NS), "netOut");
    sendDelayed(f10, SimTime(28, SIMTIME_NS), "netOut");
    sendDelayed(f11, SimTime(34, SIMTIME_NS), "netOut");
}

void AMFlitG2C3TimeoutsTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    // Error if anything arrives targeting the network
    if(strcmp(f->getArrivalGate()->getName(), "retBufIn") == 0)
        throw cRuntimeError(this, "Received flit going to the retransmission buffer!");

    if(strcmp(f->getArrivalGate()->getName(), "appIn") == 0) {
        EV << clock->getCurrentCycle() << " Got flit " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "GID " << f->getGidOrFid() << std::endl
           << "GEV " << f->getGev() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;
    }
    else if(strcmp(f->getArrivalGate()->getName(), "arqIn") == 0) {
        EV << clock->getCurrentCycle() << " Got ARQ " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "GID " << f->getGidOrFid() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
           << "ARQ modes ";
        for(auto it = f->getNcArqs().begin(); it != f->getNcArqs().end(); ++it)
            EV << "(" << it->first << "," << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(it->second) << ")";
        EV << std::endl;
    }
    else
        throw cRuntimeError(this, "Unexpected arrival gate: %s", f->getArrivalGate()->getName());

    delete f;
}

void AMFlitG2C3TimeoutsTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {

    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

