#ifndef _CONSTKALMANFILTER_H
#define _CONSTKALMANFILTER_H

#include <stdio.h>

template <typename T> class ConstKalmanFilter
{
public:
	ConstKalmanFilter()
	{
		covR = 1.0;
		covQ = 1.0;
		covP = 1.0;
		xhat = 0;
	}

	ConstKalmanFilter(float R, float Q)
	{
		covR = R;
		covQ = Q;
		covP = 1.0;
		xhat = 0;
	}

	T getFilteredValue(T rawValue)
	{
		// x[k+1] = x[k];
		// y[k+1] = x[k+1];

		// prediction
		covP = covP + covQ;

		// update
		float Kgain = covP / (covP + covR);
		// printf("%f, %d, %d\n",Kgain,rawValue,xhat);	// debug
		xhat = xhat + Kgain * (rawValue - xhat);
		covP = (1.0 - Kgain) * covP;

		return xhat;
	}

	// debug code

	void printParms()
	{
		printf("covR = %f, covQ = %f, covP = %f, xhat = %d\n",
			   covR, covQ, covP, xhat);
	}

private:
	float covR, covQ, covP;
	T xhat;
};

#endif
