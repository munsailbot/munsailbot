#include "Motor.h"

/*
Motor class encapsulates all values to be sent/received to/from the motor controller
*/

Motor::Motor(int16_t position, int16_t conversionFactorA, int16_t conversionFactorB){
    _pwmMax = 255;

    _commandedPosition = position;
    _measuredPosition = position;
    _conversionFactorA = conversionFactorA;
    _conversionFactorB = conversionFactorB;

    _measuredPositionRaw = (static_cast<int32_t>(_conversionFactorA - position)*_conversionFactorB)/1000;
    _commandedPositionRaw = (static_cast<int32_t>(_conversionFactorA - position)*_conversionFactorB)/1000;
}

void Motor::setCommandedPosition(int16_t position){
    _commandedPosition = position;
    _commandedPositionRaw = (static_cast<int32_t>(_conversionFactorA - position)*_conversionFactorB)/1000;
}

int16_t Motor::getCommandedPosition(){
    return _commandedPosition;
}

void Motor::setMeasuredPosition(int32_t positionRaw){
    _measuredPosition = _conversionFactorA - ((positionRaw*1000)/_conversionFactorB);
    _measuredPositionRaw = positionRaw;
}

int32_t Motor::getMeasuredPosition(){
    return _measuredPosition;
}

int32_t Motor::getMeasuredPositionRaw(){
    return _measuredPositionRaw;
}

int32_t Motor::getCommandedPositionRaw(){
    return _commandedPositionRaw;
}
