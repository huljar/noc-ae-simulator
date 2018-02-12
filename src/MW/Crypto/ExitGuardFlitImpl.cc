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

    // Reset scheduling priority
    flit->setSchedulingPriority(0);

    // Send to correct output
    if(flit->getStatus() == STATUS_ENCODING) {
        flit->setStatus(STATUS_NONE);
        send(flit, "netOut");
    }
    else if(flit->getStatus() == STATUS_DECODING) {
        flit->setStatus(STATUS_NONE);
        send(flit, "appOut");
    }
    else {
        throw cRuntimeError(this, "Received a flit with an unexpected status: %u", flit->getStatus());
    }
}

}}} //namespace
