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

void WifiPump::start()
{
  disconnect();

//  delay(10000);

  switch (_config->wifiMode)
  {

    case ACCESSPOINT:
    {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(_config->wifiAp.network.ssid, _config->wifiAp.network.key);
      //display.printInitIp(WiFi.softAPIP().toString());
    }
    break;
    case CLIENT:
    {
      WiFi.mode(WIFI_STA);
    //  WiFi.reconnect();
        if(!_config->wifiClient.dhcpClient)
        {
          IPAddress ip, dns, gateway, subnet;
          ip=ip.fromString(_config->wifiClient.network.ip);
          gateway= gateway.fromString(_config->wifiClient.network.gateway);
          subnet= subnet.fromString(_config->wifiClient.network.mask);
          dns= dns.fromString(_config->wifiClient.network.dns);
          WiFi.config(ip, gateway, subnet, dns);
        }

        WiFi.begin(_config->wifiClient.network.ssid, _config->wifiClient.network.key);
    }
    break;
    case ACCESSPOINTCLIENT:
    {
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

      WiFi.begin(_config->wifiClient.network.ssid, _config->wifiClient.network.key);
      Serial.print("Connecting to ");
      Serial.print(_config->wifiClient.network.ssid); Serial.println(" ...");

      // int i = 0;
      // while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
      //   delay(1000);
      //   Serial.print(++i); Serial.print(' ');
      // }
    }
    break;
    case DISABLED:
    {
      WiFi.softAPdisconnect();
      WiFi.disconnect();
    }
    break;
    default:
    break;
  }
}
