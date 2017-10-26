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

namespace HaecComm {

Define_Module(RouterXY);

void RouterXY::handleMessageInternal(cMessage *msg)
{
    if(!msg->hasPar("targetId")){
        EV << " got message without target - drop it like it's hot! " << msg->str() << std::endl;
        dropAndDelete(msg);
    }

    int targetId = (int) msg->par("targetId");
    // TODO maybe this could be a independent preceding MW
    if(targetId == parentId){
        // set out port to NI port and pass it
        msg->par("outPort") = 4;
        send(msg,"out");
        return;
    }

    // Decide on next port based on id
    int mCols = (int) getAncestorPar("columns");
    int tX = targetId % mCols;
    int tY = targetId / mCols;

    // Since it is definitely not this node (see above) the following is sufficient
    if(tX != X){
        // Move in X direction
        msg->par("outPort") = tX < X ? 3 : 1; // implicit knowledge
    } else {
        // Move in Y direction
        msg->par("outPort") = tY < Y ? 0 : 2;
    }
    send(msg,"out");
}

} //namespace
