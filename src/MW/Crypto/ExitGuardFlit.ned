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

package HaecComm.MW.Crypto;

import HaecComm.Buffers.PacketQueue;
import HaecComm.MW.Delay;

module ExitGuardFlit {
    parameters:
        @display("i=block/join");
        int busyCyclesEnc;
        int busyCyclesAuth;
    gates:
        input  encIn[];
        input  authIn[];
        output netOut;
        output appOut;
    submodules:
        delayEnc[sizeof(encIn)]: Delay if busyCyclesAuth - busyCyclesEnc > 1 {
            @display("p=80,80,column,100");
            waitCycles = busyCyclesAuth - busyCyclesEnc - 1;
        }
        delayAuth[sizeof(authIn)]: Delay if busyCyclesEnc - busyCyclesAuth > 1 {
            @display("p=80,80,column,100");
            waitCycles = busyCyclesEnc - busyCyclesAuth - 1;
        }
        exitGuard: ExitGuardFlitImpl {
            @display("p=200,120");
            mode = busyCyclesEnc - busyCyclesAuth;
            gates:
                encIn[sizeof(encIn)];
                authIn[sizeof(authIn)];
        }
        netSerializer: PacketQueue {
            @display("p=320,80;i=block/transport");
            syncFirstPacket = false;
        }
        appSerializer: PacketQueue {
            @display("p=320,180;i=block/transport");
            syncFirstPacket = false;
        }
    connections:
        //# NED does not allow nested if/for blocks as of OMNeT++ 5.2
        // Connect encryption units to delays or directly to the exit guard
        for i=0..sizeof(encIn) {
            encIn[i] --> delayEnc[i].in if busyCyclesAuth - busyCyclesEnc > 1;
            exitGuard.encIn[i] <-- delayEnc[i].out if busyCyclesAuth - busyCyclesEnc > 1;
            
            encIn[i] --> exitGuard.encIn[i] if busyCyclesAuth - busyCyclesEnc <= 1;
        }
        
        // Connect authentication units to delays or directly to the exit guard
        for i=0..sizeof(authIn) {
            authIn[i] --> delayAuth[i].in if busyCyclesEnc - busyCyclesAuth > 1;
            exitGuard.authIn[i] <-- delayAuth[i].out if busyCyclesEnc - busyCyclesAuth > 1;
            
            authIn[i] --> exitGuard.authIn[i] if busyCyclesEnc - busyCyclesAuth <= 1;
        }
        
        exitGuard.netOut --> netSerializer.in;
        netOut <-- netSerializer.out;
        
        exitGuard.appOut --> appSerializer.in;
        appOut <-- appSerializer.out;
}
