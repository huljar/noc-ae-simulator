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
enum NcMode;

//
// Message that represents an ARQ timer. It is mainly used by ArrivalManagers as self-scheduled messages to
// track ARQ timeouts.
//
message ArqTimer {
    @customize(true);
    
    // For encoded flits: Generation ID. Specifies the generation that this ARQ belongs to.
    // For decoded/uncoded flits: Flit ID.
    uint32_t gidOrFid; //# real IDs are only 24 bit long
    
    // Address of the sender of the transmission unit that this ARQ belongs to.
    Address2D source;
    
    // ARQ marker for the network coding mode
    uint8_t ncMode @enum(NcMode) = NC_UNCODED;
}
