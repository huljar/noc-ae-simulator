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
module NetworkInterfaceNcAuth like INetworkInterface {
    parameters:
        int ncGenerationSize;
        int ncNumCombinations;
        int authMode;
        int authCryptoUnits;
        int authCycles;
        @display("i=block/control");
    gates:
        inout app;
        inout router;
    submodules:
        //#### Input queue for all auth modes ####//
        appInputQueue: PacketQueue {
            @display("p=80,120");
            awaitSendRequests = true;
        }
        
        //#### Auth modes 0 and 2 (positioning of modules varies) ####//
        ncEncoderFirst: Encoder if authMode != 1 {
            @display("p=200,120");
            generationSize = ncGenerationSize;
            numCombinations = ncNumCombinations;
        }
        authQueue: PacketQueue if authMode != 1 {
            @display("p=320,120");
            awaitSendRequests = true;
        }
        authLoadBalancerSecond: LoadBalancer if authMode != 1 {
            @display("p=440,120");
            busyCycles = authCycles;
            gates: out[authCryptoUnits];
        }
        authUnitFlit[authCryptoUnits]: AuthFlit if authMode == 0 {
            @display("p=560,40,column,80");
            computeCycles = authCycles;
        }
        authUnitHalfFlit[authCryptoUnits]: AuthHalfFlit if authMode == 2 {
            @display("p=560,40,column,80");
            computeCycles = authCycles;
        }
        authSerializerSecond: MultiInputPacketQueue if authMode != 1 {
            @display("p=680,120");
            syncFirstPacket = false;
            gates: in[authCryptoUnits];
        }
        
        //#### Auth mode 1 (positioning of modules varies) ####//
        authLoadBalancerFirst: LoadBalancer if authMode == 1 {
            @display("p=200,120");
            busyCycles = authCycles;
            gates: out[authCryptoUnits];
        }
        authUnitGen[authCryptoUnits]: AuthGeneration if authMode == 1 {
            @display("p=320,40,column,80");
            computeCycles = authCycles;
            generationSize = ncGenerationSize;
        }
        authSerializerFirst: MultiInputPacketQueue if authMode == 1 {
            @display("p=440,120");
            syncFirstPacket = false;
            gates: in[authCryptoUnits];
        }
        ncEncoderQueue: PacketQueue if authMode == 1 {
            @display("p=560,120");
            awaitSendRequests = true;
        }
        ncEncoderSecond: Encoder if authMode == 1 {
            @display("p=680,120");
            generationSize = ncGenerationSize;
            numCombinations = ncNumCombinations;
        }
        
        //#### Auth mode independent modules ####//
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=800,120");
        }
        arqSplitter: ArqSplitter {
            @display("p=800,420");
        }
        arqQueue: PacketQueue {
            @display("p=750,270");
        }
        routerInputQueue: PacketQueue {
            @display("p=680,420");
            awaitSendRequests = true;
        }
        ncDecode: Decoder {
            @display("p=560,420");
            generationSize = ncGenerationSize;
            numCombinations = ncNumCombinations;
        }
        ncDecodeQueue: PacketQueue {
            @display("p=440,420");
            awaitSendRequests = false;
        }
    connections:
        app$i --> appInputQueue.in;
        
        if authMode != 1 {
            appInputQueue.out --> ncEncoderFirst.in;
            ncEncoderFirst.out --> authQueue.in;
            authQueue.out --> authLoadBalancerSecond.in;
            authSerializerSecond.out --> retransmissionBuffer.in;
        }
        if authMode == 1 {
            appInputQueue.out --> authLoadBalancerFirst.in;
            authSerializerFirst.out --> ncEncoderQueue.in;
            ncEncoderQueue.out --> ncEncoderSecond.in;
            ncEncoderSecond.out --> retransmissionBuffer.in;
        }
        
        for i=0..authCryptoUnits-1 {
            authUnitFlit[i].in <-- authLoadBalancerSecond.out[i] if authMode == 0;
            authUnitFlit[i].out --> authSerializerSecond.in[i] if authMode == 0;
            authUnitGen[i].in <-- authLoadBalancerFirst.out[i] if authMode == 1;
            authUnitGen[i].out --> authSerializerFirst.in[i] if authMode == 1;
            authUnitHalfFlit[i].in <-- authLoadBalancerSecond.out[i] if authMode == 2;
            authUnitHalfFlit[i].out --> authSerializerSecond.in[i] if authMode == 2;
        }
        
        retransmissionBuffer.out --> router$o;
        
        arqSplitter.in <-- router$i;
        arqSplitter.arqOut --> arqQueue.in;
        arqQueue.out --> retransmissionBuffer.arqIn;
        arqSplitter.othersOut --> routerInputQueue.in;
        ncDecode.in <-- routerInputQueue.out;
        ncDecode.out --> ncDecodeQueue.in;
        ncDecodeQueue.out --> app$o;
}
