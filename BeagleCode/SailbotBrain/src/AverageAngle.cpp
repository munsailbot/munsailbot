#include "AverageAngle.h"

AverageAngle::AverageAngle(){
    _sampleCount = 0;
    _angles[0] = _angles[1] = 0;

    _isReady = false;
}

bool AverageAngle::isReady(){
    return _isReady;
}

void AverageAngle::addAngle(int angle){
    _angles[_sampleCount % 2] = angle;
    _sampleCount++;

    if(_sampleCount % 2 == 0) _isReady = true;
}

int AverageAngle::getAverage(){
	int a = _angles[0];
	int b = _angles[1];

	if(a < 0) a += 360;
	if(b < 0) b += 360;

	int diff = ( ( a - b + 180 + 360 ) % 360 ) - 180;
	int angle = (360 + b + ( diff / 2 ) ) % 360;

	if(angle > 180) angle -= 360;

	_isReady = false;

	return angle;
}
