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

#ifndef UTIL_NCGEN_H_
#define UTIL_NCGEN_H_


#include "omnetpp.h"
#include "NcCombination_m.h"

using namespace omnetpp;

namespace HaecComm {

class NcGen {
private:
    int id;
    int mSize;
    int cSize;
    bool coded;

public:
    NcGen() { throw cRuntimeError(NULL, "Trying to create NC gen without id!"); };
    NcGen(int id, int mSize, int cSize) : id(id), mSize(mSize), cSize(cSize) {
        combinations = new cArray; messages = new cArray;
    };
    virtual ~NcGen() { delete(combinations); delete(messages); };

    cArray *messages;
    cArray *combinations;
    NcGen *origin;

    void setCoded(bool b) {coded = b; };
    int getId() { return id;};
    bool isDecodeable() { return cSize == combinations->size(); };
    bool isCodeable() { return mSize == messages->size(); };

    void addCombination(NcCombination *msg);
    void addMessage(cMessage *msg);
    bool code();
    bool decode();

};

} /* namespace HaecComm */

#endif /* UTIL_NCGEN_H_ */
