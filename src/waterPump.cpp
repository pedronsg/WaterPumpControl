#include <Arduino.h>
#include "html_handlers.h"
#include <WiFiClient.h>
#include <Adafruit_ADS1015.h>
#include "eeprom.h"
#include "tank.h"
#include "pump.h"

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



void setup() {
  //pin initialization
  // for ( int x = 0 ; x < 5 ; x++ ) {
  //   pinMode(GPIOPIN[x],OUTPUT);
  // }

  //PUMP pin
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  //serial communication
  Serial.begin ( 115200 );

  delay (5000 );

  //read eeprom data
  eepromBegin();
  config = readEeprom();

  delay ( 500 );

  //ads initialization
  ads.setGain(GAIN_ONE); // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();

  delay ( 500 );

  //wifi initialization
  WiFi.begin ( ssid, password );
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ); Serial.print ( "." );
  }
  //WiFi connection is OK
  Serial.println ( "" );
  Serial.print ( "Connected to " ); Serial.println ( ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );

  //starting http server
  server.on ( "/", handleRoot );
  server.on("/readData", handleData);
  server.begin();
  Serial.println ( "HTTP server started" );

//testing
  config.maxAmps=2.50;
  config.minAmps=1.00;
  config.minBars=1.00;
  config.maxBars=2.50;

}



uint64_t serialMillis=0;

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
    serialMillis=millis();
  }


}
