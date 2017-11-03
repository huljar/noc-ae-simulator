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

#include "GenTraffic.h"
#include <sstream>

namespace HaecComm {

Define_Module(GenTraffic);

void GenTraffic::handleCycle(cPacket* packet) {
	if(packet) {
		EV_WARN << "Received a packet in GenTraffic, where no packet should arrive. Discarding it." << std::endl;
		delete packet;
	}

    // Decide if we should generate a packet based on injection probability parameter
    double prob = par("injectionProb");
    if(prob == 0.0) // Explicit zero check because uniform(0.0, 1.0) can return 0
    	return;

    if(uniform(0.0, 1.0) < prob) {
    	// Generate a packet
    	// We assume that we are operating in a 2-dimensional grid topology
    	int gridWidth = 0, gridHeight = 0;
		int targetNodeId;

		try {
			gridWidth = getAncestorPar("rows");
			gridHeight = getAncestorPar("columns");
		}
		catch(const cRuntimeError& ex) {
			EV_WARN << " ancestors don't have width/height parameters!" << std::endl;
		}

		if(gridWidth == 0 && gridHeight == 0) {
			targetNodeId = 0;
		} else {
			// TODO create paramterized target selection class
			// Uniform target selection
			do {
				targetNodeId = static_cast<int>(intrand(gridWidth * gridHeight));
			} while(targetNodeId == parent->getNodeId());
		}

		std::ostringstream packetName;
		packetName << "packet-" << parent->getNodeId() << "-" << targetNodeId;

		cPacket* newPacket = createPacket(packetName.str().c_str()); // TODO: use custom packet class with targetId parameter
		newPacket->addPar("targetId") = targetNodeId;

		EV << this->getFullPath() << " sending packet " << newPacket << std::endl;
		send(newPacket, "out");
    }
}

void GenTraffic::handleMessageInternal(cPacket* packet) {
	EV_WARN << "Received a packet in GenTraffic, where no packet should arrive. Discarding it." << std::endl;
	delete packet;
}

} //namespace
