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

#include "RouterRamm.h"
#include <Messages/Flit.h>
#include <Util/Constants.h>
#include <cmath>

using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace Routers {

Define_Module(RouterRamm);

void RouterRamm::preprocessFlit(Flit* flit, int inPort) const {
    if(inPort == -1) {
        // If the flit arrives from the local node, assign an intermediate node to it
        int targetX = flit->getTarget().x();
        int targetY = flit->getTarget().y();

        // First check if there needs to be an intermediate node (has no effect if target lies
        // on the same row or column as the sender)
        if(targetX == nodeX || targetY == nodeY) {
            // We have already "reached" the intermediate node, as there is none
            flit->setIntermediateReached(true);
        }
        else {
            // Randomly select an intermediate node from
            // within the rectangle spanned by sender and receiver
            int interX;
            int interY;

            do {
                interX = intrand(std::abs(targetX - nodeX) + 1) + std::min(targetX, nodeX); // +1 because intrand returns from [0,n) range
                interY = intrand(std::abs(targetY - nodeY) + 1) + std::min(targetY, nodeY);
            } while((interX == nodeX && interY == nodeY) || (interX == targetX && interY == targetY)); // repeat until we are not rolling this node or the target node

            // Set flit parameters
            flit->setIntermediateAddr(Address2D(interX, interY));
            flit->setIntermediateReached(false);
        }
    }
    else {
        // Check if we are the intermediate node; if yes, set the flit parameter
        int interX = flit->getIntermediateAddr().x();
        int interY = flit->getIntermediateAddr().y();

        if(interX == nodeX && interY == nodeY) {
            flit->setIntermediateReached(true);
        }
    }
}

int RouterRamm::computeDestinationPort(const Flit* flit) const {
    // Check if the intermediate node was already reached
    int targetX;
    int targetY;

    if(flit->getIntermediateReached()) {
        // Routing target is the flit destination
        targetX = flit->getTarget().x();
        targetY = flit->getTarget().y();
    }
    else {
        // Routing target is the intermediate node
        targetX = flit->getIntermediateAddr().x();
        targetY = flit->getIntermediateAddr().y();
    }

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
        ASSERT(flit->getIntermediateReached());
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
