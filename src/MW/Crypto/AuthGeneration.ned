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

import HaecComm.Buffers.Serializer;
import HaecComm.MW.*;

//
// Compound module wrapper around the ~AuthGenerationImpl simple module.
// In addition to the authentication, a computation delay and
// serialization (one packet per clock cycle) are performed.
//
module AuthGeneration like ICryptoUnit {
    parameters:
        int computeCycles;
        int generationSize;
        @display("i=block/encrypt");
    gates:
        input  in;
        output out;
    submodules:
        auth: AuthGenerationImpl {
            @display("p=60,60");
            generationSize = generationSize;
        }
        delay: Delay {
            @display("p=160,60");
            waitCycles = computeCycles - generationSize; //# -generationSize because the data flits can already be sent while the MAC is being computed
        }
        serializer: Serializer {
            @display("p=260,60");
        }
    connections:
        in --> auth.in;
        auth.out --> delay.in;
        delay.out --> serializer.in;
        serializer.out --> out;
}
