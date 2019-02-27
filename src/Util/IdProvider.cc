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

#include "IdProvider.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace Util {

IdProvider* IdProvider::instance = nullptr;

IdProvider* IdProvider::getInstance() {
    if(!instance)
        instance = new IdProvider();

    return instance;
}

uint32_t IdProvider::getNextFlitId() {
    return nextFlitId++;
}

uint32_t IdProvider::getNextGenId() {
    return nextGenId++;
}

uint32_t IdProvider::getNextFlitId(const Address2D& source, const Address2D& target) {
    return flitIdCounter[std::make_pair(source, target)]++;
}

uint32_t IdProvider::getNextGenId(const Address2D& source, const Address2D& target) {
    return genIdCounter[std::make_pair(source, target)]++;
}

IdProvider::IdProvider()
    : nextFlitId(0)
    , nextGenId(0)
{
}

}} //namespace
