#ifndef __ARDUINOENCODER_H
#define __ARDUINOENCODER_H

#include <BeagleUtil.h>
#include "SailbotBrain.h"

typedef enum{
    GPS_STR,
    COM_STR,
    WND_STR,
    MTR_STR,
    AUT_STR,
    LCK_STR,
    UCK_STR
} STRING_TYPE;

class ArduinoEncoder{
private:
    BeagleUtil::UARTInterface* _serial;

    StringMap _params;
    std::string _string;

    size_t _validStrings;
    size_t _checksum;
    bool _eos;

public:
    ArduinoEncoder(BeagleUtil::UARTInterface* serial);
    ~ArduinoEncoder();

    bool encode(char byte);
    void parse();

    StringMap getParams();
    size_t getValidStrings();

};

#endif
