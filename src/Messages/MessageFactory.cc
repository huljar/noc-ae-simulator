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

#include "MessageFactory.h"
#include <Util/Constants.h>

using namespace HaecComm::Util;

namespace HaecComm { namespace Messages {

ArqTimer* MessageFactory::createArqTimer(const char* name, uint32_t id, const Address2D& source, NcMode ncMode) {
    ArqTimer* arqTimer = new ArqTimer(name);
    arqTimer->setGidOrFid(id);
    arqTimer->setSource(source);
    arqTimer->setNcMode(ncMode);
    return arqTimer;
}

Flit* MessageFactory::createFlit(const char* name, const Address2D& source, const Address2D& target, Mode mode, uint32_t id,
                                 uint16_t gev, NcMode ncMode) {
    Flit* flit = new Flit(name);
    flit->setSource(source);
    flit->setTarget(target);
    flit->setMode(mode);
    flit->setGidOrFid(id);
    flit->setGev(gev);
    flit->setNcMode(ncMode);
    return flit;
}

Flit* MessageFactory::createFlit(const char* name, const Address2D& source, const Address2D& target, Mode mode, uint32_t id,
                                 uint16_t gev, NcMode ncMode, const std::vector<unsigned int>& origIds) {
    Flit* flit = createFlit(name, source, target, mode, id, gev, ncMode);
    flit->setOriginalIdsArraySize(origIds.size());
    for(size_t i = 0; i < origIds.size(); ++i)
        flit->setOriginalIds(i, origIds[i]);
    return flit;
}

}} //namespace
