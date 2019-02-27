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

class AMSplitUCTestApp : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
};

Define_Module(AMSplitUCTestApp);

void AMSplitUCTestApp::initialize() {
    // Test unmodified split pair
    Flit* f1 = MessageFactory::createFlit("flit1", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_1, 123);
    Flit* f2 = MessageFactory::createFlit("flit2", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_2, 123);

    send(f1, "netOut");
    send(f2, "netOut");

    // Test modified split pair
    Flit* f3 = MessageFactory::createFlit("flit3", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_1, 666);
    Flit* f4 = MessageFactory::createFlit("flit4", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_2, 666);
    f4->setModified(true);

    sendDelayed(f3, SimTime(4, SIMTIME_NS), "netOut");
    sendDelayed(f4, SimTime(4, SIMTIME_NS), "netOut");

    // Test split pair with bit error
    Flit* f5 = MessageFactory::createFlit("flit5", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_1, 777);
    Flit* f6 = MessageFactory::createFlit("flit6", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_2, 777);
    f5->setBitError(true);

    sendDelayed(f6, SimTime(6, SIMTIME_NS), "netOut");
    sendDelayed(f5, SimTime(6, SIMTIME_NS), "netOut");

    // Test split pair with both splits modified/biterrored
    Flit* f7 = MessageFactory::createFlit("flit7", Address2D(2, 3), Address2D(0, 0), MODE_SPLIT_1, 888);
    Flit* f8 = MessageFactory::createFlit("flit8", Address2D(2, 3), Address2D(0, 0), MODE_SPLIT_2, 888);
    f7->setBitError(true);
    f8->setModified(true);

    sendDelayed(f7, SimTime(16, SIMTIME_NS), "netOut");
    sendDelayed(f8, SimTime(16, SIMTIME_NS), "netOut");
}

void AMSplitUCTestApp::handleMessage(cMessage* msg) {
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
           << "FID " << f->getGidOrFid() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
           << "ARQ mode " << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(f->getUcArqs()) << std::endl;

        if(f->getGidOrFid() == 666) {
            // Answer to ARQ with unmodified split
            ASSERT(f->getUcArqs() == ARQ_SPLIT_2);

            Flit* f4 = MessageFactory::createFlit("flit4-new", Address2D(3, 3), Address2D(0, 0), MODE_SPLIT_2, 666);
            sendDelayed(f4, SimTime(2, SIMTIME_NS), "netOut");
        }
        else if(f->getGidOrFid() == 888) {
            // Answer to ARQ with unmodified splits
            ASSERT(f->getUcArqs() == ARQ_SPLITS_BOTH);

            Flit* f7 = MessageFactory::createFlit("flit7-new", Address2D(2, 3), Address2D(0, 0), MODE_SPLIT_1, 888);
            Flit* f8 = MessageFactory::createFlit("flit8-new", Address2D(2, 3), Address2D(0, 0), MODE_SPLIT_2, 888);

            sendDelayed(f7, SimTime(2, SIMTIME_NS), "netOut");
            sendDelayed(f8, SimTime(6, SIMTIME_NS), "netOut");
        }
    }
    else
        throw cRuntimeError(this, "Unexpected arrival gate: %s", f->getArrivalGate()->getName());

    delete f;
}

} //namespace

