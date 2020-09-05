#include "currentSensor.h"
#include <PZEM004Tv30.h>

#define PZEM004_NO_SWSERIAL
PZEM004Tv30 pzem(&Serial);

CurrentSensor::CurrentSensor()
{
    pzem.resetEnergy();
}

float CurrentSensor::GetCurrent()
{
    return pzem.current();
}