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

#include "RouterXY.h"
#include <Util/RoutingControlInfo.h>

namespace HaecComm {

Define_Module(RouterXY);

void RouterXY::handleCycle(cPacket* packet) {
}

void RouterXY::handleMessageInternal(cPacket* packet) {
    if(!packet->hasPar("targetId")){
        EV << " got message without target - drop it like it's hot! " << packet->getName() << std::endl;
        delete packet;
    }

    int targetId = static_cast<int>(packet->par("targetId"));
    // TODO maybe this could be a independent preceding MW
    if(targetId == parent->getNodeId()){
        // set out port to NI port and pass it
        packet->setControlInfo(new RoutingControlInfo(-1));
        send(packet, "out");
        return;
    }

    // Decide on next port based on id
    int gridCols = static_cast<int>(getAncestorPar("columns"));
    int targetX = targetId % gridCols;
    int targetY = targetId / gridCols;

    RoutingControlInfo* rcInfo = new RoutingControlInfo;

    // Since it is definitely not this node (see above) the following is sufficient
    if(targetX != parent->getNodeX()){
        // Move in X direction
    	rcInfo->setPortIdx(targetX < parent->getNodeX() ? 3 : 1); // implicit knowledge
    } else {
        // Move in Y direction
        rcInfo->setPortIdx(targetY < parent->getNodeY() ? 0 : 2);
    }
    packet->setControlInfo(rcInfo);
    send(packet, "out");
}

} //namespace
