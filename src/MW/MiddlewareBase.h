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

class MiddlewareBase: public cSimpleModule {
public:
    MiddlewareBase();
    virtual ~MiddlewareBase();

protected:
    // If you override one of these, call the parent method as first operation!
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    // Convenience stuff
    cPacket* createPacket(const char* name);
};

}} //namespace

#endif /* MW_MIDDLEWAREBASE_H_ */
