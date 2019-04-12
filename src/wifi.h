#ifndef WIFIPUMP
#define WIFIPUMP

#include <ESP8266WiFi.h>
#include "config.h"

class WifiPump
{
  private:
    ConfigData *_config;

  public:
    WifiPump();
    void init(ConfigData &config);
    void start();
    void disconnect();
};

#endif
