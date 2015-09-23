#ifndef __LSM303DLM_H
#define __LSM303DLM_H

#include <BeagleUtil.h>

namespace SailbotBrain
{

    class LSM303DLM{
        private:
        BeagleUtil::I2C* _i2c;

        public:
        static const unsigned char DLM_ACC_REG_1 = 0x20;

        public:
        LSM303DLM();

        void writeRegister(unsigned char reg, unsigned char byte);
    };

}

#endif
