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

import HaecComm.NetworkInterfaces.INetworkInterface;
import HaecComm.Routers.IRouter;
import HaecComm.Buffers.PacketQueue;

//
// Default definition of the HAEC network nodes. It consists of an App (aka.
// Processing Unit), a Network Interface, and a Router. Between the submodules
// and in front of the outgoing connections, there are packet queues handling
// clock synchronization if clocking is enabled.
//
module HaecNode like IHaecNode {
    parameters:
        int id;
        int numPorts;
        string appType;
        string niType;
        string routerType;
        @display("i=block/network");
    gates:
        inout port[numPorts];
    submodules:
        app: <appType> like IApp {
            @display("p=80,100");
        }
        ni: <niType> like INetworkInterface {
            @display("p=180,100");
        }
        router: <routerType> like IRouter {
            @display("p=280,100");
            numPorts = numPorts;
        }
    connections:
        app.net <--> ni.app;
        ni.router <--> router.local;
        
        for i=0..numPorts-1 {
            router.port[i] <--> port[i];
        }
}