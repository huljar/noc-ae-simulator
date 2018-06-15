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

package HaecComm.Buffers;

//
// TODO auto-generated module
//
moduleinterface IPacketQueueBase {
    parameters:
        @signal[flitSent](type=HaecComm::Messages::Flit);
        @signal[queueLength](type=long); // not using unsigned because queues return signed values for length
        @signal[flitDrop](type=HaecComm::Messages::Flit);
        @signal[timeInQueue](type=unsigned long);
        @signal[queueFull](type=bool);
        @statistic[sendCount](source=flitSent; record=count; title="Flits sent");
        @statistic[queueLength](source=queueLength; record=max,timeavg,vector?; title="Queue Length");
        @statistic[dropCount](source=flitDrop; record=count; title="Flits dropped");
        @statistic[timeInQueue](source=timeInQueue; record=max,avg,vector?; title="Time flits are enqueued");
        bool awaitSendRequests;
        bool syncFirstPacket;
        int maxLength;
}
