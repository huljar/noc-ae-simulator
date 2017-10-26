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

import HaecComm.Core.IHaecNode;
import HaecComm.Core.IHaecChannel;

//
// A generated network with grid topology.
//
network Mesh2D
{
    parameters:
        bool isClocked = default(true);
        int columns = default(4);
        int rows = default(4);
        string nodeType;
        string channelType;
    submodules:
        node[columns*rows]: <nodeType> like IHaecNode {
            @display("p=,,matrix,$columns");
            gates: port[4];
        }
    connections allowunconnected:
        for i=0..rows-1, for j=0..columns-1 {
            node[i*columns+j].port[1] <--> <channelType> like IHaecChannel <--> node[(i+1)*columns+j].port[0] if i!=rows-1;
            node[i*columns+j].port[3] <--> <channelType> like IHaecChannel <--> node[i*columns+j+1].port[2] if j!=columns-1;
        }
}
