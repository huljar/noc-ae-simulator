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

package HaecComm.MW.Crypto;

import HaecComm.MW.IMiddlewareBase;

//
// Authentication module implementing the half flit authentication scheme
// as described in the paper. Each incoming flit is split into two flits,
// with the first flit containing the first half of the payload and the
// second one containing the second half. For each of these Flits, a short MAC is
// computed and put into the (now empty) second half of the payload. Then,
// both flits are sent out.
//
simple AuthSplitImpl like IMiddlewareBase {
    parameters:
        @display("i=block/cogwheel");
    gates:
        input  in;
        output out;
}
