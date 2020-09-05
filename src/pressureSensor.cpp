#include "pressureSensor.h"
#include <Adafruit_ADS1015.h>

#define ADS_1115_RESOLUTION     0.1875
#define EXPO_VAL                0.667
#define SENSOR_VOLTAGE          5
#define MPA_TO_BAR              10
#define ATMOSFEREPRESSURE       0.5

Adafruit_ADS1115 ads1115(0x48);

PressureSensor::PressureSensor()
{
    ads1115.begin();
}

float PressureSensor::GetVoltage()
{
    return ads1115.readADC_SingleEnded(0)*ADS_1115_RESOLUTION/1000;
}

float PressureSensor::GetBars(float &calibratedValue)
{
    return (((GetVoltage()-calibratedValue)/(SENSOR_VOLTAGE*EXPO_VAL))*MPA_TO_BAR)+ATMOSFEREPRESSURE;
}
    