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

#ifndef __HAECCOMM_NETWORKCODINGAUTHGEN_H_
#define __HAECCOMM_NETWORKCODINGAUTHGEN_H_

#include <omnetpp.h>
#include "cMiddlewareBase.h"
#include "NetworkCodingManager.h"
#include "CryptoManager.h"
#include "NcCombination_m.h"

using namespace omnetpp;

namespace HaecComm {

class NetworkCodingAuthGen: public cMiddlewareBase {
protected:
    virtual void initialize();
    virtual void handleCycle(cMessage *msg);
    virtual void handleMessageInternal(cMessage *msg);

private:
    cQueue *inQueue;
    cQueue *outQueue;
    NetworkCodingManager *NC;
    CryptoManager *CU;
};

} //namespace

#endif