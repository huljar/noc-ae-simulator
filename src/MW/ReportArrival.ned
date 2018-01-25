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

package HaecComm.MW;

//
// Special middleware which reports message arrivals
//
// This middleware logs information about incoming messages. Afterwards,
// the messages are deleted.
//
// Note: This module will not send out any messages. Its output gate
// is supposed to stay unconnected, since this module consumes all
// incoming traffic.
//
simple ReportArrival like IMiddlewareBase {
    parameters:
        @signal[pktconsume](type=long);
        @statistic[flitIds](source=pktconsume; record=vector; title="Flits consumed");
        @display("i=block/arrival");
    gates:
        input  in;
        output out @loose;
}
