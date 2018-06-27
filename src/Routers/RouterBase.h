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

#ifndef ROUTERS_ROUTERBASE_H_
#define ROUTERS_ROUTERBASE_H_

#include <omnetpp.h>
#include <Buffers/PacketQueueBase.h>
#include <Messages/Flit.h>
#include <map>

using namespace omnetpp;

namespace HaecComm { namespace Routers {

/**
 * \brief Base class for all router module implementations
 */
class RouterBase : public cSimpleModule, public cListener {
public:
	RouterBase();
	virtual ~RouterBase();

protected:
	virtual void initialize() override;
	virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) override;

    virtual void preprocessFlit(Messages::Flit* flit, int inPort) const;
    virtual int computeDestinationPort(const Messages::Flit* flit) const = 0;
    virtual bool decideToAttack(const Messages::Flit* flit) const;

	int gridColumns;
	int nodeId;

	int nodeX;
	int nodeY;

	bool softQueueLimits;

	double attackProb;

	simsignal_t sendFlitSignal;
	simsignal_t receiveFlitSignal;
	simsignal_t forwardFlitSignal;

	std::map<int, int> modulePortMap;
	std::map<int, bool> portReadyMap;
	std::map<int, Buffers::PacketQueueBase*> portQueueMap;
	std::map<int, int> sourceDestinationCache;
};

}} //namespace

#endif /* ROUTERS_ROUTERBASE_H_ */
