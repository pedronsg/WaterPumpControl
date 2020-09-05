#ifndef PUMP_H
#define PUMP_H

enum PumpStatus
{
  OFF=0,
  ON,
  NOWATER,
  OVERLOAD,
  FLOODPROTECTION,
  UNKNOWN
};
#define PumpStatusText ((char const*[]){ "OFF", "ON", "NO WATER", "OVERLOAD", "PROTECTION", "UNKNOWN" })

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

  bool unprotectedStartTime();
  void checkFlood();
  void checkNoWater();
  void checkOverload();
  void off();
  void on();
public:
  bool webStop;
  void start();
  void stop(const bool &isWebCommand);
  PumpStatus getStatus();
  String  getTextStatus();
  float getAmps();
  void init();
  Pump();
  void update();
};

#endif
