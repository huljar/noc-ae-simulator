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

package HaecComm.MW;

//
// Middleware to simulate work which takes a certain amount of
// time. All packets passing through this module will be held
// for the configured time and then sent out unmodified.
// This can e.g. be used to simulate encryption algorithms
// which take a predefined number of clock cycles to execute.
//
// The delay is measured in clock cycles and determined by the
// <em>waitCycles</em> parameter.
//
// The Delay middleware does not enforce any restrictions on how many
// messages can be held back at the same time.
//
simple Delay like IMiddlewareBase {
    parameters:
        int waitCycles = default(10);
        @display("i=block/delay");
    gates:
        input  in;
        output out;
}
