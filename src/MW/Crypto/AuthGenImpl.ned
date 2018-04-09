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
// Authentication module implementing the whole generation authentication scheme
// as described in the paper. Flits are collected until enough flits of the same
// generation are present. Then, a MAC over the whole generation is computed and
// put into a separate flit. Afterwards, the data flits of the generation are
// sent out, followed immediately by the corresponding MAC flit.
//
simple AuthGenImpl like IMiddlewareBase {
    parameters:
        @display("i=block/cogwheel");
        int generationSize;
    gates:
        input  in;
        output out;
}
