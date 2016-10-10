#ifndef __UTILITY_H
#define __UTILITY_H

#include "Common.h"

class Beagle{
public:
    static void sendLockSentence();
    static void sendUnlockSentence();

    static void sendGpsSentence(int32_t lat, int32_t lon, int32_t speed, int32_t course);
    static void sendAutonomySentence(int enableAutonomy);
};

#endif // __UTILITY_H
