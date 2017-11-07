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
            @display("p=60,60");
        }
        appQueue: BidirectionalPacketQueue {
            @display("p=160,60");
        }
        ni: <niType> like INetworkInterface {
            @display("p=260,60");
        }
        niQueue: BidirectionalPacketQueue {
            @display("p=360,60");
        }
        router: <routerType> like IRouter {
            @display("p=460,60");
            gates: port[sizeof(port)];
        }
        routerQueue[sizeof(port)]: BidirectionalPacketQueue {
            @display("p=310,180,row,40");
        }
    connections:
        app.net <--> appQueue.left;
        appQueue.right <--> ni.app;
        
        ni.router <--> niQueue.left;
        niQueue.right <--> router.local;
        
        for i=0..sizeof(port)-1 {
            router.port[i] <--> routerQueue[i].left;
            port[i] <--> routerQueue[i].right;
        }
}