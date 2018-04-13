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

#ifndef MESSAGES_FLIT_H_
#define MESSAGES_FLIT_H_

#include <Messages/Flit_m.h>
#include <cinttypes>
#include <set>

namespace HaecComm { namespace Messages {

class Flit : public Flit_Base {
public:
    Flit(const char *name=nullptr, short kind=0) : Flit_Base(name,kind) { adjustMsgKind(); }
    Flit(const Flit& other) : Flit_Base(other) {copy(other);}
    Flit& operator=(const Flit& other) {if (this==&other) return *this; Flit_Base::operator=(other); copy(other); return *this;}
    virtual Flit *dup() const override {return new Flit(*this);}

    // ADD CODE HERE to redefine and implement pure virtual functions from Flit_Base

    // Other helper methods
    bool isArq() const;
    unsigned short getGenSize() const;
    unsigned short getNumCombinations() const;

    // Message kind (for colorization in Qtenv)
    virtual void setMode(uint8_t mode) override;
    virtual void setNcMode(uint8_t ncMode) override;
    void adjustMsgKind();

    // ARQ merging
    virtual void setNcArqs(const GevArqMap& ncArqs) override;

    void mergeNcArqModesFlit(Mode newMode, const GevArqMap& newArqModes);
    void removeFromNcArqFlit(const GevArqMap& arqModes);

    void mergeNcArqModesGen(Mode newMode, const GevArqMap& newArqModes, bool macArqMode);
    void removeFromNcArqGen(const GevArqMap& arqModes, bool removeMac);

    void mergeNcArqModesSplit(Mode newMode, const GevArqMap& newArqModes);
    void removeFromNcArqSplit(const GevArqMap& arqModes);

private:
    void copy(const Flit& other);

    // ARQ merging
    void mergeNcArqModesFlitUnion(const GevArqMap& newArqModes);
    void mergeNcArqModesFlitWithout(const GevArqMap& newArqModes);
    GevArqMap invertNcArqModesFlit(const GevArqMap& toInvert);

    void mergeNcArqModesGenUnion(const GevArqMap& newArqModes, bool newMacArqMode);
    void mergeNcArqModesGenWithout(const GevArqMap& newArqModes, bool newMacArqMode);
    GevArqMap invertNcArqDataModesGen(const GevArqMap& toInvert);

    void mergeNcArqModesSplitUnion(const GevArqMap& newArqModes);
    void mergeNcArqModesSplitWithout(const GevArqMap& newArqModes);
    GevArqMap invertNcArqModesSplit(const GevArqMap& toInvert);

    std::set<uint16_t> knownGevs;
};

}} //namespace

#endif /* MESSAGES_FLIT_H_ */
