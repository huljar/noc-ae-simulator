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

#include "AuthHalfFlitImpl.h"
#include <Messages/Flit_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(AuthHalfFlitImpl);

void AuthHalfFlitImpl::initialize() {
	MiddlewareBase::initialize();
}

void AuthHalfFlitImpl::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

	// TODO: actual MAC computations

	send(flit, "out");
}

}}} //namespace
