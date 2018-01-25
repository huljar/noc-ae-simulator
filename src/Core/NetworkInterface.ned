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

import HaecComm.Clocking.*;
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
        int authCryptoUnits;
        int authCycles;
        @display("i=block/control");
    gates:
        inout app;
        inout router;
    submodules:
        ncEncode: Encoder {
            @display("p=60,120");
        }
        ncEncodeQueue: PacketQueue {
            @display("p=160,120");
            waitForSignal = true;
        }
        authLoadBalancer: LoadBalancer {
            @display("p=260,120");
            busyCycles = authCycles;
            gates: out[authCryptoUnits];
        }
        authUnit[authCryptoUnits]: AuthFlit {
            @display("p=360,40,column,80");
            computeCycles = authCycles;
        }
        authLoadMerger: MultiInputPacketQueue {
            @display("p=460,120;i=block/join");
            syncFirstPacket = false;
            gates: in[authCryptoUnits];
        }
        ncDecode: Decoder {
            @display("p=60,420");
        }
        ncDecodeQueue: PacketQueue {
            @display("p=160,420");
            waitForSignal = true;
        }
    connections:
        ncEncode.in <-- app$i;
        ncEncode.out --> ncEncodeQueue.in;
        ncEncodeQueue.out --> authLoadBalancer.in;
        for i=0..authCryptoUnits-1 {
            authUnit[i].in <-- authLoadBalancer.out[i];
            authUnit[i].out --> authLoadMerger.in[i];
        }
        authLoadMerger.out --> router$o;
        
        ncDecode.in <-- router$i;
        ncDecode.out --> ncDecodeQueue.in;
        ncDecodeQueue.out --> app$o; // TODO: add conditional queues in between? (-> locallyClocked)
}
