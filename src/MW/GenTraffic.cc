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
#include <Messages/MessageFactory.h>
#include <Util/IdProvider.h>
#include <sstream>

using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace MW {

Define_Module(GenTraffic);

GenTraffic::GenTraffic()
    : enabled(true)
    , injectionProb(0.0)
    , generatePairs(false)
    , singleTarget(false)
    , singleTargetId(0)
    , useGlobalTransmissionIds(false)
    , gridRows(1)
    , gridColumns(1)
    , nodeId(0)
    , nodeX(0)
    , nodeY(0)
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

    enabled = par("enabled");

    injectionProb = par("injectionProb");
    if(injectionProb < 0.0 || injectionProb > 1.0)
        throw cRuntimeError(this, "Injection probability must be between 0 and 1, but is %f", injectionProb);

    generatePairs = par("generatePairs");
    singleTarget = par("singleTarget");

    useGlobalTransmissionIds = getAncestorPar("useGlobalTransmissionIds");

    gridRows = getAncestorPar("rows");
    gridColumns = getAncestorPar("columns");
    nodeId = getAncestorPar("id");

    singleTargetId = par("singleTargetId");
    if(singleTarget && (singleTargetId < 0 || singleTargetId >= gridRows * gridColumns))
        throw cRuntimeError(this, "Single Target ID must be between 0 and %i, but is %i", gridRows * gridColumns - 1, singleTargetId);

    nodeX = nodeId % gridColumns;
    nodeY = nodeId / gridColumns;

    generateFlitSignal = registerSignal("generateFlit");
}

void GenTraffic::handleMessage(cMessage* msg) {
	EV_WARN << "Received a message in GenTraffic, where nothing should arrive. Discarding it." << std::endl;
	delete msg;
}

void GenTraffic::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
	    // Do nothing if the module is not enabled
	    if(!enabled)
	        return;

	    // First, check if we need to serve the second flit of a pair
	    if(useCachedTarget) {
	        // Generate flit
	        generateFlit(cachedTargetX, cachedTargetY);

	        // Mark that we have served the second flit
	        useCachedTarget = false;
	    }
	    else {
            // Decide if we should generate a packet based on injection probability parameter
            if(uniform(0.0, 1.0) < injectionProb) {
                // Generate a flit
                int targetNodeId;

                // Check if single target is enabled
                if(singleTarget) {
                    targetNodeId = singleTargetId;
                }
                else {
                    // Uniform target selection
                    do {
                        targetNodeId = static_cast<int>(intrand(gridRows * gridColumns));
                    } while(targetNodeId == nodeId);
                }

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
    // Get parameters
    Address2D source(nodeX, nodeY);
    Address2D target(targetX, targetY);

    // Get local or global flit ID
    IdProvider* idp = IdProvider::getInstance();
    uint32_t fid = useGlobalTransmissionIds ? idp->getNextFlitId() : idp->getNextFlitId(source, target);

    // Build packet name
    std::ostringstream packetName;
    packetName << "uc-" << fid << "-s" << source << "-t" << target;

    // Create the flit
    Flit* flit = MessageFactory::createFlit(packetName.str().c_str(), source, target, MODE_DATA, fid);
    take(flit);

    // Send flit
    emit(generateFlitSignal, flit);
    EV << "Sending flit \"" << flit->getName() << "\" from " << flit->getSource()
       << " to " << flit->getTarget() << " (ID: " << flit->getGidOrFid() << ")" << std::endl;
    send(flit, "out");
}

}} //namespace
