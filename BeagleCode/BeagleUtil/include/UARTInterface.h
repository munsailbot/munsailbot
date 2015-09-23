#ifndef __UARTINTERFACE_H
#define __UARTINTERFACE_H

#include "Common.h"
#include "UART.h"

namespace BeagleUtil
{

    class UARTInterface{
        private:
        int _handle;

        public:
        UARTInterface(UART* uart);
        ~UARTInterface();

        char readByte();
        char* readBytes(size_t numBytes);

        void writeByte(uint8_t byte);
        void writeBytes(uint8_t* bytes, size_t numBytes);
        void writeString(std::string str);

        int getHandle();
    };

}

#endif
