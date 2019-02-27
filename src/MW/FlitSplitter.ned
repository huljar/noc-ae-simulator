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

package HaecComm.MW;

import HaecComm.Buffers.Serializer;

//
// Wrapper module for the flit splitter that is used in the interwoven authentication protocol
//
module FlitSplitter like IMiddlewareBase {
    parameters:
        @display("i=block/tx");
    gates:
        input  in;
        output out;
    submodules:
        flitSplitterImpl: FlitSplitterImpl {
            @display("p=80,100");
        }
        serializer: Serializer {
            @display("p=180,100");
        }
    connections:
        in --> flitSplitterImpl.in;
        serializer.in <-- flitSplitterImpl.out;
        serializer.out --> out;
}
