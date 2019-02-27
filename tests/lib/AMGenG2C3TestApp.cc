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

class AMGenG2C3TestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    int count;
};

Define_Module(AMGenG2C3TestApp);

void AMGenG2C3TestApp::initialize() {
    count = 0;

    // Test unmodified generation
    Flit* d11 = MessageFactory::createFlit("data11", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 123, 123, NC_G2C3, {5, 6});

    Flit* d12 = d11->dup();
    d12->setName("data12");
    d12->setGev(124);

    Flit* m1 = MessageFactory::createFlit("mac1", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 123, 0, NC_G2C3);

    send(d11, "netOut");
    send(d12, "netOut");
    sendDelayed(m1, SimTime(2, SIMTIME_NS), "netOut");

    // Test partially modified generation (first data flit modified) and don't answer ARQ. Verification will still succeed with 2nd+3rd flit.
    Flit* d21 = MessageFactory::createFlit("data21", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 5, 1, NC_G2C3, {10, 11});

    Flit* d22 = d21->dup();
    d22->setName("data22");
    d22->setGev(2);

    Flit* d23 = d21->dup();
    d23->setName("data23");
    d23->setGev(3);

    d21->setModified(true);

    Flit* m2 = MessageFactory::createFlit("mac2", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 5, 9999, NC_G2C3);

    sendDelayed(d21, SimTime(10, SIMTIME_NS), "netOut");
    sendDelayed(m2,  SimTime(14, SIMTIME_NS), "netOut");
    sendDelayed(d22, SimTime(16, SIMTIME_NS), "netOut");
    sendDelayed(d23, SimTime(20, SIMTIME_NS), "netOut");

    // Test partially modified generation (second data flit modified) and answer ARQ. Verification will succeed with 1st+2nd flit.
    Flit* d31 = MessageFactory::createFlit("data31", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 6, 1, NC_G2C3, {15, 16});

    Flit* d32 = d31->dup();
    d32->setName("data32");
    d32->setGev(14);

    d32->setModified(true);

    Flit* m3 = MessageFactory::createFlit("mac3", Address2D(3, 3), Address2D(0, 0), MODE_MAC, 6, 0, NC_G2C3);

    sendDelayed(d31, SimTime(50, SIMTIME_NS), "netOut");
    sendDelayed(m3,  SimTime(54, SIMTIME_NS), "netOut");
    sendDelayed(d32, SimTime(56, SIMTIME_NS), "netOut");
}

void AMGenG2C3TestApp::handleMessage(cMessage* msg) {
    Flit* f = check_and_cast<Flit*>(msg);

    // Error if anything arrives targeting the network
    if(strcmp(f->getArrivalGate()->getName(), "retBufIn") == 0)
        throw cRuntimeError(this, "Received flit going to the retransmission buffer!");

    if(strcmp(f->getArrivalGate()->getName(), "appIn") == 0) {
        EV << ++count << " Got flit " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "FID " << f->getGidOrFid() << std::endl
           << "OldGID " << f->getOriginalIds(0) << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
           << "NC mode " << cEnum::get("HaecComm::Messages::NcMode")->getStringFor(f->getNcMode()) << std::endl;
    }
    else if(strcmp(f->getArrivalGate()->getName(), "arqIn") == 0) {
        EV << ++count << " Got ARQ " << f->getName() << std::endl
           << "Source " << f->getSource() << std::endl
           << "Target " << f->getTarget() << std::endl
           << "GID " << f->getGidOrFid() << std::endl
           << "GEV " << f->getGev() << std::endl
           << "Mode " << cEnum::get("HaecComm::Messages::Mode")->getStringFor(f->getMode()) << std::endl
           << "NC mode " << cEnum::get("HaecComm::Messages::NcMode")->getStringFor(f->getNcMode()) << std::endl;
        if(f->getNcArqs().empty())
            EV << "No ARQ modes";
        else {
            EV << "ARQ modes ";
            for(auto it = f->getNcArqs().begin(); it != f->getNcArqs().end(); ++it)
                EV << "(" << it->first << "," << cEnum::get("HaecComm::Messages::ArqMode")->getStringFor(it->second) << ")";
            }
        EV << std::endl
           << "ARQ MAC " << std::boolalpha << f->getNcArqGenMac() << std::noboolalpha << std::endl;

        if(f->getGidOrFid() == 6) {
            // Answer to ARQ with unmodified flits (not answering the MAC on purpose)
            Flit* d31 = MessageFactory::createFlit("data31-new", Address2D(3, 3), Address2D(0, 0), MODE_DATA, 6, 1, NC_G2C3, {15, 16});
            take(d31);

            Flit* d32 = d31->dup();
            take(d32);
            d32->setName("data32-new");
            d32->setGev(14);

            sendDelayed(d31, SimTime(2, SIMTIME_NS), "netOut");
            sendDelayed(d32, SimTime(4, SIMTIME_NS), "netOut");
        }
    }
    else
        throw cRuntimeError(this, "Unexpected arrival gate: %s", f->getArrivalGate()->getName());

    delete f;
}

} //namespace

