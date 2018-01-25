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

#include "ReportArrival.h"
#include <Messages/Flit_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(ReportArrival);

void ReportArrival::initialize() {
	MiddlewareBase::initialize();

	pktconsumeSignal = registerSignal("pktconsume");
}

void ReportArrival::handleMessage(cMessage* msg) {
	if(Flit* flit = dynamic_cast<Flit*>(msg)) {
		emit(pktconsumeSignal, flit->getOriginalIds(0)); // TODO: take into account lost packets
		EV << "Received flit \"" << flit->getName() << "\" from " << flit->getSource().str()
		   << " at " << flit->getTarget().str() << " with hop count " << +flit->getHopCount()
		   << " (original ID: " << flit->getOriginalIds(0) << ")" << std::endl;
	}
	else {
		EV << "Received message \"" << msg->getName() << "\"" << std::endl;
	}
    delete msg;
}

}} //namespace
