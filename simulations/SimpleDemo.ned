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

package HaecComm.simulations;

import HaecComm.Core.Clock;
import HaecComm.MW.GenTraffic;
import HaecComm.MW.ReportArrival;
import HaecComm.MW.Delay;
import HaecComm.Util.PacketQueue;

//
// Simple demo network with a source and a sink, and a queue in between.
// A global clock is also present, which the modules use for synchronization.
//
network SimpleDemo
{
    parameters:
        bool isClocked = true;
        int id = 0;
        int columns = 0;
        int rows = 0;
    submodules:
        clock: Clock {
            @display("p=40,40");
        }
        source: GenTraffic {
            @display("p=200,40");
        }
        delay: Delay {
            @display("p=200,120");
        }
        queue: PacketQueue {
            @display("p=200,200");
        }
        sink: ReportArrival {
            @display("p=200,280");
        }

    connections:
        source.out --> delay.in;
        delay.out --> queue.in;
        queue.out --> sink.in;
}
