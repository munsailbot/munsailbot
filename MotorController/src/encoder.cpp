/*
 * encoder.cpp
 *
 *  Created on: 2010-06-15
 *      Author: brianclaus
 *
 * place this code at the bottom of the main file and call the encoderISR function
 * of the already instantiated object
ISR(PCINT0_vect)
 {
	cli(); //disable interrupts
	encoderObject0.encoderISR();
	sei(); //enable interrupts
}
*/


#include "encoder.h"

using namespace std;

inline unsigned char get_val(unsigned char pin)
{
	// Note: get_val will work (i.e. always return the same value)
	// even with invalid pin values, since the bit shift on the final
	// return will cause the port value to be shifted all the way to
	// 0.
	if(pin <= 7)
		return (PIND >> pin) & 1;
	if(pin <= 13)
		return (PINB >> (pin-8)) & 1;
	return (PINC >> (pin-14)) & 1;
}

static void enable_interrupts_for_pin(unsigned char p)
{
	// check what block it's in and do the right thing
	if(p <= 7)
	{
		PCICR |= 1 << PCIE2;
		DDRD &= ~(1 << p);
		PCMSK2 |= 1 << p;
	}
	else if(p <= 13)//this one doesn't work
	{
		PCICR |= 1 << PCIE0;
		DDRB &= ~(1 << (p - 8));//changed to 9
		PCMSK0 |= 1 << (p - 8);
	}
	else if(p <= 19)
	{
		PCICR |= 1 << PCIE1;
		DDRC &= ~(1 << (p - 14));
		PCMSK1 |= 1 << (p - 14);
	}
	// Note: this will work with invalid port numbers, since there is
	// no final "else" clause.
}

Encoder::Encoder(){

}
Encoder::Encoder(int pinCHA, int pinCHB, int position){
	this->pinCHA = pinCHA;
	this->pinCHB = pinCHB;
	this->position = position;
	this->direction = 0;
    this->pinCHAlast = 0;
    this->pinCHBlast = 0;
    this->error = 0;
}

void Encoder::init( ){

	this->pinCHAlast = digitalRead(this->pinCHA);
	this->pinCHBlast = digitalRead(this->pinCHB);

	//setup interrupts
	enable_interrupts_for_pin(this->pinCHA);
	enable_interrupts_for_pin(this->pinCHB);


	debug("encoder initialized on pins %d, %d",pinCHA,pinCHB);
}

long Encoder::getPosition(void){
	return this->position;
}
void Encoder::setPosition(int position){
	this->position = position;
}
int Encoder::getDirection(void){
	return this->direction;
}
int Encoder::getErrors(void){
	return this->error;
}

void Encoder::encoderISR(void){
	int chA = get_val(this->pinCHA);
	int chB = get_val(this->pinCHB);

	if(chA ^ this->pinCHBlast){
		this->position += 1;
		this->direction = 1;
	}
	if(chB ^ this->pinCHAlast){
		this->position -= 1;
		this->direction = 0;
	}
	//check that only one bit had a transition
	if(chA != this->pinCHAlast && chB != this->pinCHBlast)
		this->error += 1;
	this->pinCHAlast = chA;
	this->pinCHBlast = chB;
}

Encoder::~Encoder(){

}



