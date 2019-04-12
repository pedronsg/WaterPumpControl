#include <Arduino.h>
#include "webServer.h"
//#include "html_handlers.h"
#include "wifi.h"
//#include <WiFiClient.h>
#include <Adafruit_ADS1015.h>
#include "eeprom.h"
#include "tank.h"
#include "pump.h"
#include "display.h"
#include "spiffs.h"

//#include "images.h"

//#define ssid      "Linksys"      // WiFi SSID
//#define password  "inesdanielapedro2011"  // WiFi password

Adafruit_ADS1115 ads;
float amps=0;
float bars=0;

ConfigData config;
WifiPump wifi;
Tank tank;
Pump pump;
Eeprom eeprom;

WebServer webServer;
Display display;
Spiffs spiffs;

uint64_t serialMillis=0;
bool blink=false;



void setup() {

  //serial communication
  Serial.begin ( 115200 );

  display.printProgress();

  //SPIFFS files initialization
  spiffs.begin();

  //PUMP pin
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  //STOP Switch
  pinMode(STOP_SWITCH, INPUT);

  // read eeprom data
  eeprom.begin();
  eeprom.reset();
  config = eeprom.read();

  delay ( 500 );

  //ads initialization
  ads.setGain(GAIN_ONE); // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();

  delay ( 500 );
  wifi.init(config);
  wifi.start();
  webServer.init(config, eeprom, wifi, pump, tank);
  webServer.start();
}







void loop() {

  int16_t adc0, adc1;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);

  adc0=(adc0<0)?0:adc0;
  adc1=(adc1<0)?0:adc1;

  amps=(float)adc0*0.125/1000.000;
  bars=(float)adc1*0.125/1000.000;

  pump.set(amps,config);
  tank.set(bars,config);

  if(!pump.needInit())
  {
    if(tank.getStatus()==EMPTY)
    {
      pump.start();
    }else if(tank.getStatus()==FULL)
    {
      pump.stop();
    }
  }


//  server.handleClient();
//  webServer.update();

  //each second debug messages
  if(millis()-serialMillis>1000)
  {
    Serial.print("Amps: "); Serial.println(amps);
    Serial.print("Bars: "); Serial.println(bars);
    Serial.print("PumpStatus: "); Serial.println(pump.getStatus());
    Serial.print("TankStatus: "); Serial.println(tank.getStatus());
    Serial.println(" ");

    display.drawBars(config.minBars, config.maxBars, bars);
    display.drawAmps(config.minAmps, config.maxAmps, amps);

    if(!blink)
    {
      display.drawStatus(pump.getTextStatus());
    }

    blink=!blink;
    display.print();

    serialMillis=millis();
  }


}
