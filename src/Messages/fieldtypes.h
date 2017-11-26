/*
 * types.h
 *
 *  Created on: Nov 24, 2017
 *      Author: julian
 */

#ifndef MESSAGES_FIELDTYPES_H_
#define MESSAGES_FIELDTYPES_H_

#include <omnetpp.h>
#include <cinttypes>

using namespace omnetpp;

namespace HaecComm { namespace Messages {

/**
 * \brief Stores the address of a node in a 2D grid
 *
 * This class stores the address of a node in a 2D grid. It provides
 * an X and Y coordinate, which are limited to 4 bit each, i.e. both
 * lie in the [0, 15] interval.
 *
 * \remark In debug builds, providing a value greater than 15 to any
 * of the functions throws an exception at runtime.
 */
class Address2D {
public:
	Address2D() : storage(0) { }
	Address2D(uint8_t x, uint8_t y) {
		ASSERT(x < 16);
		ASSERT(y < 16);
		storage = (x << 4) + y;
	}
	Address2D(const Address2D& other) : storage(other.storage) { }
	Address2D& operator=(const Address2D& other) { storage = other.storage; return *this; }

	uint8_t x() const { return storage >> 4; }
	uint8_t y() const { return storage & 0x0F; }
	void setX(uint8_t x) { ASSERT(x < 16); storage = (x << 4) + (storage & 0x0F); }
	void setY(uint8_t y) { ASSERT(y < 16); storage = (storage & 0xF0) + y; }

private:
	uint8_t storage;
};

inline bool operator<(const Address2D& lhs, const Address2D& rhs) {
	return lhs.x() == rhs.x() ? lhs.y() < rhs.y() : lhs.x() < rhs.x();
}

}} //namespace

#endif /* MESSAGES_FIELDTYPES_H_ */
