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

package HaecComm.NetworkInterfaces;

import HaecComm.Buffers.PacketQueue;
import HaecComm.Buffers.RetransmissionBuffer;
import HaecComm.Util.ArqSplitter;

//
// Minimal implementation of a Network Interface.
// It contains just a retransmission buffer.
//
module NetworkInterface like INetworkInterface {
    parameters:
        @display("i=block/control");
        int id;
        bool networkCoding;
    gates:
        inout app;
        inout router;
    submodules:
        appInputQueue: PacketQueue {
            @display("p=80,100");
        }
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=200,100");
        }
        arqSplitter: ArqSplitter {
            @display("p=200,250");
        }
        arqQueue: PacketQueue {
            @display("p=160,175");
        }
        routerInputQueue: PacketQueue {
            @display("p=80,250");
        }
    connections:
        app$i --> appInputQueue.in;
        retransmissionBuffer.dataIn <-- appInputQueue.out;
        retransmissionBuffer.out --> router$o;
        
        arqSplitter.in <-- router$i;
        arqSplitter.arqOut --> arqQueue.in;
        arqQueue.out --> retransmissionBuffer.arqIn;
        arqSplitter.othersOut --> routerInputQueue.in;
        routerInputQueue.out --> app$o;
}
