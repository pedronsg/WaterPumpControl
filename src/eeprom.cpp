#include <Arduino.h>
#include "eeprom.h"
#include <ESP_EEPROM.h>

void resetEeprom();

bool saveEeprom(ConfigData data)
{
  EEPROM.put(0, data);
  // write the data to EEPROM
  bool ok= EEPROM.commit();
  Serial.println((ok) ? "Commit OK" : "Commit failed");
  return ok;
}

ConfigData readEeprom()
{
  ConfigData conf;
  EEPROM.get(0, conf);
  Serial.println("Ok, EEPROM data read!");
  return conf;
}

void eepromBegin()
{
  Serial.println("Reading EEPROM...");
  EEPROM.begin(sizeof(ConfigData));
  if(EEPROM.percentUsed()<0)
  {
    Serial.println("Setting default EEPROM values...");
    resetEeprom();
  }
  else
  {
    Serial.println("Found EEPROM values...");
  }
}

void resetEeprom()
{
  ConfigData data;
  data.maxAmps=3.00;
  data.minAmps=1.00;
  data.offAmps=0.50;
  data.minBars=1.00;
  data.maxBars=3.00;
  bool saved= saveEeprom(data);
  if (saved)
  {
    Serial.println("Eeprom reset");
  }
}
