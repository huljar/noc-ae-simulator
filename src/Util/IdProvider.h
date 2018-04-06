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

#ifndef UTIL_IDPROVIDER_H_
#define UTIL_IDPROVIDER_H_

#include <Messages/fieldtypes.h>
#include <cinttypes>
#include <utility>

namespace HaecComm { namespace Util {

class IdProvider {
public:
    static IdProvider* getInstance();

    uint32_t getNextFlitId();
    uint32_t getNextGenId();

    uint32_t getNextFlitId(const Messages::Address2D& source, const Messages::Address2D& target);
    uint32_t getNextGenId(const Messages::Address2D& source, const Messages::Address2D& target);

private:
    typedef std::pair<Messages::Address2D, Messages::Address2D> NodePair;

    IdProvider();

    uint32_t nextFlitId;
    uint32_t nextGenId;

    std::map<NodePair, uint32_t> flitIdCounter;
    std::map<NodePair, uint32_t> genIdCounter;

    static IdProvider* instance;
};

}} //namespace

#endif /* UTIL_IDPROVIDER_H_ */
