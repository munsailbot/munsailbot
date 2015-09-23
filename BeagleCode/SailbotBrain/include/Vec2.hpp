#ifndef __VEC2_HPP
#define __VEC2_HPP

#include <math.h>

template<typename T>
class Vec2{
public:
    T x, y;
    T theta;

public:
    Vec2(){
        x = static_cast<T>(1);
        y = static_cast<T>(0);
        theta = 0;
    }

    Vec2(T _x, T _y){
        x = _x;
        y = _y;
        theta = acos(dot(Vec2(1, 0)));
    }

    T getTheta(){
        return acos(dot(Vec2(1, 0)));
    }

    Vec2 add(Vec2 rhs){
        return Vec2(x + rhs.x, y + rhs.y);
    }

    void mult(T s){
        x *= s;
        y *= s;
    }

    T dot(Vec2 rhs){
        return static_cast<T>(x*rhs.x + y*rhs.y);
    }

    T length(){
        return static_cast<T>(sqrt(pow(x, 2) + pow(y, 2)));
    }

    void normalize(){
        x /= length();
        y /= length();
    }

    void rotate(T theta){
        x = dot(Vec2(cos(theta), sin(theta)));
        y = dot(Vec2(-sin(theta), cos(theta)));
    }
};

#endif // __VEC2_HPP
