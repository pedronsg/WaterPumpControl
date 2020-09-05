//#define DEBUG_ENABLED

#include "config.h"
#include "eeprom.h"
#include "pressureSensor.h"
#include <ESP_EEPROM.h>



ConfigData *eeprom_data;

Eeprom::Eeprom()
{
    eeprom_data=CConfig::GetInstance();
}

bool Eeprom::save()
{
  #ifdef DEBUG_ENABLED 
  Serial1.println("Saving EEPROM data!");
  Serial1.print("HTTP user:"); Serial1.println(eeprom_data->http_username);
  Serial1.print("HTTP pass:"); Serial1.println(eeprom_data->http_password);
  Serial1.print("Max amps:"); Serial1.println(eeprom_data->maxAmps);
  Serial1.print("Max run time:"); Serial1.println(eeprom_data->maxRunningtime);
  Serial1.print("Max Bar:"); Serial1.println(eeprom_data->maxBars);
  Serial1.print("Min Amps:"); Serial1.println(eeprom_data->minAmps);
  Serial1.print("Mqtt server:"); Serial1.println(eeprom_data->mqttClient.address);
  Serial1.print("Mqtt pass"); Serial1.println(eeprom_data->mqttClient.pass);
  Serial1.print("Mqtt port:"); Serial1.println(eeprom_data->mqttClient.port);
  Serial1.print("Mqtt user:"); Serial1.println(eeprom_data->mqttClient.user);
  Serial1.print("No water timeout:"); Serial1.println(eeprom_data->noWaterTime);
  Serial1.print("Off Amps:"); Serial1.println(eeprom_data->offAmps);
  Serial1.print("Pressure calib:"); Serial1.println(eeprom_data->pressure_calibration); 
  Serial1.print("Unprot delay:"); Serial1.println(eeprom_data->unprotectedStartDelay);
  Serial1.print("Wifi client dhcp:"); Serial1.println(eeprom_data->wifiClient.dhcpClient);
  Serial1.print("Wifi client dns:"); Serial1.println(eeprom_data->wifiClient.network.dns);
  Serial1.print("Wifi client gateway:"); Serial1.println(eeprom_data->wifiClient.network.gateway);
  Serial1.print("Wifi client ip:"); Serial1.println(eeprom_data->wifiClient.network.ip);
  Serial1.print("Wifi client key:"); Serial1.println(eeprom_data->wifiClient.network.key);
  Serial1.print("Wifi client mask:");  Serial1.println(eeprom_data->wifiClient.network.mask);
  Serial1.print("Wifi client ssid:"); Serial1.println(eeprom_data->wifiClient.network.ssid);
  Serial1.print("Wifi mode:"); Serial1.println(eeprom_data->wifiMode);          
  Serial1.print("Wifi Ap SSID:"); Serial1.println(eeprom_data->wifiAp.network.ssid);
  Serial1.print("Wifi Ap dns:"); Serial1.println(eeprom_data->wifiAp.network.dns);
  Serial1.print("Wifi Ap gateway:"); Serial1.println(eeprom_data->wifiAp.network.gateway);
  Serial1.print("Wifi Ap ip:"); Serial1.println(eeprom_data->wifiAp.network.ip);
  Serial1.print("Wifi Ap key:"); Serial1.println(eeprom_data->wifiAp.network.key);
  Serial1.print("Wifi Ap mask:"); Serial1.println(eeprom_data->wifiAp.network.mask);  
  Serial1.print("Usemqtt="); Serial1.println(eeprom_data->useMQTT);
  #endif

  EEPROM.put(0, *eeprom_data);
  // write the data to EEPROM
  bool ok= EEPROM.commit();
  return ok;
}

void Eeprom::read()
{
  EEPROM.get(0, *eeprom_data);
  #ifdef DEBUG_ENABLED 
  Serial1.println("Ok, EEPROM data read!");
  Serial1.printf("SSID:"); Serial1.println(eeprom_data->wifiAp.network.ssid);
  Serial1.print("MQTT_Enabled="); Serial1.println(eeprom_data->useMQTT);
  #endif
}

void Eeprom::begin()
{
  #ifdef DEBUG_ENABLED 
  Serial1.println("Reading EEPROM...");
  Serial1.print("Struct size="); Serial1.print(sizeof(ConfigData)); Serial1.println(" bytes");
  #endif
  EEPROM.begin(sizeof(ConfigData));
  #ifdef DEBUG_ENABLED 
  Serial1.print("EEPROM used "); Serial1.print(EEPROM.percentUsed()); Serial1.println("%");
  #endif
  if(EEPROM.percentUsed()<0)
  {
    #ifdef DEBUG_ENABLED 
    Serial1.println("Setting default EEPROM values...");
    #endif
    reset();
  }
  #ifdef DEBUG_ENABLED 
  else
  {
    Serial1.println("Found EEPROM values...");
  }
  #endif
}

void Eeprom::reset()
{
  eeprom_data->maxAmps=8.00;
  eeprom_data->minAmps=2.00;
  eeprom_data->offAmps=0.50;
  eeprom_data->minBars=2.00;
  eeprom_data->maxBars=4.00;
  eeprom_data->pressure_calibration= PressureSensor::GetVoltage();
  strcpy(eeprom_data->http_password,"admin");
  strcpy(eeprom_data->http_username,"admin");
  eeprom_data->unprotectedStartDelay=4000; //milliseconds avoiding pump protection to discard high amp values
  eeprom_data->maxRunningtime=3; //maximum value of hours for flood protection
  eeprom_data->noWaterTime=5; //seconds before enter in nowater mode
  eeprom_data->wifiMode = WifiMode::ACCESSPOINT;
  eeprom_data->wifiAp.dhcpServer=true;
  eeprom_data->wifiClient.dhcpClient=true;
  strcpy(eeprom_data->wifiAp.network.ssid,"PumpControl");
  strcpy(eeprom_data->wifiAp.network.key,"");
  strcpy(eeprom_data->wifiClient.network.ssid,"");
  strcpy(eeprom_data->wifiClient.network.key,"");
  strcpy(eeprom_data->wifiClient.network.mask,"");
  strcpy(eeprom_data->wifiClient.network.dns,"");
  strcpy(eeprom_data->wifiClient.network.gateway,"");
  strcpy(eeprom_data->wifiClient.network.ip,"");

  eeprom_data->useMQTT=false;
  eeprom_data->mqttClient.port=1883;
  strcpy(eeprom_data->mqttClient.address,"192.168.1.1");
  strcpy(eeprom_data->mqttClient.user,"");
  strcpy(eeprom_data->mqttClient.pass,"");
  
  if (save())
  {
    #ifdef DEBUG_ENABLED   
    Serial1.println("Eeprom reset success");
    #endif
  }
  else
  {
    #ifdef DEBUG_ENABLED   
    Serial1.println("Failed to save Eeprom");
    #endif
  }
  
 
}
