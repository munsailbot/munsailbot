#ifndef __SAILBOTBRAIN_H
#define __SAILBOTBRAIN_H

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "Timer.h"

//Forward delcare for global
class Timer;

typedef uint8_t byte;
typedef std::map<std::string, std::string> StringMap;

typedef struct{
    double latitude;
    double longitude;
    int windDirection;
    int windSpeed;
    int trueWindDirection;
    int trueWindSpeed;
    double gpsHeading;
    int magHeading; //should be double
    double speed;
    int main;
    int jib;
    int rudder;
} state_t;

typedef struct{
    int main;
    int jib;
    int rudder;
} motorstate_t;

extern Timer* timer;

#endif
