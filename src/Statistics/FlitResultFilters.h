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

#ifndef STATISTICS_FLITRESULTFILTERS_H_
#define STATISTICS_FLITRESULTFILTERS_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace HaecComm { namespace Statistics {

class FlitIdFilter : public cObjectResultFilter {
protected:
    virtual void receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) override;
};

class FlitSourceFilter : public cObjectResultFilter {
protected:
    virtual void receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) override;
};

class FlitTargetFilter : public cObjectResultFilter {
protected:
    virtual void receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) override;
};

}} //namespace

#endif /* STATISTICS_FLITRESULTFILTERS_H_ */
