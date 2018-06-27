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

#ifndef __HAECCOMM_RETRANSMISSIONBUFFERIMPLFLIT_H_
#define __HAECCOMM_RETRANSMISSIONBUFFERIMPLFLIT_H_

#include <omnetpp.h>
#include <Buffers/RetransmissionBufferImplBase.h>

using namespace omnetpp;

namespace HaecComm { namespace Buffers {

/**
 * Retransmission Buffer implementation for the individual authentication protocol
 */
class RetransmissionBufferImplFlit : public RetransmissionBufferImplBase {
public:
	RetransmissionBufferImplFlit();
	virtual ~RetransmissionBufferImplFlit();

protected:
    virtual void handleArqMessage(Messages::Flit* flit) override;

private:
    bool retrieveSpecifiedFlits(const ModeCache& cache, Messages::ArqMode mode, FlitQueue& outQueue);
    bool retrieveMissingFlits(const GevCache& cache, const Messages::GevArqMap& modes, FlitQueue& outQueue);
    unsigned int countMissingFlits(const Messages::GevArqMap& modes);
};

}} //namespace

#endif
