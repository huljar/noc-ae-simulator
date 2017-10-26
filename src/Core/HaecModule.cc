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

#include "HaecModule.h"
#include <string>
#include <utility>

namespace HaecComm {

Define_Module(HaecModule);

HaecModule::HaecModule() {}

HaecModule::~HaecModule() {}

void HaecModule::createMiddleware() {
    size_t mwPipelineCount = static_cast<size_t>(par("mwPipelineCount"));
    const char* mwDefinition = par("mwDefinition");

    // Split middleware string on whitespace
    std::vector<std::string> pipelineDefs = cStringTokenizer(mwDefinition).asVector();

    // Check if middleware count matches string definition
    if(pipelineDefs.size() != mwPipelineCount)
        throw cRuntimeError(this, "Specified number of pipelines (%u) doesn't match string definition (%u)", mwPipelineCount, pipelineDefs.size());

    // Iterate over pipelines
    for(size_t i = 0; i < pipelineDefs.size(); ++i) {
        // Split pipeline definition on commas
        std::vector<std::string> mwDefs = cStringTokenizer(pipelineDefs[i].c_str(), ",").asVector();

        // Iterate over the middlewares of this pipeline
        cModule* lastModule = nullptr;
        for(auto it = mwDefs.begin(); it != mwDefs.end(); ++it) {
            // Create middleware
            cModuleType* mwType = cModuleType::get(it->c_str());
            cModule* middleware = mwType->create(mwName, this);

            // TODO: is there a way to check if the module implements the IMiddlewareBase interface?

            middleware->par("parentId") = id;
            middleware->finalizeParameters();

            // If this is the first middleware, connect to entry gate, else connect to previous middleware
            if(!lastModule)
                gate("mwEntry", static_cast<int>(i))->connectTo(middleware->gate("in"));
            else
                lastModule->gate("out")->connectTo(middleware->gate("in"));

            // Update last created module
            lastModule = middleware;
        }

        // Connect last module to the exit gate. If there were no modules, directly connect entry and exit
        if(lastModule)
            lastModule->gate("out")->connectTo(gate("mwExit", static_cast<int>(i)));
        else
            gate("mwEntry", static_cast<int>(i))->connectTo(gate("mwExit", static_cast<int>(i)));

        // Start up the modules
        for(auto it = modules.begin(); it != modules.end(); ++it) {
            (*it)->buildInside();
            (*it)->scheduleStart(simTime());
        }
    }
}

void HaecModule::initialize() {
    id = par("id");
    X  = id % static_cast<int>(getAncestorPar("columns"));
    Y  = id / static_cast<int>(getAncestorPar("columns"));

    isClocked = getAncestorPar("isClocked");
    if (isClocked) {
        // setup queues for in & out ports
//        for (int i = 0; i < gateSize("inPorts"); i++) {
//            inQueues.addAt(i, new cQueue);
//        }
//
//        for (int i = 0; i < gateSize("outPorts"); i++) {
//            outQueues.addAt(i, new cQueue);
//        }

        // subscribe to clock signal
        getSimulation()->getSystemModule()->subscribe("clock", this);
    }

    // Set up middleware
    createMiddleware();
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
