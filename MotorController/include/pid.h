/*
 * pid.h
 *
 *  Created on: 2011-03-02
 *      Author: brianclaus
 */

#ifndef PID_H_
#define PID_H_

#include <Arduino.h>
#include "common.h"

enum { tune_pid_p_gain='a',tune_pid_i_gain,tune_pid_d_gain,tune_pid_ierror_limit,tune_pid_output_limit,tune_pid_output_min};

class pid{
	long setpoint;
	int error;
	int ierror;
	int derror;
	int previous_value;
	int p_gain;
	int i_gain;
	int d_gain;
	int max_commanded_value;
	int min_commanded_value;
	int ierror_limit;
	int pid_output_limit;
	int pid_output_min;
	int pid_output;
public:
	pid(int, int, int, int, int, int, int, int, int);
	pid();
	void init();
	void setPIDSetpoint(long);
	void setPIDPgain(int);
	void setPIDIgain(int);
	void setPIDDgain(int);
	void setPIDIerrorlimit(int);
	void setPIDOutputlimit(int);
	void setPIDOutputmin(int);
	int getPIDError(long);
	int getPIDIError(long);
	int getPIDDError(long);
	int getPIDOutput(void);
	int getPIDOutputlimit(void);
	int getPIDOutputmin(void);
    long getPIDSetpoint();
	int computePID(long);
	~pid();
};


#endif /* PID_H_ */
