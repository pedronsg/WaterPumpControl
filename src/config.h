#ifndef CONFIG_H
#define CONFIG_H

#define STOP_SWITCH D5
#define PUMP_PIN D6
#define UNPROTECTED_START_DELAY 5000 //milliseconds avoiding pump protection to discard high amp values
#define MAX_RUNNING_TIME 30 //maximum value of minutes for flood protection
#define NO_WATER_TIME 20 //seconds before enter in nowater mode
#define DEBOUNCE 10 //milliseconds to debounce values

#define DISPLAYADDRESS 0x3c
#define SDAPIN D2
#define SCLPIN D1

  struct ConfigData{
  //    char status[10]="NOTSET";
      float offAmps;
      float minAmps;
      float maxAmps;
      float minBars;
      float maxBars;
    };

#endif
