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

#ifndef __HAECCOMM_AUTHGENERATION_H_
#define __HAECCOMM_AUTHGENERATION_H_

#include <omnetpp.h>
#include <Messages/Flit.h>
#include <MW/MiddlewareBase.h>
#include <map>
#include <vector>

using namespace omnetpp;

namespace HaecComm { namespace MW { namespace Crypto {

/**
 * TODO - Generated class
 */
class AuthGenImpl : public MiddlewareBase {
public:
    typedef std::vector<Messages::Flit*> FlitVector;

	AuthGenImpl();
	virtual ~AuthGenImpl();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;

private:
    int generationSize;
    FlitVector flitCache;
};

}}} //namespace

#endif
