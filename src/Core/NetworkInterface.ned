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
module NetworkInterface like INetworkInterface {
    parameters:
        bool ncEnabled;
        int ncGenerationSize;
        int ncNumCombinations;
        
        bool authEnabled;
        int authMode;
        int authCryptoUnits;
        int authCycles;
        
        @display("i=block/control");
    gates:
        inout app;
        inout router;
    submodules:
        ncEncode: Encoder if ncEnabled {
            @display("p=60,120");
            generationSize = ncGenerationSize;
            numCombinations = ncNumCombinations;
        }
        ncEncodeQueue: PacketQueue if ncEnabled {
            @display("p=160,120");
            awaitSendRequests = true;
        }
        authLoadBalancer: LoadBalancer if authEnabled {
            @display("p=260,120");
            busyCycles = authCycles;
            gates: out[authCryptoUnits];
        }
        authUnit[authCryptoUnits]: AuthFlit if authEnabled && authMode == 0 {
            @display("p=360,40,column,80");
            computeCycles = authCycles;
        }
        authUnit[authCryptoUnits]: AuthGeneration if authEnabled && authMode == 1 {
            @display("p=360,40,column,80");
            computeCycles = authCycles;
            generationSize = ncGenerationSize;
        }
        authUnit[authCryptoUnits]: AuthHalfFlit if authEnabled && authMode == 2 {
            @display("p=360,40,column,80");
            computeCycles = authCycles;
        }
        authSerializer: MultiInputPacketQueue {
            @display("p=410,120");
            syncFirstPacket = false;
            gates: in[authCryptoUnits];
        }
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=460,120");
        }
        ncDecode: Decoder if ncEnabled {
            @display("p=60,420");
            generationSize = ncGenerationSize;
            numCombinations = ncNumCombinations;
        }
        ncDecodeQueue: PacketQueue if ncEnabled {
            @display("p=160,420");
            awaitSendRequests = false;
        }
    connections:
        //# TODO: move input queue inside the module and use awaitSendRequests
        //# NED doesn't support nested connection groups and "else" statements
        //# as of OMNeT++ version 5.2
        if ncEnabled && authEnabled && authMode==1 {
            //# mode: authenticate whole generation (before network coding)
            app$i --> authLoadBalancer.in;
            authSerializer.out --> retransmissionBuffer.in;
        }
        if ncEnabled && authEnabled && authMode!=1 {
        	//# other authentication modes (flits, half-flits)
            app$i --> ncEncode.in;
        }
        if ncEnabled && !authEnabled {
            
        }
        if !ncEnabled && authEnabled && authMode==1 {
            
        }
        if !ncEnabled && authEnabled && authMode!=1 {
            
        }
        ncEncode.out --> ncEncodeQueue.in;
        ncEncodeQueue.out --> authLoadBalancer.in;
        for i=0..authCryptoUnits-1 {
            authUnit[i].in <-- authLoadBalancer.out[i];
            authUnit[i].out --> retransmissionBuffer.in[i];
        }
        retransmissionBuffer.out --> router$o;
        
        ncDecode.in <-- router$i;
        ncDecode.out --> ncDecodeQueue.in;
        ncDecodeQueue.out --> app$o;
}
