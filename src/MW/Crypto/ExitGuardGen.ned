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

import HaecComm.Buffers.Serializer;

module ExitGuardGen {
    parameters:
        @display("i=block/join");
        int generationSize;
    gates:
        input  encIn[];
        input  authIn[];
        output netOut;
        output appOut;
        output entryOut;
    submodules:
        exitGuard: ExitGuardGenImpl {
            @display("p=80,120");
            gates:
                encIn[sizeof(encIn)];
                authIn[sizeof(authIn)];
        }
        netSerializer: Serializer {
            @display("p=200,80");
        }
        appSerializer: Serializer {
            @display("p=200,180");
        }
    connections:
        // Connect encryption units to the exit guard
        for i=0..sizeof(encIn)-1 {
            encIn[i] --> exitGuard.encIn[i];
        }
        
        // Connect authentication units to the exit guard
        for i=0..sizeof(authIn)-1 {
            authIn[i] --> exitGuard.authIn[i];
        }
        
        exitGuard.netOut --> netSerializer.in;
        netOut <-- netSerializer.out;
        
        exitGuard.appOut --> appSerializer.in;
        appOut <-- appSerializer.out;
        
        entryOut <-- exitGuard.entryOut;
}
