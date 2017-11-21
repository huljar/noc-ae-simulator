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

#include "RouterXY.h"

namespace HaecComm { namespace Routers {

Define_Module(RouterXY);

void RouterXY::initialize() {
}

void RouterXY::handleMessage(cMessage* msg) {
	// Confirm that this is a packet
	if(!msg->isPacket()) {
		EV_WARN << "Received a message that is not a packet. Discarding it." << std::endl;
		delete msg;
		return;
	}

	cPacket* packet = static_cast<cPacket*>(msg); // No need for dynamic_cast or check_and_cast here

	if(!packet->hasPar("targetId")) {
		EV << " got message without target - drop it like it's hot! " << packet->getName() << std::endl;
		delete packet;
		return;
	}

	int myId = static_cast<int>(getAncestorPar("id"));
	int targetId = static_cast<int>(packet->par("targetId"));

	if(targetId == myId) {
		send(packet, "local$o");
		return;
	}

	// Decide on next port based on id
	int gridCols = static_cast<int>(getAncestorPar("columns"));
	int targetX = targetId % gridCols;
	int targetY = targetId / gridCols;
	int myX = myId % gridCols;
	int myY = myId / gridCols;

	// Since it is definitely not this node (see above) the following is sufficient
	if(targetX != myX) {
		// Move in X direction
		send(packet, "port$o", targetX < myX ? 3 : 1); // implicit knowledge
	} else {
		// Move in Y direction
		send(packet, "port$o", targetY < myY ? 0 : 2);
	}
}

}} //namespace
