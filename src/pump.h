#ifndef PUMP_H
#define PUMP_H

#include "config.h"

enum PumpStatus
{
  OFF,
  ON,
  NOWATER,
  OVERLOAD,
  FLOODPROTECTION
};



class Pump{
private:
  uint64_t unprotectedStartMillis;
  uint64_t floodMillis;
  uint64_t noWaterMillis;
  uint64_t overloadMillis;
  PumpStatus status;
  PumpStatus previousStatus;

  bool timersFloodSet;
  bool timersOverloadSet;
  bool timersNoWaterSet;
  float amps;
  ConfigData configs;

  bool unprotectedStartTime();
  void checkFlood();
  void checkNoWater();
  void checkOverload();
public:
  bool webStop;
  void start();
  void stop(const bool &isWebCommand);
  void set(float amp, ConfigData conf);
  PumpStatus getStatus();
  String  getTextStatus();
  float getAmps();
  void init();
  Pump();
};

#endif
