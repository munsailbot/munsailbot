#include "UART.h"

namespace BeagleUtil
{

    UART::UART(UART_PORT port, unsigned short flags, speed_t baud){
        //std::ofstream fout;
        //fout.open(path.c_str(), std::ios::out | std::ios::binary);

        //Check and see which UART we want, and append the mode bits to the end
        //if((path.compare(UART2_RXD) == 0) || (path.compare(UART2_TXD) == 0))
        //    flags |= 0x0001;

        //fout << std::hex << flags;
        //std::cout <<  std::hex << flags << std::endl;

        //fout.close();

        _port = port;
        //_path = path;
        _baud = baud;
    }

    UART_PORT UART::getPort(){
        return _port;
    }

    /*std::string UART::getPath(){
        return _path;
    }*/

    speed_t UART::getBaud(){
        return _baud;
    }

}
