/*
 * common.cpp
 *
 *  Created on: 2010-01-11
 *      Author: torben
 */

#include<stdio.h>
#include<stdarg.h>
#include "common.h"

#if DEBUG_ALL
void initDebug(void){
	Serial.begin(SERIAL_BAUD_RATE);
}
void debug(const char *fmt, ...)
{
	char msgBuf [80];
	va_list ap;
	va_start(ap,fmt);

	vsprintf(msgBuf, fmt,ap);
	va_end(ap);

	Serial.println(msgBuf);
	//Serial.flush();
}
void error(const char *fmt, ...)
{
	char msgBuf [80];
	va_list ap;
	va_start(ap,fmt);

	vsprintf(msgBuf, fmt,ap);
	va_end(ap);

	Serial.println(msgBuf);
	//Serial.flush();
}
#else
	#if DEBUG_ERROR
	void error(const char *fmt, ...)
	{
		char msgBuf [80];
		va_list ap;
		va_start(ap,fmt);

		vsprintf(msgBuf, fmt,ap);
		va_end(ap);

		Serial.println(msgBuf);
		//Serial.flush();
	}
	#else
	void error(const char *fmt, ...){

	}
	#endif
void debug(const char *fmt, ...){

}

void initDebug(void){

}

#endif
