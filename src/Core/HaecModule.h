//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __HAECCOMM_HAECMOD_H
#define __HAECCOMM_HAECMOD_H

#include <omnetpp.h>

using namespace omnetpp;

namespace HaecComm {

/**
 * Base class for all network components
 */
class HaecModule: public cSimpleModule, public cListener {
public:
    HaecModule();
    virtual ~HaecModule();
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l,
            cObject* details);
    int getNodeId() const;
    int getNodeX() const;
    int getNodeY() const;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage* msg);

    void createMiddleware();
    bool processQueue(cPacketQueue* queue, const char* targetGate, int targetGateIndex = -1);

    bool isClocked;

private:
    int id, X, Y;
};

}; // namespace

#endif
