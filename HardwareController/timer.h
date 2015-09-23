/*
 * timer.h
 *
 *  Created on: Apr 20, 2010
 *      Author: ch_w10
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "avr/io.h"
#include "avr/interrupt.h"
#include "WProgram.h"

#define RISING_EDGE() TCCR5B|=_BV(ICES5)	//input capture interrupt triggerd on rising edge
#define FALLING_EDGE() TCCR5B&=~_BV(ICES5)	//input capture interrupt triggerd on rising edge
#define CHECK_RISING_EDGE() TCCR5B & _BV(ICES5)	//check whether is rising edge

void timer_init(void);
void start_pid(void);
void stop_pid(void);


#endif /* TIMER_H_ */
