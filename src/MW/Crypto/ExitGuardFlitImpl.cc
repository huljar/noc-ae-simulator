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

ExitGuardFlitImpl::ExitGuardFlitImpl() {
}

ExitGuardFlitImpl::~ExitGuardFlitImpl() {
    for(auto it = ucFlitCache.begin(); it != ucFlitCache.end(); ++it)
        delete it->second;
    for(auto it = ncFlitCache.begin(); it != ncFlitCache.end(); ++it)
        delete it->second;
}

void ExitGuardFlitImpl::initialize() {
}

void ExitGuardFlitImpl::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    // Get parameters
    uint32_t id = flit->getGidOrFid();
    uint16_t gev = flit->getGev();
    const Address2D& target = flit->getTarget();

    // Check flit status flag
    if(flit->getStatus() == STATUS_ENCRYPTING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encIn") == 0);

        // Store encrypted flit (select cache based on network coding mode)
        if(flit->getNcMode() == NC_UNCODED) {
            ucFlitCache.emplace(std::make_pair(id, target), flit);
        }
        else {
            ncFlitCache.emplace(std::make_tuple(id, gev, target), flit);
        }

        // Send back a copy for authentication
        flit->setStatus(STATUS_NONE);
        Flit* mac = flit->dup();
        send(mac, "entryOut");
    }
    else if(flit->getStatus() == STATUS_AUTHENTICATING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "authIn") == 0);

        // Retrieve corresponding encrypted flit, send both out to the network
        Flit* cached;
        if(flit->getNcMode() == NC_UNCODED) {
            UcKey key = std::make_pair(id, target);
            cached = ucFlitCache.at(key);
            ucFlitCache.erase(key);
        }
        else {
            NcKey key = std::make_tuple(id, gev, target);
            cached = ncFlitCache.at(key);
            ncFlitCache.erase(key);
        }

        // Send both out, encrypted data first
        flit->setStatus(STATUS_NONE);
        send(cached, "netOut");
        send(flit, "netOut");
    }
    else if(flit->getStatus() == STATUS_DECRYPTING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encIn") == 0);

        // Send decrypted flit back to the app
        flit->setStatus(STATUS_NONE);
        send(flit, "appOut");
    }
    else if(flit->getStatus() == STATUS_VERIFYING) {
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
