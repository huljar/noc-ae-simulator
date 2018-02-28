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

#include "DecoderImpl.h"
#include <Messages/Flit.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace MW { namespace NetworkCoding {

Define_Module(DecoderImpl);

DecoderImpl::DecoderImpl() {
}

DecoderImpl::~DecoderImpl() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            delete *jt;
}

void DecoderImpl::initialize() {
    NetworkCodingBase::initialize();
}

void DecoderImpl::handleMessage(cMessage* msg) {
	// Confirm that this is a flit
	Flit* flit = dynamic_cast<Flit*>(msg);
	if(!flit) {
		EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
		delete msg;
		return;
	}

    // Get parameters
    Mode mode = static_cast<Mode>(flit->getMode());

	if(mode == MODE_DATA) {
	    // Get parameters
	    uint32_t gid = flit->getGidOrFid();
	    const Address2D& source = flit->getSource();
        IdSourceKey key = std::make_pair(gid, flit->getSource());

		// Insert flit into cache (indexed by source address and generation ID)
		FlitVector& combinations = flitCache[key];
		combinations.push_back(flit);

        EV_DEBUG << "Caching flit " << flit->getName() << " for decoding (GID: " << gid << ", source: " << source << ")" << std::endl;
        EV_DEBUG << "We now have " << combinations.size() << " flit" << (combinations.size() != 1 ? "s" : "") << " cached for " << gid << "/" << source << std::endl;

		if(combinations.size() == static_cast<size_t>(generationSize)) {
			// TODO: do actual network decoding

            // Logging
            EV << "Decoding generation (ID: " << gid << ", " << source << "->" << flit->getTarget() << ") containing flit IDs ";
            for(int i = 0; i < generationSize - 1; ++i)
                EV << combinations[0]->getOriginalIds(i) << "+";
            EV << combinations[0]->getOriginalIds(generationSize-1) << std::endl;

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

                // Set name
                std::ostringstream packetName;
                packetName << "uc-" << fid << "-s" << decoded->getSource().str()
                           << "-t" << decoded->getTarget().str();
                decoded->setName(packetName.str().c_str());

				// Send the decoded flit
				send(decoded, "out");
			}

			// Clean up encoded flits
			for(auto it = combinations.begin(); it != combinations.end(); ++it)
			    delete *it;
			flitCache.erase(key);
		}
	}
	else if(mode == MODE_MAC) {
		delete flit;
		// TODO: implement MAC validation in separate module (afterwards, MACs shouldn't arrive here)
	}
	else {
		delete flit;
		// TODO: do something useful for MODE_DATA_MAC and MODE_ARQ
	}
}

}}} //namespace
