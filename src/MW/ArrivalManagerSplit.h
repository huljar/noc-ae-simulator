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

#ifndef __HAECCOMM_ARRIVALMANAGERSPLIT_H_
#define __HAECCOMM_ARRIVALMANAGERSPLIT_H_

#include <omnetpp.h>
#include <Messages/ArqTimer.h>
#include <Messages/Flit.h>
#include <Util/ShiftRegister.h>
#include <cinttypes>
#include <map>
#include <queue>
#include <set>
#include <vector>

using namespace omnetpp;

namespace HaecComm { namespace MW {

/**
 * TODO - Generated class
 */
class ArrivalManagerSplit : public cSimpleModule {
public:
    typedef std::set<uint32_t> IdSet;
    typedef std::queue<uint32_t> IdQueue;
    typedef std::set<uint16_t> GevSet;
    typedef std::pair<uint32_t, Messages::Address2D> IdSourceKey;
    typedef std::map<uint16_t, Messages::Flit*> GevCache;
    typedef std::map<IdSourceKey, Messages::Flit*> FlitCache;
    typedef std::pair<Messages::Flit*, Messages::Flit*> SplitPair;
    typedef std::map<IdSourceKey, SplitPair> SplitCache;
    typedef std::map<IdSourceKey, GevCache> GenCache;
    typedef std::map<IdSourceKey, std::pair<bool, bool>> PairVerCache;
    typedef std::map<IdSourceKey, Messages::ArqTimer*> TimerCache;

    ArrivalManagerSplit();
    virtual ~ArrivalManagerSplit();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    int arqLimit;
    int arqIssueTimeout;
    int arqAnswerTimeoutBase;
    bool lastArqWaitForOngoingVerifications;
    int finishedIdsTracked;

    int gridColumns;
    int nodeId;
    int nodeX;
    int nodeY;

    // ARQ answer timeout for each network node
    // The values differ because the round trip time differs
    std::map<Messages::Address2D, int> arqAnswerTimeouts;

    // Track finished FIDs/GIDs for each sender (to recognize redundant retransmissions)
    std::map<Messages::Address2D, std::pair<IdSet, IdQueue>> finishedIds;

    // Track amount of issued ARQs
    std::map<IdSourceKey, unsigned int> issuedArqs;

    // Track ARQ timeouts
    TimerCache arqTimers;

    // Cache arriving flits for uncoded variant
    SplitCache ucReceivedSplitsCache;
    SplitCache ucDecryptedSplitsCache;
    SplitCache ucComputedMacsCache;

    // Cache arriving flits for network coded variant
    GenCache ncReceivedDataCache;
    GenCache ncReceivedMacCache;
    GenCache ncDecryptedDataCache;
    GenCache ncComputedMacCache;

    // Cache successful MAC verifications
    PairVerCache ucVerified;
    std::map<IdSourceKey, GevSet> ncVerified;

    // Track amount of flits that are currently undergoing decryption, but have to be discarded on arrival
    std::map<IdSourceKey, std::pair<unsigned int, unsigned int>> ucDiscardDecrypting;
    std::map<IdSourceKey, std::map<uint16_t, unsigned int>> ncDiscardDecrypting;

    // Network coding only: cache which (and how many) GEVs from a generation have been sent to the app
    std::map<IdSourceKey, GevSet> ncDispatchedGevs;

    // Network coding only: track planned ARQs (in case we delay an ARQ to wait for verifications to finish)
    FlitCache ncPlannedArqs;

private:
    void handleNetMessage(Messages::Flit* flit);
    void handleCryptoMessage(Messages::Flit* flit);
    void handleArqTimer(Messages::ArqTimer* timer);

    void ucStartDecryptAndAuth(const IdSourceKey& key, Messages::Mode mode);
    void ucTryVerification(const IdSourceKey& key, Messages::Mode mode);
    void ucTrySendToApp(const IdSourceKey& key);
    void ucIssueArq(const IdSourceKey& key, Messages::Mode mode, Messages::ArqMode arqMode);
    void ucCleanUp(const IdSourceKey& key);
    bool ucDeleteFromCache(FlitCache& cache, const IdSourceKey& key);

    void ncStartDecryptAndAuth(const IdSourceKey& key, uint16_t gev);
    void ncTryVerification(const IdSourceKey& key, uint16_t gev);
    void ncTrySendToApp(const IdSourceKey& key, uint16_t gev);
    void ncIssueArq(const IdSourceKey& key, Messages::Mode mode, const Messages::GevArqMap& arqModes, Messages::NcMode ncMode);
    void ncTryRemoveFromPlannedArq(const IdSourceKey& key, const Messages::GevArqMap& arqModes);
    void ncTrySendPlannedArq(const IdSourceKey& key, bool forceImmediate = false);
    void ncCheckGenerationDone(const IdSourceKey& key, unsigned short generationSize);
    void ncCleanUp(const IdSourceKey& key);
    bool ncDeleteFromCache(GenCache& cache, const IdSourceKey& key);
    bool ncDeleteFromCache(GenCache& cache, const IdSourceKey& key, uint16_t gev);
    // TODO: NC: do not send ARQ immediately on verification fail if we have more redundant GEVs to verify

    Messages::Flit* generateArq(const IdSourceKey& key, Messages::Mode mode, Messages::ArqMode arqMode);
    Messages::Flit* generateArq(const IdSourceKey& key, Messages::Mode mode, const Messages::GevArqMap& arqModes, Messages::NcMode ncMode);

    void setArqTimer(const IdSourceKey& key, Messages::NcMode ncMode, bool useAnswerTime = false, bool setToMax = true);
    void cancelArqTimer(const IdSourceKey& key);

    bool ncCheckCompleteGenerationReceived(const IdSourceKey& key, unsigned short numCombinations);
    bool ncCheckVerificationOngoing(const IdSourceKey& key) const;
    bool ncCheckArqPlanned(const IdSourceKey& key) const;
};

}} //namespace

#endif
