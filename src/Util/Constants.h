/*
 * Constants.h
 *
 *  Created on: Jan 31, 2018
 *      Author: julian
 */

#ifndef UTIL_CONSTANTS_H_
#define UTIL_CONSTANTS_H_

/**
 * Namespace containing several constants that are used in various locations of the simulator
 */
namespace HaecComm { namespace Util { namespace Constants {
    const int NORTH_PORT = 0;
    const int EAST_PORT = 1;
    const int SOUTH_PORT = 2;
    const int WEST_PORT = 3;

    const short FLIT_UNCODED_KIND = 1;
    const short FLIT_NETWORK_CODED_KIND = 2;
    const short FLIT_ARQ_KIND = 0;

    const uint16_t GEN_MAC_GEV = 0;
}}}

#endif /* UTIL_CONSTANTS_H_ */
