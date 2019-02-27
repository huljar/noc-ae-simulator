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

import HaecComm.Buffers.IPacketQueueBase;

simple AlwaysFullPacketQueue like IPacketQueueBase {
    parameters:
        @signal[queueLength](type=long);
        @signal[flitDrop](type=HaecComm::Messages::Flit);
        @signal[queueFull](type=bool);
        @statistic[queueLength](source=queueLength; record=max,timeavg,vector; title="Queue Length");
        @statistic[dropCount](source=flitDrop; record=count; title="Flits dropped");
        @display("i=block/queue");
        bool awaitSendRequests = false;
        bool syncFirstPacket = true;
        int maxLength = 0;
        bool softLimit = false;
    gates:
        input  in;
        output out;
}
