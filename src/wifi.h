#ifndef WIFIPUMP
#define WIFIPUMP

#include <ESP8266WiFi.h>
#include "config.h"
#include "Ticker.h"

class WifiPump
{
  private:
    ConfigData *_config;
    Ticker _staTick;

  public:
    WifiPump();
    void init(ConfigData &config);
    void start();
    void disconnect();
    const char* getIpAddress();
    void staCheck();
};

#endif
