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
	std::string str() const {
		std::ostringstream s;
		s << '(' << +x() << "," << +y() << ')';
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

/// Equality operator
inline bool operator==(const Address2D& lhs, const Address2D& rhs) {
    return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

/// Stream insertion operator
inline std::ostream& operator<<(std::ostream& os, const Address2D& address) {
    return os << address.str();
}

/**
 * Enumerator which contains the possible values for the <em>mode</em>
 * field in flits. <b>ARQ</b> stands for <b>A</b>utomatic <b>R</b>e-transmission
 * re<b>Q</b>uest.
 */
enum Mode { // not using "enum class" (scoped enumeration) here, because the OMNeT++ msg compiler doesn't do it either
	MODE_DATA,          //!< MODE_DATA Normal data flit. This is the default value.
	MODE_MAC,           //!< MODE_MAC Flit containing a Message Authentication Code (MAC) for another flit.
	MODE_SPLIT_1,       //!< MODE_SPLIT_1 Flit where the payload consists of half data and half MAC (first part).
	MODE_SPLIT_2,       //!< MODE_SPLIT_2 Flit where the payload consists of half data and half MAC (second part).
	MODE_ARQ_DATA,      //!< MODE_ARQ_DATA Flit containing an ARQ, requesting a data flit.
	MODE_ARQ_MAC,       //!< MODE_ARQ_MAC Flit containing an ARQ, requesting a MAC flit.
	MODE_ARQ_DATA_MAC,  //!< MODE_ARQ_DATA_MAC Flit containing an ARQ, requesting both data and MAC flit.
	MODE_ARQ_SPLIT_1,   //!< MODE_ARQ_SPLIT_1 Flit containing an ARQ, requesting the first half of a split.
	MODE_ARQ_SPLIT_2,   //!< MODE_ARQ_SPLIT_2 Flit containing an ARQ, requesting the second half of a split.
	// TODO: MODE_ARQ_COMPLETE? for authGen?
};
Register_Enum(Mode, (MODE_DATA, MODE_MAC, MODE_SPLIT_1, MODE_SPLIT_2,
                     MODE_ARQ_DATA, MODE_ARQ_MAC, MODE_ARQ_DATA_MAC,
                     MODE_ARQ_SPLIT_1, MODE_ARQ_SPLIT_2));

enum Status {
    STATUS_NONE,
    STATUS_ENCRYPTING,
    STATUS_AUTHENTICATING,
    STATUS_DECRYPTING,
    STATUS_VERIFYING
};
Register_Enum(Status, (STATUS_NONE, STATUS_ENCRYPTING, STATUS_AUTHENTICATING,
                       STATUS_DECRYPTING, STATUS_VERIFYING));

/**
 * Enumerator which contains the possible network coding methods.
 * This is used as a meta field in flits.
 */
enum NC {
    NC_UNCODED,         //!< NC_UNCODED No network coding was applied to this flit.
    NC_G2C3,            //!< NC_G2C3 Network coding was applied with generation size 2 and 3 combinations.
    NC_G2C4             //!< NC_G2C4 Network coding was applied with generation size 2 and 4 combinations.
};
Register_Enum(NC, (NC_UNCODED, NC_G2C3, NC_G2C4));

}} //namespace

#endif /* MESSAGES_FIELDTYPES_H_ */
