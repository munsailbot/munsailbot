#ifndef __I2C_H
#define __I2C_H

#include "Common.h"

namespace BeagleUtil
{

    class I2C{
        private:
        int _handle;
        int _ioset;

        public:
        I2C(std::string dev, int address);
        ~I2C();

        unsigned char readByte();
        void writeByte(unsigned char byte);
        void writeBytes(unsigned char* bytes, int count);

        int getBytesAvailable();
    };

}

#endif
