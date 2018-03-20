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
// Special middleware which generates traffic
//
// On each clock tick, this middleware generates a packet an
// sends it out with a probability defined by the <em>injectionProb</em>
// parameter. In an unclocked simulation, it currently does nothing.
//
// Note: This module will discard any incoming packets. Its input gate
// is supposed to stay unconnected, since this module creates its own
// packets.
//
simple GenTraffic like IMiddlewareBase {
    parameters:
        @signal[generateFlit](type=HaecComm::Messages::Flit);
        @statistic[flits](source=generateFlit; record=vector(flitId),vector(flitTarget); title="Generated Flits");
        @display("i=block/departure");
        bool enabled = default(true);
        double injectionProb;
        bool generatePairs = default(false);
        bool singleTarget = default(false);
        int singleTargetId = default(0);
    gates:
        input  in @loose;
        output out;
}
