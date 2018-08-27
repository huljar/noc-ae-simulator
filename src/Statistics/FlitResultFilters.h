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

/**
 * Result filter to be used with OMNeT++ statistics specifications ("flitId" in NED files).
 * This filter extracts the flit ID from signals containing a flit.
 */
class FlitIdFilter : public cObjectResultFilter {
protected:
    virtual void receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) override;
};

/**
 * Result filter to be used with OMNeT++ statistics specifications ("flitSource" in NED files).
 * This filter extracts the raw flit source from signals containing a flit.
 */
class FlitSourceFilter : public cObjectResultFilter {
protected:
    virtual void receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) override;
};

/**
 * Result filter to be used with OMNeT++ statistics specifications ("flitTarget" in NED files).
 * This filter extracts the raw flit target from signals containing a flit.
 */
class FlitTargetFilter : public cObjectResultFilter {
protected:
    virtual void receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) override;
};

/**
 * Result filter to be used with OMNeT++ statistics specifications ("flitModified" in NED files).
 * This filter extracts the corruption status (modified || bitError flags) from signals containing a flit.
 */
class FlitCorruptedFilter : public cObjectResultFilter {
protected:
    virtual void receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) override;
};

}} //namespace

#endif /* STATISTICS_FLITRESULTFILTERS_H_ */
