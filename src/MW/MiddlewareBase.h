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

#ifndef MW_MIDDLEWAREBASE_H_
#define MW_MIDDLEWAREBASE_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace HaecComm { namespace MW {

/**
 * \brief Base class for all middleware modules
 *
 * This class provides a base abstract class that all middleware
 * modules should inherit (i.e. if the NED module description
 * implements the <em>IMiddlewareBase</em> interface, its C++ class
 * should inherit this class).
 */
class MiddlewareBase: public cSimpleModule {
public:
    MiddlewareBase();
    virtual ~MiddlewareBase();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override = 0;
};

}} //namespace

#endif /* MW_MIDDLEWAREBASE_H_ */
