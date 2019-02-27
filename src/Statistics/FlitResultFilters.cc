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

#include "FlitResultFilters.h"
#include <Messages/Flit.h>

using namespace HaecComm::Messages;

namespace HaecComm { namespace Statistics {

Register_ResultFilter("flitId", FlitIdFilter);
Register_ResultFilter("flitSource", FlitSourceFilter);
Register_ResultFilter("flitTarget", FlitTargetFilter);
Register_ResultFilter("flitCorrupted", FlitCorruptedFilter);

void FlitIdFilter::receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) {
    if(object) {
        if(Flit* flit = check_and_cast<Flit*>(object)) {
            fire(this, t, static_cast<unsigned long>(flit->getGidOrFid()), details);
        }
    }
}

void FlitSourceFilter::receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) {
    if(object) {
        if(Flit* flit = check_and_cast<Flit*>(object)) {
            fire(this, t, static_cast<unsigned long>(flit->getSource().raw()), details);
        }
    }
}

void FlitTargetFilter::receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) {
    if(object) {
        if(Flit* flit = check_and_cast<Flit*>(object)) {
            fire(this, t, static_cast<unsigned long>(flit->getTarget().raw()), details);
        }
    }
}

void FlitCorruptedFilter::receiveSignal(cResultFilter* prev, simtime_t_cref t, cObject* object, cObject* details) {
    if(object) {
        if(Flit* flit = check_and_cast<Flit*>(object)) {
            fire(this, t, flit->isModified() || flit->hasBitError(), details);
        }
    }
}

}} //namespace
