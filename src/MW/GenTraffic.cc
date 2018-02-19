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
#include <Messages/Flit.h>
#include <sstream>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(GenTraffic);

GenTraffic::GenTraffic()
    : injectionProb(0.0)
    , gridRows(1)
    , gridColumns(1)
    , nodeId(0)
    , nodeX(0)
    , nodeY(0)
    , fidCounter(0)
    , useCachedTarget(false)
    , cachedTargetX(0)
    , cachedTargetY(0)
{
}

GenTraffic::~GenTraffic() {
}

void GenTraffic::initialize() {
	MiddlewareBase::initialize();

    // subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

	injectionProb = par("injectionProb");
	if(injectionProb < 0.0 || injectionProb > 1.0)
		throw cRuntimeError(this, "Injection probability must be between 0 and 1, but is %f", injectionProb);

	generatePairs = par("generatePairs");

	gridRows = getAncestorPar("rows");
	gridColumns = getAncestorPar("columns");
	nodeId = getAncestorPar("id");

	nodeX = nodeId % gridColumns;
	nodeY = nodeId / gridColumns;

	pktgenerateSignal = registerSignal("pktgenerate");
}

void GenTraffic::handleMessage(cMessage* msg) {
	EV_WARN << "Received a message in GenTraffic, where nothing should arrive. Discarding it." << std::endl;
	delete msg;
}

void GenTraffic::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
	    // First, check if we need to serve the second flit of a pair
	    if(useCachedTarget) {
	        // Generate flit
	        generateFlit(cachedTargetX, cachedTargetY);

	        // Mark that we have served the second flit
	        useCachedTarget = false;
	    }
	    else {
            // Decide if we should generate a packet based on injection probability parameter
            if(injectionProb == 0.0) // Explicit zero check because uniform(0.0, 1.0) can return 0
                return;

            if(uniform(0.0, 1.0) < injectionProb) {
                // Generate a flit
                int targetNodeId;

                // TODO create paramterized target selection class
                // Uniform target selection
                do {
                    targetNodeId = static_cast<int>(intrand(gridRows * gridColumns));
                } while(targetNodeId == nodeId);

                // Get target X and Y
                int targetX = targetNodeId % gridColumns;
                int targetY = targetNodeId / gridColumns;

                // Generate flit
                generateFlit(targetX, targetY);

                // If pair generation is enabled, set the parameters for next clock tick
                if(generatePairs) {
                    useCachedTarget = true;
                    cachedTargetX = targetX;
                    cachedTargetY = targetY;
                }
            }
	    }
	}
}

void GenTraffic::generateFlit(int targetX, int targetY) {
    Address2D source(nodeX, nodeY);
    Address2D target(targetX, targetY);

    // Build packet name
    std::ostringstream packetName;
    packetName << "uc-" << fidCounter << "-s" << source.str() << "-t" << target.str();

    // Create the flit
    Flit* flit = new Flit(packetName.str().c_str());
    take(flit);

    // Set header fields
    flit->setSource(source);
    flit->setTarget(target);
    flit->setGidOrFid(fidCounter);

    emit(pktgenerateSignal, flit->getGidOrFid());
    EV << "Sending flit \"" << flit->getName() << "\" from " << flit->getSource().str()
       << " to " << flit->getTarget().str() << " (ID: " << flit->getGidOrFid() << ")" << std::endl;
    send(flit, "out");

    // Increment Flit ID counter
    ++fidCounter;
}

}} //namespace
