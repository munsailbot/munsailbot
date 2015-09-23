/* this is a helper class of static methods that encapsulates
the messy task of sending command sentences to the beaglebone */

#include "Beagle.h"

void Beagle::sendGpsSentence(int32_t lat, int32_t lon, int32_t speed, int32_t course){
    char str[100];
    size_t checksum = 0;

    sprintf(str, "$GPS,%li,%li,%li,%li,#", lat, lon, course, speed);
    for(size_t i=0; i<100; i++){
        if(str[i] != '$')
            checksum += static_cast<size_t>(str[i]);
        if(str[i] == '#')
            break;
    }
    char checkStr[30];
    itoa(checksum, checkStr, 10);
    char* inter = strcat(str, checkStr);
    char* out = strcat(inter, "|");
    Serial3.write(out);
    //Serial.println(out);
}

void Beagle::sendAutonomySentence(int enableAutonomy){
    char autStr[20];
    size_t checksum = 0;

    sprintf(autStr, "$AUT,%i,#", enableAutonomy);
    for(size_t i=0; i<20; i++){
        if(autStr[i] != '$')
            checksum += static_cast<size_t>(autStr[i]);
        if(autStr[i] == '#')
            break;
    }
    char checkStr[20];
    itoa(checksum, checkStr, 10);
    char* inter = strcat(autStr, checkStr);
    char* autOut = strcat(inter, "|");
    Serial3.write(autOut);
    Serial.write(autOut);
}
