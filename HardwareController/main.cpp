/*
 * SailbotMK1 main.cpp
 *
 *  Created on: 2010-06-09
 *      Author: brianclaus
 *      TODO create motor and encoder as objects
 */


#include "WProgram.h"
#include <stdlib.h>
#include "wiring.h"
#include <avr/io.h>
#include "common.h"
#include "LiquidCrystal.h"

LiquidCrystal lcd(11, 12, 5, 4, 3, 2);
static unsigned long timer;

int isAutoLast = 0;

static int MAIN_SAIL_POT_MAX;
static int MAIN_SAIL_POT_MIN;

static int JIB_SAIL_POT_MAX;
static int JIB_SAIL_POT_MIN;

static int RUDDER_POT_MAX;
static int RUDDER_POT_MIN;

static int DISPLAY_POT_MAX;
static int DISPLAY_POT_MIN;

enum { imuDisplay=1,
		gpsDisplay,
		navDisplay,
		manDisplay,
		leakDisplay,
		autoDisplay
};

// This code is a hack
// It is required to compile print functions in eclipse
extern "C" void __cxa_pure_virtual(void);
void __cxa_pure_virtual(void) {}

void main_init(void){
	// The init() function is defined in the Arduino library.
	// It initializes the timers and the ADC on the ATmega328p.

	init();

	Serial.begin(9600/2);  //for some reason this is actually 115200

	pinMode(BUTTON1_PIN, INPUT);
	pinMode(BUTTON2_PIN, INPUT);

	pinMode(PROG1_PIN, INPUT);
	pinMode(PROG2_PIN,INPUT);

	pinMode(LED1_PIN,OUTPUT);
	pinMode(LED2_PIN,OUTPUT);

	lcd.begin(20, 4);
	delay(1000);
	lcd.clear();
	delay(5);
	lcd.print("Sailbot Control");
	lcd.setCursor(0, 1);
	delay(5);
	lcd.print("v1.0");
	delay(1000);
};

int main(void){

	uint8_t bytetoRead[25];
	unsigned int checksumRead=0, checksumCalc=0, checksumSend=0;
	long rollRead = 0, pitchRead = 0, yawRead = 0;
	long timetoRead=0;
	int toDisplay;
	int isAuto;
	int state = 0, waypointID = 0;
	int toMainSail=0,toJibSail=0,toRudder=0;

	int angleToWind = 0;
	int trueHeading = 0;
	int distanceToWaypoint = 0;
	int courseToWaypoint = 0;
	int MS_SetP, JS_SetP, Rud_SetP, MS_Cur, JS_Cur, Rud_Cur, MS_Meas, JS_Meas, Rud_Meas;
	int windHeading,gpsHeading,magneticHeading,boatSpeed,xFromLast,yFromLast,lastLat,lastLon,lastRange,lastAngle;
	bool button1, button2, button3, button4;
	int i;
	int iniStep=1;
	int timeoutCounter=0;
	float imuRoll=0, imuPitch=0, imuYaw=0;
	long imuTime;
    long lat=0,lon=0;
    unsigned long age, speed;
    uint8_t bytetoSend[3];
    //added variables for test of endurance
    int count =0;
    int cycle= 30;
    bool cycleSwitch=1;
    bool cycleButton;
	main_init();

	lcd.clear();
	delay(2);
	lcd.print("Activate...Done!");
	delay(1000);

	lcd.clear();
	delay(2);
	lcd.print("Setup Controller");
	delay(2000);

	//Setup Main Sail Maximum
	lcd.clear();
	delay(2);
	lcd.print("Min MS Position");
	delay(500);
	while(iniStep==1){
		toMainSail=analogRead(MAIN_SAIL_PIN);
		button1=digitalRead(BUTTON1_PIN);
		if(!button1){
			MAIN_SAIL_POT_MIN=toMainSail;
			iniStep++;
			lcd.setCursor(0, 1);
			delay(1);
			lcd.print("Done!");
			delay(500);
		}
	}
	//Setup Main Sail Maximum
	lcd.clear();
	delay(2);
	lcd.print("Max MS Position");
	delay(500);
	while(iniStep==2){
		toMainSail=analogRead(MAIN_SAIL_PIN);
		button1=digitalRead(BUTTON1_PIN);
		if(!button1){
			MAIN_SAIL_POT_MAX=toMainSail;
			iniStep++;
			lcd.setCursor(0, 1);
			delay(1);
			lcd.print("Done!");
			delay(500);
		}
	}


	//Setup Jib Sail Minimum
	lcd.clear();
	delay(2);
	lcd.print("Min JS Position");
	delay(500);
	while(iniStep==3){
		toJibSail=analogRead(JIB_SAIL_PIN);
		button1=digitalRead(BUTTON1_PIN);
		if(!button1){
			JIB_SAIL_POT_MIN=toJibSail;
			iniStep++;
			lcd.setCursor(0, 1);
			delay(1);
			lcd.print("Done!");
			delay(500);
		}
	}
	//Setup Jib Sail Maximum
	lcd.clear();
	delay(2);
	lcd.print("Max JS Position");
	delay(500);
	while(iniStep==4){
		toJibSail=analogRead(JIB_SAIL_PIN);
		button1=digitalRead(BUTTON1_PIN);
		if(!button1){
			JIB_SAIL_POT_MAX=toJibSail;
			iniStep++;
			lcd.setCursor(0, 1);
			delay(1);
			lcd.print("Done!");
			delay(500);
		}
	}

	//Setup Rudder Minimum
	lcd.clear();
	delay(2);
	lcd.print("Min R Position");
	delay(500);
	while(iniStep==5){
		toRudder=analogRead(RUDDER_PIN);
		button1=digitalRead(BUTTON1_PIN);
		if(!button1){
			RUDDER_POT_MIN=toRudder;
			iniStep++;
			lcd.setCursor(0, 1);
			delay(1);
			lcd.print("Done!");
			delay(500);
		}
	}
	//Setup Rudder Maximum
	lcd.clear();
	delay(2);
	lcd.print("Max R Position");
	delay(500);
	while(iniStep==6){
		toRudder=analogRead(RUDDER_PIN);
		button1=digitalRead(BUTTON1_PIN);
		if(!button1){
			RUDDER_POT_MAX=toRudder;
			iniStep++;
			lcd.setCursor(0, 1);
			delay(1);
			lcd.print("Done!");
			delay(500);
		}
	}
/*
	//Setup Display Minimum
	lcd.clear();
	delay(2);
	lcd.print("Min D Position");
	delay(500);
	while(iniStep==7){
		toDisplay=analogRead(DISPLAY_PIN);
		button1=digitalRead(BUTTON1_PIN);
		if(!button1){
			DISPLAY_POT_MIN=toDisplay;
			iniStep++;
			lcd.setCursor(0, 1);
			delay(1);
			lcd.print("Done!");
			delay(500);
		}
	}

	//Setup Display Maximum
	lcd.clear();
	delay(2);
	lcd.print("Max D Position");
	delay(500);
	while(iniStep==8){
		toDisplay=analogRead(DISPLAY_PIN);
		button1=digitalRead(BUTTON1_PIN);
		if(!button1){
			DISPLAY_POT_MAX=toDisplay;
			iniStep++;
			lcd.setCursor(0, 1);
			delay(1);
			lcd.print("Done!");
			delay(500);
		}
	}
*/
	// Request that the rudder is sent to center
	lcd.clear();
	delay(2);
	lcd.print("Rudder to Zero");
	delay(500);
	while(iniStep==7){
		toRudder=analogRead(RUDDER_PIN);
		toRudder=map(toRudder, RUDDER_POT_MIN, RUDDER_POT_MAX, RUDDER_VALUE_MIN, RUDDER_VALUE_MAX);
		lcd.setCursor(1,2);
		delay(5);
		lcd.print("            ");
		lcd.setCursor(1,2);
		delay(5);
		lcd.print(toRudder);
		delay(500);
		if(toRudder==0){
			iniStep++;
		}
	}

	lcd.clear();
	delay(2);
	lcd.print("Initializing");
	//delay(25000); // Wireless comms takes a good while
	lcd.print("...Done!");
	delay(2000);

	lcd.clear();
	delay(2);

	// Main controller loop
	while(true){

		//SENDS SERIAL DATA FROM CONTROLLER
		//this must be slower than the parsing rate of the shipside controller
		//as of writing it is 3.5Hz
		//loop here is 2Hz
		if((millis()-timer)>=1200){
			//debug("main motor position %d error %d direction %d",main_encoder.getPosition(),main_encoder.getErrors(),main_encoder.getDirection());
			timer = millis();

			toMainSail = analogRead(MAIN_SAIL_PIN);
			toJibSail = analogRead(JIB_SAIL_PIN);
			toRudder = analogRead(RUDDER_PIN);
			//changed for endurance test Zhi Li 5.21
			//loop is 2Hz should know how long it takes for the sail to go from all in to all out
			//toMainSail = MAIN_SAIL_POT_MAX;
			//after several seconds toMainSail = MAIN_SAIL_POT_MIN;
			//toJibSail = JIB_SAIL_POT_MAX;
			//after several seconds toJibSail = JIB_SAIL_POT_MIN;

			toMainSail = map(toMainSail, MAIN_SAIL_POT_MIN, MAIN_SAIL_POT_MAX, MAIN_SAIL_VALUE_MIN, MAIN_SAIL_VALUE_MAX);
			toJibSail = map(toJibSail, JIB_SAIL_POT_MIN, JIB_SAIL_POT_MAX, JIB_SAIL_VALUE_MIN, JIB_SAIL_VALUE_MAX);
			toRudder = map(toRudder, RUDDER_POT_MIN, RUDDER_POT_MAX, RUDDER_VALUE_MIN, RUDDER_VALUE_MAX);

			//turn on and off cycle condition
			//button1=digitalRead(BUTTON1_PIN);
			/*//button2=digitalRead(BUTTON2_PIN);
			button1=false;
			button2=false;

			if (!button1 && button2){
				cycleButton=1;
			}
			if(button1&&!button2){
				cycleButton=0;
			}

			if (cycleButton==1){
				//added to test endurance of system
				if (count>=cycle){
					if (cycleSwitch==true){
					cycleSwitch=false;
					}
					else{
						cycleSwitch=true;
					}
					count=0;
				}
				if (cycleSwitch){
					toMainSail=MAIN_SAIL_VALUE_MAX;
					toJibSail=JIB_SAIL_VALUE_MAX;
					toRudder=RUDDER_VALUE_MAX;
				}
				if (!cycleSwitch){
					toMainSail=MAIN_SAIL_VALUE_MIN;
					toJibSail=JIB_SAIL_VALUE_MIN;
					toRudder=RUDDER_VALUE_MIN;
				}
				count=count+1;
			}

*/



			//lcd.clear();
			isAuto = digitalRead(PROG1_PIN);


			//if isAuto then send auto enable command to boat
			if(isAuto == HIGH){
				//send auto
				bytetoSend[0] = 'g';
				bytetoSend[1] = true;
				Serial.write(bytetoSend,2);
				isAutoLast = HIGH;
				//lcd.clear();
				delay(2);
				lcd.setCursor(0,0);
				lcd.print("AUTO");

				 checksumCalc = 0;

				if(Serial.available() > 0){             //check if all data in buffer


						for(i=0;i<14;i++){                              //read bytes less checksum
								bytetoRead[i]=(uint8_t)Serial.read();
								checksumCalc += (uint8_t)bytetoRead[i];

						}

						bytetoRead[14]=Serial.read();
						bytetoRead[15]=Serial.read();
						checksumRead = (unsigned int)bytetoRead[14] + (((unsigned int)bytetoRead[15])<<8);


						if(checksumRead == checksumCalc){//if checksums do not agree throw away data goto next task
								state = (int)bytetoRead[0] + ((int)bytetoRead[1]<<8);
								waypointID = (int)bytetoRead[2] + ((int)bytetoRead[3]<<8);
								speed = (int)bytetoRead[4] + ((int)bytetoRead[5]<<8);
								angleToWind = (int)bytetoRead[6] + ((int)bytetoRead[7]<<8);
								trueHeading = (int)bytetoRead[8] + ((int)bytetoRead[9]<<8);
								distanceToWaypoint = (int)bytetoRead[10] + ((int)bytetoRead[11]<<8);
								courseToWaypoint = (int)bytetoRead[12] + ((int)bytetoRead[13]<<8);


						}
						else{
							lcd.clear();
							lcd.setCursor(0,0);
							lcd.print("error");

						}
						Serial.flush(); //all done get rid of anything else

						lcd.clear();
						lcd.setCursor(0,1);
						lcd.print("st");
						lcd.setCursor(3,1);
						lcd.print(state);
						lcd.setCursor(0,2);
						lcd.print("wp");
						lcd.setCursor(3,2);
						lcd.print(waypointID);
						lcd.setCursor(0,3);
						lcd.print("sp");
						lcd.setCursor(3,3);
						lcd.print(speed);

						lcd.setCursor(10,0);
						lcd.print("wi");
						lcd.setCursor(13,0);
						lcd.print(angleToWind);
						lcd.setCursor(10,1);
						lcd.print("he");
						lcd.setCursor(13,1);
						lcd.print(trueHeading);
						lcd.setCursor(10,2);
						lcd.print("di");
						lcd.setCursor(13,2);
						lcd.print(distanceToWaypoint);
						lcd.setCursor(10,3);
						lcd.print("co");
						lcd.setCursor(13,3);
						lcd.print(courseToWaypoint);

				}
				else{

						Serial.flush(); //all done get rid of anything else
						Serial.write("f");                                      //read imu


				}
			}
			else if (isAuto == LOW && isAutoLast == HIGH){
				lcd.clear();
				isAutoLast = LOW;
				bytetoSend[0] = 'g';
				bytetoSend[1] = false;
				Serial.write(bytetoSend,2);


			}
			else{


				checksumSend = 0;	//zero checksum
				Serial.write('a');//control motors

				//control mainsail setpoint

				for(i=0;i<2;i++){
					bytetoSend[i]=toMainSail>>(i*8);
					checksumSend += (unsigned int)bytetoSend[i];
				}
				Serial.write(bytetoSend,2);

				//control jibsail setpoint

				for(i=0;i<2;i++){
					bytetoSend[i]=toJibSail>>(i*8);
					checksumSend += (unsigned int)bytetoSend[i];
				}
				Serial.write(bytetoSend,2);

				//control rudder1 setpoint

				for(i=0;i<2;i++){
					bytetoSend[i]=toRudder>>(i*8);
					checksumSend += (unsigned int)bytetoSend[i];
				}

				Serial.write(bytetoSend,2);
				//control rudder2 setpoint
				for(i=0;i<2;i++){
					bytetoSend[i]=toRudder>>(i*8);
					checksumSend += (unsigned int)bytetoSend[i];
				}

				Serial.write(bytetoSend,2);


				for(i=0;i<2;i++){
					bytetoSend[i]=checksumSend>>(i*8);
				}
				Serial.write(bytetoSend,2);

                //lcd.clear();
                //lcd.setCursor(0,1);
                //lcd.print("I");


				//Added N. Smith May 19 2012
				lcd.clear();
				lcd.setCursor(10,0);
				lcd.print("R=");
				lcd.setCursor(13,0);
				lcd.print(toRudder);
				lcd.setCursor(10,1);
				lcd.print("J=");
				lcd.setCursor(13,1);
				lcd.print(toJibSail);
				lcd.setCursor(10,2);
				lcd.print("M=");
				lcd.setCursor(13,2);
				lcd.print(toMainSail);







			}
		}

		//delay(500);
	}
}


