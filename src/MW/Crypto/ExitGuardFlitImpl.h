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

#ifndef __HAECCOMM_EXITGUARDFLIT_H_
#define __HAECCOMM_EXITGUARDFLIT_H_

#include <omnetpp.h>
#include <Messages/Flit.h>
#include <map>

using namespace omnetpp;

namespace HaecComm { namespace MW { namespace Crypto {

/**
 * TODO - Generated class
 */
class ExitGuardFlitImpl : public cSimpleModule {
public:
    typedef std::pair<uint32_t, Messages::Address2D> UcKey;
    typedef std::tuple<uint32_t, uint16_t, Messages::Address2D> NcKey;
    typedef std::map<UcKey, Messages::Flit*> UncodedFlitMap;
    typedef std::map<NcKey, Messages::Flit*> NetworkCodedFlitMap;

    ExitGuardFlitImpl();
    virtual ~ExitGuardFlitImpl();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    bool encode;

    UncodedFlitMap ucFlitCache;
    NetworkCodedFlitMap ncFlitCache;
};

}}} //namespace

#endif
