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

import HaecComm.Routers.IRouter;
import HaecComm.Clocking.PacketQueue;

//
// Default definition of the HAEC network nodes. It consists of an App (aka.
// Processing Unit), a Network Interface, and a Router. Between the submodules
// and in front of the outgoing connections, there are packet queues handling
// clock synchronization if clocking is enabled.
//
module HaecNode like IHaecNode {
    parameters:
        int id;
        string appType;
        string niType;
        string routerType;
        @display("i=block/network");
    gates:
        inout port[];
    submodules:
        app: <appType> like IApp {
            @display("p=100,255");
        }
        appNiQueue: PacketQueue {
            @display("p=300,205");
        }
        niAppQueue: PacketQueue {
            @display("p=200,305");
        }
        ni: <niType> like INetworkInterface {
            @display("p=400,255");
        }
        niRouterQueue: PacketQueue {
            @display("p=600,205");
            waitForSignal = true;
        }
        routerNiQueue: PacketQueue {
            @display("p=500,305");
        }
        router: <routerType> like IRouter {
            @display("p=700,255");
            gates: port[sizeof(port)];
        }
        routerInputQueue[sizeof(port)]: PacketQueue {
            @display("p=850,105,column,100");
            waitForSignal = true;
        }
    connections:
        app.net$i <-- niAppQueue.out;
        app.net$o --> appNiQueue.in;
        
        ni.app$i <-- appNiQueue.out;
        ni.app$o --> niAppQueue.in;
        
        ni.router$i <-- routerNiQueue.out;
        ni.router$o --> niRouterQueue.in;
        
        router.local$i <-- niRouterQueue.out;
        router.local$o --> routerNiQueue.in;
        
        for i=0..sizeof(port)-1 {
            router.port$i[i] <-- routerInputQueue[i].out;
            router.port$o[i] --> port$o[i];
            port$i[i] --> routerInputQueue[i].in;
        }
}