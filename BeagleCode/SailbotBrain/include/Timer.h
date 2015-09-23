#ifndef __TIMER_H
#define __TIMER_H

#include "SailbotBrain.h"

/* Utility class that implements timing functions,
 * in particular, the millis() method found on the arduino
 */

class Timer{
private:
    struct timespec _init;

public:
    Timer();

    uint64_t millis();

private:
    timespec* timeSpecDiff(struct timespec *ts1, struct timespec *ts2);
};

#endif
