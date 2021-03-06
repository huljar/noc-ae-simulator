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

class ExitGuardGenTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(ExitGuardGenTestApp);

void ExitGuardGenTestApp::initialize() {
    // Send some flits from the crypto units
    Flit* enc1 = MessageFactory::createFlit("enc1", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 121);
    enc1->setStatus(STATUS_ENCRYPTING);
    Flit* enc2 = MessageFactory::createFlit("enc2", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 122);
    enc2->setStatus(STATUS_ENCRYPTING);
    Flit* enc3 = MessageFactory::createFlit("enc3", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 123);
    enc3->setStatus(STATUS_ENCRYPTING);

    send(enc1, "encUnits", 0);
    send(enc2, "encUnits", 0);
    send(enc3, "encUnits", 0);

    Flit* auth1 = MessageFactory::createFlit("auth1", Address2D(0, 0), Address2D(1, 1), MODE_MAC, 131);
    auth1->setStatus(STATUS_AUTHENTICATING);
    send(auth1, "authUnits", 2);
}

void ExitGuardGenTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << " at " << f->getArrivalGate()->getFullName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "FID " << f->getGidOrFid() << std::endl
       << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl;
    delete f;
}

} //namespace

