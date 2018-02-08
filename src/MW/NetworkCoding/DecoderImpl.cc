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
		delete it->second;
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

	if(flit->getMode() == MODE_DATA) {
		// Insert flit into cache (indexed by source address and generation ID)
		auto key = std::make_pair(flit->getSource(), flit->getGidOrFid());
		cArray*& combinations = flitCache[key];
		if(!combinations)
			combinations = new cArray;
		combinations->add(flit);

		if(combinations->size() == numCombinations) {
			// TODO: do actual network decoding

			// right now we just copy the first flit a few times because
			// there is no payload yet
			for(int i = 0; i < generationSize; ++i) {
				Flit* decoded = static_cast<Flit*>(combinations->get(0))->dup();

				// Remove network coding metadata
				decoded->setGev(0);

				// Restore original flit ID into the GID field using the originalIds vector
				decoded->setGidOrFid(static_cast<uint32_t>(static_cast<Flit*>(combinations->get(0))->getOriginalIds(i)));

				// Send the decoded flit
				send(decoded, "out");
			}
			delete combinations;
			flitCache.erase(key);
		}
	}
	else if(flit->getMode() == MODE_MAC) {
		delete flit;
		// TODO: implement MAC validation in separate module (afterwards, MACs shouldn't arrive here)
	}
	else {
		delete flit;
		// TODO: do something useful for MODE_DATA_MAC and MODE_ARQ
	}
}

}}} //namespace
