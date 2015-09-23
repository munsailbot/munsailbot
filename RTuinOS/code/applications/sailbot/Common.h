#ifndef __COMMON_H
#define __COMMON_H

#include <Arduino.h>
#include <util/delay.h>
#include <avr/wdt.h>
//#include "Wire/Wire.h"
#include "WSWire/WSWire.h"
//#include "I2C/I2C.h"

#include "Beagle.h"

//macro defines
/*************************
 * Update Oct 16, 2013: Copying this from the old code. Not sure yet if it will stay the same for the new boat
 *
 * conversion coefficients for the sensors to real world units
 * conversion is:
 * raw = ((A-real)*B)/1000
 * real = A - ((raw*1000)/B
 *
 * main sheets at -1573 draw 117cm of line
 * jib sheets at -1573 draw 120cm of line
 *************************/
#define MAIN_MOTOR_CONVERSION_A	        0
#define MAIN_MOTOR_CONVERSION_B	        -9600//-2845

#define RUDDER_MOTOR_CONVERSION_A		45
#define RUDDER_MOTOR_CONVERSION_B		1422

#define JIB_MOTOR_CONVERSION_A	        0
#define JIB_MOTOR_CONVERSION_B	        -3414
//#define JIB_MOTOR_CONVERSION_A	        0
//#define JIB_MOTOR_CONVERSION_B	        -3414

#define MAIN_START					    90
#define JIB_START					    90
#define RUDDER_START				    0

#define isSailPosition(x) ((x >= 0) && (x <= 90))
#define isRudderPosition(x) ((x >= 0) && (x <= 70))

//Forward class declaration
//This is done so headers can use these as types, without having to include the definition
//Reduces file depencency
class Beagle;
class Motor;

typedef enum {
    MOTOR_COMMAND='a',                      //DO NOT REMOVE OR INSERT NEW ITEMS WITHOUT UPDATING CONTROLLER
    READ_IMU,                               //b
    READ_GPS,                               //c
    NAVIGATION,                             //d
    MANEUVER,                               //e
    STATUS,                                 //f
    SET_AUTONOMOUS_MODE,                    //g
    COMMAND_OS
} CONTROLLER_COMMAND;

#endif // __COMMON_H
