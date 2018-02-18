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

bool Flit::isArq() const {
    return mode == MODE_ARQ_DATA || mode == MODE_ARQ_MAC || mode == MODE_ARQ_DATA_MAC
        || mode == MODE_ARQ_SPLIT_1 || mode == MODE_ARQ_SPLIT_2;
}

void Flit::copy(const Flit& other) {
}

}} // namespace
