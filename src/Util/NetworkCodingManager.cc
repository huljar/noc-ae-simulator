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

#include <NetworkCodingManager.h>

namespace HaecComm {

int NetworkCodingManager::getNextGenerationId() {
    return 42;
}

ncGen* NetworkCodingManager::getOrCreateGeneration(int i) {
    if (!knownGen[i]) { // implicit creation is okay, because we fill it now
        ncGen *g = new ncGen(getNextGenerationId(), genSize, combiSize);
        knownGen[i] = g;
        return g;
    } else {
        return knownGen[i];
    }
}

ncGen* NetworkCodingManager::getOrCreateGeneration() {
    ncGen *g = new ncGen(getNextGenerationId(), genSize, combiSize);
    knownGen[newId] = g;
    return g;
}

void ncGen::addCombination(cMessage *msg) {
    combinations->add(msg);
}

void ncGen::addMessage(cMessage *msg) {
    messages->add(msg);
}

bool ncGen::code() {
    if (!coded && isCodeable()) {
        for (int i = 0; i < c; i++) {
            // this "emulates" the combination calculation
            NcCombination *c = new NcCombination;
            c->setOrigin(this);
            combinations->add(c);
        }
        coded = true;
        return true;
    } else {
        return false;
    }
}

void ncGen::decode() {
    if (coded && isDecodeable()) {
        origin = combinations[0]->origin;
        delete (messages); // this is pretty invasive
        combinations->clear();
        message = new cArray(origin->messages);
        coded = false;
        return true;
    } else {
        return false;
    }
}

} /* namespace HaecComm */
