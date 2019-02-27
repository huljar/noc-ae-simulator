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

class EntryGuardGenTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(EntryGuardGenTestApp);

void EntryGuardGenTestApp::initialize() {
    // Send some flits into the 3 queues
    Flit* app1 = MessageFactory::createFlit("app1", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 121);
    Flit* app2 = MessageFactory::createFlit("app2", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 122);

    send(app1, "appInputQueue");
    send(app2, "appInputQueue");

    Flit* exit1 = MessageFactory::createFlit("exit1", Address2D(0, 0), Address2D(2, 0), MODE_DATA, 131);
    Flit* exit2 = MessageFactory::createFlit("exit2", Address2D(0, 0), Address2D(2, 0), MODE_DATA, 132);
    Flit* exit3 = MessageFactory::createFlit("exit3", Address2D(0, 0), Address2D(2, 0), MODE_DATA, 133);
    Flit* exit4 = MessageFactory::createFlit("exit4", Address2D(0, 0), Address2D(2, 0), MODE_DATA, 134);

    send(exit1, "exitInputQueue");
    send(exit2, "exitInputQueue");
    send(exit3, "exitInputQueue");
    send(exit4, "exitInputQueue");

    Flit* net1 = MessageFactory::createFlit("net1", Address2D(1, 1), Address2D(0, 0), MODE_DATA, 141);
    Flit* net2 = MessageFactory::createFlit("net2", Address2D(1, 1), Address2D(0, 0), MODE_DATA, 142);
    Flit* net3 = MessageFactory::createFlit("net3", Address2D(1, 2), Address2D(0, 0), MODE_DATA, 143);
    Flit* net4 = MessageFactory::createFlit("net4", Address2D(1, 2), Address2D(0, 0), MODE_DATA, 144);

    send(net1, "netInputQueue");
    send(net2, "netInputQueue");
    send(net3, "netInputQueue");
    send(net4, "netInputQueue");
}

void EntryGuardGenTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << " at " << f->getArrivalGate()->getFullName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "FID " << f->getGidOrFid() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;
    delete f;
}

} //namespace

