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
#include <Messages/Flit.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW {

Define_Module(ReportArrival);

void ReportArrival::initialize() {
	MiddlewareBase::initialize();

	receiveFlitSignal = registerSignal("receiveFlit");
}

void ReportArrival::handleMessage(cMessage* msg) {
    Flit* flit = check_and_cast<Flit*>(msg);

    emit(receiveFlitSignal, flit);
    EV << "Received " << (flit->isModified() || flit->hasBitError() ? "corrupted " : "")
       << "flit \"" << flit->getName() << "\" from " << flit->getSource()
       << " at " << flit->getTarget() << " with hop count " << +flit->getHopCount()
       << " (flit ID: " << flit->getGidOrFid() << ")" << std::endl;

    delete flit;
}

}} //namespace
