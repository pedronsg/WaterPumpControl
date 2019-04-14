#ifndef CONFIG_H
#define CONFIG_H

#define STOP_SWITCH D5
#define PUMP_PIN D6
#define DEBOUNCE 10 //milliseconds to debounce values

#define DISPLAYADDRESS 0x3c
#define SDAPIN D2
#define SCLPIN D1


enum WifiMode{
  ACCESSPOINT,
  CLIENT,
  ACCESSPOINTCLIENT,
  DISABLED
};

struct WifiNetwork
{
  char ssid[64];
  char key[64];
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

  struct ConfigData{
  //    char status[10]="NOTSET";
      float offAmps;
      float minAmps;
      float maxAmps;
      float minBars;
      float maxBars;
      uint16_t unprotectedStartDelay; //milliseconds avoiding pump protection to discard high amp values
      uint8_t maxRunningtime; //maximum value of hours for flood protection
      uint8_t noWaterTime; //seconds before enter in nowater mode
      char http_username[64];
      char http_password[64];
      bool useHttps;
      WifiMode wifiMode;
      ApMode wifiAp;
      ClientMode wifiClient;
    };

#endif
