#include "VectorFilter.h"

VectorFilter::VectorFilter(){
    _sampleCount = 0;
    _vectors.resize(2);
}

void VectorFilter::addAngle(int a){
    Vec2<double> v;
    v.rotate(static_cast<double>(a));

    addVector(v);
}

void VectorFilter::addVector(Vec2<double> v){
    _vectors[_sampleCount %2] = v;
    _sampleCount++;
}

Vec2<double> VectorFilter::getAverage(){
    Vec2<double> a = _vectors[0].add(_vectors[1]);
    a.mult(0.5);
}
