#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#include "config.h"

  class Eeprom
  {
  private:

  public:
    void begin();
    bool save(ConfigData &data);
    ConfigData read();
    void reset();
  };
#endif
