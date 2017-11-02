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
    	// We assume that we are operating in a 2-dimensional mesh
    	// TODO: continue work here
    	int mWidth = 0, mHeight = 0;
		int trg;

		try {
			mWidth = getAncestorPar("rows");
			mHeight = getAncestorPar("columns");
		} catch (const cRuntimeError ex) {
			EV << " ancestors don't have width/height parameters!" << std::endl;
		}

		if(mWidth == 0 && mHeight == 0) {
			trg = 0;
		} else {
			// TODO create paramterized target selection class
			// Uniform target selection
			do {
				trg = (int) uniform(0, (double) mWidth*mHeight);
			} while (trg == parentId);

		}

		char msgName[128] = {0};
		sprintf(msgName, "msg-%02d-%02d-%05lu", parentId, trg, currentCycle);

		cPacket* newPacket = createMessage(msgName);
		newPacket->addPar("targetId");
		newPacket->par("targetId") = trg;
		newPacket->par("outPort")  = 0;

		EV << this->getFullPath() << " sending msg " << newPacket << " at cycle " << currentCycle << std::endl;
		send(newPacket, "out");
    }
}

void GenTraffic::handleMessageInternal(cPacket* packet) {
	EV_WARN << "Received a packet in GenTraffic, where no packet should arrive. Discarding it." << std::endl;
	delete packet;
}

} //namespace
