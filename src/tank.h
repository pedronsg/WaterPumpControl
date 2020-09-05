#ifndef TANK_H
#define TANK_H

enum TankStatus
{
  EMPTY,
  NORMAL,
  FULL
};

#define TankStatusText ((char const*[]){ "EMPTY", "NORMAL", "FULL"})

class Tank{
private:
  float bar;
  TankStatus status;
  uint64_t debounceEmptyMillis;
  uint64_t debounceFullMillis;
  bool debouncedEmpty;
  bool debouncedFull;
  void checkEmpty();
  void checkFull();
public:
  Tank();
  float getBars();
  TankStatus getStatus();
  void update();
  String  getTextStatus();
};

#endif
