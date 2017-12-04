/*
 * fieldtypes.h
 *
 *  Created on: Nov 24, 2017
 *      Author: julian
 */

#ifndef MESSAGES_FIELDTYPES_H_
#define MESSAGES_FIELDTYPES_H_

#include <omnetpp.h>
#include <array>
#include <cinttypes>
#include <sstream>
#include <string>

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
	/// Default constructor. Initializes with (x,y)=(0,0).
	Address2D() : address(0) { }
	/**
	 * Constructor with a specified address.
	 *
	 * \param x X coordinate. Must be in [0,15] range (4 bit).
	 * \param y Y coordinate. Must be in [0,15] range (4 bit).
	 */
	Address2D(uint8_t x, uint8_t y) {
		ASSERT(x < 16);
		ASSERT(y < 16);
		address = (x << 4) + y;
	}
	/// Copy constructor
	Address2D(const Address2D& other) : address(other.address) { }
	/// Copy assignment operator
	Address2D& operator=(const Address2D& other) { address = other.address; return *this; }

	/// Get X coordinate
	uint8_t x() const { return address >> 4; }
	/// Get Y coordinate
	uint8_t y() const { return address & 0x0F; }
	/// Set X coordinate. Must be in [0,15] range (4 bit).
	void setX(uint8_t x) { ASSERT(x < 16); address = (x << 4) + (address & 0x0F); }
	/// Set Y coordinate. Must be in [0,15] range (4 bit).
	void setY(uint8_t y) { ASSERT(y < 16); address = (address & 0xF0) + y; }

	/// Get string representation of the address
	std::string toString() const {
		std::ostringstream s;
		s << '(' << +x() << ", " << +y() << ')';
		return s.str();
	}

private:
	/// The X coordinate is stored in the most significant 4 bit and the
	/// Y coordinate in the least significant 4 bit.
	uint8_t address;
};

/// Strict weak ordering for Address2D objects
inline bool operator<(const Address2D& lhs, const Address2D& rhs) {
	return lhs.x() == rhs.x() ? lhs.y() < rhs.y() : lhs.x() < rhs.x();
}

/**
 * Enumerator which contains the possible values for the <em>mode</em>
 * field in flits.
 */
enum Mode { // not using "enum class" (scoped enumeration) here, because the OMNeT++ msg compiler doesn't do it either
	MODE_DATA = 0,     //!< MODE_DATA Normal data flit. This is the default value.
	MODE_MAC = 1,      //!< MODE_MAC Flit containing a Message Authentication Code (MAC) for another flit.
	MODE_DATA_MAC = 2, //!< MODE_DATA_MAC Flit where the payload consists of half data and half MAC.
	MODE_ARQ = 3       //!< MODE_ARQ Flit containing an <b>A</b>utomatic <b>R</b>e-transmission re<b>Q</b>uest.
};
// TODO: make Mode a flag enum (bitwise-or allowed)?

}} //namespace

#endif /* MESSAGES_FIELDTYPES_H_ */
