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

#ifndef MESSAGES_ARQTIMER_H_
#define MESSAGES_ARQTIMER_H_

#include <Messages/ArqTimer_m.h>

namespace HaecComm { namespace Messages {

/**
 * Message class that is used to measure ARQ timeouts. It is mainly used by ArrivalManagers as self-messages.
 */
class ArqTimer : public ArqTimer_Base {
public:
    ArqTimer(const char *name=nullptr, short kind=0) : ArqTimer_Base(name,kind) {}
    ArqTimer(const ArqTimer& other) : ArqTimer_Base(other) {copy(other);}
    ArqTimer& operator=(const ArqTimer& other) {if (this==&other) return *this; ArqTimer_Base::operator=(other); copy(other); return *this;}
    virtual ArqTimer *dup() const override {return new ArqTimer(*this);}

    // ADD CODE HERE to redefine and implement pure virtual functions from ArqTimer_Base

    // Other helper methods
    unsigned short getGenSize() const;
    unsigned short getNumCombinations() const;

private:
    void copy(const ArqTimer& other);
};

}} //namespace

#endif /* MESSAGES_ARQTIMER_H_ */
