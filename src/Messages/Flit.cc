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

#include "Flit.h"

namespace HaecComm { namespace Messages {

Register_Class(Flit);

void Flit::setPayloadArraySize(unsigned int size) {
    if(size == 8) { // 64 bit payload
        payload.resize(8, 0);
        setBitLength(153);
    }
    else if(size == 16) { // 128 bit payload
        payload.resize(16, 0);
        setBitLength(217);
    }
    else {
        throw cRuntimeError(this, "Invalid payload length: %u", size);
    }
}

unsigned int Flit::getPayloadArraySize() const {
    return payload.size();
}

uint8_t Flit::getPayload(unsigned int k) const {
    return payload.at(k);
}

void Flit::setPayload(unsigned int k, uint8_t payload) {
    this->payload.at(k) = payload;
}

void Flit::setSmallFlit() {
    setPayloadArraySize(8);
}

void Flit::setLargeFlit() {
    setPayloadArraySize(16);
}

bool Flit::isArq() const {
    return mode == MODE_ARQ_DATA || mode == MODE_ARQ_MAC || mode == MODE_ARQ_DATA_MAC
        || mode == MODE_ARQ_SPLIT_1 || mode == MODE_ARQ_SPLIT_2;
}

void Flit::copy(const Flit& other) {
    this->payload = other.payload;
}

}} // namespace
