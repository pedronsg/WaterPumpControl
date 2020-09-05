#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "pump.h"
#include "tank.h"

class Display{
private:
  uint64_t blinkMillis;
  bool blink;
  void clear();
  void drawBars(const float min, const float max, const float bars);
  void drawAmps(const float min, const float max, const float amps);
  void drawProgressBarValue(const int progress);
  void drawMsg(const char* msg);
  void drawMsg(const char* msg, int x, int y);
  void print();
public:
    Display();
    void init();
    void printInitIp(const String ip);
    void printConnectingWifi();
    void printProgressValue(const uint8_t value, const char* msg);
    void printFirmwareVersion();
    void printRunning(Pump &pump, Tank &tank);
    void printFlashing(uint8_t &percent);
    void printRebooting();
    void printReset();
};

#endif
