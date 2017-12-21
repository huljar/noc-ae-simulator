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

#include "RouterYX.h"
#include <Messages/Flit_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace Routers {

Define_Module(RouterYX);

RouterYX::RouterYX()
	: nodeX(0)
	, nodeY(0)
{
}

RouterYX::~RouterYX() {
}

void RouterYX::initialize() {
	RouterBase::initialize();

	nodeX = nodeId % gridColumns;
	nodeY = nodeId / gridColumns;
}

void RouterYX::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	// Get node information
	int targetX = flit->getTarget().x();
	int targetY = flit->getTarget().y();

	// Increase hop count if we are not the sender node
	if(strcmp(flit->getArrivalGate()->getName(), "local$i") != 0) {
		flit->setHopCount(flit->getHopCount() + 1);
	}

	EV << "Routing flit: " << flit->getSource().str() << " -> " << flit->getTarget().str() << std::endl;

	// Route the flit
	if(targetY != nodeY) {
		// Move in Y direction
		send(flit, "port$o", targetY < nodeY ? 0 : 2); // implicit knowledge
	}
	else if(targetX != nodeX) {
		// Move in X direction
		send(flit, "port$o", targetX < nodeX ? 3 : 1);
	}
	else {
		// This node is the destination
		send(flit, "local$o");
	}
}

}} //namespace