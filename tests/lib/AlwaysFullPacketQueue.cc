#include <omnetpp.h>
#include <Buffers/PacketQueueBase.h>

using namespace omnetpp;
using namespace HaecComm::Buffers;

namespace HaecCommTest {

class AlwaysFullPacketQueue : public PacketQueueBase {
protected:
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
};

Define_Module(AlwaysFullPacketQueue);

void AlwaysFullPacketQueue::handleMessage(cMessage* msg) {
    throw cRuntimeError(this, "Always full queue received a message!");
}

void AlwaysFullPacketQueue::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    PacketQueueBase::receiveSignal(source, signalID, l, details);

    if(signalID == registerSignal("clock") && l == 0) {
        EV_DEBUG << "Emitting qfull signal (true)" << std::endl;
        emit(qfullSignal, true);
    }
}

} //namespace
