//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __HAECCOMM_RETRANSMISSIONBUFFERIMPLSPLIT_H_
#define __HAECCOMM_RETRANSMISSIONBUFFERIMPLSPLIT_H_

#include <omnetpp.h>
#include <Buffers/RetransmissionBufferImplBase.h>

using namespace omnetpp;

namespace HaecComm { namespace Buffers {

/**
 * Retransmission Buffer implementation for the interwoven authentication protocol
 */
class RetransmissionBufferImplSplit : public RetransmissionBufferImplBase {
public:
	RetransmissionBufferImplSplit();
	virtual ~RetransmissionBufferImplSplit();

protected:
    virtual void handleArqMessage(Messages::Flit* flit) override;

private:
    bool retrieveSpecifiedFlits(const ModeCache& cache, Messages::ArqMode mode, FlitQueue& outQueue, bool networkCoded);
    bool retrieveMissingFlits(const GevCache& cache, const Messages::GevArqMap& modes, FlitQueue& outQueue);
    unsigned int countMissingFlits(const Messages::GevArqMap& modes);
};

}} //namespace

#endif
