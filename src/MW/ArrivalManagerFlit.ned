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

package HaecComm.MW;

//
// TODO auto-generated module
//
simple ArrivalManagerFlit {
    parameters:
        @display("i=block/decrypt");
        int arqLimit;
        int arqIssueTimeout;
        int arqResendTimeout;
        int outOfOrderIdGracePeriod = default(2);
    gates:
        input  netIn;
        input  cryptoIn;
        output appOut;
        output cryptoOut;
        output arqOut;
}