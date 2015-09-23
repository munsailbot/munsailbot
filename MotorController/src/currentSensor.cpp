/*
 * currentSensor.cpp
 *
 *  Created on: 2011-05-21
 *      Author: brianclaus
 */

#include "currentSensor.h"

currentSensor::currentSensor(int Pin) {
	this->sensePin = Pin;
}
void currentSensor::init(int type){
	this->sensorType = type;
	this->current = 0;
}
/*current in mA
 * 0.13 V/A
*/
unsigned int currentSensor::get_measuredCurrent(void){
	if(this->sensorType == motorCurrentSensor){
		this->current = analogRead(sensePin)*38;
		return this->current;
	}
	else if(this->sensorType == batteryCurrentSensor){
		this->current = (analogRead(sensePin)-100)*50;
		return this->current;
	}
	else{
		return 0;
	}
}

currentSensor::~currentSensor() {

}
