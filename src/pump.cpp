#include <Arduino.h>
#include "pump.h"
#include "currentSensor.h"
#include "config.h"

#define NANOHOUR 3600000ul

ConfigData *configs;

Pump::Pump(){
  configs= CConfig::GetInstance();
  init();
}

void Pump::init()
{
  status=PumpStatus::OFF;
  previousStatus=PumpStatus::OFF;
  timersOverloadSet=true;
  timersNoWaterSet=true;
  timersFloodSet=true;
  unprotectedStartMillis=0;
  floodMillis=0;
  noWaterMillis=0;
  overloadMillis=0;
  webStop=false;
}


float Pump::getAmps()
{
  return amps;
}

void Pump::start()
{
  if (!webStop)
  {
    status=PumpStatus::ON;
    on();
  }  
}

void Pump::stop(const bool &isWebCommand)
{
  webStop=isWebCommand;
  timersNoWaterSet=true;
  status=PumpStatus::OFF;
  previousStatus=PumpStatus::OFF;
  off();
}

//Pump unprotected start timer
bool Pump::unprotectedStartTime()
{
  if(previousStatus==PumpStatus::OFF && status==PumpStatus::ON)
  {
    unprotectedStartMillis=millis();
    previousStatus=PumpStatus::ON;
    return true;
  }
  else if(previousStatus==PumpStatus::ON &&
    (millis()-unprotectedStartMillis)<=configs->unprotectedStartDelay &&
    status==PumpStatus::ON)
  {
    return true;
  }

  //previousStatus=PumpStatus::OFF;
  return false;
}

//Pump flood protection
void Pump::checkFlood()
{
  if(status==PumpStatus::FLOODPROTECTION)
  {
    return;
  }
  if (timersFloodSet && amps>=configs->minAmps && amps<=configs->maxAmps)
  {
    timersFloodSet=false;
    floodMillis = millis();
  }

  if(!timersFloodSet && ((millis()-floodMillis)>(uint64_t)configs->maxRunningtime*NANOHOUR) &&
    status==PumpStatus::ON  && amps>=configs->minAmps && amps<=configs->maxAmps)
  {
    status=PumpStatus::FLOODPROTECTION;
    off();
  }

  if (amps<configs->minAmps || amps>configs->maxAmps)
  {
    timersFloodSet=true;
    floodMillis = millis();
  }

}


//Pump no Water Protection
void Pump::checkNoWater()
{
  if(status == PumpStatus::NOWATER)
  {
    return;
  }

  //entered no water mode, start timer
  if (timersNoWaterSet && amps<configs->minAmps && status==PumpStatus::ON)
  {
    timersNoWaterSet=false;
    noWaterMillis = millis();
    return;
  }

  //no water time > time interval
  if(!timersNoWaterSet && amps<configs->minAmps && ((millis()-noWaterMillis)>(configs->noWaterTime*1000)) && status==PumpStatus::ON)
  {
    noWaterMillis = millis();
    status=PumpStatus::NOWATER;
    off();
    return;
  }

  //normal mode
  if(amps>=configs->minAmps)
  {
    timersNoWaterSet=true;
    noWaterMillis = millis();
  }
}


//pump overcurrent protection
void Pump::checkOverload()
{
  if(status == PumpStatus::OVERLOAD)
  {
    return;
  }
  if (timersOverloadSet && amps>configs->maxAmps)
  {
    timersOverloadSet=false;
    overloadMillis = millis();
  }

  if(!timersOverloadSet && amps>configs->maxAmps && millis()-overloadMillis>DEBOUNCE)
  {
    status = PumpStatus::OVERLOAD;
    off();
  }

  if(amps<=configs->maxAmps)
  {
    timersOverloadSet=true;
    overloadMillis = millis();
  }
}


PumpStatus Pump::getStatus()
{
  return status;
}

String Pump::getTextStatus()
{
  return PumpStatusText[status];
}

void Pump::update()
{
  amps= CurrentSensor::GetCurrent();
  if(isnan(amps)) 
    amps=0;

  if (!unprotectedStartTime())
  {
      checkOverload();
  }

  checkNoWater();
  checkFlood();
}

void Pump::off()
{
  digitalWrite(PUMP_PIN, HIGH);
}
void Pump::on()
{
  digitalWrite(PUMP_PIN, LOW);
}