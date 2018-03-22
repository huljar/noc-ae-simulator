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
    gates:
        input  in;
        output out;
}
