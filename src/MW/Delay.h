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

#ifndef __HAECCOMM_DELAY_H_
#define __HAECCOMM_DELAY_H_

#include <omnetpp.h>
#include <MW/MiddlewareBase.h>
#include <Util/ShiftRegister.h>
#include <set>

using namespace omnetpp;

namespace HaecComm {

/**
 * TODO - Generated class
 */
class Delay : public MiddlewareBase, public cListener {
public:
	Delay();
	virtual ~Delay();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

private:
    cArray* shift(); // moves everything in the vector down by 1 index. First element is popped and returned. vector is as large as waitCycles.

    bool isClocked;
    int waitCycles;
    double waitTime;

    ShiftRegister<cArray*> shiftRegister;
    std::set<cMessage*> registry;
};

} //namespace

#endif
