#ifndef __WINDSTATFILTER_H
#define __WINDSTATFILTER_H

#include "SailbotBrain.h"

class WindStatFilter{
private:
    std::vector<int> _values;

public:
    void addRawValue(int val);
    int getFilteredMean();

    bool isReady();

private:
    void maxMinFilter();
    void medianFilter(std::vector<int>& r);
    int getMedianVal(int* s);

    int translateAndFilter(std::vector<int> r, int val);

    bool containsBothSigns();

};

#endif
