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
#include "fieldtypes.h"
}};

namespace HaecComm::Messages;

class noncobject Address2D;

//
// TODO generated message class
//
packet Flit {
    bool burst;
    Address2D source;
    Address2D target;
    uint8_t mode; //# TODO: make this an enum? or at least assert 4 bit size
    uint32_t memAddress; //# TODO: how exactly is this supposed to be used?
    
    uint32_t gid; //# TODO: assert 24 bit size
    uint16_t gev; //# TODO: assert 12 bit size
}
