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
module NetworkInterfaceNcEncAuthFlit like INetworkInterface {
    parameters:
        @display("i=block/control");
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
        }
        encoder: Encoder {
            @display("p=200,120");
            generationSize = generationSize;
            numCombinations = numCombinations;
        }
        authQueue: PacketQueue {
            @display("p=320,120");
            awaitSendRequests = true;
        }
        loadBalancer: LoadBalancer {
            @display("p=440,120");
            busyCycles = authCycles;
            gates: out[authCryptoUnits];
        }
        authUnit[authUnits]: AuthFlit {
            @display("p=560,40,column,80");
            computeCycles = authCycles;
        }
        serializer: MultiInputPacketQueue {
            @display("p=680,120;i=block/join");
            syncFirstPacket = false;
            gates: in[authUnits];
        }
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=800,120");
        }
        
        routerInputQueue: PacketQueue {
            @display("p=800,420");
        }
        arqSplitter: ArqSplitter {
            @display("p=680,420");
        }
        macSplitter: MacSplitter {
            @display("p=560,420");
        }
        verifier: VerifyFlit {
            @display("p=440,420");
        }
        decoderQueue: PacketQueue {
            @display("p=320,420");
        }
        decoder: Decoder {
            @display("p=200,420");
            generationSize = generationSize;
            numCombinations = numCombinations;
        }
    connections:
        app$i --> appInputQueue.in;
        encoder.in <-- appInputQueue.out;
        encoder.out --> authQueue.in;
        loadBalancer.in <-- authQueue.out;
        
        for i=0..authUnits-1 {
            authUnit[i].in <-- loadBalancer.out[i];
            authUnit[i].out --> serializer.in[i];
        }
        
        serializer.out --> retransmissionBuffer.in;
        router$o <-- retransmissionBuffer.out;
        
        router$i --> routerInputQueue.in;
        arqSplitter.in <-- routerInputQueue.out;
        arqSplitter.arqOut --> retransmissionBuffer.arqIn;
        arqSplitter.othersOut --> macSplitter.in;
        macSplitter.macOut --> verifier.macIn;
        macSplitter.othersOut --> loadBalancer.in; //# TODO: load balancer multiple in gates, status flag in flit
        serializer.out --> verifier.authIn; //# TODO: serializer multiple out gates, status flag in flit
        verifier.out --> decoderQueue.in;
        decoder.in <-- decoderQueue.out;
        decoder.out --> app$o;
}
