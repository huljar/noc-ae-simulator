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

#ifndef __HAECCOMM_DECODER_H_
#define __HAECCOMM_DECODER_H_

#include <omnetpp.h>
#include <Messages/Flit.h>
#include <MW/NetworkCoding/NetworkCodingBase.h>
#include <cinttypes>
#include <map>
#include <vector>

using namespace omnetpp;

namespace HaecComm { namespace MW { namespace NetworkCoding {

/**
 * TODO - Generated class
 */
class DecoderImpl : public NetworkCodingBase {
public:
    typedef std::pair<uint32_t, Messages::Address2D> IdSourceKey;
    typedef std::vector<Messages::Flit*> FlitVector;

	DecoderImpl();
	virtual ~DecoderImpl();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

private:
    std::map<IdSourceKey, FlitVector> flitCache;
};

}}} //namespace

#endif
