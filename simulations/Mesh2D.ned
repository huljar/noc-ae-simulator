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
import HaecComm.Core.IHaecNode;
import HaecComm.Core.IHaecChannel;

//
// A network with a 2D grid layout of nodes. In addition, a global clock
// is present which emits a clock signal in regular intervals.
//
network Mesh2D
{
    parameters:
        bool isClocked = default(true); // should all communication be synced with the global clock module?
        int columns = default(4);       // number of grid columns
        int rows = default(4);          // number of grid rows
        string nodeType;                // NED type of the nodes (must implement IHaecNode)
        string channelType;             // NED type of the channels between nodes (must implement IHaecChannel)
    submodules:
        //
        // The global clock module. It is used by nodes to synchronize message
        // transmissions. The clock emits a "clock" signal in regular intervals.
        //
        clock: Clock {
            @display("p=30,40");
        }
        //
        // The nodes of the network. They must implement the IHaecNode interface.
        // To set the actual type, use the nodeType string parameter.
        //
        node[columns*rows]: <nodeType> like IHaecNode {
            @display("p=100,40,matrix,$columns,100,100");
            id = index;
            gates: port[4];
        }
    connections allowunconnected:
        // port indices: top=0, right=1, bottom=2, left=3
        for i=0..rows-1, for j=0..columns-1 {
            node[i*columns+j].port[2] <--> <channelType> like IHaecChannel <--> node[(i+1)*columns+j].port[0] if i!=rows-1;
            node[i*columns+j].port[1] <--> <channelType> like IHaecChannel <--> node[i*columns+j+1].port[3] if j!=columns-1;
        }
}
