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

#ifndef __HAECCOMM_NETWORKCODINGBASE_H_
#define __HAECCOMM_NETWORKCODINGBASE_H_

#include <omnetpp.h>
#include <Buffers/PacketQueueBase.h>
#include <Messages/Flit.h>
#include <MW/MiddlewareBase.h>

using namespace omnetpp;

namespace HaecComm { namespace MW { namespace NetworkCoding {

class NetworkCodingBase: public MiddlewareBase {
public:
    typedef std::pair<uint32_t, Messages::Address2D> IdSourceKey;
    typedef std::vector<Messages::Flit*> FlitVector;

	NetworkCodingBase();
	virtual ~NetworkCodingBase();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override = 0;

    virtual uint32_t encodeAndSendGeneration(FlitVector& generation, const Messages::Address2D& source, const Messages::Address2D& target);
    virtual void decodeAndSendGeneration(FlitVector& combinations, uint32_t gid, const Messages::Address2D& source, const Messages::Address2D& target);
    virtual bool checkGenerationModified(FlitVector& combinations) const;
    virtual bool checkGenerationBitError(FlitVector& combinations) const;

    int generationSize;
    int numCombinations;

    bool useGlobalTransmissionIds;

    Buffers::PacketQueueBase* inputQueue;
};

}}} //namespace

#endif
