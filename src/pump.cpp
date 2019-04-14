#include <Arduino.h>
#include "pump.h"




Pump::Pump(){
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

void Pump::set(float amp, ConfigData conf)
{
  amps=amp;
  configs=conf;
  if (!unprotectedStartTime())
  {
      checkOverload();
  }

  checkNoWater();
  checkFlood();
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
    digitalWrite(PUMP_PIN, HIGH);
  }  
}

void Pump::stop(const bool &isWebCommand)
{
  webStop=isWebCommand;
  timersNoWaterSet=true;
  status=PumpStatus::OFF;
  previousStatus=PumpStatus::OFF;
  digitalWrite(PUMP_PIN, LOW);
}

//Pump unprotected start timer
bool Pump::unprotectedStartTime()
{
  bool rtn=false;
  if(previousStatus==PumpStatus::OFF && status==PumpStatus::ON)
  {
    unprotectedStartMillis=millis();
    rtn=true;
    previousStatus=PumpStatus::ON;
  }
  else if(previousStatus==PumpStatus::ON &&
    (millis()-unprotectedStartMillis)<=configs.unprotectedStartDelay &&
    status==PumpStatus::ON)
  {
    rtn=true;
  }

  return rtn;
}

//Pump flood protection
void Pump::checkFlood()
{
  if(status==PumpStatus::FLOODPROTECTION)
  {
    return;
  }
  if (timersFloodSet && amps>=configs.minAmps && amps<=configs.maxAmps)
  {
    timersFloodSet=false;
    floodMillis = millis();
  }

  if(!timersFloodSet && ((millis()-floodMillis)>(uint64_t)configs.maxRunningtime*1000*60*60) &&
    status==PumpStatus::ON  && amps>=configs.minAmps && amps<=configs.maxAmps)
  {
    status=PumpStatus::FLOODPROTECTION;
    digitalWrite(PUMP_PIN, LOW);
  }

  if (amps<configs.minAmps || amps>configs.maxAmps)
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
  if (timersNoWaterSet && amps<configs.minAmps && status==PumpStatus::ON)
  {
    timersNoWaterSet=false;
    noWaterMillis = millis();
    return;
  }

  //no water time > time interval
  if(!timersNoWaterSet && amps<configs.minAmps && ((millis()-noWaterMillis)>(configs.noWaterTime*1000)) && status==PumpStatus::ON)
  {
    noWaterMillis = millis();
    status=PumpStatus::NOWATER;
    digitalWrite(PUMP_PIN, LOW);
    return;
  }

  //normal mode
  if(amps>=configs.minAmps)
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
  if (timersOverloadSet && amps>configs.maxAmps)
  {
    timersOverloadSet=false;
    overloadMillis = millis();
  }

  if(!timersOverloadSet && amps>configs.maxAmps && millis()-overloadMillis>DEBOUNCE)
  {
    status = PumpStatus::OVERLOAD;
    digitalWrite(PUMP_PIN, LOW);
  }

  if(amps<=configs.maxAmps)
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
  String rtn="UNKNOWN";
  switch (status) {
    case PumpStatus::OFF:
      rtn="OFF";
      break;
    case PumpStatus::ON:
      rtn="ON";
      break;
    case PumpStatus::NOWATER:
      rtn="NO WATER";
      break;
    case PumpStatus::OVERLOAD:
      rtn="OVERLOAD";
      break;
    case PumpStatus::FLOODPROTECTION:
      rtn="PROTECTION";
      break;
  }
  return rtn;
}

