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

#include "RoutingControlInfo.h"

namespace HaecComm {

RoutingControlInfo::RoutingControlInfo(int portIdx)
	: portIdx(portIdx)
{
}

RoutingControlInfo::~RoutingControlInfo() {
}

/**
 * Return the port index to be used for routing. Index of -1 means that
 * the message shall be sent to the local node output gate.
 */
int RoutingControlInfo::getPortIdx() const {
	return portIdx;
}

/**
 * Set the port index to be used for routing. Index of -1 means that
 * the message shall be sent to the local node output gate.
 */
void RoutingControlInfo::setPortIdx(int portIdx) {
	this->portIdx = portIdx;
}

} /* namespace HaecComm */
