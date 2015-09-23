#ifndef __ANALOG_H
#define __ANALOG_H

#include "Common.h"

namespace BeagleUtil
{

    class Analog{
        private:
        std::string _path;
        int _value;

        public:
        Analog(std::string pin);

        int getValue();
    };

}

#endif
