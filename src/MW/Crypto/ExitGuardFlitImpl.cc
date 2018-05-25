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
    : encode(false)
{
}

ExitGuardFlitImpl::~ExitGuardFlitImpl() {
    for(auto it = ucFlitCache.begin(); it != ucFlitCache.end(); ++it)
        delete it->second;
    for(auto it = ncFlitCache.begin(); it != ncFlitCache.end(); ++it)
        delete it->second;
}

void ExitGuardFlitImpl::initialize() {
    encode = getAncestorPar("networkCoding");
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
    Status status = static_cast<Status>(flit->getStatus());

    // Check flit status flag
    if(status == STATUS_ENCRYPTING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encIn") == 0);
        ASSERT(flit->getNcMode() == NC_UNCODED);

        // Check if we are supposed to send the flit to the encoder
        if(encode) {
            // Send the flit to the encoder
            flit->setStatus(STATUS_ENCODING);
            send(flit, "encoderOut");
        }
        else {
            // Store encrypted flit
            flit->setStatus(STATUS_NONE);
            ucFlitCache.emplace(std::make_pair(id, target), flit);

            // Send back a copy for authentication
            Flit* mac = flit->dup();
            send(mac, "entryOut");
        }
    }
    else if(status == STATUS_ENCODING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encoderIn") == 0);
        ASSERT(encode);
        ASSERT(flit->getNcMode() != NC_UNCODED);

        // Store encrypted and encoded flit
        flit->setStatus(STATUS_NONE);
        ncFlitCache.emplace(std::make_tuple(id, gev, target), flit);

        // Send back a copy for authentication
        Flit* mac = flit->dup();
        send(mac, "entryOut");
    }
    else if(status == STATUS_AUTHENTICATING) {
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
    else if(status == STATUS_DECRYPTING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encIn") == 0);

        // Send decrypted flit back to the app
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
