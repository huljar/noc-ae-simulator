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
#include <Messages/FlitSmall_m.h>
#include <Messages/FlitLarge_m.h>
#include <sstream>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(GenTraffic);

GenTraffic::GenTraffic()
	: injectionProb(0.0)
	, makeLargeFlits(false)
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

	makeLargeFlits = par("makeLargeFlits");
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
			// Generate a flit
			// We assume that we are operating in a 2-dimensional grid topology
			int gridRows = 1, gridCols = 1;
			int targetNodeId;

			try {
				gridRows = getAncestorPar("rows");
				gridCols = getAncestorPar("columns");
			}
			catch(const cRuntimeError& ex) {
				EV_WARN << " ancestors don't have width/height parameters!" << std::endl;
			}

			if(gridRows == 1 && gridCols == 1) {
				targetNodeId = 0;
			} else {
				// TODO create paramterized target selection class
				// Uniform target selection
				do {
					targetNodeId = static_cast<int>(intrand(gridRows * gridCols));
				} while(targetNodeId == myId);
			}

			// Get target X and Y
			int targetX = targetNodeId % gridCols;
			int targetY = targetNodeId / gridCols;

			// Build packet name
			std::ostringstream packetName;
			packetName << "flit-s" << myId << "-t" << targetNodeId << "-c" << l;

			// Create the flit
			// TODO: use a FlitFactory class or factory method?
			Flit* flit;
			if(makeLargeFlits)
				flit = new FlitLarge(packetName.str().c_str());
			else
				flit = new FlitSmall(packetName.str().c_str());
			take(flit);

			// Set header fields
			flit->setSource(Address2D(myId % gridCols, myId / gridCols));
			flit->setTarget(Address2D(targetX, targetY));

			EV << this->getFullPath() << " sending flit " << flit << std::endl;
			send(flit, "out");
		}
	}
}

}} //namespace
