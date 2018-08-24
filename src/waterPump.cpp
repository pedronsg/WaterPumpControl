#include <Arduino.h>
#include "html_handlers.h"
//#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Adafruit_ADS1015.h>
#include "eeprom.h"
#include "tank.h"
#include "pump.h"
#include "display.h"
#include "spiffs.h"

//#include "images.h"

#define ssid      "Linksys"      // WiFi SSID
#define password  "inesdanielapedro2011"  // WiFi password

Adafruit_ADS1115 ads;
float amps=0;
float bars=0;

ConfigData config;
Tank tank;
Pump pump;


String etatGpio[4] = {"OFF","OFF","OFF","OFF"};
ESP8266WebServer server ( 80 );
//SSD1306Wire  display(0x3c, D2, D1);
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
  eepromBegin();
  config = readEeprom();

  delay ( 500 );

  //ads initialization
  ads.setGain(GAIN_ONE); // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();

  delay ( 500 );

  if(digitalRead(STOP_SWITCH)==HIGH)
  {
    WiFi.softAP("ssid", password);
    Serial.println ( "" );
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }
  else
  {


    //wifi initialization
    WiFi.begin ( ssid, password );
    bool wifiConnected=false;
    while ( WiFi.status() != WL_CONNECTED ) {
      display.clear();
      if (!wifiConnected)
      {
        display.printConnectingWifi();
        Serial.print ( "." );
      }
      delay ( 500 );
      wifiConnected=!wifiConnected;
    }
  }

  //WiFi connection is OK
  Serial.println ( "" );
  Serial.print ( "Connected to " ); Serial.println ( ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );


  display.printInitIp(WiFi.localIP().toString());

  //starting http server
  server.on ( "/", handleRoot );
  server.on("/readData", handleData);
  server.begin();
  Serial.println ( "HTTP server started" );

//testing
  config.maxAmps=2.00;
  config.minAmps=1.00;
  config.minBars=1.00;
  config.maxBars=2.50;

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


  server.handleClient();

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
