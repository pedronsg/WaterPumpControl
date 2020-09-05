//#define DEBUG_ENABLED
#include "wifi.h"
#include "config.h"

WiFiEventHandler wifiConnectHandler_;
WiFiEventHandler wifiDisconnectHandler_;
Ticker wifiReconnectTimer_;

ConfigData *wifi_conf;


CWIFI::CWIFI(MQTT_ESP &mqtt_)
{
  wifi_mqtt=&mqtt_;
  wifi_conf= CConfig::GetInstance();
  WiFi.hostname("PumpControl");
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  wifiConnectHandler_ = WiFi.onStationModeGotIP(std::bind(&CWIFI::onWifiConnect,this,std::placeholders::_1));
  wifiDisconnectHandler_ = WiFi.onStationModeDisconnected(std::bind(&CWIFI::onWifiDisconnected,this,std::placeholders::_1));
}

void CWIFI::onWifiConnect(const WiFiEventStationModeGotIP& event) 
{
  #ifdef DEBUG_ENABLED 
  Serial1.print("Wifi INFO: Connecting to Wi-Fi. (");
  Serial1.print(WiFi.localIP());
  Serial1.println(")...");
  #endif
  wifi_mqtt->onWifiConnected();
}

void CWIFI::onWifiDisconnected(const WiFiEventStationModeDisconnected& event) 
{
  #ifdef DEBUG_ENABLED 
  Serial1.println("Wifi INFO: Disconnected from Wi-Fi.");
  #endif
  wifiReconnectTimer_.once(4, std::bind(&CWIFI::start, this));
  wifi_mqtt->onWifiDisconnected();
}

void CWIFI::disconnect()
{
  #ifdef DEBUG_ENABLED
    Serial1.println("Disconnecting Wifi...");
  #endif
  WiFi.softAPdisconnect();
  WiFi.disconnect();
}

String CWIFI::getIpAddress()
{
  if (WiFi.localIP().toString() != "(IP unset)")
  {
    return WiFi.localIP().toString();
  }
  else
  {
    return WiFi.softAPIP().toString();
  }
  
}

void CWIFI::staCheck(){
  _staTick.detach();
  if(!(uint32_t)WiFi.localIP()){
    WiFi.mode(WIFI_AP);
  }
}

void CWIFI::start()
{ 
//  disconnect();

  switch (wifi_conf->wifiMode)
  {

    case ACCESSPOINT:
    {
      WiFi.hostname("PumpControl");
      WiFi.mode(WIFI_AP);
      WiFi.softAP(wifi_conf->wifiAp.network.ssid, wifi_conf->wifiAp.network.key);
    }
    break;
    case CLIENT:
    {
      #ifdef DEBUG_ENABLED
        Serial1.println("Configuring to Client mode...");
      #endif

      WiFi.mode(WIFI_STA);

      if(!wifi_conf->wifiClient.dhcpClient)
      {
        IPAddress ip, dns, gateway, subnet;
        ip=ip.fromString(wifi_conf->wifiClient.network.ip);
        gateway= gateway.fromString(wifi_conf->wifiClient.network.gateway);
        subnet= subnet.fromString(wifi_conf->wifiClient.network.mask);
        dns= dns.fromString(wifi_conf->wifiClient.network.dns);
        WiFi.config(ip, gateway, subnet, dns);
        #ifdef DEBUG_ENABLED
        Serial1.println(wifi_conf->wifiClient.network.ip);
        Serial1.println(wifi_conf->wifiClient.network.gateway);
        Serial1.println(wifi_conf->wifiClient.network.mask);
        Serial1.println(wifi_conf->wifiClient.network.dns);
        Serial1.println(wifi_conf->wifiClient.network.ssid);
        Serial1.println(wifi_conf->wifiClient.network.key);
        #endif
      }

      WiFi.hostname("PumpControl");
      WiFi.begin(wifi_conf->wifiClient.network.ssid, wifi_conf->wifiClient.network.key);
      
    }
    break;
    case ACCESSPOINTCLIENT:
    {
      _staTick.attach(10, std::bind(&CWIFI::staCheck, this));

      WiFi.mode(WIFI_AP_STA);
      
      WiFi.softAP(wifi_conf->wifiAp.network.ssid, wifi_conf->wifiAp.network.key);

      if(!wifi_conf->wifiClient.dhcpClient)
      {
        IPAddress ip, dns, gateway, subnet;
        ip=ip.fromString(wifi_conf->wifiClient.network.ip);
        gateway= gateway.fromString(wifi_conf->wifiClient.network.gateway);
        subnet= subnet.fromString(wifi_conf->wifiClient.network.mask);
        dns= dns.fromString(wifi_conf->wifiClient.network.dns);
        WiFi.config(ip, gateway, subnet, dns);
      }

      WiFi.hostname("PumpControl");
      WiFi.begin(wifi_conf->wifiClient.network.ssid, wifi_conf->wifiClient.network.key);
      
      #ifdef DEBUG_ENABLED
        Serial1.print("Connecting to ");
        Serial1.print(wifi_conf->wifiClient.network.ssid); Serial1.println(" ...");
      #endif
    }
    break;
    case DISABLED:
    {
      WiFi.softAPdisconnect();
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
    }
    break;
    default:
    break;
  }
}
