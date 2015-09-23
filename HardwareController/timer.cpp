/*
 * sonar.cpp
 *
 * Calculating the distance from blocking object to sonar.
 * Within 6 inches, the sonar will not return the correct value
 * Digital pin 49 is reserved for PW pin as a Input Capture Unit
 * output from timer 4 (ICR4)
 *
 *  Created on: Jan 18, 2010
 *      Author: yangkexiao
 */

#include "timer.h"

uint16_t count_0 = 0;
uint16_t count_1 = 0;
uint32_t total_count = 0;

/*
 * initialize interrupts
 */
static void rising_edge_handler(void);
static void falling_edge_handler(void);

void timer_init(void)
{
	// Set prescale to clk
	TCCR5B &= ~(_BV(CS52)&_BV(CS51));
	TCCR5B |= _BV(CS50);

	// Enable noise cancelling
	TCCR5B |= _BV(ICNC5);

	// Set timer running in normal mode WGMn3:0 = 0000
	TCCR5A &= ~(_BV(WGM50)|
				_BV(WGM51));
	TCCR5B &= ~(_BV(WGM52)|
				_BV(WGM53));

	// Enable Input Capture Interrupt
	TIMSK5 |= _BV(ICIE5);

	// Enable Timer Overflow Interrupt
	TIMSK5 |= _BV(TOIE5);

	// Time capture start with a rising edge
	RISING_EDGE();

	//set PWM to input
//	pinMode(PWM, INPUT);

	// Set RX pin to output
	pinMode(48, INPUT);
	pinMode(31, OUTPUT);
	// Delay 100ms to avoid first reading
	delay(100);
}

/*
 * Trigger sonar
 */
void start_pid(void)
{
	digitalWrite(31, HIGH);
}

void stop_pid(void)
{
	digitalWrite(31, LOW);
}
/*
 * Actions need to be done in rising edge
 */
static void rising_edge_handler(void)
{
	count_0 = ICR5;								// Get first reading
	FALLING_EDGE();							// Setup edge detection
	TIFR5|=_BV(ICF5);							// Clear the interrupt flag
}

/*
 * Actions need to be done in fallen edge
 */
static void falling_edge_handler(void)
{
	count_1 = ICR5; 								// Get second reading
	RISING_EDGE();							// Setup edge detection
	TIFR5|=_BV(ICF5);							// Clear the interrupt flag
	total_count = count_1-count_0; // Calculate the distance into inch
//	Serial.println(distance[index]);
}

/* ISR:
 *      Input Capture Interrupt
 *      Timer Overflow Interrupt
 */
ISR(TIMER5_CAPT_vect)
{
	TIMSK5 &= ~_BV(ICIE5);			// Disable interrupt
	if(CHECK_RISING_EDGE())			// Check whether it is a rising edge
	{
		rising_edge_handler();
	}
	else
	{
		falling_edge_handler();
	}
//	Serial.println("irq tirggered");
	TIMSK5 |= _BV(ICIE5);			// Enbale Input Capture Interrupt
}

ISR(TIMER5_OVF_vect)
{
	TIMSK5 &= ~_BV(TOIE5); 			// Disable Timer Overflow Interrupt
	count_1+=0xffff;					// Reset timer
	TIFR5|=_BV(TOV5);				// Clear Overflow Flag
	TIMSK5 |= _BV(TOIE5);			// Enable Timer Overflow Interrupt
}

/*
 * return the distance calculated
 */

