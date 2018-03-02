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

#include "RetransmissionBufferImplGen.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace Buffers {

Define_Module(RetransmissionBufferImplGen);

RetransmissionBufferImplGen::RetransmissionBufferImplGen() {
}

RetransmissionBufferImplGen::~RetransmissionBufferImplGen() {
}

void RetransmissionBufferImplGen::handleDataMessage(Messages::Flit* flit) {
    // TODO: intercept all MACs, store them in special cache without GEV key
    Mode mode = static_cast<Mode>(flit->getMode());
    if(mode == MODE_MAC) {
        // This is a MAC for the whole generation

        // Check that we don't have a MAC for this generation yet

        // Send out the duplicated MAC
    }
    else {
        // Do the usual data flit handling
        RetransmissionBufferImplBase::handleDataMessage(flit);
    }
}

void RetransmissionBufferImplGen::handleArqMessage(Flit* flit) {
    // TODO
}

}} //namespace
