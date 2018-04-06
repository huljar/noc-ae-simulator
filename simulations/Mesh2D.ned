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

// Created: 2017-9-24 for project HaecComm

package HaecComm.simulations;

import HaecComm.Core.Clock;
import HaecComm.Core.IApp;
import HaecComm.Core.IHaecChannel;
import HaecComm.NetworkInterfaces.INetworkInterface;
import HaecComm.Routers.IRouter;

//
// A network with a 2D grid layout of nodes. In addition, a global clock
// is present which emits a clock signal in regular intervals.
//
network Mesh2D {
    parameters:
        int columns;                    // number of grid columns
        int rows;                       // number of grid rows
        string appType;                 // NED type of the app (processing element) (must implement IApp)
        string niType;                  // NED type of the network interface (must implement INetworkInterface)
        string routerType;              // NED type of the router (must implement IRouter)
        string channelType;             // NED type of the channels between nodes (must implement IHaecChannel)
        double clockPeriod @unit(s);    // Clock speed
        bool useGlobalTransmissionIds;  // Use globally unique IDs for flits and generations
    submodules:
        //
        // The global clock module. It is used by nodes to synchronize message
        // transmissions. The clock emits a "clock" signal in regular intervals.
        //
        clock: Clock {
            @display("p=30,80");
            inter = clockPeriod;
        }
        //
        // The processing elements of the network. They must implement the IApp interface.
        // To set the actual type, use the appType string parameter.
        //
        app[columns*rows]: <appType> like IApp {
            @display("p=120,80,matrix,$columns,200,200");
            id = index;
        }
        //
        // The network interfaces of the network. They must implement the INetworkInterface interface.
        // To set the actual type, use the niType string parameter.
        //
        ni[columns*rows]: <niType> like INetworkInterface {
            @display("p=170,130,matrix,$columns,200,200");
            id = index;
        }
        //
        // The routers of the network. They must implement the IRouter interface.
        // To set the actual type, use the routerType string parameter.
        //
        router[columns*rows]: <routerType> like IRouter {
            @display("p=220,180,matrix,$columns,200,200");
            id = index;
            numPorts = 4;
        }
    connections allowunconnected:
        // port indices: top=0, right=1, bottom=2, left=3
        for i=0..rows-1, for j=0..columns-1 {
            app[i*columns+j].net <--> ni[i*columns+j].app; // app-ni connections
            ni[i*columns+j].router <--> router[i*columns+j].local; // ni-router connections
            router[i*columns+j].port[2] <--> <channelType> like IHaecChannel <--> router[(i+1)*columns+j].port[0] if i!=rows-1;  // top-bottom connections
            router[i*columns+j].port[1] <--> <channelType> like IHaecChannel <--> router[i*columns+j+1].port[3] if j!=columns-1; // left-right connections
        }
}
