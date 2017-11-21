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

namespace HaecComm { namespace MW {

Define_Module(GenTraffic);

GenTraffic::GenTraffic()
	: injectionProb(0.0)
{
}

GenTraffic::~GenTraffic() {
}

void GenTraffic::initialize() {
	MiddlewareBase::initialize();

	if(getAncestorPar("isClocked")) {
		// subscribe to clock signal
		getSimulation()->getSystemModule()->subscribe("clock", this);
	}

	injectionProb = par("injectionProb");
	if(injectionProb < 0.0 || injectionProb > 1.0)
		throw cRuntimeError(this, "Injection probability must be between 0 and 1, but is %f", injectionProb);
}

void GenTraffic::handleMessage(cMessage* msg) {
	EV_WARN << "Received a message in GenTraffic, where nothing should arrive. Discarding it." << std::endl;
	delete msg;
}

void GenTraffic::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		// Decide if we should generate a packet based on injection probability parameter
		if(injectionProb == 0.0) // Explicit zero check because uniform(0.0, 1.0) can return 0
			return;

		int myId = getAncestorPar("id");
		if(uniform(0.0, 1.0) < injectionProb) {
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
				} while(targetNodeId == myId);
			}

			std::ostringstream packetName;
			packetName << "packet-" << myId << "-" << targetNodeId << "-" << l;

			cPacket* newPacket = createPacket(packetName.str().c_str()); // TODO: use custom packet class with targetId parameter
			newPacket->addPar("targetId") = targetNodeId;

			EV << this->getFullPath() << " sending packet " << newPacket << std::endl;
			send(newPacket, "out");
		}
	}
}

}} //namespace
