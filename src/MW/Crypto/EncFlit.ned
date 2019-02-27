//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package HaecComm.MW.Crypto;

import HaecComm.MW.*;

//
// Compound module wrapper around the ~EncFlitImpl simple module.
// In addition to the encryption, a computation delay was added.
//
module EncFlit like ICryptoUnit {
    parameters:
        int computeCycles; // Number of clock cycles to delay outgoing packets
        @display("i=block/encrypt");
    gates:
        input  in;
        output out;
    submodules:
        enc: EncFlitImpl {
            @display("p=80,80");
        }
        delay: Delay {
            @display("p=200,80");
            waitCycles = computeCycles;
        }
    connections:
        in --> enc.in;
        enc.out --> delay.in;
        delay.out --> out;
}
