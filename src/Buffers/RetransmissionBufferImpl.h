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

#ifndef __HAECCOMM_RETRANSMISSIONBUFFER_H_
#define __HAECCOMM_RETRANSMISSIONBUFFER_H_

#include <omnetpp.h>
#include <Messages/Flit.h>
#include <map>
#include <queue>
#include <tuple>

using namespace omnetpp;

namespace HaecComm { namespace Buffers {

/**
 * TODO - Generated class
 */
class RetransmissionBufferImpl : public cSimpleModule {
public:
    typedef std::pair<uint32_t, Messages::Mode> UcKey;
    typedef std::map<UcKey, Messages::Flit*> UncodedFlitMap;
    typedef std::tuple<uint32_t, uint16_t, Messages::Mode> NcKey;
    typedef std::map<NcKey, Messages::Flit*> NetworkCodedFlitMap;

	RetransmissionBufferImpl();
	virtual ~RetransmissionBufferImpl();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    int bufSize;

    UncodedFlitMap ucFlitCache;
    NetworkCodedFlitMap ncFlitCache;
    std::queue<UcKey> ucFlitQueue;
    std::queue<NcKey> ncFlitQueue;
};

}} //namespace

#endif
