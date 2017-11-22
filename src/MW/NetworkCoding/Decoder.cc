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

#include "Decoder.h"

namespace HaecComm { namespace MW { namespace NetworkCoding {

Define_Module(Decoder);

Decoder::Decoder()
	: packetCache(nullptr)
{
}

Decoder::~Decoder() {
	delete packetCache;
}

void Decoder::initialize() {
    NetworkCodingBase::initialize();

    packetCache = new cArray;
}

void Decoder::handleMessage(cMessage* msg) {
	// Confirm that this is a packet
	if(!msg->isPacket()) {
		EV_WARN << "Received a message that is not a packet. Discarding it." << std::endl;
		delete msg;
		return;
	}

	cPacket* packet = static_cast<cPacket*>(msg); // No need for dynamic_cast or check_and_cast here

	packetCache->add(packet);

	if(packetCache->size() == numCombinations) {
		// TODO: do actual network decoding
		packetCache->clear();
		for(int i = 0; i < generationSize; ++i) {
			cPacket* newPacket = createPacket("decoded-packet");
			send(newPacket, "out");
		}
	}
}

}}} //namespace
