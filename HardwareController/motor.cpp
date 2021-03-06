/*
 * Motor.cpp
 *
 *  Created on: 2010-01-11
 *      Author: wfraser, torben
 *      Rev 0.2 Brian Jun 15 2010
 *
 */
#include "Motor.h"

using namespace std;

Motor::Motor(){

}

Motor::Motor(int pwmPin, int in1Pin, int in2Pin, int direction){
	this->pwmPin = pwmPin;
	this->in1Pin = in1Pin;
	this->in2Pin = in2Pin;
	this->direction = direction;
}

void Motor::init(){
	pinMode(in1Pin, OUTPUT);      // sets the digital pin as output
	pinMode(in2Pin, OUTPUT);      // sets the digital pin as output
	pinMode(pwmPin, OUTPUT);      // sets the digital pin as output

	// Make sure we are all stopped
	analogWrite(pwmPin, 0);
	this->forward();
	debug("motor initialized");
}

void Motor::setSpeed(int speed){
		analogWrite(pwmPin, speed);
}

void Motor::forward(){
	digitalWrite(in1Pin, LOW);
	digitalWrite(in2Pin, HIGH);
}

void Motor::brake(){
	digitalWrite(in1Pin, LOW);
	digitalWrite(in2Pin, LOW);
}

void Motor::backward(){
	digitalWrite(in1Pin, HIGH);
	digitalWrite(in2Pin, LOW);
}

void Motor::stop(){
	digitalWrite(in1Pin, LOW);
	digitalWrite(in2Pin, LOW);
}

Motor::~Motor(){

}

