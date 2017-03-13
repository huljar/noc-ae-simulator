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

#ifndef CORE_NETWORKCODINGMANAGER_H_
#define CORE_NETWORKCODINGMANAGER_H_

#include "omnetpp.h"

using namespace omnetpp;

namespace HaecComm {

class ncGen {
private:
    int id;
    int mSize;
    int cSize;

public:
    ncGen() { throw cRuntimeError(this, "Trying to create NC gen without id!"); };
    ncGen(int id, int mSize, int cSize) : id(id), mSize(mSize), cSize(cSize) {
        combinations = new cArray; messages = new cArray;
    };
    virtual ~ncGen() { delete(combinations); delete(messages); };

    cArray *messages;
    cArray *combinations;
    ncGen *origin;

    int getId() { return id;};
    bool isDecodeable() { return cSize == combinations->size(); };
    bool isCodeable() { return mSize == messages->size(); };

    void addCombination(NcCombination *msg);
    void addMessage(cMessage *msg);
    bool code();
    bool decode();

};


class NetworkCodingManager {
private:
    int genSize, combiSize;
    int genIdCounter;
    std::map<int, ncGen*> knownGen;

    int getNextGenerationId();

public:
    NetworkCodingManager(int genSize, int combiSize) : genSize(genSize), combiSize(combiSize) {};
    virtual ~NetworkCodingManager() {};

    ncGen* getOrCreateGeneration();
    ncGen* getOrCreateGeneration(int id);
    void deleteGeneration(int id);
};


} /* namespace HaecComm */

#endif /* CORE_NETWORKCODINGMANAGER_H_ */
