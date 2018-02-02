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

using namespace HaecComm::Buffers;

namespace HaecComm { namespace MW { namespace NetworkCoding {

NetworkCodingBase::NetworkCodingBase()
	: generationSize(1)
	, numCombinations(1)
{
}

NetworkCodingBase::~NetworkCodingBase() {
}

void NetworkCodingBase::initialize() {
    MiddlewareBase::initialize();

    // subscribe to clock signal
    getSimulation()->getSystemModule()->subscribe("clock", this);

    generationSize = par("generationSize");
    if(generationSize < 1)
    	throw cRuntimeError(this, "Generation size must be greater than 0, but received %i", generationSize);

    numCombinations = par("numCombinations");
    if(numCombinations < 1)
    	throw cRuntimeError(this, "Number of combinations must be greater than 0, but received %i", numCombinations);

    // Retrieve pointer to the input queue
    cGate* inGate = gate("in");
    if(!inGate->isPathOK())
        throw cRuntimeError(this, "Input gate of the network coding module is not properly connected");

    inputQueue = check_and_cast<PacketQueueBase*>(inGate->getPathStartGate()->getOwnerModule());
}

void NetworkCodingBase::receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) {
    if(signalID == registerSignal("clock")) {
        inputQueue->requestPacket();
    }
}

}}} //namespace
