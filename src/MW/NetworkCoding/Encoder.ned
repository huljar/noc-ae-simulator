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

package HaecComm.MW.NetworkCoding;

import HaecComm.MW.IMiddlewareBase;

//
// Simple module implementing network encoding. Flits with the same destination
// are grouped into a generation. For each generation, a number of linear
// combinations is computed, which are then sent out. On the receiver side,
// a ~Decoder module is required to obtain the original flits.
//
simple Encoder like INetworkCodingBase {
    parameters:
        int generationSize;
        int numCombinations;
        @display("i=block/switch");
    gates:
        input  in;
        output out;
}
