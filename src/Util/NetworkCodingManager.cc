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

#include "NetworkCodingManager.h"

namespace HaecComm {

int NetworkCodingManager::getNextGenerationId() {
    return 42;
}

NcGen* NetworkCodingManager::getOrCreateGeneration(int i) {
    if (!knownGen[i]) { // implicit creation is okay, because we fill it now
        NcGen *g = new NcGen(getNextGenerationId(), genSize, combiSize);
        g->setCoded(true); // this is the receiving side, we receive coded stuff
        knownGen[i] = g;
        return g;
    } else {
        return knownGen[i];
    }
}

NcGen* NetworkCodingManager::getOrCreateGeneration() {
    NcGen *g = new NcGen(getNextGenerationId(), genSize, combiSize);
    knownGen[g->getId()] = g;
    return g;
}

void NetworkCodingManager::deleteGeneration(int id){
    knownGen.erase(id);
}

} /* namespace HaecComm */
