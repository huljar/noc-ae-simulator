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

#include "NcGen.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace Util {

void NcGen::addCombination(NcCombination *msg) {
    combinations->add(msg);
}

void NcGen::addMessage(cMessage *msg) {
    messages->add(msg);
}

bool NcGen::code() {
    if (!coded && isCodeable()) {
        cMessage *orig = (cMessage *) messages->get(0); // we take the meta info from first msg
        char msgName[128] = {0};
        for (int i = 0; i < cSize; i++) {
            // this "emulates" the combination calculation
            sprintf(msgName, "%s-C%d",orig->getName(),i);
            NcCombination *c = new NcCombination(msgName);
            c->addPar("targetId");
            c->par("targetId") = orig->par("targetId");
            c->setGenerationId(id);
            c->setOrigin((long)this); // TODO Fix the pointer madness
            combinations->add(c);
        }
        coded = true;
        return true;
    } else {
        return false;
    }
}

bool NcGen::decode() {
    if (coded && isDecodeable()) {
        NcCombination *n = (NcCombination *) combinations->get(0);
        origin = (NcGen *) n->getOrigin(); // TODO Fix the pointer madness
        delete(messages); // this is pretty invasive
        combinations->clear();
        messages = origin->messages->dup();
        coded = false;
        return true;
    } else {
        return false;
    }
}

}} //namespace
