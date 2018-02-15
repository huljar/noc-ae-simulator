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

#include "ExitGuardFlitImpl.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(ExitGuardFlitImpl);

ExitGuardFlitImpl::ExitGuardFlitImpl()
    : mode(0)
    , netCache(nullptr)
    , appCache(nullptr)
{
}

ExitGuardFlitImpl::~ExitGuardFlitImpl() {
    delete netCache;
    delete appCache;
}

void ExitGuardFlitImpl::initialize() {
    mode = par("mode");
}

void ExitGuardFlitImpl::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    // Check flit status flag
    if(flit->getStatus() == STATUS_ENCRYPTING) {
        // TODO: Store encrypted flit, send back a copy for authentication
        Flit* mac = flit->dup();
        send(mac, "entryOut");

        flit->setStatus(STATUS_NONE);
    }
    else if(flit->getStatus() == STATUS_AUTHENTICATING) {
        // TODO: Retrieve corresponding encrypted flit, send both out to the network
        flit->setStatus(STATUS_NONE);
        send(flit, "appOut");
    }
    else if(flit->getStatus() == STATUS_DECRYPTING) {
        // Send decrypted flit back to the app
        flit->setStatus(STATUS_NONE);
        send(flit, "appOut");
    }
    else if(flit->getStatus() == STATUS_VERIFYING) {
        // Send verification MAC back to the app
        flit->setStatus(STATUS_NONE);
        send(flit, "appOut");
    }
    else {
        throw cRuntimeError(this, "Received a flit with an unexpected status: %u", flit->getStatus());
    }
}

}}} //namespace
