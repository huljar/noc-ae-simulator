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

#include "LoadMerger.h"

namespace HaecComm { namespace Util {

Define_Module(LoadMerger);

LoadMerger::LoadMerger()
	: cycleFree(true)
{
}

LoadMerger::~LoadMerger() {
}

void LoadMerger::initialize() {
	if(getAncestorPar("isClocked")) {
		// subscribe to clock signal
		getSimulation()->getSystemModule()->subscribe("clock", this);
	}
}

void LoadMerger::handleMessage(cMessage* msg) {
	if(!cycleFree) {
		EV_WARN << "Received a message, but another message already arrived in the same clock tick. Discarding it." << std::endl;
		delete msg;
		return;
	}

	send(msg, "out");
	cycleFree = false;
}

void LoadMerger::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
	if(signalID == registerSignal("clock")) {
		cycleFree = true;
	}
}

}} //namespace
