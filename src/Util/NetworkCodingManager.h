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

#include <omnetpp.h>
#include <Util/NcGen.h>
#include <Messages/NcCombination_m.h>

using namespace omnetpp;

namespace HaecComm {

class NetworkCodingManager {
private:
    int genSize, combiSize;
    int genIdCounter;
    std::map<int, NcGen*> knownGen;

    int getNextGenerationId();

public:
    NetworkCodingManager(int genSize, int combiSize) : genSize(genSize), combiSize(combiSize) {};
    virtual ~NetworkCodingManager() {};

    NcGen* getOrCreateGeneration();
    NcGen* getOrCreateGeneration(int id);
    void deleteGeneration(int id);
};

} /* namespace HaecComm */

#endif /* CORE_NETWORKCODINGMANAGER_H_ */
