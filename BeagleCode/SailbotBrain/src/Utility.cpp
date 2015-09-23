#include "Utility.h"
#include <math.h>

int Utility::strToInt(std::string str){
    int r;

    if(str != "")
        std::stringstream(str) >> r;
    else
        r = 9999;

    return r;
}

double Utility::strToDouble(std::string str){
    double r;

    if(str != "")
        std::stringstream(str) >> r;
    else
        r = 9999;

    return r;
}

void Utility::sendMotorValues(BeagleUtil::UARTInterface* serial, uint8_t main, uint8_t jib, uint8_t rudder){
    uint8_t buffer[6];
    buffer[0] = 'm';
    buffer[1] = static_cast<uint8_t>(main);
    buffer[2] = static_cast<uint8_t>(jib);
    buffer[3] = static_cast<uint8_t>(rudder);

    size_t checksum = 0;
    for(size_t i=1; i<4; i++)
        checksum += static_cast<size_t>(buffer[i]);

    buffer[4] = static_cast<uint8_t>(checksum);
    buffer[5] = static_cast<uint8_t>(checksum >> 8);
    //std::cout << std::dec << checksum << std::endl;

    //tcflush(serial->getHandle(), TCOFLUSH);
    serial->writeBytes(buffer, 6);
}
