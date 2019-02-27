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

#ifndef UTIL_CONSTANTS_H_
#define UTIL_CONSTANTS_H_

/**
 * Namespace containing several constants that are used in various locations of the simulator
 */
namespace HaecComm { namespace Util { namespace Constants {
    const int NORTH_PORT = 0;
    const int EAST_PORT = 1;
    const int SOUTH_PORT = 2;
    const int WEST_PORT = 3;

    const short FLIT_UNCODED_KIND = 1;
    const short FLIT_NETWORK_CODED_KIND = 2;
    const short FLIT_ARQ_KIND = 0;

    const uint16_t GEN_MAC_GEV = 0;
}}}

#endif /* UTIL_CONSTANTS_H_ */
