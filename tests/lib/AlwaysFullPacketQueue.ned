import HaecComm.Buffers.IPacketQueueBase;

simple AlwaysFullPacketQueue like IPacketQueueBase {
    parameters:
        @signal[qlen](type=long); // not using unsigned because queues return signed values for length
        @signal[pktdrop](type=cPacket);
        @signal[qfull](type=bool);
        @statistic[queueLength](source=qlen; record=max,timeavg,vector?; title="Queue Length");
        @statistic[dropCount](source=count(pktdrop); record=last; title="Packets dropped");
        @display("i=block/queue");
        bool awaitSendRequests = false;
        bool syncFirstPacket = true;
        int maxLength = 0;
    gates:
        input  in;
        output out;
}
