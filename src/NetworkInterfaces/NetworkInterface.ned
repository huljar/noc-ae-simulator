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

package HaecComm.NetworkInterfaces;

import HaecComm.Buffers.PacketQueue;
import HaecComm.Buffers.Serializer;

//
// Minimal implementation of a Network Interface.
// It contains pretty much nothing.
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
        netStream: Serializer {
            @display("p=180,100");
        }
        routerInputQueue: PacketQueue {
            @display("p=80,250");
        }
    connections:
        app$i --> appInputQueue.in;
        netStream.in <-- appInputQueue.out;
        netStream.out --> router$o;
        
        router$i --> routerInputQueue.in;
        app$o <-- routerInputQueue.out;
}
