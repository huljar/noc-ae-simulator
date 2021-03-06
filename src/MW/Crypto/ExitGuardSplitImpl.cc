//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "ExitGuardSplitImpl.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(ExitGuardSplitImpl);

ExitGuardSplitImpl::ExitGuardSplitImpl() {
}

ExitGuardSplitImpl::~ExitGuardSplitImpl() {
}

void ExitGuardSplitImpl::initialize() {
}

void ExitGuardSplitImpl::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    // Get parameters
    Status status = static_cast<Status>(flit->getStatus());

    // Check flit status flag
    if(status == STATUS_ENCRYPTING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encIn") == 0);
        ASSERT(flit->getNcMode() == NC_UNCODED);

        // Send encrypted flit to the splitter (and encoder in case of network coding)
        flit->setStatus(STATUS_ENCODING);
        send(flit, "encoderOut");
    }
    else if(status == STATUS_ENCODING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encoderIn") == 0);

        // Send (possibly encoded) split back for authentication
        flit->setStatus(STATUS_NONE);
        send(flit, "entryOut");
    }
    else if(status == STATUS_AUTHENTICATING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "authIn") == 0);

        // Send encrypted and authenticated split out to the network
        flit->setStatus(STATUS_NONE);
        send(flit, "netOut");
    }
    else if(status == STATUS_DECRYPTING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encIn") == 0);

        // Send decrypted split back to the app
        flit->setStatus(STATUS_NONE);
        send(flit, "appOut");
    }
    else if(status == STATUS_VERIFYING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "authIn") == 0);

        // Send verification MAC back to the app
        flit->setStatus(STATUS_NONE);
        send(flit, "appOut");
    }
    else {
        throw cRuntimeError(this, "Received a flit with an unexpected status: %s", cEnum::get("HaecComm::Messages::Status")->getStringFor(flit->getStatus()));
    }
}

}}} //namespace
