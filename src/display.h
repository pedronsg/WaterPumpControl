#ifndef DISPLAY_H
#define DISPLAY_H
#include "SSD1306Wire.h"

class Display{
private:
  SSD1306Wire ssdDisplay;
  void drawProgressBarDemo(const int progress);
public:
    Display();
    void clear();
    void printInitIp(const String ip);
    void drawBars(const float min, const float max, const float bars);
    void drawAmps(const float min, const float max, const float amps);
    void drawStatus(const String status);
    void printConnectingWifi();
    void print();
    void printProgress();
};


#endif
