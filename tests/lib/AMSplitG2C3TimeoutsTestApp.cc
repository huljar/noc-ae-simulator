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

class AMSplitG2C3TimeoutsTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(AMSplitG2C3TimeoutsTestApp);

void AMSplitG2C3TimeoutsTestApp::initialize() {
    // Test single split
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 123, 1, NC_G2C3, {24, 24});
    send(f1, "netOut");

    // Test two splits (no ARQ triggered)
    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(2, 2), Address2D(0, 0), MODE_SPLIT_NC, 123, 2, NC_G2C3, {26, 26});
    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(2, 2), Address2D(0, 0), MODE_SPLIT_NC, 123, 5, NC_G2C3, {26, 26});

    sendDelayed(f2, SimTime(2, SIMTIME_NS), "netOut");
    sendDelayed(f3, SimTime(8, SIMTIME_NS), "netOut");

    // Test two partially modified splits (earlier one is modified)
    Flit* f4 = MessageFactory::createFlit("flit4", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 222, 1, NC_G2C3, {27, 27});
    f4->setModified(true);

    Flit* f5 = MessageFactory::createFlit("flit5", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 222, 42, NC_G2C3, {27, 27});

    sendDelayed(f4, SimTime(4, SIMTIME_NS), "netOut");
    sendDelayed(f5, SimTime(10, SIMTIME_NS), "netOut");

    // Test three partially modified splits (middle one is modified)
    Flit* f6 = MessageFactory::createFlit("flit6", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 333, 1, NC_G2C3, {28, 28});

    Flit* f7 = MessageFactory::createFlit("flit7", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 333, 42, NC_G2C3, {28, 28});
    f7->setModified(true);

    Flit* f8 = MessageFactory::createFlit("flit8", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_NC, 333, 88, NC_G2C3, {28, 28});

    sendDelayed(f6, SimTime(22, SIMTIME_NS), "netOut");
    sendDelayed(f7, SimTime(30, SIMTIME_NS), "netOut");
    sendDelayed(f8, SimTime(34, SIMTIME_NS), "netOut");
}

void AMSplitG2C3TimeoutsTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    // Error if anything arrives targeting the network
    if(strcmp(f->getArrivalGate()->getName(), "retBufIn") == 0)
        throw cRuntimeError(this, "Received flit going to the retransmission buffer!");

    if(strcmp(f->getArrivalGate()->getName(), "appIn") == 0) {
        EV << "Got flit " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "FID " << f->getGidOrFid() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;
    }
    else if(strcmp(f->getArrivalGate()->getName(), "arqIn") == 0) {
        EV << "Got ARQ " << f->getName() << std::endl
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

} //namespace

