/*
 * Motor.cpp
 *
 *  Created on: 2010-01-11
 *      Author: wfraser, torben
 *      Rev 0.2 Brian Jun 15 2010
 *
 */
#include "motor.h"

using namespace std;

Motor::Motor(){

}

Motor::Motor(int in1Pin, int in2Pin, int pwmPin, int motor_state){
	this->pwmPin = pwmPin;
	this->in1Pin = in1Pin;
	this->in2Pin = in2Pin;
	this->motor_state = motor_state;
}

void Motor::init(){
	pinMode(in1Pin, OUTPUT);      // sets the digital pin as output
	pinMode(in2Pin, OUTPUT);      // sets the digital pin as output
	pinMode(pwmPin, OUTPUT);      // sets the digital pin as output

	// Make sure we are all stopped
	analogWrite(pwmPin, 0);
	if(motor_state){
		this->forward();
	}
	else{
		this->backward();
	}
	debug("motor initialized with in1, %d in2, %d pwm, %d",in1Pin,in2Pin,pwmPin);
}

void Motor::setSpeed(int speed){
		analogWrite(pwmPin, speed);
}

void Motor::forward(){
	digitalWrite(in1Pin, LOW);
	digitalWrite(in2Pin, HIGH);
	this->motor_state = motor_forward;
}

void Motor::brake(){
	digitalWrite(in1Pin, LOW);
	digitalWrite(in2Pin, LOW);
	analogWrite(pwmPin, 255);
	this->motor_state = motor_brake;
}

void Motor::backward(){
	digitalWrite(in1Pin, HIGH);
	digitalWrite(in2Pin, LOW);
	this->motor_state = motor_backward;
}

void Motor::stop(){
	digitalWrite(in1Pin, LOW);
	digitalWrite(in2Pin, LOW);
	analogWrite(pwmPin, 0);
	this->motor_state = motor_stop;
}

int Motor::getMotorState(void){
	return this->motor_state;
}
Motor::~Motor(){

}

