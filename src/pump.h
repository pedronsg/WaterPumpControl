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
  bool _needInit;
  uint64_t unprotectedStartMillis;
  uint64_t floodMillis;
  uint64_t noWaterMillis;
  uint64_t overloadMillis;
  PumpStatus status;

  PumpStatus previousStatus;
  bool timersUnprotectedSet;
  bool timersFloodSet;
  bool timersOverloadSet;
  bool timersNoWaterSet;
  float amps;
  ConfigData configs;

//  void startOnTimers();
  bool unprotectedStartTime();
  void checkFlood();
  void checkNoWater();
  void checkOverload();
public:
  bool needInit();
  void start();
  void stop();
  void set(float amp, ConfigData conf);
  PumpStatus getStatus();
  Pump();

//  PumpStatus getStatus(float ampValue, ConfigData config);
};

#endif
