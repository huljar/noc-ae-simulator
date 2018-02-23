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
#include <Messages/ArqTimer_m.h>
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
    typedef std::set<uint32_t> IdSet;
    typedef std::pair<uint32_t, Messages::Address2D> IdSourceKey;
    typedef std::tuple<uint32_t, Messages::Address2D, uint16_t> IdSourceGevKey;
    typedef std::map<IdSourceKey, Messages::Flit*> FlitCache;
    typedef std::map<uint16_t, Messages::Flit*> GevCache;
    typedef std::map<IdSourceKey, GevCache> GenCache;

    ArrivalManagerFlit();
    virtual ~ArrivalManagerFlit();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    int arqLimit;
    int arqIssueTimeout;
    int arqResendTimeout;
    int outOfOrderIdGracePeriod;

    int gridColumns;
    int nodeId;
    int nodeX;
    int nodeY;

    // Track currently active FIDs/GIDs for each sender (to recognize redundant retransmissions)
    std::map<Messages::Address2D, IdSet> activeIds;
    std::map<Messages::Address2D, uint32_t> highestKnownIds;

    // Track amount of issued ARQs
    std::map<IdSourceKey, int> issuedArqs;

    // Cache successful MAC verifications
    std::set<IdSourceKey> ucVerified;
    std::set<IdSourceGevKey> ncVerified;

    // Cache arriving flits for uncoded variant
    FlitCache ucReceivedDataCache;
    FlitCache ucReceivedMacCache;
    FlitCache ucDecryptedDataCache;
    FlitCache ucComputedMacCache;

    // Cache arriving flits for network coded variant
    GenCache ncReceivedDataCache;
    GenCache ncReceivedMacCache;
    GenCache ncDecryptedDataCache;
    GenCache ncComputedMacCache;

private:
    void handleNetMessage(Messages::Flit* flit);
    void handleCryptoMessage(Messages::Flit* flit);
    void handleArqTimer(Messages::ArqTimer* timer);

    void ucStartDecryptAndAuth(const IdSourceKey& key);
    void ucTryVerification(const IdSourceKey& key);
    void ucTrySendToApp(const IdSourceKey& key);

    void ucCleanUp(const IdSourceKey& key);
    void ncCleanUp(const IdSourceKey& key);

    void generateArq(const IdSourceKey& key, Messages::Mode mode); // TODO: more mode options
};

}} //namespace

#endif
