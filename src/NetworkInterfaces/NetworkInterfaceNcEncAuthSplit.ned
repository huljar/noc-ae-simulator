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
module NetworkInterfaceNcEncAuthSplit like INetworkInterface
{
    parameters:
        @display("i=block/control");
        int id;
        bool networkCoding;
        int generationSize;
        int numCombinations;
        int encUnits;
        int encCycles;
        int authUnits;
        int authCycles;
    gates:
        inout app;
        inout router;
    submodules:
        //# app->net
        appInputQueue: PacketQueue {
            @display("p=70,210");
            awaitSendRequests = true;
        }
        entryGuard: EntryGuardSplit {
            @display("p=160,230");
            busyCyclesEnc = encCycles;
            busyCyclesAuth = authCycles;
            gates:
                encOut[encUnits];
                authOut[authUnits];
        }
        encUnit[encUnits]: EncSplit {
            @display("p=110,30,row,80");
            computeCycles = encCycles;
        }
        authUnit[authUnits]: AuthSplit {
            @display("p=110,110,row,80");
            computeCycles = authCycles;
        }
        exitGuard: ExitGuardSplit {
            @display("p=420,230");
            gates:
                encIn[encUnits];
                authIn[authUnits];
        }
        flitSplitter: FlitSplitter {
            @display("p=500,170");
        }
        encoder: Encoder {
            @display("p=430,150");
            generationSize = generationSize;
            numCombinations = numCombinations;
            internalType = "HaecComm.MW.NetworkCoding.EncoderImplSplit";
        }
        authCryptoQueue: PacketQueue {
            @display("p=300,230");
            awaitSendRequests = true;
        }
        retransmissionBuffer: RetransmissionBuffer {
            @display("p=520,250");
            internalType = "HaecComm.Buffers.RetransmissionBufferImplSplit";
        }
        netStream: MultiInputSerializer {
            @display("p=630,270");
            gates:
                in[2];
        }

        //# net->app
        routerInputQueue: PacketQueue {
            @display("p=570,365");
        }
        arqSplitter: ArqSplitter {
            @display("p=480,365");
        }
        arrivalManager: ArrivalManagerSplit {
            @display("p=380,365");
        }
        verCryptoQueue: PacketQueue {
            @display("p=270,300");
            awaitSendRequests = true;
        }
        decCryptoQueue: PacketQueue {
            @display("p=160,310");
            awaitSendRequests = true;
        }
        decoder: Decoder {
            @display("p=290,380");
            generationSize = generationSize;
            numCombinations = numCombinations;
            internalType = "HaecComm.MW.NetworkCoding.DecoderImplSplit";
        }
        splitMerger: SplitMerger {
            @display("p=210,380");
        }
    connections:
        app$i --> appInputQueue.in;
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
        arrivalManager.verOut --> verCryptoQueue.in;
        arrivalManager.decOut --> decoder.in;
        arrivalManager.appOut --> app$o;

        decoder.out --> splitMerger.in;
        decCryptoQueue.in <-- splitMerger.out;
        decCryptoQueue.out --> entryGuard.decIn;

        arqSplitter.arqOut --> retransmissionBuffer.arqIn;
        exitGuard.appOut --> arrivalManager.cryptoIn;
        arrivalManager.arqOut --> netStream.in[1];
        entryGuard.verIn <-- verCryptoQueue.out;
        exitGuard.entryOut --> authCryptoQueue.in;
        exitGuard.encoderOut --> flitSplitter.in;
        encoder.in <-- flitSplitter.out;
        encoder.out --> exitGuard.encoderIn;
}
