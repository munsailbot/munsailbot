#include "LSM303DLM.h"

namespace SailbotBrain
{

    LSM303DLM::LSM303DLM(){
        writeRegister(DLM_ACC_REG_1, 0x27);
    }

    void LSM303DLM::writeRegister(unsigned char reg, unsigned char byte){
        _i2c->writeByte(reg);
        _i2c->writeByte(byte);
    }

}
