#include "UARTInterface.h"
#include <arpa/inet.h>

namespace BeagleUtil
{

    UARTInterface::UARTInterface(UART* uart){
        _handle = -1;

        if(uart->getPort() == UART1)
            _handle = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_NONBLOCK);
        else if(uart->getPort() == UART2)
            _handle = open("/dev/ttyO2", O_RDWR | O_NOCTTY | O_NONBLOCK);
        else if(uart->getPort() == UART4)
            _handle = open("/dev/ttyO4", O_RDWR | O_NOCTTY | O_NONBLOCK);
        else if(uart->getPort() == UART5)
            _handle = open("/dev/ttyO5", O_RDWR | O_NOCTTY);
        else
            _handle = -1;

        if(_handle >= 0){
            //std::cout << "Opened UART port" << std::endl;
            struct termios options;

            //fcntl(_handle, F_SETFL, 0); /* Configure port reading */

            if(tcgetattr(_handle, &options) != -1){ /* Get the current options for the port */
                cfsetispeed(&options, uart->getBaud());
                cfsetospeed(&options, uart->getBaud());
                options.c_cflag |= (CLOCAL | CREAD); /* Enable the receiver and set local mode */
                options.c_cflag &= ~PARENB; /* Mask the character size to 8 bits, no parity */
                options.c_cflag &= ~CSTOPB;
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; /* Select 8 data bits */
                options.c_cflag &= ~CRTSCTS; /* Disable hardware flow control */
                options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);/* Enable data to be processed as raw input */
                options.c_iflag &= ~ (IXON | IXOFF | IXANY); //turn off software flow control
                options.c_oflag &= ~OPOST; //make raw

                //options.c_cc[VMIN] = 0;
                //options.c_cc[VTIME] = 0;

                tcsetattr(_handle, TCSANOW, &options);
                if(tcsetattr(_handle, TCSAFLUSH, &options) < 0)
                    std::cout << "Couldn't set UART port attributes." << std::endl;
            }
            else 
                std::cout << "Could not get UART port options" << std::endl;
        }
    }

    UARTInterface::~UARTInterface(){
        close(_handle);
    }

    char UARTInterface::readByte(){
        char ret;
        int bytesRead = read(_handle, &ret, 1);

        if(bytesRead)
            return ret;
        else
            return -1;
    }

    char* UARTInterface::readBytes(size_t numBytes){
        char* ret = new char[numBytes];
        int bytesRead = read(_handle, ret, numBytes);

        return ret;
    }

    void UARTInterface::writeByte(uint8_t byte){
        write(_handle, &byte, 1);
    }

    void UARTInterface::writeBytes(uint8_t* bytes, size_t numBytes){
        tcflush(_handle, TCIOFLUSH);
        write(_handle, &bytes[0], numBytes);
    }

    void UARTInterface::writeString(std::string str){
        write(_handle, str.c_str(), str.size());
    }

    int UARTInterface::getHandle(){
        return _handle;
    }

}
