#include <Arduino.h>
#include "tank.h"



Tank::Tank(){
  debounceEmptyMillis=0;
  debounceFullMillis=0;
  debouncedEmpty=true;
  debouncedFull=true;
  status=FULL;
}

void Tank::set(float _bar, ConfigData _conf)
{
  bar=_bar;
  conf=_conf;
}

float Tank::getBars()
{
  return bar;
}

TankStatus Tank::getStatus()
{
  status=NORMAL;
  checkEmpty();
  checkFull();

  return status;
}


void Tank::checkEmpty()
{
  if(bar<=conf.minBars && debouncedEmpty)
  {
    debouncedEmpty=false;
    debounceEmptyMillis = millis();
  }

  if(!debouncedEmpty && bar<=conf.minBars && millis()-debounceEmptyMillis>DEBOUNCE)
  {
    status=EMPTY;
    debouncedFull=true;
  }
}

void Tank::checkFull()
{
  if(bar>=conf.maxBars && debouncedFull)
  {
    debouncedFull=false;
    debounceFullMillis = millis();
  }

  if(!debouncedFull && bar>=conf.maxBars && millis()-debounceFullMillis>DEBOUNCE)
  {
    status=FULL;
    debouncedEmpty=true;

  }
}
