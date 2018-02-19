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

package HaecComm.Routers;

import HaecComm.Buffers.PacketQueue;

module Router like IRouter {
    parameters:
        @display("i=block/routing");
        string internalType;
        int numPorts;
    gates:
        inout local;
        inout port[numPorts];
    submodules:
        localInputQueue: PacketQueue {
            @display("p=80,250");
            awaitSendRequests = true;
        }
        routerImpl: <internalType> like IRouterBase {
            @display("p=230,250");
            gates: port[numPorts];
        }
        nodeInputQueue[numPorts]: PacketQueue {
            @display("p=380,100,column,100");
            awaitSendRequests = true;
        }
    connections:
        localInputQueue.in <-- local$i;
        localInputQueue.out --> routerImpl.local$i;
        local$o <-- routerImpl.local$o;
        
        for i=0..numPorts-1 {
            port$i[i] --> nodeInputQueue[i].in;
            routerImpl.port$i[i] <-- nodeInputQueue[i].out;
            routerImpl.port$o[i] --> port$o[i];
        }
}
