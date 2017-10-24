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

//TODO refactor for in/out queues with fixed size

#include <Core/HaecModule.h>

namespace HaecComm {

Define_Module(HaecModule);

HaecModule::HaecModule() {}

HaecModule::~HaecModule() {}

void HaecModule::initMiddleware() {
    cModule *lastMW = NULL;
    std::vector<std::string> mws;
    this->addGate("MWinput", cGate::INPUT);

    cStringTokenizer tok(par("middleware").stringValue(), ",");

    while(tok.hasMoreTokens()){
        const char *mwName = tok.nextToken();
        cModuleType *moduleType = cModuleType::get(mwName);
        cModule *module = moduleType->create(mwName, this);

        module->par("parentId") = this->id;
        module->finalizeParameters();
        module->addGate("in",  cGate::INPUT);
        module->addGate("out", cGate::OUTPUT);

        if(!lastMW){
            // this is the first MW
            this->middlewareEntryGate = module->gate("in");
        } else {
            lastMW->gate("out")->connectTo(module->gate("in"));
        }

        if(!tok.hasMoreTokens()) {
            // this is the last MW
            module->gate("out")->connectTo(this->gate("MWinput"));
        }

        module->buildInside();
        module->scheduleStart(simTime());
        lastMW = module;
    }
}

void HaecModule::initialize() {
    id = par("id");
    X  = id % (int) getAncestorPar("columns");
    Y  = id / (int) getAncestorPar("columns");

    if (hasPar("middleware")) {
        initMiddleware();
    } else {
        throw cRuntimeError(this, "middleware par required");
    }

    this->isClocked = false;
    if (getAncestorPar("isClocked")) {
        this->isClocked = true;

        // setup queues for in & out ports
        for (int i = 0; i < gateSize("inPorts"); i++) {
            inQueues.addAt(i, new cQueue);
        }

        for (int i = 0; i < gateSize("outPorts"); i++) {
            outQueues.addAt(i, new cQueue);
        }

        // subscribe to clock signal
        getSimulation()->getSystemModule()->subscribe("clock", this);
    }
}

void HaecModule::receiveSignal(cComponent *, simsignal_t id, unsigned long l,
        cObject *details) {
    cMessage *tmp;
    // TODO create arbiter class for parameterized inqueue selection
    // round robin over all inPorts
    for (int i = 0; i < inQueues.size(); i++) {
        nextIn = (nextIn + 1) % inQueues.size();
        if (inQueues[nextIn]) {
            cQueue *q = (cQueue *) inQueues[nextIn];
            if (!q->isEmpty()) {
                tmp = (cMessage *) q->pop();
                take(tmp);
                sendDirect(tmp, this->middlewareEntryGate);
            }
        }
    }

    // round robin over aloutPorts (no state keeper needed)
    for (int i = 0; i < outQueues.size(); i++) {
        if (outQueues[i]) {
            cQueue *q = (cQueue *) outQueues[i];
            if (!q->isEmpty()) {
                tmp = (cMessage *) q->pop();
                take(tmp);
                send(tmp, gate("outPorts", i));
            }
        }
    }
}

void HaecModule::handleMessage(cMessage *msg) {
    if (msg->getArrivalGate() == this->gate("MWinput")) {
        // a message from the local middleware stack
        if (!msg->hasPar("outPort")) {
            EV << this->getFullName()
                      << " received local message without destination!"
                      << std::endl;
            return;
        }

        if (isClocked) {
            cQueue *q =
                    (cQueue *) outQueues[(int) msg->par("outPort").longValue()];
            q->insert(msg);
        } else {
            send(msg, gate(msg->par("outPort").longValue()));
        }
    } else if (msg->getArrivalGate()->getType() == cGate::INPUT) {
        // a message from a local input
        int inPortNr = msg->getArrivalGateId() - (gateBaseId("inPorts"));
        msg->addPar("inPort");
        msg->par("inPort").setLongValue((long) inPortNr);
        if (isClocked) {
            cQueue *q = (cQueue *) inQueues[inPortNr];
            q->insert(msg);
        } else {
            send(msg, this->middlewareEntryGate);
        }
    } else {
        // this probably should not exist...
        throw cRuntimeError(this, "packet arrived from nowhere (%s)",
                msg->str().c_str());
    }

}

}; // namespace
