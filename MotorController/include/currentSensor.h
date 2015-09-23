/*
 * currentSensor.h
 *
 *  Created on: 2011-05-21
 *      Author: brianclaus
 */

#ifndef CURRENTSENSOR_H_
#define CURRENTSENSOR_H_

#include <avr/io.h>
#include <Arduino.h>
#include "common.h"

enum{motorCurrentSensor = 1, batteryCurrentSensor};

class currentSensor {
	int sensePin;
	unsigned int current;	//in mA
	int sensorType;
public:
	currentSensor(int);
	void init(int);
	unsigned int get_measuredCurrent(void);

	~currentSensor();
};

#endif /* CURRENTSENSOR_H_ */
