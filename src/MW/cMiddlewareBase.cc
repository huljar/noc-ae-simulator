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

#include "cMiddlewareBase.h"

namespace HaecComm {

cMiddlewareBase::cMiddlewareBase() {
    this->isClocked = false;
    this->locallyClocked = false;
    this->currentCycle = 0;
    q = new cQueue();
}

cMiddlewareBase::~cMiddlewareBase() {
    delete(q);
}

void cMiddlewareBase::handleCycle(){
    // handle clock tick
}

void cMiddlewareBase::handleCycle(cMessage *msg){
    // handle clock tick with pending message
}

void cMiddlewareBase::handleMessageInternal(cMessage *msg) {
    // handle message directly (no local input clocking)
}

void cMiddlewareBase::initialize() {
    if (getAncestorPar("isClocked")) {
        this->isClocked = true;
        getSimulation()->getSystemModule()->subscribe("clock", this);
    }

    if (par("inputClocked")) {
        this->locallyClocked = true;
    }

    HaecModule *parent = dynamic_cast<HaecModule *>(getParentModule());

    parentId = par("parentId");
    X = parent->getX();
    Y = parent->getY();

    q->clear();
}
void cMiddlewareBase::receiveSignal(cComponent *source, simsignal_t id,
        unsigned long l, cObject *details) {
    if (id == registerSignal("clock")) {
        // this is a tick
        this->currentCycle = l;
        if (q->isEmpty()) {
            handleCycle();
        } else {
            handleCycle((cMessage *) q->pop());
        }
    }
}

void cMiddlewareBase::handleMessage(cMessage *msg) {
    if (locallyClocked) {
        // enqueue until signal
        q->insert(msg);
    } else {
        handleMessageInternal(msg);
    }
}

cMessage* cMiddlewareBase::createMessage(const char* name) {
    cMessage *lol = new cMessage(name);
    take(lol);
    lol->addPar("outPort");
    return lol;
}

} /* namespace HaecComm */
