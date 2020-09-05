#include <Arduino.h>
#include "tank.h"
#include "pressureSensor.h"
#include "config.h"

ConfigData *conf;

Tank::Tank(){
  debounceEmptyMillis=0;
  debounceFullMillis=0;
  debouncedEmpty=true;
  debouncedFull=true;
  status=TankStatus::FULL;
  conf= CConfig::GetInstance();
}

float Tank::getBars()
{
  return (bar<0)?0.50:bar;
}

void Tank::update()
{
  bar = PressureSensor::GetBars(conf->pressure_calibration)+0.5;

  status=TankStatus::NORMAL;
  checkEmpty();

  if(status!=TankStatus::EMPTY)
    checkFull();
}

TankStatus Tank::getStatus()
{
  return status;
}


void Tank::checkEmpty()
{
  if(bar<=conf->minBars && debouncedEmpty)
  {
    debouncedEmpty=false;
    debounceEmptyMillis = millis();
  }

  if(!debouncedEmpty && bar<=conf->minBars && millis()-debounceEmptyMillis>DEBOUNCE)
  {
    status=TankStatus::EMPTY;
    debouncedFull=true;
  }
}

void Tank::checkFull()
{
  if(bar>=conf->maxBars && debouncedFull)
  {
    debouncedFull=false;
    debounceFullMillis = millis();
  }

  if(!debouncedFull && bar>=conf->maxBars && millis()-debounceFullMillis>DEBOUNCE)
  {
    status=TankStatus::FULL;
    debouncedEmpty=true;
  }
}

String Tank::getTextStatus()
{
  return TankStatusText[status];
}