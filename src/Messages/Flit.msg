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

cplusplus {{
#include <Messages/fieldtypes.h>
}};

namespace HaecComm::Messages;

class noncobject Address2D;
enum Mode;

//
// Base class representing a HAEC flit. This class contains only the headers
// that are specified in the paper. Use subclasses like ~FlitLarge and ~FlitSmall
// for actual messages.
//
packet Flit {
    bool burst; // Burst bit. Marks this flit as part of a burst of several flits.
    Address2D source; // Address of the sender of this flit.
    Address2D target; // Address of the destination of this flit.
    uint8_t mode @enum(Mode); // Flags which specify the type of flit and how memAddress is used.
    uint32_t memAddress; // 32 bit memory address marking a region to be read from/written to, depending on mode.
    //# TODO: how exactly is this field used?
    
    uint32_t gid; // Generation ID. Specifies the generation that this flit belongs to.
    //# TODO: assert 24 bit size
    uint16_t gev; // Global Encoding Vector. Only used in network coding combinations.
    //# TODO: assert 12 bit size
    
    //# The following fields are helpers which are not "officially" part of the
    //# message (and thus should not be considered when setting the bit length
    //# of the flit).
    uint8_t hopCount; // Hop count (how many nodes were visited before reaching the destination)
}
