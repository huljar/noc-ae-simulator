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

#include "NetworkCodingBase.h"
#include <Messages/Flit.h>
#include <Util/IdProvider.h>

using namespace HaecComm::Buffers;
using namespace HaecComm::Messages;
using namespace HaecComm::Util;

namespace HaecComm { namespace MW { namespace NetworkCoding {

NetworkCodingBase::NetworkCodingBase()
	: generationSize(1)
	, numCombinations(1)
    , useGlobalTransmissionIds(false)
{
}

NetworkCodingBase::~NetworkCodingBase() {
}

void NetworkCodingBase::initialize() {
    MiddlewareBase::initialize();

    generationSize = par("generationSize");
    if(generationSize < 1)
    	throw cRuntimeError(this, "Generation size must be greater than 0, but received %i", generationSize);

    numCombinations = par("numCombinations");
    if(numCombinations < 1)
    	throw cRuntimeError(this, "Number of combinations must be greater than 0, but received %i", numCombinations);

    useGlobalTransmissionIds = getAncestorPar("useGlobalTransmissionIds");
}

uint32_t NetworkCodingBase::encodeAndSendGeneration(FlitVector& generation, const Address2D& source, const Address2D& target) {
    // Get local or global generation ID
    IdProvider* idp = IdProvider::getInstance();
    uint32_t gid = useGlobalTransmissionIds ? idp->getNextGenId() : idp->getNextGenId(source, target);

    // TODO: do actual network coding

    // Logging
    EV << "Creating new generation (ID: " << gid << ", " << source << "->" << target << ") from flit IDs ";
    for(int i = 0; i < generationSize - 1; ++i)
        EV << generation[i]->getGidOrFid() << "+";
    EV << generation[generationSize-1]->getGidOrFid() << std::endl;

    // right now we just copy the first flit a few times because
    // there is no payload yet
    for(int i = 0; i < numCombinations; ++i) {
        Flit* combination = generation[0]->dup();

        // Set original IDs vector
        combination->setOriginalIdsArraySize(generationSize);
        for(int j = 0; j < generationSize; ++j) {
            combination->setOriginalIds(j, generation[j]->getGidOrFid());
        }

        // Set network coding metadata
        combination->setGidOrFid(gid);
        combination->setGev(static_cast<uint16_t>(i));

        if(generationSize == 2 && numCombinations == 3)
            combination->setNcMode(NC_G2C3);
        else if(generationSize == 2 && numCombinations == 4)
            combination->setNcMode(NC_G2C4);
        else
            throw cRuntimeError(this, "Cannot set NC mode to NC_G%uC%u", generationSize, numCombinations);

        // Set name
        std::ostringstream packetName;
        packetName << "nc-" << gid << "-" << i << "-s" << combination->getSource().str()
                   << "-t" << combination->getTarget().str();
        combination->setName(packetName.str().c_str());

        // Check if the mode needs to be adjusted (there is no distinction between first and second split after encoding)
        if(combination->getMode() == MODE_SPLIT_1 || combination->getMode() == MODE_SPLIT_2)
            combination->setMode(MODE_SPLIT_NC);

        // Send the encoded flit
        send(combination, "out");
    }

    return gid;
}

void NetworkCodingBase::decodeAndSendGeneration(FlitVector& combinations, uint32_t gid, const Address2D& source, const Address2D& target) {
    // TODO: do actual network decoding

    // Logging
    EV << "Decoding generation (ID: " << gid << ", " << source << "->" << target << ") containing flit IDs ";
    for(int i = 0; i < generationSize - 1; ++i)
        EV << combinations[0]->getOriginalIds(i) << "+";
    EV << combinations[0]->getOriginalIds(generationSize-1) << std::endl;

    // Check if a combination was modified
    bool modified = checkGenerationModified(combinations);
    bool bitError = checkGenerationBitError(combinations);

    // right now we just copy the first flit a few times because
    // there is no payload yet
    for(int i = 0; i < generationSize; ++i) {
        Flit* decoded = combinations[0]->dup();

        // Remove network coding metadata
        decoded->setGev(0);
        decoded->setNcMode(NC_UNCODED);

        // Get original flit ID
        uint32_t fid = static_cast<uint32_t>(decoded->getOriginalIds(i));

        // Restore original flit ID
        decoded->setGidOrFid(fid);

        // Set first original ID to the old generation ID (required)
        decoded->setOriginalIds(0, static_cast<long>(gid));

        // Set modified/bit error bits
        decoded->setModified(modified);
        decoded->setBitError(bitError);

        // Set name
        std::ostringstream packetName;
        packetName << "uc-" << fid << "-s" << decoded->getSource().str()
                   << "-t" << decoded->getTarget().str();
        decoded->setName(packetName.str().c_str());

        // Check if the mode needs to be adjusted
        if(decoded->getMode() == MODE_SPLIT_NC) {
            if(i == 0)
                decoded->setMode(MODE_SPLIT_1);
            else if(i == 1)
                decoded->setMode(MODE_SPLIT_2);
        }

        // Send the decoded flit
        send(decoded, "out");
    }
}

bool NetworkCodingBase::checkGenerationModified(FlitVector& combinations) const {
    for(auto it = combinations.begin(); it != combinations.end(); ++it) {
        if((*it)->isModified())
            return true;
    }
    return false;
}

bool NetworkCodingBase::checkGenerationBitError(FlitVector& combinations) const {
    for(auto it = combinations.begin(); it != combinations.end(); ++it) {
        if((*it)->hasBitError())
            return true;
    }
    return false;
}

}}} //namespace
