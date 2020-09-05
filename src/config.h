#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define STOP_SWITCH             D5
#define RESET_BUTTON            D3
#define PUMP_PIN                D6
#define DEBOUNCE                10 //milliseconds to debounce values
#define LONG_PRESS_TIME         5000 //long press seconds to reset    

#define LCDBLINKMSGINTERVAL     1000
#define DISPLAYADDRESS          0x3c
#define SDAPIN                  D2
#define SCLPIN                  D1
#define FIRMWAREVERSION         "1.0.0"
#define ANALOGPIN               A0
#define VOLTAGE                 3.30
#define ANALOGPRECISION         1024
#define READSENSORSINTERVAL     400
#define MQTTUPDATEINTERVAL      1000


enum WifiMode{
  ACCESSPOINT,
  CLIENT,
  ACCESSPOINTCLIENT,
  DISABLED
};

struct WifiNetwork
{
  char ssid[32];
  char key[32];
  char ip[16];
  char mask[16];
  char gateway[16];
  char dns[16];
};

struct ClientMode{
  bool dhcpClient;
  WifiNetwork network;
};

struct ApMode{
  bool dhcpServer;
  WifiNetwork network;
};

struct MQTTClient
{
  char address[16];
  int port;
  char user[32];
  char pass[32];
};

  struct ConfigData{
      float offAmps;
      float minAmps;
      float maxAmps;
      float minBars;
      float maxBars;
      uint16_t unprotectedStartDelay; //milliseconds avoiding pump protection to discard high amp values
      uint8_t maxRunningtime; //maximum value of hours for flood protection
      uint8_t noWaterTime; //seconds before enter in nowater mode
      char http_username[32];
      char http_password[32];
      bool useMQTT;
      float pressure_calibration; //read sensor value without any pressure
      WifiMode wifiMode;
      ApMode wifiAp;
      ClientMode wifiClient;
      MQTTClient mqttClient;
    };

  


class CConfig
{
  private:
    CConfig();
    
  public:
  static ConfigData *GetInstance();
};


#endif
