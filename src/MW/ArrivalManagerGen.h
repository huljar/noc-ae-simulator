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

#ifndef __HAECCOMM_ARRIVALMANAGERGEN_H_
#define __HAECCOMM_ARRIVALMANAGERGEN_H_

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
 * \brief Implementation of the ArrivalManager module for the full-generation authentication protocol.
 *
 * This class manages flits arriving from the network, including their forwarding to the crypto modules,
 * verification, and ARQ issuance.
 */
class ArrivalManagerGen : public cSimpleModule {
public:
    typedef std::set<uint32_t> IdSet;
    typedef std::queue<uint32_t> IdQueue;
    typedef std::set<uint16_t> GevSet;
    typedef std::vector<Messages::Flit*> FlitVector;
    typedef std::pair<uint32_t, Messages::Address2D> IdSourceKey;
    typedef std::map<uint16_t, Messages::Flit*> GevCache;
    typedef std::map<IdSourceKey, Messages::Flit*> FlitCache;
    typedef std::map<IdSourceKey, FlitVector> DecryptedCache;
    typedef std::map<IdSourceKey, GevCache> GenCache;
    typedef std::map<IdSourceKey, Messages::ArqTimer*> TimerCache;

    ArrivalManagerGen();
    virtual ~ArrivalManagerGen();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

    int arqLimit;
    int arqIssueTimeout;
    int arqAnswerTimeoutBase;
    bool lastArqWaitForOngoingVerifications;
    int finishedIdsTracked;

    bool networkCoding;
    unsigned short generationSize;

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

    // Cache arriving flits
    GenCache receivedDataCache;
    FlitCache receivedMacCache;
    DecryptedCache decryptedDataCache;
    FlitCache computedMacCache;

    // Cache GEVs that were sent to the decoder together
    std::map<IdSourceKey, std::vector<GevSet>> decodedGevs;

    // Cache if we are decoding etc. right now or waiting for the network MAC to do verification
    std::set<IdSourceKey> currentlyWorkingOn;

    // Cache successful MAC verifications
    std::set<IdSourceKey> verified;

    // Track which data flits (GEVs) were requested via ARQs
    std::map<IdSourceKey, GevSet> dataRequestedViaArq;
    std::set<IdSourceKey> macRequestedViaArq;

    // Track amount of flits that are currently undergoing decryption, but have to be discarded on arrival
    std::map<IdSourceKey, unsigned int> discardDecrypting;

    // Track planned ARQs (in case we delay an ARQ to wait for verifications to finish)
    FlitCache plannedArqs;

    // Signals
    simsignal_t generateArqSignal;

private:
    void handleNetMessage(Messages::Flit* flit);
    void handleCryptoMessage(Messages::Flit* flit);
    void handleArqTimer(Messages::ArqTimer* timer);

    bool tryStartDecodeDecryptAndAuth(const IdSourceKey& key);
    void tryVerification(const IdSourceKey& key);
    void trySendToApp(const IdSourceKey& key);
    void issueArq(const IdSourceKey& key, Messages::Mode mode, const Messages::GevArqMap& arqModes, bool macArqMode, Messages::NcMode ncMode);
    void tryRemoveDataFromPlannedArq(const IdSourceKey& key, const Messages::GevArqMap& arqModes);
    void tryRemoveMacFromPlannedArq(const IdSourceKey& key);
    void trySendPlannedArq(const IdSourceKey& key, bool forceImmediate = false);
    void cleanUp(const IdSourceKey& key);
    bool deleteFromCache(FlitCache& cache, const IdSourceKey& key);
    bool deleteFromCache(GenCache& cache, const IdSourceKey& key);
    bool deleteFromCache(GenCache& cache, const IdSourceKey& key, uint16_t gev);
    unsigned short deleteFromCache(DecryptedCache& cache, const IdSourceKey& key);

    Messages::Flit* generateArq(const IdSourceKey& key, Messages::Mode mode, const Messages::GevArqMap& arqModes, bool requestMac, Messages::NcMode ncMode);

    void setArqTimer(const IdSourceKey& key, Messages::NcMode ncMode, bool useAnswerTime = false, bool setToMax = true);
    void cancelArqTimer(const IdSourceKey& key);

    bool checkCompleteGenerationReceived(const IdSourceKey& key, unsigned short numCombinations);
    bool checkVerificationOngoing(const IdSourceKey& key) const;
    bool checkArqPlanned(const IdSourceKey& key) const;
    bool checkArqTimerActive(const IdSourceKey& key) const;
};

}} //namespace

#endif
