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

#ifndef MW_MIDDLEWAREBASE_H_
#define MW_MIDDLEWAREBASE_H_

#include <omnetpp.h>
#include <Core/HaecModule.h>

using namespace omnetpp;

namespace HaecComm {

class MiddlewareBase: public cSimpleModule, public cListener {
public:
    MiddlewareBase();
    virtual ~MiddlewareBase();

protected:
    bool isClocked;
    bool locallyClocked;
    int parentId, X, Y, queueLength;
    unsigned long currentCycle;
    cQueue *q;

    // If you override one of these, call the parent method as first operation!
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void receiveSignal(cComponent *, simsignal_t signalID, unsigned long l,
                cObject *);

    // Override these for your functionality
    virtual void handleCycle(cMessage *msg);
    virtual void handleMessageInternal(cMessage *msg);

    // Convenience stuff
    cMessage* createMessage(const char*);

};

} /* namespace HaecComm */

#endif /* MW_MIDDLEWAREBASE_H_ */
