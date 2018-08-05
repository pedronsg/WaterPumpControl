#ifndef CONFIG_H
#define CONFIG_H

#define PUMP_PIN D6
#define UNPROTECTED_START_DELAY 10000 //milliseconds avoiding pump protection to discard high amp values
#define MAX_RUNNING_TIME 1 //maximum value of minutes for flood protection
#define NO_WATER_TIME 30 //seconds before enter in nowater mode
#define DEBOUNCE 10 //milliseconds to debounce values

  struct ConfigData{
  //    char status[10]="NOTSET";
      float offAmps;
      float minAmps;
      float maxAmps;
      float minBars;
      float maxBars;
    };

#endif
