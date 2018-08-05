#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#include "config.h"


  void eepromBegin();
  bool saveEeprom(ConfigData data);
  ConfigData readEeprom();


#endif
