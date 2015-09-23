#include "Vec2.hpp"
#include "SailbotBrain.h"

class VectorFilter{
private:
    size_t _sampleCount;
    std::vector< Vec2<double> > _vectors;

public:
    VectorFilter();

    void addAngle(int a);
    void addVector(Vec2<double> v);
    Vec2<double> getAverage();
};
