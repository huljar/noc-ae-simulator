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

#include "RouterYX.h"
#include <Messages/Flit_m.h>
#include <Util/Constants.h>

using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace Routers {

Define_Module(RouterYX);

RouterYX::RouterYX() {
}

RouterYX::~RouterYX() {
}

int RouterYX::computeDestinationPort(const Messages::Flit* flit) const {
    // Get node information
    int targetX = flit->getTarget().x();
    int targetY = flit->getTarget().y();

    // Compute destination port
    if(targetY != nodeY) {
        // Move in Y direction
        return targetY < nodeY ? Constants::NORTH_PORT : Constants::SOUTH_PORT;
    }
    else if(targetX != nodeX) {
        // Move in X direction
        return targetX < nodeX ? Constants::WEST_PORT : Constants::EAST_PORT;
    }
    else {
        // This node is the destination
        return -1;
    }
}

}} //namespace
