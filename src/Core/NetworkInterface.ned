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

package HaecComm.Core;

import HaecComm.Buffers.PacketQueue;
import HaecComm.Buffers.RetransmissionBuffer;

//
// Minimal implementation of the Network Interface from the HAEC project.
// It contains just a retransmission buffer.
//
module NetworkInterface like INetworkInterface {
    parameters:
        @display("i=block/control");
    gates:
        inout app;
        inout router;
    submodules:
        appInputQueue: PacketQueue {
            @display("p=80,100");
        }
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=220,100");
        }
    connections:
        app$i --> appInputQueue.in;
        retransmissionBuffer.in <-- appInputQueue.out;
        retransmissionBuffer.out --> router$o;
        
        router$i --> app$o;
}
