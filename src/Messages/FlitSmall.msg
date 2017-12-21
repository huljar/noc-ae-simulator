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
#include <Messages/Flit_m.h>
}}

namespace HaecComm::Messages;

packet Flit;

//
// Implementation of small Flits. Adds a 64 bit payload to the headers
// specified in ~Flit.
//
packet FlitSmall extends Flit {
    uint8_t payload[8]; // 64 bit payload, represented as a byte array.
    bitLength = 153;
}
