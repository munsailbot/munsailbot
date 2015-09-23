#ifndef __MOTOR_H
#define __MOTOR_H

#include "Common.h"

class Motor{
private:
    int16_t _position;
    int16_t _conversionFactorA;
    int16_t _conversionFactorB;

    int32_t _measuredPosition;
    int32_t _commandedPosition;
    int32_t _measuredPositionRaw;
    int32_t _commandedPositionRaw;

    uint8_t _pwmMax;
    uint8_t _pwmMin;
    uint8_t _measuredPwmMax;

public:
    Motor(int16_t position, int16_t coversionFactorA, int16_t conversionFactorB);

    void setCommandedPosition(int16_t position);
    int16_t getCommandedPosition();

    void setMeasuredPosition(int32_t positionRaw);
    int32_t getMeasuredPosition();

    int32_t getMeasuredPositionRaw();
    int32_t getCommandedPositionRaw();

};

#endif // __MOTOR_H
