#ifndef HTML_HANDLERS
#define HTML_HANDLERS

#include <ESP8266WebServer.h>

extern float amps;
extern float bars;
extern String etatGpio[4];
const uint8_t GPIOPIN[4] = {D5,D6,D7,D8};
extern ESP8266WebServer server;

void updateGPIO(int gpio, String DxValue);
void handleData();
void handleBar();
void handleD5();
void handleD6();
void handleD7();
void handleD8();
void handleRoot();

#endif
