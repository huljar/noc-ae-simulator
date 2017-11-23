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

#ifndef __HAECCOMM_ROUTERXY_H_
#define __HAECCOMM_ROUTERXY_H_

#include <omnetpp.h>
#include <Routers/RouterBase.h>

using namespace omnetpp;

namespace HaecComm { namespace Routers {

/**
 * \brief Router with an XY routing scheme.
 *
 * This router implementation should be used with a two-dimensional
 * grid network topology. It routes the packet horizontally (in X
 * direction) until the column of the destination node is reached.
 * Then, it is routed vertically (Y direction) until the destination
 * node is reached.
 */
class RouterXY : public RouterBase {
protected:
	virtual void initialize();
	virtual void handleMessage(cMessage* msg);
};

}} //namespace

#endif
