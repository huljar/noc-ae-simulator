/*
 * ShiftRegister.h
 *
 *  Created on: Nov 19, 2017
 *      Author: julian
 */

#ifndef UTIL_SHIFTREGISTER_H_
#define UTIL_SHIFTREGISTER_H_

#include <omnetpp.h>
#include <utility>
#include <vector>

using namespace omnetpp;

namespace HaecComm { namespace Util {

/**
 * \brief Container template class acting like a shift register
 *
 * This generic container template implements a simple shift register using
 * an std::vector for the underlying storage.
 *
 * \tparam T The type of the contained objects
 */
template <typename T>
class ShiftRegister {
public:
	ShiftRegister() { construct(1, T()); }
	ShiftRegister(size_t size, const T& init = T()) {
		if(size < 1)
			throw cRuntimeError("ShiftRegister: size must be greater than 0");
		construct(size, init);
	}
	virtual ~ShiftRegister() { };

	T& front() { return values.front(); }
	const T& front() const { return values.front(); }

	T& back() { return values.back(); }
	const T& back() const { return values.back(); }

	T& operator[](int idx) { return values[idx]; }
	const T& operator[](int idx) const { return values[idx]; }

	size_t size() const { return values.size(); }

	typename std::vector<T>::iterator begin() { return values.begin(); }
	typename std::vector<T>::iterator end() { return values.end(); }

	T shift(const T& newElement = T()) {
		// get first element
		T front = values.front();

		// move everything one index down
		for(size_t i = 1; i < values.size(); ++i) {
			values[i-1] = std::move(values[i]);
		}

		// insert new element at the back
		values.back() = newElement;

		// return "popped" element
		return front;
	}

private:
	void construct(size_t size, const T& init) {
		values = std::vector<T>(size, init);
	}

	std::vector<T> values;
};

}} //namespace

#endif /* UTIL_SHIFTREGISTER_H_ */
