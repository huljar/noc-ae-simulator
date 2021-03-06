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

#ifndef __HAECCOMM_DELAY_H_
#define __HAECCOMM_DELAY_H_

#include <omnetpp.h>
#include <MW/MiddlewareBase.h>
#include <Util/ShiftRegister.h>

using namespace omnetpp;

namespace HaecComm { namespace MW {

/**
 * \brief Middleware to simulate computation delays
 *
 * The Delay middleware can be added to a middleware pipeline to
 * simulate computation delays. Essentially, it holds back incoming
 * messages for a certain amount of time before sending them out.
 *
 * The delay is measured in clock cycles and determined by the
 * <em>waitCycles</em> NED parameter.
 *
 * The Delay middleware does not enforce any restrictions on how many
 * messages can be held back at the same time.
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
    int waitCycles;

    Util::ShiftRegister<cArray*> shiftRegister;
};

}} //namespace

#endif
