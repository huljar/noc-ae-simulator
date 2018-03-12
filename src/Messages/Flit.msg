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
class noncobject GevArqMap;
enum Mode;
enum Status;
enum NcMode;
enum ArqMode;

//
// Main packet class representing a HAEC flit. This class contains the headers
// that are specified in the paper and some additional fields for meta information.
//
packet Flit {
    @customize(true);
    
    // Burst bit. Marks this flit as part of a burst of several flits.
    bool burst = false;
    
    // Address of the sender of this flit.
    Address2D source;
    
    // Address of the destination of this flit.
    Address2D target;
    
    // Flags which specify the type of flit and how memAddress is used.
    uint8_t mode @enum(Mode) = MODE_DATA;
    
    // 32 bit memory address marking a region to be read from/written to, depending on mode.
    uint32_t memAddress; //# TODO: how exactly is this field used?
    
    // For encoded flits: Generation ID. Specifies the generation that this flit belongs to.
    // For decoded/uncoded flits: Flit ID.
    uint32_t gidOrFid; //# TODO: assert 24 bit size
    
    // Global Encoding Vector. Only used in network coding combinations.
    uint16_t gev = 0; //# TODO: this should be 8 bit (uint8_t)
    
    //# There is a payload of 64 bit. In the simulation, it is only used for ARQs to specify
    //# missing/available flits, so we implement it for this purpose.
    
    // When this is an ARQ for uncoded flits: specify the missing parts
    uint8_t ucArqs @enum(ArqMode);
    
    // When this is an ARQ for network coded flits: specify the received or missing parts + GEVs
    GevArqMap ncArqs;
    
    // When this is an ARQ for network coded flits using one MAC per generation: specify if we need/have this MAC
    bool ncArqGenMac = false;
    
    // The bit length of the flit
    bitLength = 149; //# TODO: this is not always true; UC flits are 8 bit shorter than NC flits (they don't have a GEV)
    
    //# The following fields are helpers which are not "officially" part of the
    //# message (and thus should not be considered when setting the bit length
    //# of the flit).
    
    // Hop count (how many nodes were visited before reaching the destination)
    uint8_t hopCount = 0;
    
    // For encoded flits: the flit IDs from which the generation was formed
    long originalIds[];
    
    // Flit markers for correct distribution inside the network interface
    uint8_t status @enum(Status) = STATUS_NONE;
    
    // Flit marker for the network coding mode
    uint8_t ncMode @enum(NcMode) = NC_UNCODED;

    // Flag to specify that this flit was modified by an attacker
    bool modified @getter(isModified) = false;
}
