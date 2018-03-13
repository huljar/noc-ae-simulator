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
// Module interface for Router modules.
// Routers used in the HaecNode compound module must implement this interface.
//
moduleinterface IRouterBase {
    parameters:
        @signal[pktsend](type=cPacket);
        @signal[pktreceive](type=cPacket);
        @signal[pktroute](type=cPacket);
        @statistic[sendCount](source=count(pktsend); record=last; title="Packets sent into the network");
        @statistic[receiveCount](source=count(pktreceive); record=last; title="Packets arriving from the network");
        @statistic[routeCount](source=count(pktroute); record=last; title="Packets routed between different nodes");
        double modificationProb;
        double dropProb;
    gates:
        inout local;
        inout port[];
}
