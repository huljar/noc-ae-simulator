/*
 * fieldtypes.h
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
	Address2D() : address(0) { }
	Address2D(uint8_t x, uint8_t y) {
		ASSERT(x < 16);
		ASSERT(y < 16);
		address = (x << 4) + y;
	}
	Address2D(const Address2D& other) : address(other.address) { }
	Address2D& operator=(const Address2D& other) { address = other.address; return *this; }

	uint8_t x() const { return address >> 4; }
	uint8_t y() const { return address & 0x0F; }
	void setX(uint8_t x) { ASSERT(x < 16); address = (x << 4) + (address & 0x0F); }
	void setY(uint8_t y) { ASSERT(y < 16); address = (address & 0xF0) + y; }

private:
	/// The X coordinate is stored in the most significant 4 bit and the
	/// Y coordinate in the least significant 4 bit.
	uint8_t address;
};

inline bool operator<(const Address2D& lhs, const Address2D& rhs) {
	return lhs.x() == rhs.x() ? lhs.y() < rhs.y() : lhs.x() < rhs.x();
}

}} //namespace

#endif /* MESSAGES_FIELDTYPES_H_ */