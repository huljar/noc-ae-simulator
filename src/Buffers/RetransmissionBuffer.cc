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

#include "RetransmissionBuffer.h"
#include <Messages/Flit_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace Buffers {

Define_Module(RetransmissionBuffer);

void RetransmissionBuffer::initialize() {
}

void RetransmissionBuffer::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    if(strcmp(flit->getArrivalGate()->getName(), "arqIn") == 0) {
        // Confirm that this is an ARQ
        if(flit->getMode() != MODE_ARQ) {
            EV_WARN << "Received a flit on the ARQ line that is not an ARQ. Discarding it." << std::endl;
            delete msg;
            return;
        }

        // TODO: look up flit from buffer and send out
        delete flit;
    }
    else {
        // TODO: copy flit (dup()) to buffer, implement caching strategy
        send(flit, "out");
    }
}

}} //namespace
