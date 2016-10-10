/*
 * pid.cpp
 *
 *  Created on: 2011-03-02
 *      Author: brianclaus
 */

#include "pid.h"

using namespace std;

pid::pid(int p_gain, int i_gain, int d_gain, int ierror_limit, int max_value, int min_value, int pid_output_limit, int pid_output_min, int setpoint){
	this->p_gain = p_gain;
	this->i_gain = i_gain;
	this->d_gain = d_gain;
	this->max_commanded_value = max_value;
	this->min_commanded_value = min_value;
	this->ierror_limit = ierror_limit;
	this->pid_output_limit = pid_output_limit;
	this->pid_output_min = pid_output_min;
	this->setpoint = setpoint;
}

pid::pid(){

}
void pid::init(){
	//give initial values
	this->previous_value = 0;
	this->error = 0;
	this->ierror = 0;
	this->derror = 0;
	this->pid_output = 0;
	this->pid_output_min = 0;
}

void pid::setPIDSetpoint(long setpoint){
	if(setpoint > max_commanded_value){
		this->setpoint = max_commanded_value;
	}
	else if(setpoint < min_commanded_value){
		this->setpoint = min_commanded_value;
	}
	else{
		this->setpoint = setpoint;
	}
}
void pid::setPIDPgain(int setpoint){
	this->p_gain = setpoint;
}
void pid::setPIDIgain(int setpoint){
	this->i_gain = setpoint;
}
void pid::setPIDDgain(int setpoint){
	this->d_gain = setpoint;
}
void pid::setPIDIerrorlimit(int setpoint){
	this->ierror_limit = setpoint;
}
void pid::setPIDOutputlimit(int setpoint){
	this->pid_output_limit = setpoint;
}
void pid::setPIDOutputmin(int setpoint){
	this->pid_output_min = setpoint;
}
int pid::getPIDError(long current_value){
	this->error = this->setpoint - current_value;
	return this->error;
}
int pid::getPIDIError(long current_value){
	this->ierror += this->setpoint - current_value;
	//prevent integral windup
	if(this->ierror > this->ierror_limit){
		this->ierror = this->ierror_limit;
	}
	if (this->ierror < -this->ierror_limit){
		this->ierror = -this->ierror_limit;
	}
	return ierror;
}
int pid::getPIDDError(long current_value){
	this->derror = this->previous_value - current_value;
	return derror;
}
int pid::getPIDOutput(void){
	return this->pid_output;
}

int pid::getPIDOutputlimit(void){
	return this->pid_output_limit;

}
int pid::getPIDOutputmin(void){
	return this->pid_output_min;

}

long pid::getPIDSetpoint(){
    return this->setpoint;
}

int pid::computePID(long current_value){
	this->pid_output = this->getPIDError(current_value)*this->p_gain +
			this->getPIDIError(current_value)*this->i_gain +
			this->getPIDDError(current_value)*this->d_gain;
	this->previous_value = current_value;
	//limit pid output
	if(this->pid_output > this->pid_output_limit){
		this->pid_output = this->pid_output_limit;
	}
	if (this->pid_output < -this->pid_output_limit){
		this->pid_output = -this->pid_output_limit;
	}
	if(this->pid_output < this->pid_output_min && this->pid_output > 0){
		this->pid_output = this->pid_output_min;
	}
	if (this->pid_output > -this->pid_output_min && this->pid_output < 0){
		this->pid_output = -this->pid_output_min;
	}
	return this->pid_output;
}
pid::~pid(){

}

