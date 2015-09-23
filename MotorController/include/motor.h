/*
 * motor.h
 *
 *  Created on: Jun 14, 2010
 *      Author: Brian
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include <Arduino.h>
#include "common.h"

enum { motor_forward=1,motor_backward,motor_brake,motor_stop};

class Motor{
	int pwmPin;
	int in1Pin;
	int in2Pin;
	int motor_state;
public:
	void init();
	// int in1Pin, int in2Pin, int pwmPin,int motor_state
	//direction is forward if 1, backward if 0
	Motor(int, int, int, int);
	Motor();
	void setSpeed(int);
	void forward();
	void backward();
	void brake();//sets PWM to max and motor controller to Brake (motor will resist moving)
	void stop();//sets PWM to min and motor controller to Brake (will allow motor to move)
	int  getMotorState();
	~Motor();
};


#endif /* MOTOR_H_ */
