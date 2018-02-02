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

package HaecComm.Core;

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
        appInputQueue: PacketQueue {
            @display("p=80,120");
            awaitSendRequests = true;
        }
        ncEncoder: Encoder {
            @display("p=200,120");
            generationSize = ncGenerationSize;
            numCombinations = ncNumCombinations;
        }
        authQueue: PacketQueue {
            @display("p=320,120");
            awaitSendRequests = true;
        }
        authLoadBalancer: LoadBalancer {
            @display("p=440,120");
            busyCycles = authCycles;
            gates: out[authCryptoUnits];
        }
        authUnit0[authCryptoUnits]: AuthFlit if authMode == 0 {
            @display("p=560,40,column,80");
            computeCycles = authCycles;
        }
        authUnit1[authCryptoUnits]: AuthGeneration if authMode == 1 {
            @display("p=560,40,column,80");
            computeCycles = authCycles;
            generationSize = ncGenerationSize;
        }
        authUnit2[authCryptoUnits]: AuthHalfFlit if authMode == 2 {
            @display("p=560,40,column,80");
            computeCycles = authCycles;
        }
        authSerializer: MultiInputPacketQueue {
            @display("p=680,120");
            syncFirstPacket = false;
            gates: in[authCryptoUnits];
        }
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=800,120");
        }
        routerInputQueue: PacketQueue {
            @display("p=80,420");
            awaitSendRequests = true;
        }
        ncDecode: Decoder {
            @display("p=200,420");
            generationSize = ncGenerationSize;
            numCombinations = ncNumCombinations;
        }
        ncDecodeQueue: PacketQueue {
            @display("p=320,420");
            awaitSendRequests = false;
        }
    connections:
        app$i --> appInputQueue.in;
        
        for i=0..authCryptoUnits-1 {
            authUnit0[i].in <-- authLoadBalancer.out[i] if authMode == 0;
            authUnit0[i].out --> authSerializer.in[i] if authMode == 0;
            authUnit1[i].in <-- authLoadBalancer.out[i] if authMode == 1;
            authUnit1[i].out --> authSerializer.in[i] if authMode == 1;
            authUnit2[i].in <-- authLoadBalancer.out[i] if authMode == 2;
            authUnit2[i].out --> authSerializer.in[i] if authMode == 2;
        }
        
        retransmissionBuffer.out --> router$o;
        
        if authMode != 1 {
            appInputQueue.out --> ncEncoder.in;
            ncEncoder.out --> authQueue.in;
            authQueue.out --> authLoadBalancer.in;
            authSerializer.out --> retransmissionBuffer.in;
        }
        if authMode == 1 {
        	appInputQueue.out --> authLoadBalancer.in;
        	authSerializer.out --> authQueue.in;
        	authQueue.out --> ncEncoder.in;
        	ncEncoder.out --> retransmissionBuffer.in;
        }
        
        router$i --> routerInputQueue.in;
        ncDecode.in <-- routerInputQueue.out;
        ncDecode.out --> ncDecodeQueue.in;
        ncDecodeQueue.out --> app$o;
}
