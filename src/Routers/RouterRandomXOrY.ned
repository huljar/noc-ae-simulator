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

package HaecComm.Routers;

//
// TODO auto-generated module
//
simple RouterRandomXOrY like IRouterBase {
    parameters:
        @signal[sendFlit](type=HaecComm::Messages::Flit);
        @signal[receiveFlit](type=HaecComm::Messages::Flit);
        @signal[forwardFlit](type=HaecComm::Messages::Flit);
        @statistic[flitsSent](source=sendFlit; record=vector(flitId); title="Packets sent into the network");
        @statistic[flitsReceived](source=receiveFlit; record=vector(flitId); title="Packets arriving from the network");
        @statistic[flitsForwarded](source=forwardFlit; record=vector(flitId); title="Packets forwarded");
        @display("i=block/routing");
        double modificationProb;
        double dropProb;
    gates:
        inout local;
        inout port[];
}
