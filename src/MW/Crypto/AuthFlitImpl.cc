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

#include "AuthFlitImpl.h"
#include <Messages/Flit_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(AuthFlitImpl);

void AuthFlitImpl::initialize() {
	AuthBase::initialize();
}

void AuthFlitImpl::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* dataFlit = dynamic_cast<Flit*>(msg);
	if(!dataFlit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	// Duplicate the flit (because MAC flit has the same headers)
	Flit* macFlit = dataFlit->dup();

	// Set mode flag that this is a MAC flit
	macFlit->setMode(MODE_MAC);

	// TODO: do actual MAC computation

	// Send out data flit first, then MAC
	send(dataFlit, "out");
	send(macFlit, "out");
}

}}} //namespace
