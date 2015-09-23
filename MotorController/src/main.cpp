/*
 * SailbotMK1 main.cpp
 *
 *  Created on: 2010-06-09
 *      Author: brianclaus
 *      TODO create motor and encoder as objects
 */



#include "common.h"



//serial responder states
enum { M1_speed='a',
	M2_speed,
	M1_state,
	M2_state,
	M1_position,			//e
	M2_position,			//f
	M1_setpoint,      		//g
	M2_setpoint,			//h
	M1_pid_params,			//i
	M2_pid_params};			//j


static int PIDoutput;
static unsigned long timer;

// This code is a hack
// It is required to compile print functions in eclipse
extern "C" void __cxa_pure_virtual(void);
void __cxa_pure_virtual(void) {}

Motor M1_motor(M1_IN1,M1_IN2,M1_PWM,motor_stop);
Motor M2_motor(M2_IN1,M2_IN2,M2_PWM,motor_stop);
Encoder M1_encoder(M1_CHA,M1_CHB,M1_start<<scale_factor);
Encoder M2_encoder(M2_CHA,M2_CHB,M2_start<<scale_factor_M2);
pid  M1_pid(M1_pid_p_gain,M1_pid_i_gain,M1_pid_d_gain,M1_pid_i_error_limit,M1_pid_max_commanded_value,M1_pid_min_commanded_value,M1_pid_pwm_limit,M1_pid_pwm_min,M1_start);
pid  M2_pid(M2_pid_p_gain,M2_pid_i_gain,M2_pid_d_gain,M2_pid_i_error_limit,M2_pid_max_commanded_value,M2_pid_min_commanded_value,M2_pid_pwm_limit,M2_pid_pwm_min,M2_start);
currentSensor M1_Current(current_sense_pin_M1);
currentSensor M2_Current(current_sense_pin_M1);


void requestEvent();
void receiveEvent(int howMany);

void main_init(void){
	// The init() function is defined in the Arduino library.
	// It initializes the timers and the ADC on the ATmega328p.
	init();
	Wire.begin(MOTORBOARDADDRESS);
	Wire.onReceive(receiveEvent); // register event
	Wire.onRequest(requestEvent); // register event
	Serial.begin(57600);  //for some reason this is actually 115200
	TCCR1B = TCCR1B & 0b11111010; //set pin 9 and 10 pwm to 3.94kHz
	TCCR2B &= ~_BV(CS22); //disable cs22 and enable cs21 make clock scaler 2 (divisor 8)
	TCCR2B |= _BV(CS21); //set pin 3 and 11 pwm to 3.94kHz


	debug("main init");
}

void motor_init(void){

	M1_motor.init();
	M2_motor.init();

}

void encoder_init(void){
	cli();
	M1_encoder.init();
	M2_encoder.init();
	sei();
}




int main(void){

    pinMode(8,OUTPUT);
	main_init();
	motor_init();
	encoder_init();
	M1_Current.init(motorCurrentSensor);
	M2_Current.init(motorCurrentSensor);
	//should read commands from main controller
	//and update the motor states/speeds accordingly



	while(1){ //main loop at 10 Hz
		if((millis()-timer)>=50){
            //Serial.println("loop");
			timer = millis();
			//motor M1
			PIDoutput = M1_pid.computePID(M1_encoder.getPosition()>>scale_factor);
			if( PIDoutput > pwm_duty_deadband){
				M1_motor.backward();
				M1_motor.setSpeed(PIDoutput);
				debug(" position1 %ld, speed %d, current %d,", M1_encoder.getPosition()>>scale_factor,PIDoutput,M1_Current.get_measuredCurrent());
			}
			else if(PIDoutput < -pwm_duty_deadband) {
				M1_motor.forward();
				M1_motor.setSpeed(-PIDoutput);
				debug(" position1 %ld, speed %d, current %d,", M1_encoder.getPosition()>>scale_factor,PIDoutput,M1_Current.get_measuredCurrent());
			}
			else{
				//debug(" position %ld, speed %d", encoder.getPosition()>>scale_factor,PIDoutput);
				M1_motor.brake();
			}

			//current limiting
			if(M1_Current.get_measuredCurrent() > M1_CurrentLimit && M1_pid.getPIDOutputlimit() > M1_PWMStep){
				M1_pid.setPIDOutputlimit(M1_pid.getPIDOutputlimit()-M1_PWMStep);
				debug(" PWM1 %d",M1_pid.getPIDOutputlimit());
			}
			else if ( M1_pid.getPIDOutputlimit() < (255-M1_PWMStep)){
				M1_pid.setPIDOutputlimit(M1_pid.getPIDOutputlimit()+M1_PWMStep);
				if(M1_pid.getPIDOutputlimit()>(255-M1_PWMStep))M1_pid.setPIDOutputlimit(255);
				debug(" PWM1 %d",M1_pid.getPIDOutputlimit());
			}

			//motor M2
			PIDoutput = M2_pid.computePID(M2_encoder.getPosition()>>scale_factor_M2);
			if( PIDoutput > pwm_duty_deadband){
				M2_motor.backward();
				M2_motor.setSpeed(PIDoutput);
				debug(" position2 %ld, speed %d, current %d,", M2_encoder.getPosition()>>scale_factor_M2,PIDoutput,M2_Current.get_measuredCurrent());
			}
			else if(PIDoutput < -pwm_duty_deadband) {
				M2_motor.forward();
				M2_motor.setSpeed(-PIDoutput);
				debug(" position2 %ld, speed %d, current %d,", M2_encoder.getPosition()>>scale_factor_M2,PIDoutput,M2_Current.get_measuredCurrent());
			}
			else{
				//debug(" position %ld, speed %d", encoder.getPosition()>>scale_factor,PIDoutput);
				M2_motor.brake();
			}

			//current limiting
			if(M2_Current.get_measuredCurrent() > M2_CurrentLimit && M2_pid.getPIDOutputlimit() > M2_PWMStep){
				M2_pid.setPIDOutputlimit(M2_pid.getPIDOutputlimit()-M2_PWMStep);
				debug(" PWM2 %d",M2_pid.getPIDOutputlimit());

			}
			else if ( M2_pid.getPIDOutputlimit() < (255-M2_PWMStep)){
				M2_pid.setPIDOutputlimit(M2_pid.getPIDOutputlimit()+M2_PWMStep);
				if(M2_pid.getPIDOutputlimit()>(255-M2_PWMStep))M2_pid.setPIDOutputlimit(255);
				debug(" PWM2 %d",M2_pid.getPIDOutputlimit());
			}

		}

		digitalWrite(8,LOW);
	}//while


}


// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
//Send for motor M1/M2:
// measured position (long)
// PID output limit (int)
// measured current (uint)
// checksum (uint)
// total 18 bytes
void requestEvent()
{
    unsigned int checksumSend=0;
	uint8_t bytetoSend[22];
	int i;

	for(i=0;i<4;i++){
		bytetoSend[i]=(M1_encoder.getPosition()>>scale_factor)>>(i*8);
		checksumSend += (unsigned int)bytetoSend[i];
	}


	for(i=4;i<6;i++){
		bytetoSend[i]=M1_pid.getPIDOutputlimit()>>((i-4)*8);
		checksumSend += (unsigned int)bytetoSend[i];
	}



	for(i=6;i<8;i++){
		bytetoSend[i]=M1_Current.get_measuredCurrent()>>((i-6)*8);
		checksumSend += (unsigned int)bytetoSend[i];
	}


	for(i=8;i<12;i++){
		bytetoSend[i]=(M2_encoder.getPosition()>>scale_factor_M2)>>((i-8)*8);
		checksumSend += (unsigned int)bytetoSend[i];
	}



	for(i=12;i<14;i++){
		bytetoSend[i]=M2_pid.getPIDOutputlimit()>>((i-12)*8);
		checksumSend += (unsigned int)bytetoSend[i];
	}


	for(i=14;i<16;i++){
		bytetoSend[i]=M2_Current.get_measuredCurrent()>>((i-14)*8);
		checksumSend += (unsigned int)bytetoSend[i];
	}


	for(i=16;i<18;i++){
		bytetoSend[i]=checksumSend>>((i-16)*8);
	}
	Wire.write(bytetoSend,18);


}

void receiveEvent(int howMany)
{
    Serial.println("print");
	int firstByte, secondByte,thirdByte,fourthByte,fifthByte;
	int checksumCalc = 0, checksumRead = 0;
	uint8_t bytetoRead[8];
    int i;
	digitalWrite(8,HIGH);
	checksumCalc = 0;
	debug("receive");
	if (Wire.available() > 0) {
		//Checksum calc implementation requires all motor commands to be three bytes

		digitalWrite(8,LOW);
		for(i=0;i<5;i++){				//read bytes less checksum
			bytetoRead[i]=(uint8_t)Wire.read();
			checksumCalc += (uint8_t)bytetoRead[i];
		}
		bytetoRead[5]=Wire.read();
		bytetoRead[6]=Wire.read();
		checksumRead = (unsigned int)bytetoRead[5] + (((unsigned int)bytetoRead[6])<<8);

		firstByte = bytetoRead[0]; 	//first byte specifies which motor/action
		secondByte = bytetoRead[1]; 	//Second value of action if any (PWM)
		thirdByte = bytetoRead[2]; 	//third value of action if any (PWM)
		fourthByte = bytetoRead[3]; 	//fourth value of action if any (PWM)
		fifthByte = bytetoRead[4]; 	//fifth value of action if any (PWM)
		debug(" %d = %d",checksumRead,checksumCalc);
        Serial.println(firstByte);
		if(checksumRead == checksumCalc){//if checksums do not agree throw away data
			digitalWrite(8,HIGH);
			switch (firstByte) {

			case M1_speed:   //set  pwm
				debug(" speed %d",secondByte);
				M1_motor.setSpeed(secondByte);
				break;

			case M1_state:   //set  state
				debug(" state %d",secondByte);
				if(secondByte!=M1_motor.getMotorState()){ //if different from current change
					switch(secondByte){
					case motor_forward:
						M1_motor.forward();
						break;
					case motor_backward:
						M1_motor.backward();
						break;
					case motor_brake:
						M1_motor.brake();
						break;
					case motor_stop:
						M1_motor.stop();
						break;
					default:
						break;//do nothing should not be here
					}
				}
				break;

			case M1_position:
				debug(" motor position %ld error %d direction %d",M1_encoder.getPosition(),M1_encoder.getErrors(),M1_encoder.getDirection());
				//encoder.setPosition((thirdByte<<8)+secondByte);
				break;

			case M1_setpoint:
				debug(" setpoint1 %d",(thirdByte<<8)+secondByte);

				M1_pid.setPIDSetpoint((thirdByte<<8)+secondByte); //each setpoint increment is one revolution of encoder wheel
				M2_pid.setPIDSetpoint((fifthByte<<8)+fourthByte); //each setpoint increment is one revolution of encoder wheel
                
                char checkStr[20];
                itoa(M1_pid.getPIDSetpoint(), checkStr, 10);   
                Serial.println(checkStr);     
                
				break;

			case M1_pid_params:
				debug("set  param %d with %d",secondByte, thirdByte);
				switch(secondByte){
					case tune_pid_p_gain:
						M1_pid.setPIDPgain(thirdByte);
						break;
					case tune_pid_i_gain:
						M1_pid.setPIDIgain(thirdByte);
						break;
					case tune_pid_d_gain:
						M1_pid.setPIDDgain(thirdByte);
						break;
					case tune_pid_ierror_limit:
						M1_pid.setPIDIerrorlimit(thirdByte);
						break;
					case tune_pid_output_limit:
						M1_pid.setPIDOutputlimit(thirdByte);
						break;
					case tune_pid_output_min:
						M1_pid.setPIDOutputmin(thirdByte);
						break;
					default:
						break;//do nothing should not be here
					}
				break;

			default:
				break;
			}//switch
			digitalWrite(8,HIGH);
		}//if
	}//if

}


ISR(PCINT1_vect)
 {
	cli(); //disable interrupts
	M1_encoder.encoderISR();
	sei(); //enable interrupts
}

ISR(PCINT0_vect){
	cli(); //disable interrupts
	M2_encoder.encoderISR();
	sei(); //enable interrupts
}

