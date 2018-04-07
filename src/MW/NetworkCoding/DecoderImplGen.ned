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

//
// Simple module implementing network decoding. Flits with the same generation
// ID (combinations) are collected until enough flits are present. These flits
// are decoded using the <i>gev</i> header field and sent out afterwards. On
// the sender side, an ~Encoder module is required create the linear combinations
// of the original flits.
//
simple DecoderImplGen like INetworkCodingBase {
    parameters:
        int generationSize;
        int numCombinations;
        @display("i=block/cogwheel");
    gates:
        input  in;
        output out;
}
