//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package HaecComm.MW;

//
// Special middleware which reports flit arrivals
//
// This middleware logs information about incoming flits. Afterwards,
// the flits are deleted.
//
// Note: This module will not send out any flits. Its output gate
// is supposed to stay unconnected, since this module consumes all
// incoming traffic.
//
simple ReportArrival like IMiddlewareBase {
    parameters:
        @signal[receiveFlit](type=HaecComm::Messages::Flit);
        @statistic[flitsConsumed](source=receiveFlit; record=vector(flitId),vector(flitSource)?,vector(flitCorrupted)?; title="Consumed Flits");
        @display("i=block/arrival");
    gates:
        input  in;
        output out @loose;
}
