#include <Arduino.h>
#include "pump.h"




Pump::Pump(){
  status=OFF;
  _needInit=false;
  timersOverloadSet=true;
  timersNoWaterSet=true;
  timersFloodSet=true;
  timersUnprotectedSet=true;
  unprotectedStartMillis=0;
  floodMillis=0;
  noWaterMillis=0;
  overloadMillis=0;
}


void Pump::set(float amp, ConfigData conf)
{
  amps=amp;
  configs=conf;
//  previousStatus=status;
  if (unprotectedStartTime())
  {
      checkOverload();
  }

  checkNoWater();
  checkFlood();
}


void Pump::start()
{
  digitalWrite(PUMP_PIN, HIGH);
  status=ON;
}


//Pump unprotected start timer
bool Pump::unprotectedStartTime()
{
  bool rtn=false;
  if(timersUnprotectedSet && status==ON)
  {
    timersUnprotectedSet=false;
    unprotectedStartMillis=millis();
    rtn=false;
  }

  if(!timersUnprotectedSet && (millis()-unprotectedStartMillis)>UNPROTECTED_START_DELAY && status==ON)
  {
    timersUnprotectedSet=true;
    rtn=true;
  }

  if(status!=ON)
  {
    timersUnprotectedSet=true;
    unprotectedStartMillis = 0;
    rtn=true;
  }
  return rtn;
}

//Pump flood protection
void Pump::checkFlood()
{
  if(_needInit)
  {
    return;
  }
  if (timersFloodSet && amps>=configs.minAmps && amps<=configs.maxAmps)
  {
    timersFloodSet=false;
    floodMillis = millis();
  }

  if(!timersFloodSet && ((millis()-floodMillis)>MAX_RUNNING_TIME*1000*60) &&
    status==ON  && amps>=configs.minAmps && amps<=configs.maxAmps)
  {
    status=FLOODPROTECTION;
    _needInit=true;
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
  if(_needInit)
  {
    return;
  }
  if (timersNoWaterSet && amps<configs.minAmps && status==ON)
  {
    timersNoWaterSet=false;
    noWaterMillis = millis();
  }

  if(!timersNoWaterSet && amps<configs.minAmps && millis()-noWaterMillis>NO_WATER_TIME*1000 && status==ON)
  {
    status=NOWATER;
    _needInit=true;
    digitalWrite(PUMP_PIN, LOW);
  }

  if(amps>=configs.minAmps)
  {
    timersNoWaterSet=true;
    noWaterMillis = millis();
  }
}


//pump overcurrent protection
void Pump::checkOverload()
{
  if(_needInit)
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
    status=OVERLOAD;
    _needInit=true;
    digitalWrite(PUMP_PIN, LOW);
  }

  if(amps<=configs.maxAmps)
  {
    timersOverloadSet=true;
    overloadMillis = millis();
  }
}



void Pump::stop()
{
  status=OFF;
  digitalWrite(PUMP_PIN, LOW);
}

PumpStatus Pump::getStatus()
{
  return status;
}



bool Pump::needInit()
{
  return _needInit;
}
