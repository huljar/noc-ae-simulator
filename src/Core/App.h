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

#ifndef __HAECCOMM_APP_H_
#define __HAECCOMM_APP_H_

#include <omnetpp.h>
#include <Core/HaecModule.h>

using namespace omnetpp;

namespace HaecComm {

/**
 * TODO - Generated class
 */
class App : public HaecModule
{
public:
	App();
	virtual ~App();
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details);

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

private:
    cPacketQueue* sendQueue;
    cPacketQueue* receiveQueue;
};

} //namespace

#endif
