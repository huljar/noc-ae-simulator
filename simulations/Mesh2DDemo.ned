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

// Created: 2017-2-7 for project HaecComm

package HaecComm.simulations;

import HaecComm.Core.Clock;
import HaecComm.Core.HaecModule;

network Mesh2DDemo
{
    parameters:
        bool isClocked = true;
        int columns = default(4);
        int rows = default(4);
    submodules:
        clock: Clock {
            @display("p=30,40");
        };
        app[columns*rows]: HaecModule {
            id = index;
            portNumber = 1;
            
            @display("p=100,40,matrix,$columns,170,170");
        };
        ni[columns*rows]: HaecModule {
            id = index;
            portNumber = 2;
            
            @display("p=140,80,matrix,$columns,170,170");
        };
        router[columns*rows]: HaecModule {
            id = index;
            portNumber = 5;
            
            @display("p=180,120,matrix,$columns,170,170");
        };

    connections allowunconnected:
        for i=0..rows-1, for j=0..columns-1 {
            // Processing element with ni
            app[i*columns + j].outPorts[0] --> ni[i*columns + j].inPorts[1];
            app[i*columns + j].inPorts[0] <-- ni[i*columns + j].outPorts[1];

            // Ni to router
            ni[i*columns + j].outPorts[0] --> router[i*columns + j].inPorts[4];
            ni[i*columns + j].inPorts[0] <-- router[i*columns + j].outPorts[4];

            // Router with each other; 0 - North, 1 - East, 2 - South, 3 - West
            router[i*columns + j].outPorts[2] --> router[(i+1)*columns + j].inPorts[0] if (i < rows-1);
            router[i*columns + j].inPorts[2] <-- router[(i+1)*columns + j].outPorts[0] if (i < rows-1);

            router[i*columns + j].outPorts[1] --> router[i*columns + (j+1)].inPorts[3] if (j < columns-1);
            router[i*columns + j].inPorts[1] <-- router[i*columns + (j+1)].outPorts[3] if (j < columns-1);
        }
}
