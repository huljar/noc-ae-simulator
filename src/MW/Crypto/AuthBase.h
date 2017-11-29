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

#ifndef MW_CRYPTO_AUTHBASE_H_
#define MW_CRYPTO_AUTHBASE_H_

#include <omnetpp.h>
#include <MW/MiddlewareBase.h>

using namespace omnetpp;

namespace HaecComm { namespace MW { namespace Crypto {

class AuthBase: public MiddlewareBase {
public:
	AuthBase();
	virtual ~AuthBase();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override = 0;
};

}}} //namespace

#endif /* MW_CRYPTO_AUTHBASE_H_ */
