#ifndef __UTILITY_H
#define __UTILITY_H

#include <BeagleUtil.h>
#include "SailbotBrain.h"

class Utility{
public:
    static int strToInt(std::string str);
    static double strToDouble(std::string str);
    static void sendMotorValues(BeagleUtil::UARTInterface* serial, uint8_t main, uint8_t jib, uint8_t rudder);
};

#endif
