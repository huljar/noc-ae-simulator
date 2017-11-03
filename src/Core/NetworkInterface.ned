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

module NetworkInterface {
    parameters:
        @display("i=block/control");
    gates:
        inout app;
        inout router;
    submodules:
        ncEnc: NetworkCoding {
            @display("p=60,60;i=block/encrypt");
        }
        ncDec: NetworkCoding {
            @display("p=160,60;i=block/decrypt");
        }
    connections:
        ncEnc.in <-- app$i;
        ncEnc.out --> router$o;
        
        ncDec.in <-- router$i;
        ncDec.out --> app$o; // TODO: add conditional queues in between (-> locallyClocked)
}
