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

package HaecComm.NetworkInterfaces;

import HaecComm.Buffers.*;
import HaecComm.MW.*;
import HaecComm.MW.Crypto.*;
import HaecComm.MW.NetworkCoding.*;
import HaecComm.Util.*;

//
// Default implementation of the Network Interface from the HAEC project.
// It contains a network encoder and decoder.
//
module NetworkInterfaceNcEncAuthGen like INetworkInterface {
    parameters:
        @display("i=block/control");
        int id;
        int generationSize;
        int numCombinations;
        int authUnits;
        int authCycles;
    gates:
        inout app;
        inout router;
    submodules:
        appInputQueue: PacketQueue {
            @display("p=80,120");
            awaitSendRequests = false;
        }
//        entryGuard: LoadBalancer {
//            @display("p=200,120");
//            busyCycles = authCycles;
//            gates: out[authCryptoUnits];
//        }
//        authUnit[authUnits]: AuthGeneration {
//            @display("p=320,40,column,80");
//            computeCycles = authCycles;
//            generationSize = generationSize;
//        }
//        serializer: MultiInputPacketQueue {
//            @display("p=440,120;i=block/join");
//            syncFirstPacket = false;
//            gates: in[authUnits];
//        }
//        encoderQueue: PacketQueue {
//            @display("p=560,120");
//            awaitSendRequests = true;
//        }
//        encoder: Encoder {
//            @display("p=680,120");
//            generationSize = generationSize;
//            numCombinations = numCombinations;
//        }
//        retransmissionBuffer: RetransmissionBuffer {
//            @display("p=800,120");
//        }
//        routerInputQueue: PacketQueue {
//            @display("p=680,420");
//            awaitSendRequests = true;
//        }
//        arqSplitter: ArqSplitter {
//            @display("p=800,420");
//        }
//        arqQueue: PacketQueue {
//            @display("p=750,270");
//        }
//        ncDecode: Decoder {
//            @display("p=560,420");
//            generationSize = ncGenerationSize;
//            numCombinations = ncNumCombinations;
//        }
//        ncDecodeQueue: PacketQueue {
//            @display("p=440,420");
//            awaitSendRequests = false;
//        }
    connections:
        app <--> router;
//        app$i --> appInputQueue.in;
//        
//        //# NED does not allow nested if/for blocks as of OMNeT++ 5.2
//        if authMode != 2 {
//            appInputQueue.out --> ncEncoderFirst.in if authMode == 1;
//            appInputQueue.out --> flitSplitter.in if authMode == 3;
//            flitSplitter.out --> ncEncoderFirst.in if authMode == 3;
//            ncEncoderFirst.out --> authQueue.in;
//            authQueue.out --> authLoadBalancerSecond.in;
//            authSerializerSecond.out --> retransmissionBuffer.in;
//        }
//        if authMode == 2 {
//            appInputQueue.out --> authLoadBalancerFirst.in;
//            authSerializerFirst.out --> ncEncoderQueue.in;
//            ncEncoderQueue.out --> ncEncoderSecond.in;
//            ncEncoderSecond.out --> retransmissionBuffer.in;
//        }
//        
//        for i=0..authCryptoUnits-1 {
//            authUnitFlit[i].in <-- authLoadBalancerSecond.out[i] if authMode == 1;
//            authUnitFlit[i].out --> authSerializerSecond.in[i] if authMode == 1;
//            authUnitGen[i].in <-- authLoadBalancerFirst.out[i] if authMode == 2;
//            authUnitGen[i].out --> authSerializerFirst.in[i] if authMode == 2;
//            authUnitHalfFlit[i].in <-- authLoadBalancerSecond.out[i] if authMode == 3;
//            authUnitHalfFlit[i].out --> authSerializerSecond.in[i] if authMode == 3;
//        }
//        
//        retransmissionBuffer.out --> router$o;
//        
//        arqSplitter.in <-- router$i;
//        arqSplitter.arqOut --> arqQueue.in;
//        arqQueue.out --> retransmissionBuffer.arqIn;
//        arqSplitter.othersOut --> routerInputQueue.in;
//        ncDecode.in <-- routerInputQueue.out;
//        ncDecode.out --> ncDecodeQueue.in;
//        ncDecodeQueue.out --> app$o;
}
