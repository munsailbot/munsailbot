#include "I2C.h"

namespace BeagleUtil
{

    I2C::I2C(std::string dev, int address){
        _handle = open(dev.c_str(), O_RDWR);

        if(_handle != -1){
            _ioset = ioctl(_handle, I2C_SLAVE, address);
        }
        else
            std::cout << "Error opening I2C bus at address " << address << std::endl;
    }

    I2C::~I2C(){
        close(_handle);
    }

    unsigned char I2C::readByte(){
        if(_ioset != -1){
            unsigned char ret;
            int bytesRead = read(_handle, &ret, sizeof(unsigned char));
            //std::cout << "Read: " << bytesRead << "Bytes." << std::endl;

            if(bytesRead)
                return ret;
            else
                return -1;
        }

        return -1;
    }

    void I2C::writeByte(unsigned char byte){
        unsigned char buf[1];
        buf[0] = byte;
        if(_ioset != -1){
            int bytesWritten = write(_handle, buf, 1);

            if(bytesWritten == -1)
                std::cout << "I2C: error writing byte!" << std::endl;
        }
    }

    void I2C::writeBytes(unsigned char* bytes, int count){
        if(_ioset != -1){
            int bytesWritten = write(_handle, bytes, count);
            //int bytesWritten = i2c_smbus_write_byte_data(_handle, bytes[0], bytes[1]);

            if(bytesWritten == -1)
                std::cout << "I2C: error writing bytes!" << std::endl;
        }
    }

    int I2C::getBytesAvailable(){
        int bytes;
        ioctl(_handle, FIONREAD, &bytes);

        return bytes;
    }

}
