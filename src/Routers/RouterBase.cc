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

#include "RouterBase.h"

namespace HaecComm { namespace Routers {

RouterBase::RouterBase()
	: gridColumns(1)
	, nodeId(0)
{
}

RouterBase::~RouterBase() {
}

void RouterBase::initialize() {
	gridColumns = getAncestorPar("columns");
	nodeId = getAncestorPar("id");

	// subscribe to the "queue full" signal of the input queues
	// of the connected routers
	for(int i = 0; i < gateSize("port"); ++i) {
        cGate* outGate = gate("port", i);
        if(!outGate->isPathOK())
            throw cRuntimeError(this, "Output gate of the router is not properly connected");

        cModule* connectedModule = outGate->getPathEndGate()->getOwnerModule();
        connectedModule->subscribe("qfull", this);

        EV_DEBUG << "Subscribed to \"" << connectedModule->getFullPath() << "\"'s qfull signal." << std::endl;

        modulePortMap.emplace(connectedModule->getId(), i);
        portReadyMap.emplace(i, true);

        // TODO: store queues in map
	}
}

void RouterBase::receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) {
    // If we receive a "queue is full" signal, set the ready state for the port that connects to
    // the source module to false. If the queue is not full any more, set ready state to true.
    if(signalID == registerSignal("qfull")) {
        portReadyMap.at(modulePortMap.at(source->getId())) = !b;
    }
}

}} //namespace
