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

#ifndef __HAECCOMM_GENTRAFFIC_H_
#define __HAECCOMM_GENTRAFFIC_H_

#include <omnetpp.h>
#include <Messages/Flit.h>
#include <MW/MiddlewareBase.h>
#include <cinttypes>
#include <map>
#include <queue>

using namespace omnetpp;

namespace HaecComm { namespace MW {

/**
 * \brief Special middleware which generates traffic
 *
 * On each clock tick, this middleware generates a flit and
 * sends it out with a probability defined by the <em>injectionProb</em>
 * parameter.
 *
 * \note This module will discard any incoming flits. Its input gate
 * is supposed to stay unconnected, since this module creates its own
 * flits.
 */
class GenTraffic: public MiddlewareBase, public cListener {
public:
	GenTraffic();
	virtual ~GenTraffic();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    virtual void generateFlit(int targetX, int targetY);
    virtual void processQueue();

    bool enabled;
    double injectionProb;
    bool generatePairs;
    bool singleTarget;
    int singleTargetId;

    bool useGlobalTransmissionIds;
    int gridRows;
    int gridColumns;
    int nodeId;
    int nodeX;
    int nodeY;

    std::queue<Messages::Flit*> sendQueue;

    simsignal_t generateFlitSignal;
};

}} //namespace

#endif
