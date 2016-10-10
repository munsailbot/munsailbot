/*
 * encoder.h
 *
 *  Created on: 2010-06-15
 *      Author: brianclaus
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "common.h"


class Encoder{
	int pinCHA;
	int pinCHB;
	int direction;
	long position;
	int pinCHAlast;
	int pinCHBlast;
	int error;
public:
	void init();
	Encoder(int,int,int);
	Encoder();
	long getPosition(void);
	void setPosition(int);
	int getDirection(void);
	int getErrors(void);
	void encoderISR(void);
	~Encoder();

};

#endif /* ENCODER_H_ */
