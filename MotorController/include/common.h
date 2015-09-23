#ifndef COMMON_H_
#define COMMON_H_

#include <Arduino.h>
//#include "wiring.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Wire/Wire.h>
#include "motor.h"
#include "encoder.h"
#include "pid.h"
//#include "wire.h"
#include "currentSensor.h"


#define Disable_Interrupt()    asm volatile ("cli"::)
#define Enable_Interrupt()     asm volatile ("sei"::)

//winch motor controller address
#define MOTORBOARDADDRESS 2

/*
 * CONSTANTS
 */
#define SERIAL_BAUD_RATE 57600

/*
 * DEBUG ON/OFF
 */
#define DEBUG_ALL 0
#define DEBUG_ERROR 0
#define DEBUG_RADIO 0

/*
 * PID CONSTANTS
 */
//only one set of constants for winchs as should be identical

#define M1_pid_p_gain 			12
#define M1_pid_i_gain 	    	0
#define M1_pid_d_gain 			4
#define M1_pid_i_error_limit 	100
#define M1_pid_pwm_limit 		255
#define M1_pid_pwm_min 		0
#define M1_pid_max_commanded_value	414//256		//240=90
#define M1_pid_min_commanded_value	0		    //2880=1080
#define M1_start				414//256

/*#define M2_pid_p_gain 			12
#define M2_pid_i_gain 	    	0
#define M2_pid_d_gain 			4
#define M2_pid_i_error_limit 	100
#define M2_pid_pwm_limit 		255
#define M2_pid_pwm_min 		0
#define M2_pid_max_commanded_value	308	//240=90
#define M2_pid_min_commanded_value	0		    //2880=1080
#define M2_start				308*/
#define M2_pid_p_gain 			7
#define M2_pid_i_gain 	    	0
#define M2_pid_d_gain 			7
#define M2_pid_i_error_limit 	50
#define M2_pid_pwm_limit 		255
#define M2_pid_pwm_min 		0
#define M2_pid_max_commanded_value	128	//240=90
#define M2_pid_min_commanded_value	0		    //2880=1080
#define M2_start				64

#define M1_CurrentLimit		3000
#define M1_PWMStep			5
//#define M2_CurrentLimit		3000
//#define M2_PWMStep			5
#define M2_CurrentLimit		2000
#define M2_PWMStep			10

/*
 * MOTOR/ENCODER DEFINITIONS
 */
#define pwm_duty_deadband 25
#define scale_factor 2
#define scale_factor_M2 0


//winch MOTOR M1
#define M1_IN1  2
#define M1_IN2  4
#define M1_PWM  3
//winch encoder
#define M1_CHA 14	//PC0/PCINT8
#define M1_CHB 15	//PC1/PCINT9

//winch MOTOR M2
#define M2_IN1  5
#define M2_IN2  7
#define M2_PWM  6
//winch encoder
#define M2_CHA 12	//PB4/PCINT3
#define M2_CHB 13	//PB3/PCINT4

/*************************
 * ANALOG INPUT PINS
 *************************/
#define current_sense_pin_M1 3	//U$2
#define current_sense_pin_M2 2	//U$2

/**********************
 * HELPER FUNCTIONS
 **********************/
void debug(const char *fmt, ...);
void error(const char *fmt, ...);
void initDebug(void);



#endif /* COMMON_H_ */
