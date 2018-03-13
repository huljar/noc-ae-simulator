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

#include "Flit.h"
#include <Util/Constants.h>

using namespace HaecComm::Util;

namespace HaecComm { namespace Messages {

Register_Class(Flit);

bool Flit::isArq() const {
    return mode == MODE_ARQ_TELL_RECEIVED || mode == MODE_ARQ_TELL_MISSING;
}

unsigned short Flit::getGenSize() const {
    if(ncMode == NC_UNCODED)
        return 1;
    if(ncMode == NC_G2C3 || ncMode == NC_G2C4)
        return 2;

    throw cRuntimeError(this, "Unexpected NC mode: %s", cEnum::get("HaecComm::Messages::NcMode")->getStringFor(ncMode));
}

unsigned short Flit::getNumCombinations() const {
    if(ncMode == NC_UNCODED)
        return 1;
    if(ncMode == NC_G2C3)
        return 3;
    if(ncMode == NC_G2C4)
        return 4;

    throw cRuntimeError(this, "Unexpected NC mode: %s", cEnum::get("HaecComm::Messages::NcMode")->getStringFor(ncMode));
}

void Flit::setMode(uint8_t mode) {
    Flit_Base::setMode(mode);
    adjustMsgKind();
}

void Flit::setNcMode(uint8_t ncMode) {
    Flit_Base::setNcMode(ncMode);
    adjustMsgKind();
}

void Flit::adjustMsgKind() {
    setKind(isArq() ? Constants::FLIT_ARQ_KIND : (getNcMode() == NC_UNCODED ? Constants::FLIT_UNCODED_KIND : Constants::FLIT_NETWORK_CODED_KIND));
}

void Flit::setNcArqs(const GevArqMap& ncArqs) {
    Flit_Base::setNcArqs(ncArqs);

    // Add new GEVs to known GEVs
    for(auto it = ncArqs.begin(); it != ncArqs.end(); ++it)
        knownGevs.insert(it->first);
    ASSERT(knownGevs.size() <= getNumCombinations());
}

void Flit::mergeNcArqModesFlit(Mode newMode, const GevArqMap& newArqModes) {
    ASSERT(isArq() && ncMode != NC_UNCODED);

    // Add new GEVs to known GEVs
    for(auto it = newArqModes.begin(); it != newArqModes.end(); ++it)
        knownGevs.insert(it->first);
    ASSERT(knownGevs.size() <= getNumCombinations());

    // Check flit mode
    if(mode == MODE_ARQ_TELL_MISSING && newMode == MODE_ARQ_TELL_MISSING) {
        // We have a TELL_MISSING ARQ and received new TELL_MISSING modes
        // Merge using the union of both ARQ modes
        mergeNcArqModesFlitUnion(newArqModes);
    }
    else if(mode == MODE_ARQ_TELL_MISSING && newMode == MODE_ARQ_TELL_RECEIVED) {
        // We have a TELL_MISSING ARQ and received new TELL_RECEIVED modes
        // Check if we can merge to a TELL_MISSING ARQ
        if(knownGevs.size() >= getNumCombinations()) {
            // Merge to a TELL_MISSING ARQ with the new modes inverted
            GevArqMap inverted = invertNcArqModesFlit(newArqModes);
            mergeNcArqModesFlitUnion(inverted);
        }
        else {
            // Merge to a new TELL_RECEIVED ARQ which does not contain the old missing modes
            // Store the old missing modes
            GevArqMap oldMissing = ncArqs;

            // Set modes to the new modes
            setMode(MODE_ARQ_TELL_RECEIVED);
            setNcArqs(newArqModes);

            // Remove any modes that were previously marked as missing
            mergeNcArqModesFlitWithout(oldMissing);
        }
    }
    else if(mode == MODE_ARQ_TELL_RECEIVED && newMode == MODE_ARQ_TELL_MISSING) {
        // We have a TELL_RECEIVED ARQ and received new TELL_MISSING modes
        // Check if we can merge to a TELL_MISSING ARQ
        if(knownGevs.size() >= getNumCombinations()) {
            // Merge to a new TELL_MISSING ARQ with the old modes inverted
            GevArqMap inverted = invertNcArqModesFlit(ncArqs);

            // Set modes to the new modes
            setMode(MODE_ARQ_TELL_MISSING);
            setNcArqs(newArqModes);

            mergeNcArqModesFlitUnion(inverted);
        }
        else {
            // Merge to a TELL_RECEIVED ARQ which does not contain the new missing modes
            mergeNcArqModesFlitWithout(newArqModes);
        }
    }
    else {
        // This should never happen
        throw cRuntimeError(this, "Unexpected ARQ merge modes: current %s, new %s",
                            cEnum::get("HaecComm::Messages::Mode")->getStringFor(mode),
                            cEnum::get("HaecComm::Messages::Mode")->getStringFor(newMode));
    }
}

void Flit::removeFromNcArqFlit(const GevArqMap& arqModes) {
    // If we have a TELL_MISSING ARQ, remove the modes from the missing ones
    // If we have a TELL_RECEIVED ARQ, add the modes as received ones

    // Add new GEVs to known GEVs
    for(auto it = arqModes.begin(); it != arqModes.end(); ++it)
        knownGevs.insert(it->first);
    ASSERT(knownGevs.size() <= getNumCombinations());

    if(mode == MODE_ARQ_TELL_MISSING) {
        mergeNcArqModesFlitWithout(arqModes);
    }
    else if(mode == MODE_ARQ_TELL_RECEIVED) {
        mergeNcArqModesFlitUnion(arqModes);

        // Check if we can convert this to a TELL_MISSING ARQ now
        if(knownGevs.size() >= getNumCombinations()) {
            setMode(MODE_ARQ_TELL_MISSING);
            setNcArqs(invertNcArqModesFlit(getNcArqs()));
        }
    }
}

void Flit::copy(const Flit& other) {
    this->knownGevs = other.knownGevs;
}

void Flit::mergeNcArqModesFlitUnion(const GevArqMap& newArqModes) {
    // Iterate over all new modes
    for(auto it = newArqModes.begin(); it != newArqModes.end(); ++it) {
        // Check if there already is an ARQ mode for this GEV
        GevArqMap::iterator modeIter = ncArqs.find(it->first);
        if(modeIter == ncArqs.end()) {
            // This mode is not yet in the ARQ, insert it
            ncArqs.insert(*it);
        }
        // else: this mode already exists, check if it needs to be changed
        else if(it->second == ARQ_DATA_MAC || (it->second == ARQ_MAC && modeIter->second == ARQ_DATA)
                || (it->second == ARQ_DATA && modeIter->second == ARQ_MAC)) {
            modeIter->second = ARQ_DATA_MAC;
        }
        // else: no changes required
    }
}

void Flit::mergeNcArqModesFlitWithout(const GevArqMap& newArqModes) {
    // Subtract the new modes from the current modes
    for(auto it = newArqModes.begin(); it != newArqModes.end(); ++it) {
        GevArqMap::iterator mergedIter = ncArqs.find(it->first);
        if(mergedIter != ncArqs.end()) {
            if(it->second == ARQ_DATA_MAC || (it->second == ARQ_DATA && mergedIter->second == ARQ_DATA)
                    || (it->second == ARQ_MAC && mergedIter->second == ARQ_MAC)) {
                ncArqs.erase(mergedIter);
            }
            else if(it->second == ARQ_DATA && mergedIter->second == ARQ_DATA_MAC) {
                mergedIter->second = ARQ_MAC;
            }
            else if(it->second == ARQ_MAC && mergedIter->second == ARQ_DATA_MAC) {
                mergedIter->second = ARQ_DATA;
            }
        }
    }
}

GevArqMap Flit::invertNcArqModesFlit(const GevArqMap& toInvert) {
    GevArqMap inverted;
    for(auto it = knownGevs.begin(); it != knownGevs.end(); ++it) {
        GevArqMap::const_iterator toInvIter = toInvert.find(*it);
        if(toInvIter == toInvert.end())
            inverted.emplace(*it, ARQ_DATA_MAC);
        else if(toInvIter->second == ARQ_DATA)
            inverted.emplace(*it, ARQ_MAC);
        else if(toInvIter->second == ARQ_MAC)
            inverted.emplace(*it, ARQ_DATA);
    }
    return inverted;
}

}} //namespace
