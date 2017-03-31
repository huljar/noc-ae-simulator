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

#include "NetworkCodingAuthGen.h"

namespace HaecComm {

Define_Module(NetworkCodingAuthGen);

void NetworkCodingAuthGen::initialize() {
    cMiddlewareBase::initialize();

    NC = new NetworkCodingManager(par("generationSize"), par("combinations"));
    CU = new CryptoManager(par("cryptoUnits"), par("cryptoCycles"), NULL);
}

void NetworkCodingAuthGen::handleCycle(cMessage *msg) {
    CU->tick();

    // processed messages will now be pushed into the local queue
}

void NetworkCodingAuthGen::handleMessageInternal(cMessage *msg) {
    if ((int) msg->par("inPort") == 0) {        // Message from router
        NcCombination *msgNc = check_and_cast<NcCombination *>(msg);
        NcGen *g = NC->getOrCreateGeneration(msgNc->getGenerationId());

        g->addCombination(msgNc);
        if (g->decode()) {
            for (int i = g->messages->size(); i > 0; --i) {
                cMessage *m = (cMessage *) g->messages->get(i - 1)->dup();
                m->addPar("outPort");
                m->par("outPort") = 1; // send to app
                send(m, "out");
            }
            // the generation is done
            NC->deleteGeneration(g->getId());
        }
    } else if ((int) msg->par("inPort") == 1) { // Message from app
        NcGen *g = NC->getOrCreateGeneration();
        g->addMessage(msg);
        if (g->code()) {
            for (int i = g->combinations->size(); i > 0; --i) {
                cMessage *m = (cMessage *) g->combinations->get(i - 1)->dup();
                m->addPar("outPort");
                m->par("outPort") = 0; // send to router
                send(m, "out");
            }
        }
    } else {
        throw cRuntimeError(this, "Received msg %s with stupid inPort %d",
                msg->getName(), msg->par("inPort"));
    }
}

} //namespace
