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

#include "RetransmissionBuffer.h"
#include <Messages/Flit_m.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace Buffers {

Define_Module(RetransmissionBuffer);

RetransmissionBuffer::RetransmissionBuffer()
	: bufSize(10)
{
}

RetransmissionBuffer::~RetransmissionBuffer() {
	for(auto it = flitCache.begin(); it != flitCache.end(); ++it)
		delete it->second;
}

void RetransmissionBuffer::initialize() {
	bufSize = par("bufSize");
	if(bufSize < 0)
		throw cRuntimeError(this, "Retransmission buffer size must be greater or equal to 0, but received %i", bufSize);
}

void RetransmissionBuffer::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    if(strcmp(flit->getArrivalGate()->getName(), "arqIn") == 0) {
        // Confirm that this is an ARQ
        if(flit->getMode() != MODE_ARQ) {
            EV_WARN << "Received a flit on the ARQ line that is not an ARQ. Discarding it." << std::endl;
            delete flit;
            return;
        }

        // Find the requested flit
        FlitKey lookupKey = static_cast<FlitKey>(flit->getGid()); // TODO: use the correct identifier
        auto lookupResult = flitCache.find(lookupKey);

        // If it is still in the buffer, copy it and send out
        if(lookupResult != flitCache.end()) {
        	Flit* toSend = lookupResult->second->dup();
        	send(toSend, "out");
        }

        // Delete ARQ flit
        delete flit;
    }
    else {
    	// Copy flit
    	Flit* flitCopy = flit->dup();

    	// Add copy to cache
    	FlitKey flitKey = flit->getId(); // TODO: use the correct identifier
    	flitQueue.push(flitKey);
    	flitCache.emplace(flitKey, flitCopy);

    	// Check if cache size was exceeded
    	while(flitQueue.size() > bufSize) {
    		FlitKey toRemove = flitQueue.front();
    		delete flitCache.at(toRemove);
    		flitCache.erase(toRemove);
    		flitQueue.pop();
    	}

    	// Send out original flit
        send(flit, "out");
    }
}

}} //namespace
