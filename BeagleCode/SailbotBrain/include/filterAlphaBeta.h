/*
 * filterAlphaBeta.h
 *
 *  Created on: 2011-06-08
 *      Author: Brian
 */

#ifndef FILTERALPHABETA_H_
#define FILTERALPHABETA_H_

class filterAlphaBeta {
	long predictedValue;
	long lastPredictedValue;
	long lastPredictedRate;
	long predictedRate;
	long estimationError;
	long measuredValue;
	long upperLimit;
	long offset;
	int alpha;
	int beta;
public:
	filterAlphaBeta(int,int);
	void init();
	void init(long);
	void filterValue(long, long);
	void setUpperLimit(long);
	long getFilteredValue();
	long getFilteredRate();
	~filterAlphaBeta();
};

#endif /* FILTERALPHABETA_H_ */
