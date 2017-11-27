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

import HaecComm.MW.*;
import HaecComm.MW.Crypto.*;
import HaecComm.MW.NetworkCoding.*;
import HaecComm.Util.LoadBalancer;
import HaecComm.Util.LoadMerger;

//
// Default implementation of the Network Interface from the HAEC project.
// It contains a network encoder and decoder.
//
module NetworkInterface like INetworkInterface {
    parameters:
        int encCryptoUnits;
        @display("i=block/control");
    gates:
        inout app;
        inout router;
    submodules:
        ncEncode: Encoder {
            @display("p=60,100");
        }
        authLoadBalancer: LoadBalancer {
            @display("p=160,100");
            gates: out[encCryptoUnits];
        }
        authDelay[encCryptoUnits]: Delay {
            @display("p=260,40,column,80");
        }
        authLoadMerger: LoadMerger {
            @display("p=360,100");
            gates: in[encCryptoUnits];
        }
        ncDecode: Decoder {
            @display("p=60,400");
        }
    connections:
        ncEncode.in <-- app$i;
        ncEncode.out --> authLoadBalancer.in;
        for i=0..encCryptoUnits-1 {
            authDelay[i].in <-- authLoadBalancer.out[i];
            authDelay[i].out --> authLoadMerger.in[i];
        }
        authLoadMerger.out --> router$o;
        
        ncDecode.in <-- router$i;
        ncDecode.out --> app$o; // TODO: add conditional queues in between (-> locallyClocked)
}
