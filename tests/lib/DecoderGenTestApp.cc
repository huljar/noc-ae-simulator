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

class DecoderGenTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(DecoderGenTestApp);

void DecoderGenTestApp::initialize() {
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(0, 0), Address2D(1, 1), MODE_DATA, 123, 5, NC_G2C3, {222, 333});
    take(f1);

    Flit* f2 = f1->dup();
    f2->setName("flit2");
    f2->setGev(8);

    send(f1, "out");
    send(f2, "out");

    Flit* mac12 = MessageFactory::createFlit("mac12", Address2D(0, 0), Address2D(1, 1), MODE_MAC, 123, 0, NC_G2C3);
    sendDelayed(mac12, SimTime(2, SIMTIME_NS), "out");
}

void DecoderGenTestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);
    EV << "Got flit " << f->getName() << std::endl
       << "Source " << f->getSource() << std::endl
       << "Target " << f->getTarget() << std::endl
       << "FID " << f->getGidOrFid() << std::endl;
    if(f->getOriginalIdsArraySize() > 0)
        EV << "OrigID0 " << f->getOriginalIds(0) << std::endl;
    else
        EV << "No OrigID0" << std::endl;
    delete f;
}

} //namespace

