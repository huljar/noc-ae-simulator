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
enum Status;

//
// Base class representing a HAEC flit. This class contains only the headers
// that are specified in the paper. Use subclasses like ~FlitLarge and ~FlitSmall
// for actual messages.
//
packet Flit {
    @customize(true);
    
    // Burst bit. Marks this flit as part of a burst of several flits.
    bool burst;
    
    // Address of the sender of this flit.
    Address2D source;
    
    // Address of the destination of this flit.
    Address2D target;
    
    // Flags which specify the type of flit and how memAddress is used.
    uint8_t mode @enum(Mode);
    
    // 32 bit memory address marking a region to be read from/written to, depending on mode.
    uint32_t memAddress; //# TODO: how exactly is this field used?
    
    // For encoded flits: Generation ID. Specifies the generation that this flit belongs to.
    // For decoded/uncoded flits: Flit ID.
    uint32_t gidOrFid; //# TODO: assert 24 bit size
    
    // Global Encoding Vector. Only used in network coding combinations.
    uint16_t gev; //# TODO: assert 12 bit size
    
    // Payload, represented as a byte array.
    abstract uint8_t payload[];
    
    //# The following fields are helpers which are not "officially" part of the
    //# message (and thus should not be considered when setting the bit length
    //# of the flit).
    
    // Hop count (how many nodes were visited before reaching the destination)
    uint8_t hopCount;
    
    // For encoded flits: the flit IDs from which the generation was formed
    long originalIds[];
    
    // Flit markers for correct distribution inside the network interface
    uint8_t status @enum(Status);
}
