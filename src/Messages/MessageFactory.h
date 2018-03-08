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

#ifndef MESSAGES_MESSAGEFACTORY_H_
#define MESSAGES_MESSAGEFACTORY_H_

#include <Messages/ArqTimer.h>
#include <Messages/Flit.h>

namespace HaecComm { namespace Messages {

class MessageFactory {
public:
    static ArqTimer* createArqTimer(const char* name, uint32_t id, const Address2D& source, NcMode ncMode = NC_UNCODED);

    static Flit* createFlit(const char* name, const Address2D& source, const Address2D& target, Mode mode, uint32_t id,
                            uint16_t gev = 0, NcMode ncMode = NC_UNCODED);
};

}} //namespace

#endif /* MESSAGES_MESSAGEFACTORY_H_ */
