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

#include <MW/GenTraffic.h>

namespace HaecComm {

Define_Module(GenTraffic);

void GenTraffic::handleCycle(cMessage *msg) {
    // we ignore the msg var, because we just generate traffic

    // TODO create parameter for injection prob
    double rate = par("injectionRate");
    EV << " gt hast par " << rate << std::endl;
    int r = (int) uniform(0,16);
    if(r != 4)
        return;

    int mWidth = 0, mHeight = 0;
    int trg;

    try {
        mWidth = getAncestorPar("rows");
        mHeight = getAncestorPar("columns");
    } catch (const cRuntimeError ex) {
        EV << " ancestors don't have width/height parameters!" << std::endl;
    }

    if(mWidth == 0 && mHeight == 0) {
        trg = 0;
    } else {
        // TODO create paramterized target selection class
        // Uniform target selection
        do {
            trg = (int) uniform(0, (double) mWidth*mHeight);
        } while (trg == parentId);

    }

    char msgName[128] = {0};
    sprintf(msgName, "msg-%02d-%02d-%05lu", parentId, trg, currentCycle);

    cMessage *m = createMessage(msgName);
    m->addPar("targetId");
    m->par("targetId") = trg;
    m->par("outPort")  = 0;

    EV << this->getFullPath() << " sending msg " << m << " at cycle " << currentCycle << std::endl;
    send(m, "out");

}

} //namespace
