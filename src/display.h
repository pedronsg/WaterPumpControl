#ifndef DISPLAY_H
#define DISPLAY_H
#include "SSD1306Wire.h"

class Display{
private:
  SSD1306Wire ssdDisplay;
  bool _blink;
  uint64_t _blinkMillis;
public:
    Display();
    void init();
    void clear();
    void printInitIp(const String ip);
    void drawBars(const float min, const float max, const float bars);
    void drawAmps(const float min, const float max, const float amps);
    void drawProgressBarValue(const int progress);
    void printConnectingWifi();
    void print();
    void printProgressValue(const uint8_t value, const char* msg);
    void printFirmwareVersion();
    void drawMsg(const char* msg);
};

#endif
