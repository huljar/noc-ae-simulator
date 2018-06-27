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

#ifndef BUFFERS_RETRANSMISSIONBUFFERIMPLBASE_H_
#define BUFFERS_RETRANSMISSIONBUFFERIMPLBASE_H_

#include <omnetpp.h>
#include <Messages/Flit.h>
#include <map>
#include <queue>

using namespace omnetpp;

namespace HaecComm { namespace Buffers {

/**
 * Base class for retransmission buffer implementations
 */
class RetransmissionBufferImplBase : public cSimpleModule {
public:
    typedef std::pair<uint32_t, Messages::Address2D> IdTargetKey;
    typedef std::map<Messages::Mode, Messages::Flit*> ModeCache;
    typedef std::map<uint16_t, ModeCache> GevCache;
    typedef std::map<IdTargetKey, ModeCache> FlitCache;
    typedef std::map<IdTargetKey, GevCache> GenCache;
    typedef std::queue<Messages::Flit*> FlitQueue;

    RetransmissionBufferImplBase();
    virtual ~RetransmissionBufferImplBase();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    virtual void handleDataMessage(Messages::Flit* flit);
    virtual void handleArqMessage(Messages::Flit* flit) = 0;

    void ucRemoveFromCache(Messages::Flit* flit);
    void ncRemoveFromCache(Messages::Flit* flit);

    int bufSize;
    bool networkCoding;
    int numCombinations;

    FlitCache ucFlitCache;
    GenCache ncFlitCache;
    FlitQueue ucFlitQueue;
    FlitQueue ncFlitQueue;
};

}} //namespace

#endif /* BUFFERS_RETRANSMISSIONBUFFERIMPLBASE_H_ */
