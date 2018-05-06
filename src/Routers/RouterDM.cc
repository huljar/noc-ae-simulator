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

#include "RouterDM.h"

#include <Messages/Flit.h>
#include <Util/Constants.h>

using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace Routers {

Define_Module(RouterDM);

int RouterDM::computeDestinationPort(const Flit* flit) const {
    // Get node information
    int targetX = flit->getTarget().x();
    int targetY = flit->getTarget().y();

    // Compute destination port
    // First check if there is only one direction of movement
    if(targetX != nodeX && targetY == nodeY) {
        // We have to move in X direction
        return targetX < nodeX ? Constants::WEST_PORT : Constants::EAST_PORT;
    }
    if(targetY != nodeY && targetX == nodeX) {
        // We have to move in Y direction
        return targetY < nodeY ? Constants::NORTH_PORT : Constants::SOUTH_PORT;
    }

    // Check if this node is the destination
    if(targetX == nodeX && targetY == nodeY) {
        return -1;
    }

    // At this point, we know that both X and Y movement is required to reach the destination
    // However, if either X or Y port is not ready, we select the free one
    int xPort = (targetX < nodeX ? Constants::WEST_PORT : Constants::EAST_PORT);
    int yPort = (targetY < nodeY ? Constants::NORTH_PORT : Constants::SOUTH_PORT);

    bool xReady = portReadyMap.at(xPort);
    bool yReady = portReadyMap.at(yPort);

    if(xReady && !yReady) {
        // Only X is ready, so use it
        return xPort;
    }
    if(yReady && !xReady) {
        // Only Y is ready, so use it
        return yPort;
    }

    // At this point, either both are ready or both are not ready, choose randomly
    return intrand(2) ? xPort : yPort;
}

}} //namespace
