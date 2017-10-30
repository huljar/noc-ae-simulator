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
            // Get middleware type information
            cModuleType* mwType = cModuleType::get(it->c_str());

            // Check if the middleware implements the IMiddlewareBase interface
            if(mwType->str().find("HaecComm.MW.IMiddlewareBase") == std::string::npos)
            	throw cRuntimeError(this, "Middleware %s does not implement IMiddlewareBase interface!", it->c_str());

            // Create and initialize the module
            cModule* middleware = mwType->create(it->c_str(), this);
            middleware->par("parentId") = id;
            middleware->finalizeParameters();

            // If this is the first middleware, connect to entry gate, else connect to previous middleware
            if(!lastModule)
                gate("mwEntry", static_cast<int>(i))->connectTo(middleware->gate("in"));
            else
                lastModule->gate("out")->connectTo(middleware->gate("in"));

            // Finalize module
            middleware->buildInside();
            middleware->scheduleStart(simTime());

            // Update last created module
            lastModule = middleware;
        }

        // Connect last module to the exit gate. If there were no modules, directly connect entry and exit
        if(lastModule)
            lastModule->gate("out")->connectTo(gate("mwExit", static_cast<int>(i)));
        else
            gate("mwEntry", static_cast<int>(i))->connectTo(gate("mwExit", static_cast<int>(i)));
    }
}

bool HaecModule::processQueue(cPacketQueue* queue, const char* targetGate, int targetGateIndex) {
	if(!queue->isEmpty()) {
		cPacket* packet = queue->pop();
		send(packet, targetGate, targetGateIndex);
		return true;
	}
	return false;
}

void HaecModule::initialize() {
    id = par("id");
    X  = id % static_cast<int>(getAncestorPar("columns"));
    Y  = id / static_cast<int>(getAncestorPar("columns"));

    isClocked = getAncestorPar("isClocked");
    if(isClocked) {
        // subscribe to clock signal
        getSimulation()->getSystemModule()->subscribe("clock", this);
    }

    // Set up middleware
    createMiddleware();
}

void HaecModule::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {

}

void HaecModule::handleMessage(cMessage *msg) {

}

}; // namespace
