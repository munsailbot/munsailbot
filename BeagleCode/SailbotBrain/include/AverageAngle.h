#ifndef __AVERAGEANGLE_H
#define __AVERAGEANGLE_H

#include "SailbotBrain.h"

class AverageAngle{
private:
    int _sampleCount;
    int _angles[2];

    bool _isReady;

public:
    AverageAngle();

    bool isReady();

    void addAngle(int angle);
    int getAverage();
};

#endif // __AVERAGEANGLE_H
