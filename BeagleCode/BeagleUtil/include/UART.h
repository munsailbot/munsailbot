#ifndef __UART_H
#define __UART_H

#include "Common.h"

namespace BeagleUtil
{

    class UART{
        private:
        //std::string _path;
        UART_PORT _port;
        speed_t _baud;

        public:
        UART(UART_PORT port, unsigned short flags, speed_t baud);

        UART_PORT getPort();
        //std::string getPath();
        speed_t getBaud();
    };

}

#endif
