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

package HaecComm.MW.Crypto;

//
// Module that implements the exit guard where flit streams from the crypto
// modules converge for the interwoven authentication protocol
//
simple ExitGuardSplitImpl {
    parameters:
        @display("i=block/cogwheel");
    gates:
        input  encIn[];
        input  authIn[];
        input  encoderIn;
        output netOut;
        output appOut;
        output entryOut;
        output encoderOut;
}
