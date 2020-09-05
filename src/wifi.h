#ifndef WIFI_H
#define WIFI_H

#include <ESP8266WiFi.h>
#include "mqtt_ESP.h"
#include "Ticker.h"


class CWIFI
{
  private:
    MQTT_ESP *wifi_mqtt;
    Ticker _staTick;
    void onWifiConnect(const WiFiEventStationModeGotIP& event);
    void onWifiDisconnected(const WiFiEventStationModeDisconnected& event);
  public:
    CWIFI(MQTT_ESP &mqtt_);
    void start();
    void disconnect();
    String getIpAddress();
    void staCheck();
};

#endif
