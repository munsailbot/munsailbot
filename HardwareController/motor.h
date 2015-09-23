/*
 * motor.h
 *
 *  Created on: Jun 14, 2010
 *      Author: Brian
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "WProgram.h"
#include "common.h"


class Motor{

public:
	void init();
	Motor(int, int, int, int);
	Motor();
	void setSpeed(int);
	void forward();
	void backward();
	void brake();
	void stop();
	volatile int pwmPin;
	volatile int in1Pin;
	volatile int in2Pin;
	volatile int direction;
	~Motor();
};


#endif /* MOTOR_H_ */
