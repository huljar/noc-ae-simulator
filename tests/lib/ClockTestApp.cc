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

using namespace omnetpp;

namespace HaecCommTest {

class ClockTestApp : public cSimpleModule, public cListener {
protected:
    virtual void initialize() override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
};

Define_Module(ClockTestApp);

void ClockTestApp::initialize() {
    getSimulation()->getSystemModule()->subscribe("clock", this);
}

void ClockTestApp::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        EV << "Clock cycle " << l << std::endl;
    }
    else {
        throw cRuntimeError(this, "Unexpected signal!");
    }
}

} //namespace

