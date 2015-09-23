/*
 * filterAlphaBeta.cpp
 *
 *  Created on: 2011-06-08
 *      Author: Brian
 *      what we do
 *      1)estimate the value
 *      	predictedValue = lastPredictedValue + deltaT * lastPredictedRate
 *      2)estimate the rate
 *      	predictedRate = lastPredictedRate
 *      3)compute the estimation error
 *      	estimationError = measuredValue - predictedValue
 *      4)update the predictedValue
 *      	predictedValue = predictedValue + alpha * estimationError
 *      5)update the predictedRate
 *      	predictedRate = predictedRate + (beta / deltaT) * estimationError
 *
 *
 *
 */

#include "filterAlphaBeta.h"

filterAlphaBeta::filterAlphaBeta(int alpha, int beta) {
	this->alpha = alpha;
	this->beta = beta;

}

void filterAlphaBeta::init(){
	this-> predictedValue = 0;
	this-> lastPredictedValue = 0;
	this-> lastPredictedRate = 0;
	this-> predictedRate = 0;
	this-> estimationError = 0;
	this-> measuredValue = 0;
	this-> offset = 0;
	this-> upperLimit = 10000000;
}

void filterAlphaBeta::init(long offset){
	this-> predictedValue = 0;
	this-> lastPredictedValue = 0;
	this-> lastPredictedRate = 0;
	this-> predictedRate = 0;
	this-> estimationError = 0;
	this-> measuredValue = 0;
	this-> offset = offset;
	this-> upperLimit = 10000000;
}

void filterAlphaBeta::filterValue(long value, long deltaT){
	if(value>this->upperLimit)value = this->lastPredictedValue;


	//step 1 predicted value
	this->predictedValue = this->lastPredictedValue + deltaT * this->lastPredictedRate;
	//step 2 predicted rate
	this->predictedRate = this->lastPredictedRate;
	//step 3 estimation error
	this->estimationError = value - this->predictedValue;
	//step 4 update predicted value
	this->predictedValue = this->predictedValue + (this->alpha * this->estimationError)/100;
	//step 5 update predicted rate
	this->predictedRate = this->predictedRate + ((this->beta / deltaT) * this->estimationError)/100;

	//step 6 assign current values to be past values
	this->lastPredictedValue = this->predictedValue;
	this->lastPredictedRate = this->predictedRate;

}

long filterAlphaBeta::getFilteredValue(){
	return this->predictedValue - this->offset;
}
void filterAlphaBeta::setUpperLimit(long limit){
	this->upperLimit = limit;
}

long filterAlphaBeta::getFilteredRate(){
	return this->predictedRate;
}

filterAlphaBeta::~filterAlphaBeta() {
	// TODO Auto-generated destructor stub
}
