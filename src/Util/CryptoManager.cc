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

#include "CryptoManager.h"

namespace HaecComm {

CryptoManager::CryptoManager(int units, int cycles, cQueue *out){
    cus = (CryptoUnit *) malloc(units * sizeof(CryptoUnit));
    for(int i = 0; i < units; i++){
        freeUnits.push(i);
        cus[i].remainingCycles = 0;
        cus[i].processedMsg = NULL;
    }
    this->cycles = cycles;
    outQueue = out;
}

CryptoManager::~CryptoManager(){
    delete(cus);
}

bool CryptoManager::processMessage(cMessage *msg, int unitId){
    if(unitId >= units || cus[unitId].remainingCycles != 0)
        return false;

    cus[unitId].remainingCycles = 0;
    cus[unitId].processedMsg = msg;
    return true;
}

bool CryptoManager::enqueue(cMessage *msg){
    if(q.getLength() <= queueLength) {
        q.insert(msg);
        return true;
    } else {
        // the issuing class should drop the packet
        return false;
    }
}

void CryptoManager::tick(){
    // process queued msgs with free units
    if( !q.isEmpty() && available){
        freeUnits.pop();
        processMessage((cMessage *)q.pop(), freeUnits.front());
    }

    // process crypto cycles and send out ready msgs
    for(int i = 0; i < units; i++){
        CryptoUnit *cu = &cus[i];
        if(cu->remainingCycles == 0 && cu->processedMsg == NULL) // this is a free unit
            continue;

        // in use units
        if(!(--cu->remainingCycles)) { // this always decrements
            // if it reaches 0
            outQueue->insert(cu->processedMsg);
            cu->processedMsg = NULL;
        }
    }
}


} /* namespace HaecComm */
