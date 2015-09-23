/*
Daniel Cook 2013
daniel@daniel-cook.net
*/

#ifndef __GPIO_H
#define __GPIO_H

#include "Common.h"

namespace BeagleUtil
{


    class GPIO{
        private:
        std::string _path;

        public:
        GPIO(std::string path);

        void exportPin(unsigned int pin);
        void unexportPin(unsigned int pin);

        void setPinDirection(unsigned int pin, std::string dir);
        void setPinValue(unsigned int pin, unsigned int value);

        unsigned int getPinValue(unsigned int pin);
    };

}

#endif
