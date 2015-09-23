/*
Daniel Cook 2013
daniel@daniel-cook.net
*/

#ifndef __COMMON_H
#define __COMMON_H

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

namespace BeagleUtil
{
    const std::string GPIO_DEFAULT_PATH = "/sys/class/gpio";

    const std::string PWM_P8_19 = "/sys/class/pwm/ehrpwm.2:0";
    const std::string PWM_P8_13 = "/sys/class/pwm/ehrpwm.2:1";

    const std::string PWM_P9_14 = "/sys/class/pwm/ehrpwm.1:0";
    const std::string PWM_P9_16 = "/sys/class/pwm/ehrpwm.1:1";

    const std::string PWM_P9_31 = "/sys/class/pwm/ehrpwm.0:0";
    const std::string PWM_P9_29 = "/sys/class/pwm/ehrpwm.0:1";

    //const std::string UART1_TXD = "/sys/kernel/debug/omap_mux/uart1_txd"; //pin 24
    //const std::string UART1_RXD = "/sys/kernel/debug/omap_mux/uart1_rxd"; //pin 26
    //const std::string UART2_TXD = "/sys/kernel/debug/omap_mux/spi0_d0"; //pin 21
    //const std::string UART2_RXD = "/sys/kernel/debug/omap_mux/spi0_sclk"; //pin 22
    typedef enum{
        UART1,
        UART2,
        UART4,
        UART5
    } UART_PORT;

    const std::string I2C_1 = "/dev/i2c-1";
    const std::string I2C_3 = "/dev/i2c-3";

    const std::string AIN_0 = "/sys/devices/platform/tsc/ain1";
    const std::string AIN_1 = "/sys/devices/platform/tsc/ain2";
    const std::string AIN_2 = "/sys/devices/platform/tsc/ain3";
    const std::string AIN_3 = "/sys/devices/platform/tsc/ain4";
    const std::string AIN_4 = "/sys/devices/platform/tsc/ain5";
    const std::string AIN_5 = "/sys/devices/platform/tsc/ain6";

    const short int UART_USE_PIN = 0x8000;
    const short int UART_SLOW = 0x0040;
    const short int UART_INPUT = 0x0020;
}

#endif
