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

package HaecComm.Buffers;

import HaecComm.MW.Delay;

module RetransmissionBuffer {
    parameters:
        @display("i=block/buffer");
        int bufSize; // Amount of flits that can be cached
        int bufLookupCycles = default(1);
        string internalType;
    gates:
        input  dataIn;
        input  arqIn;
        output out;
    submodules:
        retransBuf: <internalType> like IRetransmissionBufferImpl {
            @display("p=80,120");
            bufSize = bufSize;
        }
        arqLookup: Delay {
            @display("p=180,180");
            waitCycles = bufLookupCycles;
        }
        serializer: MultiInputSerializer {
            @display("p=280,120");
            gates: in[2];
        }
    connections:
        dataIn --> retransBuf.dataIn;
        arqIn --> retransBuf.arqIn;
        retransBuf.dataOut --> serializer.in[0];
        retransBuf.arqOut --> arqLookup.in;
        arqLookup.out --> serializer.in[1];
        serializer.out --> out;
}
