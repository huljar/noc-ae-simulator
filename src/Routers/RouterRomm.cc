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

#include "RouterRomm.h"
#include <Messages/Flit.h>
#include <Util/Constants.h>
#include <cmath>

using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace Routers {

Define_Module(RouterRomm);

void RouterRomm::preprocessFlit(Flit* flit, int inPort) const {
    if(inPort == -1) {
        // If the flit arrives from the local node, randomly select an intermediate node
        // within the rectangle spanned by sender and receiver
        int targetX = flit->getTarget().x();
        int targetY = flit->getTarget().y();

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
    else {
        // Check if we are the intermediate node; if yes, set the flit parameter
        int interX = flit->getIntermediateAddr().x();
        int interY = flit->getIntermediateAddr().y();

        if(interX == nodeX && interY == nodeY) {
            flit->setIntermediateReached(true);
        }
    }
}

int RouterRomm::computeDestinationPort(const Flit* flit) const {
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
    if(targetX != nodeX) {
        // Move in X direction
        return targetX < nodeX ? Constants::WEST_PORT : Constants::EAST_PORT;
    }

    if(targetY != nodeY) {
        // Move in Y direction
        return targetY < nodeY ? Constants::NORTH_PORT : Constants::SOUTH_PORT;
    }

    // This node is the destination
    ASSERT(flit->getIntermediateReached());
    return -1;
}

}} //namespace
