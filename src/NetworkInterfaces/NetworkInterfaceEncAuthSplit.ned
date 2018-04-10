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
module NetworkInterfaceEncAuthSplit like INetworkInterface {
    parameters:
        @display("i=block/control");
        int id;
        int encUnits;
        int encCycles;
        int authUnits;
        int authCycles;
    gates:
        inout app;
        inout router;
    submodules:
        //# app->net
        flitSplitter: FlitSplitter {
            @display("p=180,280");
        }
        appInputQueue: PacketQueue {
            @display("p=380,280");
            awaitSendRequests = true;
        }
        entryGuard: EntryGuardSplit {
            @display("p=480,300");
            busyCyclesEnc = encCycles;
            busyCyclesAuth = authCycles;
            gates:
                encOut[encUnits];
                authOut[authUnits];
        }
        encUnit[encUnits]: EncSplit {
            @display("p=150,80,row,80");
            computeCycles = encCycles;
        }
        authUnit[authUnits]: AuthSplit {
            @display("p=150,160,row,80");
            computeCycles = authCycles;
        }
        exitGuard: ExitGuardSplit {
            @display("p=650,300");
            gates:
                encIn[encUnits];
                authIn[authUnits];
        }
        authCryptoQueue: PacketQueue {
            @display("p=565,320");
            awaitSendRequests = true;
        }
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=750,280");
            internalType = "HaecComm.Buffers.RetransmissionBufferImplSplit";
        }
        netStream: MultiInputSerializer {
            @display("p=850,320");
            gates: in[2];
        }

        //# net->app
        routerInputQueue: PacketQueue {
            @display("p=850,450");
        }
        arqSplitter: ArqSplitter {
            @display("p=750,450");
        }
        arrivalManager: ArrivalManagerSplit {
            @display("p=600,450");
        }
        netCryptoQueue: PacketQueue {
            @display("p=500,380");
            awaitSendRequests = true;
        }
        splitMerger: SplitMerger {
            @display("p=180,450");
        }
    connections:
        app$i --> flitSplitter.in;
        appInputQueue.in <-- flitSplitter.out;
        entryGuard.appIn <-- appInputQueue.out;
        entryGuard.exitIn <-- authCryptoQueue.out;

        for i=0..encUnits-1 {
            encUnit[i].in <-- entryGuard.encOut[i];
            encUnit[i].out --> exitGuard.encIn[i];
        }

        for i=0..authUnits-1 {
            authUnit[i].in <-- entryGuard.authOut[i];
            authUnit[i].out --> exitGuard.authIn[i];
        }

        exitGuard.netOut --> retransmissionBuffer.dataIn;
        netStream.in[0] <-- retransmissionBuffer.out;
        netStream.out --> router$o;

        router$i --> routerInputQueue.in;
        arqSplitter.in <-- routerInputQueue.out;
        arqSplitter.othersOut --> arrivalManager.netIn;
        arrivalManager.cryptoOut --> netCryptoQueue.in;
        arrivalManager.appOut --> splitMerger.in;
        splitMerger.out --> app$o;

        arqSplitter.arqOut --> retransmissionBuffer.arqIn;
        exitGuard.appOut --> arrivalManager.cryptoIn;
        arrivalManager.arqOut --> netStream.in[1];
        entryGuard.netIn <-- netCryptoQueue.out;
        exitGuard.entryOut --> authCryptoQueue.in;
}
