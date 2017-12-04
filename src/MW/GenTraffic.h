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

#ifndef __HAECCOMM_GENTRAFFIC_H_
#define __HAECCOMM_GENTRAFFIC_H_

#include <omnetpp.h>
#include <MW/MiddlewareBase.h>

using namespace omnetpp;

namespace HaecComm { namespace MW {

/**
 * \brief Special middleware which generates traffic
 *
 * On each clock tick, this middleware generates a packet an
 * sends it out with a probability defined by the <em>injectionProb</em>
 * parameter. In an unclocked simulation, it currently does nothing.
 *
 * \note This module will discard any incoming packets. Its input gate
 * is supposed to stay unconnected, since this module creates its own
 * packets.
 */
class GenTraffic: public MiddlewareBase, public cListener {
public:
	GenTraffic();
	virtual ~GenTraffic();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override;

    double injectionProb;
    bool makeLargeFlits;

    int gridRows;
    int gridColumns;
    int nodeId;
    int nodeX;
    int nodeY;
};

}} //namespace

#endif
