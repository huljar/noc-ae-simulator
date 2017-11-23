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

using namespace omnetpp;

namespace HaecComm { namespace Core {

/**
 * \brief Clock class to generate a global clock signal
 *
 * The clock class generates a clock signal in a configurable, regular
 * interval. Other modules can subscribe to this signal for synchronization.
 */
class Clock: public cSimpleModule {
private:
    cMessage *timerMessage;
    simsignal_t clockSignal;
    unsigned long cycleCounter;

public:
    Clock();
    virtual ~Clock();

    unsigned long getCurrentCycle() const;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

}} //namespace

#endif /* CLOCK_H_ */
