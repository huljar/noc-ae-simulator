//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "ArqTimer.h"

namespace HaecComm { namespace Messages {

Register_Class(ArqTimer);

unsigned short ArqTimer::getGenSize() const {
    if(ncMode == NC_UNCODED)
        return 1;
    if(ncMode == NC_G2C3 || ncMode == NC_G2C4)
        return 2;

    throw cRuntimeError(this, "Unexpected NC mode: %s", cEnum::get("HaecComm::Messages::NcMode")->getStringFor(ncMode));
}

unsigned short ArqTimer::getNumCombinations() const {
    if(ncMode == NC_UNCODED)
        return 1;
    if(ncMode == NC_G2C3)
        return 3;
    if(ncMode == NC_G2C4)
        return 4;

    throw cRuntimeError(this, "Unexpected NC mode: %s", cEnum::get("HaecComm::Messages::NcMode")->getStringFor(ncMode));
}

void ArqTimer::copy(const ArqTimer& other) {
}

}} //namespace
