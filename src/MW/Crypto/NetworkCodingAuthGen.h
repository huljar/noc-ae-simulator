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
#include <MW/MiddlewareBase.h>
#include <Util/NetworkCodingManager.h>
#include <Util/CryptoManager.h>
#include <Messages/NcCombination_m.h>

using namespace omnetpp;

namespace HaecComm { namespace MW { namespace Crypto {

class NetworkCodingAuthGen: public MiddlewareBase {
protected:
    virtual void initialize() override;
    virtual void handleCycle(cPacket* packet);
    virtual void handleMessage(cMessage* msg) override;

private:
    cQueue *inQueue;
    cQueue *outQueue;
    Util::NetworkCodingManager *NC;
    Util::CryptoManager *CU;
};

}}} //namespace

#endif
