#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
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
    AsyncWebServer server;
    void handleRoot();
    void handleBootstrap();
    void handleCss();
    Eeprom *_eeprom;
    CWIFI *_wifi;
    Pump *_pump;
    Tank *_tank;

  public:
    WebServer();
    void init(Eeprom &eeprom, CWIFI &wifiPump, Pump &pump, Tank &tank);
    void update();
    void start();
    static ServerStatus status;
    static void handle_firmwareUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    static uint8_t firmwareProgress;
    static uint32_t newFirmwareSize;
};


#endif
