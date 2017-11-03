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

#ifndef UTIL_ROUTINGCONTROLINFO_H_
#define UTIL_ROUTINGCONTROLINFO_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace HaecComm {

class RoutingControlInfo : public cObject {
public:
	RoutingControlInfo();
	RoutingControlInfo(int portIdx);
	virtual ~RoutingControlInfo();

	/**
	 * Return the port index to be used for routing. Index of -1 means that
	 * the message shall be sent to the local node output gate.
	 */
	int getPortIdx() const;

	/**
	 * Set the port index to be used for routing. Index of -1 means that
	 * the message shall be sent to the local node output gate.
	 */
	void setPortIdx(int portIdx);

private:
	int portIdx;
};

} /* namespace HaecComm */

#endif /* UTIL_ROUTINGCONTROLINFO_H_ */
