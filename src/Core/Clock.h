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

#ifndef CLOCK_H_
#define CLOCK_H_

#include <omnetpp.h>
#include <Core/Signals.h>

using namespace omnetpp;

namespace HaecComm {

/**
 * Generates cycle ticks
 */
class Clock: public cSimpleModule {
private:
    cMessage *timerMessage;
    simsignal_t clockSignal;
    unsigned long cycleCounter;

public:
    Clock();
    virtual ~Clock();

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

}; /* namespace HaecComm */

#endif /* CLOCK_H_ */
