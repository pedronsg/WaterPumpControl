#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

class PressureSensor{

public:
    PressureSensor();
    static float GetVoltage();
    static float GetBars(float &calibratedValue);
};

#endif