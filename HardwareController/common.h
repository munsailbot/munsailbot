#ifndef COMMON_H_
#define COMMON_H_

#include "WProgram.h"
#include <stdlib.h>
#include "wiring.h"
#include <avr/io.h>
#include "motor.h"


/** Disable default prescaler to make processor speed 8 MHz. */
#define     clock8MHz()    CLKPR = (1<<CLKPCE); CLKPR = 0x00;

#define Disable_Interrupt()    asm volatile ("cli"::)
#define Enable_Interrupt()     asm volatile ("sei"::)

/*
 * CONSTANTS
 */
#define SERIAL_BAUD_RATE 57600

/*
 * DEBUG ON/OFF
 */
#define DEBUG_ALL 		1
#define DEBUG_ERROR 	0
#define DEBUG_RADIO 	0

/*
 * Pins
 */
//analog in
#define MAIN_SAIL_PIN 	3
#define RUDDER_PIN 		1
#define DISPLAY_PIN 	2
#define JIB_SAIL_PIN 	0

//digital in
#define BUTTON1_PIN 9
#define BUTTON2_PIN 8
#define PROG1_PIN 10
#define PROG2_PIN 13

//digitial out
#define LED1_PIN 6
#define LED2_PIN 7



/*
 * MAX/MIN VALUES OF CONTROLLER POTS
 */

/*


/*
 * RUDDER/SAIL/DISPLAY VALUE BOUNDS
 */

#define MAIN_SAIL_VALUE_MAX		90
#define MAIN_SAIL_VALUE_MIN		0

#define JIB_SAIL_VALUE_MAX		90
//#define JIB_SAIL_VALUE_MIN		0
#define JIB_SAIL_VALUE_MIN 15 //previous value 0 edited 5.26


//#define RUDDER_VALUE_MAX	90 //prev value is 30
//#define RUDDER_VALUE_MIN	-90 //prev value is -30
#define RUDDER_VALUE_MAX	35  //changed 5.26
#define RUDDER_VALUE_MIN	-35 //changed 5.26

#define DISPLAY_VALUE_MAX   6
#define DISPLAY_VALUE_MIN   1

/*************************
 * ANALOG INPUT PINS
 *************************/

/**********************
 * HELPER FUNCTIONS
 **********************/
void debug(const char *fmt, ...);
void error(const char *fmt, ...);
void initDebug(void);

#endif /* COMMON_H_ */
