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
#include <Messages/Flit_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace Routers {

Define_Module(RouterXY);

RouterXY::RouterXY()
	: nodeX(0)
	, nodeY(0)
{
}

RouterXY::~RouterXY() {
}

void RouterXY::initialize() {
	RouterBase::initialize();

	nodeX = nodeId % gridColumns;
	nodeY = nodeId / gridColumns;

	pktsendSignal = registerSignal("pktsend");
	pktreceiveSignal = registerSignal("pktreceive");
	pktrouteSignal = registerSignal("pktroute");
}

void RouterXY::handleMessage(cMessage* msg) {
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

	bool isSender = strcmp(flit->getArrivalGate()->getName(), "local$i") == 0;
	bool isReceiver = targetX == nodeX && targetY == nodeY;

	// Emit signals
	if(isSender && !isReceiver)
		emit(pktsendSignal, flit);
	if(isReceiver && !isSender)
		emit(pktreceiveSignal, flit);
	if(!isSender && !isReceiver)
		emit(pktrouteSignal, flit);

	// Increase hop count if we are not the sender node
	if(!isSender) {
		flit->setHopCount(flit->getHopCount() + 1);
	}

	EV << "Routing flit: " << flit->getSource().str() << " -> " << flit->getTarget().str() << std::endl;

	// Route the flit
	if(targetX != nodeX) {
		// Move in X direction
		send(flit, "port$o", targetX < nodeX ? 3 : 1); // implicit knowledge
	}
	else if(targetY != nodeY) {
		// Move in Y direction
		send(flit, "port$o", targetY < nodeY ? 0 : 2);
	}
	else {
		// This node is the destination
		send(flit, "local$o");
	}
}

}} //namespace
