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

#include "Clock.h"

namespace HaecComm { namespace Clocking {

Define_Module(Clock);

Clock::Clock()
    : timerMessage(nullptr)
{
}

Clock::~Clock() {
    cancelAndDelete(timerMessage);
}

void Clock::initialize(){
    cycleCounter = 0;
    clockSignal = registerSignal("clock");

    timerMessage = new cMessage("timer");
    scheduleAt(simTime(), timerMessage);
}

unsigned long Clock::getCurrentCycle() const {
	// Subtract 1 because the counter always contains the next cycle
	return cycleCounter - 1;
}

void Clock::handleMessage(cMessage *msg){
    ASSERT(msg == timerMessage);

    bubble(cycleCounter % 2 == 0 ? "tick" : "tock");
    emit(clockSignal, cycleCounter++);
    scheduleAt(simTime() + par("inter"), timerMessage);
}

}} //namespace
