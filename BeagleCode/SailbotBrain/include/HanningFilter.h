#ifndef __HANNINGFILTER_H
#define __HANNINGFILTER_H

#include <math.h>

#include <BeagleUtil.h>
#include "SailbotBrain.h"

template<typename T> class HanningFilter{
private:
    T _value[3];
    uint32_t _sampleCount;

public:
    HanningFilter(){
        _value[0] = _value[1] = _value[2] = 0;
        _sampleCount = 0;
    }

    T getFilteredValue(T rawValue){
        uint32_t idx = _sampleCount % 3;
        _value[idx] = rawValue;
        _sampleCount++;

        return floor(_value[0] + (2*_value[1]) + _value[2])/4;
    }
};

#endif
