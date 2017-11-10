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

import HaecComm.MW.*;

//
// Default implementation of the Processing Unit from the HAEC project (called "App" here).
// It contains a packet generator (source) and a packet consumer (sink).
//
module App like IApp {
    parameters:
        @display("i=block/app2");
    gates:
        inout net;
    submodules:
        producer: GenTraffic {
            @display("p=160,60");
        }
        consumer: ReportArrival {
            @display("p=60,60");
        }
    connections:
        producer.out --> net$o;
        consumer.in <-- net$i; // TODO: add conditional queues in between (-> locallyClocked)
}
