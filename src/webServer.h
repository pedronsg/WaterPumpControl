#ifndef _WEBSERVER_H
#define _WEBSERVER_H

//#include <ESP8266WebServer.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "config.h"
#include "eeprom.h"
#include "wifi.h"
#include "pump.h"
#include "tank.h"

class WebServer
{
  private:
    //ESP8266WebServer server;
    AsyncWebServer server;
    void handleRoot();
    void handleBootstrap();
    void handleCss();
    ConfigData *_config;
    Eeprom *_eeprom;
    WifiPump *_wifi;
    Pump *_pump;
    Tank *_tank;

  public:
    void init(ConfigData &config, Eeprom &eeprom, WifiPump &wifiPump, Pump &pump, Tank &tank);
    void update();
    void start();
};


#endif
