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

#ifndef __HAECCOMM_ARRIVALMANAGERFLIT_H_
#define __HAECCOMM_ARRIVALMANAGERFLIT_H_

#include <omnetpp.h>
#include <Messages/Flit.h>
#include <Util/ShiftRegister.h>
#include <cinttypes>
#include <set>
#include <vector>

using namespace omnetpp;

namespace HaecComm { namespace MW {

/**
 * TODO - Generated class
 */
class ArrivalManagerFlit : public cSimpleModule {
public:
    typedef uint32_t UcKey;
    typedef std::pair<uint32_t, uint16_t> NcKey;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    int arqLimit;
    int arqIssueTimeout;
    int arqResendTimeout;

    /// Track currently active FIDs/GIDs for each sender (to recognize redundant retransmissions)
    std::map<Messages::Address2D, std::set<uint32_t>> activeIds;

    std::map<uint32_t, Messages::Flit*> ucDecryptedDataCache;
    std::map<uint32_t, Messages::Flit*> ucComputedMacCache;
    std::map<uint32_t, Messages::Flit*> ucReceivedMacCache;

    std::map<uint32_t, std::map<uint16_t, Messages::Flit*>> ncDecryptedDataCache;
    std::map<uint32_t, std::map<uint16_t, Messages::Flit*>> ncComputedMacCache;
    std::map<uint32_t, std::map<uint16_t, Messages::Flit*>> ncReceivedMacCache;
};

}} //namespace

#endif
