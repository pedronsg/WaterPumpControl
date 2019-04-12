#ifndef TANK_H
#define TANK_H

#include "config.h"

enum TankStatus
{
  EMPTY,
  NORMAL,
  FULL
};


class Tank{
private:
  float bar;
  ConfigData conf;
  TankStatus status;
  uint64_t debounceEmptyMillis;
  uint64_t debounceFullMillis;
  bool debouncedEmpty;
  bool debouncedFull;
  void checkEmpty();
  void checkFull();
public:
  Tank();
  void set(float _bar, ConfigData _conf);
  float getBars();
  TankStatus getStatus();
};

#endif
