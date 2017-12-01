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

package HaecComm.Clocking;

//
// Packet queue for synchronization with the global clock module.
// If the simulation is not clocked, no queueing is performed, and packets are
// redirected immediately.
//
simple PacketQueue like IPacketQueueBase {
    parameters:
        @signal[qlen](type=long); // not using unsigned because queues return signed values for length
        @signal[pktdrop](type=cPacket);
        @statistic[queueLength](source=qlen; record=max,timeavg,vector?; title="Queue Length");
        @statistic[dropCount](source=count(pktdrop); record=last,vector?; title="Packets dropped");
        @display("i=block/queue");
        bool syncFirstPacket = default(true);
        // Maximum length of the queue. A value of 0 means no limit.
        int maxLength = default(0);
    gates:
        input  in;
        output out;
}
