#ifndef __WINDFILTER_H
#define __WINDFILTER_H

#include "SailbotBrain.h"

class WindFilter{
private:
    int8_t stateVector[10];
    float belief[10];

public:
    WindFilter();

    void addMeasurement(int8_t wind);
    int8_t getWindDirection();

private:
    float probability(int i, int j);
    int bestStateIdx();

};

#endif // __WINDFILTER_H
