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

#include "ExitGuardGenImpl.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace Crypto {

Define_Module(ExitGuardGenImpl);

ExitGuardGenImpl::ExitGuardGenImpl()
    : generationSize(1)
{
}

ExitGuardGenImpl::~ExitGuardGenImpl() {
    for(auto it = flitCache.begin(); it != flitCache.end(); ++it) {
        while(!it->second.empty()) {
            delete it->second.front();
            it->second.pop();
        }
    }
}

void ExitGuardGenImpl::initialize() {
    generationSize = par("generationSize");
    if(generationSize < 1)
        throw cRuntimeError(this, "Generation size must be greater than 0, but received %i", generationSize);
}

void ExitGuardGenImpl::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    // Get parameters
    const Address2D& target = flit->getTarget();

    // Check flit status flag
    if(flit->getStatus() == STATUS_ENCRYPTING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "encIn") == 0);

        // Store encrypted flit
        flitCache[target].push(flit);

        // Send back a copy for authentication
        flit->setStatus(STATUS_NONE);
        Flit* mac = flit->dup();
        send(mac, "entryOut");
    }
    else if(flit->getStatus() == STATUS_AUTHENTICATING) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "authIn") == 0);

        // Retrieve corresponding encrypted flits, send all out to the network
        FlitQueue& queue = flitCache.at(target);
        ASSERT(queue.size() >= static_cast<size_t>(generationSize));

        // Send encrypted flits
        for(int i = 0; i < generationSize; ++i) {
            send(queue.front(), "netOut");
            queue.pop();
        }

        // Send MAC
        flit->setStatus(STATUS_NONE);
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
