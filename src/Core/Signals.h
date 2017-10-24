/*
 * Signals.h
 *
 *  Created on: Mar 7, 2017
 *      Author: pawa
 */

#ifndef CORE_SIGNALS_H_
#define CORE_SIGNALS_H_

/*
 * class example : public cObject, noncopyable
 * {
 *   public:
 *     cModule *module;
 *     const char *gateName;
 *     cGate::Type gateType;
 *     bool isVector;
 * };
 *
 * */

#include <omnetpp.h>

using namespace omnetpp;

namespace HaecComm {
class HaecClockSignal : public cObject, noncopyable
{
  public:
    unsigned long cycle;

    HaecClockSignal(): cycle(0) {}
    HaecClockSignal(unsigned long c): cycle(c) {}

};

}; //namespace

#endif /* CORE_SIGNALS_H_ */
