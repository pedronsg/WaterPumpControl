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

enum ServerStatus
{
  RUNNING,
  RESTARTREQUIRED,
  WRITINGFIRMWARE
};

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
    uint64_t session_key;
    bool authenticated(AsyncWebServerRequest *request);
    void redirectToAuthentication(AsyncWebServerRequest &request);
    String uint64ToString(uint64_t input);

  public:
    WebServer();
    void init(ConfigData &config, Eeprom &eeprom, WifiPump &wifiPump, Pump &pump, Tank &tank);
    void update();
    void start();
    static ServerStatus status;
    static void handle_firmwareUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    static uint8_t firmwareProgress;
    static uint32_t newFirmwareSize;
};


#endif
