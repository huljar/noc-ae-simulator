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

package HaecComm.Core;

//
// Default implementation of the channel used to connect two routers together.
//
channel HaecChannel extends ned.DatarateChannel like IHaecChannel {
    datarate = default(0bps);
    delay = default(0s);
    ber = default(0); // bit error rate
    per = default(0); // packet error rate
}
