//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package HaecComm.NetworkInterfaces;

import HaecComm.Buffers.*;
import HaecComm.MW.*;
import HaecComm.MW.Crypto.*;
import HaecComm.MW.NetworkCoding.*;
import HaecComm.Util.*;

//
// The Network Interface for the uncoded individual authentication protocol
//
module NetworkInterfaceEncAuthFlit like INetworkInterface {
    parameters:
        @display("i=block/control");
        int id;
        bool networkCoding;
        int encUnits;
        int encCycles;
        int authUnits;
        int authCycles;
    gates:
        inout app;
        inout router;
    submodules:
        //# app->net
        helperSer: Serializer {
            @display("p=40,280");
        }
        encryptQueue: PacketQueue {
            @display("p=80,280");
            awaitSendRequests = true;
        }
        cryptoEntry: EntryGuardFlit {
            @display("p=280,300");
            busyCyclesEnc = encCycles;
            busyCyclesAuth = authCycles;
            gates:
                encOut[encUnits];
                authOut[authUnits];
        }
        encUnit[encUnits]: EncFlit {
            @display("p=150,80,row,80");
            computeCycles = encCycles;
        }
        authUnit[authUnits]: AuthFlit {
            @display("p=150,160,row,80");
            computeCycles = authCycles;
        }
        cryptoExit: ExitGuardFlit {
            @display("p=550,300");
            gates:
                encIn[encUnits];
                authIn[authUnits];
        }
        authenticateQueue: PacketQueue {
            @display("p=415,320");
            awaitSendRequests = true;
        }
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=650,280");
            internalType = "HaecComm.Buffers.RetransmissionBufferImplFlit";
        }
        netStream: MultiInputSerializer {
            @display("p=750,320");
            gates: in[2];
        }

        //# net->app
        arrivalQueue: PacketQueue {
            @display("p=750,450");
        }
        arqSplitter: ArqSplitter {
            @display("p=650,450");
        }
        arrivalManager: ArrivalManagerFlit {
            @display("p=500,450");
        }
        verifyQueue: PacketQueue {
            @display("p=400,390");
            awaitSendRequests = true;
        }
        decryptQueue: PacketQueue {
            @display("p=300,410");
            awaitSendRequests = true;
        }
    connections:
        app$i --> helperSer.in;
        encryptQueue.in <-- helperSer.out;
        cryptoEntry.appIn <-- encryptQueue.out;
        cryptoEntry.exitIn <-- authenticateQueue.out;

        for i=0..encUnits-1 {
            encUnit[i].in <-- cryptoEntry.encOut[i];
            encUnit[i].out --> cryptoExit.encIn[i];
        }

        for i=0..authUnits-1 {
            authUnit[i].in <-- cryptoEntry.authOut[i];
            authUnit[i].out --> cryptoExit.authIn[i];
        }

        cryptoExit.netOut --> retransmissionBuffer.dataIn;
        netStream.in[0] <-- retransmissionBuffer.out;
        netStream.out --> router$o;

        router$i --> arrivalQueue.in;
        arqSplitter.in <-- arrivalQueue.out;
        arqSplitter.othersOut --> arrivalManager.netIn;
        arrivalManager.verOut --> verifyQueue.in;
        arrivalManager.decOut --> decryptQueue.in;
        arrivalManager.appOut --> app$o;

        arqSplitter.arqOut --> retransmissionBuffer.arqIn;
        cryptoExit.appOut --> arrivalManager.cryptoIn;
        arrivalManager.arqOut --> netStream.in[1];
        cryptoEntry.verIn <-- verifyQueue.out;
        cryptoEntry.decIn <-- decryptQueue.out;
        cryptoExit.entryOut --> authenticateQueue.in;
}
