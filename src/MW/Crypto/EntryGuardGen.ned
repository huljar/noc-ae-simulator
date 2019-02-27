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
// Module that is used to manage the crypto modules in the full-generation authentication protocol
//
simple EntryGuardGen {
    parameters:
        @signal[enc*Busy](type=bool);
        @signal[auth*Busy](type=bool);
        @statisticTemplate[encBusy](record=timeavg,vector?; title="Encryption Unit Busy Status $name");
        @statisticTemplate[authBusy](record=timeavg,vector?; title="Authentication Unit Busy Status $name");
        @display("i=block/fork");
        int busyCyclesEnc;
        int busyCyclesAuth;
    gates:
        input  appIn;
        input  exitIn;
        input  netIn;
        output encOut[];
        output authOut[];
}
