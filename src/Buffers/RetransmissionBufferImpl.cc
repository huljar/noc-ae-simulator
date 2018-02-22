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

#include "RetransmissionBufferImpl.h"

using namespace HaecComm::Messages;

namespace HaecComm { namespace Buffers {

Define_Module(RetransmissionBufferImpl);

RetransmissionBufferImpl::RetransmissionBufferImpl()
	: bufSize(10)
{
}

RetransmissionBufferImpl::~RetransmissionBufferImpl() {
    for(auto it = ucFlitCache.begin(); it != ucFlitCache.end(); ++it)
        delete it->second;
    for(auto it = ncFlitCache.begin(); it != ncFlitCache.end(); ++it)
        delete it->second;
}

void RetransmissionBufferImpl::initialize() {
	bufSize = par("bufSize");
	if(bufSize < 0)
		throw cRuntimeError(this, "Retransmission buffer size must be greater or equal to 0, but received %i", bufSize);
}

void RetransmissionBufferImpl::handleMessage(cMessage* msg) {
    // Confirm that this is a flit
    Flit* flit = dynamic_cast<Flit*>(msg);
    if(!flit) {
        EV_WARN << "Received a message that is not a flit. Discarding it." << std::endl;
        delete msg;
        return;
    }

    // Check if this is an ARQ or not
    if(flit->isArq()) {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "arqIn") == 0);

        // Check what kind of ARQ we have
        Mode mode = static_cast<Mode>(flit->getMode());
        if(mode == MODE_ARQ_DATA || mode == MODE_ARQ_MAC || mode == MODE_ARQ_SPLIT_1 || mode == MODE_ARQ_SPLIT_2) {
            // Find and copy the requested flit
            Flit* cached = nullptr;
            uint32_t id = flit->getGidOrFid();

            if(flit->getNcMode() == NC_UNCODED) {
                UcKey key = std::make_pair(id, mode);
                auto res = ucFlitCache.find(key);
                if(res != ucFlitCache.end())
                    cached = res->second->dup();
            }
            else {
                NcKey key = std::make_tuple(id, flit->getGev(), mode);
                auto res = ncFlitCache.find(key);
                if(res != ncFlitCache.end())
                    cached = res->second->dup();
            }

            // If lookup was successful, send out the copy
            if(cached)
                send(cached, "arqOut");
        }
        else if(flit->getMode() == MODE_ARQ_DATA_MAC) {
            // Find and copy the requested flits
            Flit* cachedData = nullptr;
            Flit* cachedMac = nullptr;
            uint32_t id = flit->getGidOrFid();

            if(flit->getNcMode() == NC_UNCODED) {
                UcKey key = std::make_pair(id, MODE_DATA);
                auto res = ucFlitCache.find(key);
                if(res != ucFlitCache.end())
                    cachedData = res->second->dup();
                key = std::make_pair(id, MODE_MAC);
                res = ucFlitCache.find(key);
                if(res != ucFlitCache.end())
                    cachedMac = res->second->dup();
            }
            else {
                NcKey key = std::make_tuple(id, flit->getGev(), MODE_DATA);
                auto res = ncFlitCache.find(key);
                if(res != ncFlitCache.end())
                    cachedData = res->second->dup();
                key = std::make_tuple(id, flit->getGev(), MODE_MAC);
                res = ncFlitCache.find(key);
                if(res != ncFlitCache.end())
                    cachedMac = res->second->dup();
            }

            // If lookup of both data and MAC was successful, send out the copies
            if(cachedData && cachedMac) {
                send(cachedData, "arqOut");
                send(cachedMac, "arqOut");
            }
            else {
                delete cachedData;
                delete cachedMac;
            }
        }
        // TODO: handle ARQs where GEV of missing flit(s) is not known

        // Delete ARQ flit
        delete flit;
    }
    else {
        ASSERT(strcmp(flit->getArrivalGate()->getName(), "dataIn") == 0);

        // Add copy to cache
        if(flit->getNcMode() == NC_UNCODED) {
            UcKey key = std::make_pair(flit->getGidOrFid(), static_cast<Mode>(flit->getMode()));
            ucFlitCache.emplace(key, flit->dup());
            ucFlitQueue.push(key);
        }
        else {
            NcKey key = std::make_tuple(flit->getGidOrFid(), flit->getGev(), static_cast<Mode>(flit->getMode()));
            ncFlitCache.emplace(key, flit->dup());
            ncFlitQueue.push(key);
        }

        // Check if cache size was exceeded
        while(ucFlitQueue.size() > static_cast<size_t>(bufSize)) {
            UcKey del = ucFlitQueue.front();
            delete ucFlitCache.at(del);
            ucFlitCache.erase(del);
            ucFlitQueue.pop();
        }
        while(ncFlitQueue.size() > static_cast<size_t>(bufSize)) {
            NcKey del = ncFlitQueue.front();
            delete ncFlitCache.at(del);
            ncFlitCache.erase(del);
            ncFlitQueue.pop();
        }

        // Send out original flit
        send(flit, "dataOut");
    }
}

}} //namespace
