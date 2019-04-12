#include <Arduino.h>
#include "eeprom.h"
#include <ESP_EEPROM.h>



bool Eeprom::save(ConfigData &data)
{
  EEPROM.put(0, data);
  // write the data to EEPROM
  bool ok= EEPROM.commit();
  Serial.println((ok) ? "Commit OK" : "Commit failed");
  return ok;
}

ConfigData Eeprom::read()
{
  ConfigData conf;
  EEPROM.get(0, conf);
  Serial.println("Ok, EEPROM data read!");
  return conf;
}

void Eeprom::begin()
{
  Serial.println("Reading EEPROM...");
  EEPROM.begin(sizeof(ConfigData));
  if(EEPROM.percentUsed()<0)
  {
    Serial.println("Setting default EEPROM values...");
    reset();
  }
  else
  {
    Serial.println("Found EEPROM values...");
  }
}

void Eeprom::reset()
{
  ConfigData data;
  data.maxAmps=3.00;
  data.minAmps=1.00;
  data.offAmps=0.50;
  data.minBars=1.00;
  data.maxBars=3.00;
  strcpy(data.http_password,"admin");
  strcpy(data.http_username,"admin");
  data.unprotectedStartDelay=5000; //milliseconds avoiding pump protection to discard high amp values
  data.maxRunningtime=30; //maximum value of minutes for flood protection
  data.noWaterTime=20; //seconds before enter in nowater mode
  data.wifiMode = ACCESSPOINT;
  data.wifiAp.dhcpServer=true;
  data.wifiClient.dhcpClient=true;
  strcpy(data.wifiAp.network.ssid,"PumpControl");
  strcpy(data.wifiAp.network.key,"");
  strcpy(data.wifiClient.network.ssid,"");
  strcpy(data.wifiClient.network.key,"");
  strcpy(data.wifiClient.network.mask,"");
  strcpy(data.wifiClient.network.dns,"");
  strcpy(data.wifiClient.network.gateway,"");
  strcpy(data.wifiClient.network.ip,"");
  bool saved= save(data);
  if (saved)
  {
    Serial.println("Eeprom reset");
  }
}
