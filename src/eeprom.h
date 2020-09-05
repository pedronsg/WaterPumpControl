#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

  class Eeprom
  {
  private:

  public:
    Eeprom();
    void begin();
    bool save();
    void read();
    void reset();
  };
#endif
