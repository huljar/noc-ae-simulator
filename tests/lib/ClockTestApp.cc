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

