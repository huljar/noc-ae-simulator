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

#include "RouterXY.h"
#include <Messages/Flit.h>
#include <Util/Constants.h>

using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace Routers {

Define_Module(RouterXY);

int RouterXY::computeDestinationPort(const Flit* flit) const {
    // Get node information
    int targetX = flit->getTarget().x();
    int targetY = flit->getTarget().y();

    // Compute destination port
    if(targetX != nodeX) {
        // Move in X direction
        return targetX < nodeX ? Constants::WEST_PORT : Constants::EAST_PORT;
    }

    if(targetY != nodeY) {
        // Move in Y direction
        return targetY < nodeY ? Constants::NORTH_PORT : Constants::SOUTH_PORT;
    }

    // This node is the destination
    return -1;
}

}} //namespace
