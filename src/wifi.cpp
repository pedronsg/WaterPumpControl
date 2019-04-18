#include "wifi.h"

WifiPump::WifiPump()
{
  WiFi.hostname("PumpControl");
}

void WifiPump::init(ConfigData &config)
{
  _config= &config;
}

void WifiPump::disconnect()
{
  WiFi.softAPdisconnect();
  WiFi.disconnect();
}

const char* WifiPump::getIpAddress()
{
  if (!(uint32_t)WiFi.localIP())
  {
    return WiFi.localIP().toString().c_str();
  }
  else
  {
    return WiFi.softAPIP().toString().c_str();
  }
  
}

void WifiPump::staCheck(){
  _staTick.detach();
  if(!(uint32_t)WiFi.localIP()){
    WiFi.mode(WIFI_AP);
  }
}

void WifiPump::start()
{
  disconnect();

  switch (_config->wifiMode)
  {

    case ACCESSPOINT:
    {
      WiFi.mode(WIFI_AP);
      WiFi.hostname("PumpControl");
      WiFi.softAP(_config->wifiAp.network.ssid, _config->wifiAp.network.key);
    }
    break;
    case CLIENT:
    {
      WiFi.mode(WIFI_STA);

        if(!_config->wifiClient.dhcpClient)
        {
          IPAddress ip, dns, gateway, subnet;
          ip=ip.fromString(_config->wifiClient.network.ip);
          gateway= gateway.fromString(_config->wifiClient.network.gateway);
          subnet= subnet.fromString(_config->wifiClient.network.mask);
          dns= dns.fromString(_config->wifiClient.network.dns);
          WiFi.config(ip, gateway, subnet, dns);
        }
        WiFi.hostname("PumpControl");
        WiFi.begin(_config->wifiClient.network.ssid, _config->wifiClient.network.key);
    }
    break;
    case ACCESSPOINTCLIENT:
    {
      _staTick.attach(10, std::bind(&WifiPump::staCheck, this));

      WiFi.mode(WIFI_AP_STA);
      
      WiFi.softAP(_config->wifiAp.network.ssid, _config->wifiAp.network.key);

      if(!_config->wifiClient.dhcpClient)
      {
        IPAddress ip, dns, gateway, subnet;
        ip=ip.fromString(_config->wifiClient.network.ip);
        gateway= gateway.fromString(_config->wifiClient.network.gateway);
        subnet= subnet.fromString(_config->wifiClient.network.mask);
        dns= dns.fromString(_config->wifiClient.network.dns);
        WiFi.config(ip, gateway, subnet, dns);
      }

      WiFi.hostname("PumpControl");
      WiFi.begin(_config->wifiClient.network.ssid, _config->wifiClient.network.key);

      Serial.print("Connecting to ");
      Serial.print(_config->wifiClient.network.ssid); Serial.println(" ...");
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
