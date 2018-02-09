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

    // TODO: continue here
    if(flit->getStatus() == STATUS_ENCODING) {
        if(mode > 0) {

        }
        else if(mode < 0) {

        }
        else { // mode == 0

        }
    }
    else if(flit->getStatus() == STATUS_DECODING) {
        if(mode > 0) {

        }
        else if(mode < 0) {

        }
        else { // mode == 0

        }
    }
    else {
        throw cRuntimeError(this, "Flit with unexpected status %u arrived at the exit guard", flit->getStatus());
    }
}

}}} //namespace
