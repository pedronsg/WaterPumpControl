#include "webServer.h"
#include "wifi.h"
#include <Adafruit_ADS1015.h>
#include "eeprom.h"
#include "tank.h"
#include "pump.h"
#include "display.h"
#include "spiffs.h"


Adafruit_ADS1115 ads;
float amps=0;
float bars=0;

ConfigData config;
WifiPump wifi;
Tank tank;
Pump pump;
Eeprom eeprom;

WebServer webServer;
ServerStatus WebServer::status = ServerStatus::RUNNING;
uint8_t WebServer::firmwareProgress = 0;
uint32_t WebServer::newFirmwareSize = 0;
Display display;
Spiffs spiffs;

uint64_t blinkMillis=0;
bool blink=false;

void setup() {

  //serial communication
  Serial.begin ( 74880 );
  
  display.init();
  display.printFirmwareVersion();
  display.printProgressValue(10,String("loading...").c_str());
  //SPIFFS files initialization
  spiffs.begin();
  display.printProgressValue(20,String("loading...").c_str());
  //PUMP pin
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  //STOP Switch
  pinMode(STOP_SWITCH, INPUT);
  display.printProgressValue(30,String("loading...").c_str());
  // read eeprom data
  eeprom.begin();

  display.printProgressValue(50,String("loading...").c_str());
  //eeprom.reset();

  /*delay(7000);
  rst_info *rinfo= ESP.getResetInfoPtr();
  Serial.println("Reset reason=");
  Serial.print(rinfo->reason);
  Serial.println("");
  if(rinfo->reason == REASON_EXT_SYS_RST)
  {
    eeprom.reset();
 //   Serial.println("Reset...");
  }
*/

  config = eeprom.read();

  delay ( 500 );
  display.printProgressValue(70,String("loading...").c_str());
  //ads initialization
  ads.setGain(GAIN_ONE); // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();

  delay ( 500 );
  wifi.init(config);
  wifi.start();
  display.printProgressValue(80,String("loading...").c_str());
  webServer.init(config, eeprom, wifi, pump, tank);
  webServer.start();
  display.printProgressValue(100,String("loading...").c_str());
  delay ( 500 );
  display.printInitIp(wifi.getIpAddress());
  //display.clear();
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

  if(pump.getStatus() != PumpStatus::NOWATER && 
  pump.getStatus() != PumpStatus::FLOODPROTECTION && 
  pump.getStatus() != PumpStatus::OVERLOAD &&
  webServer.status == ServerStatus::RUNNING)
  {
    if(tank.getStatus()==TankStatus::EMPTY)
    {
      pump.start();
    }else if(tank.getStatus()==TankStatus::FULL)
    {
      pump.stop(false);
    }
  }

  switch (webServer.status)
  {
    case ServerStatus::WRITINGFIRMWARE:
      pump.stop(true);
      display.clear();
      display.drawProgressBarValue(webServer.firmwareProgress);
      display.drawMsg(String("flashing...").c_str());
      Serial.printf("Flashing %d%%\n",webServer.firmwareProgress);
      display.print();
    break;
    case ServerStatus::RUNNING:
      if(millis()-blinkMillis>1000)
      {
        display.clear();
        display.drawBars(config.minBars, config.maxBars, bars);
        display.drawAmps(config.minAmps, config.maxAmps, amps);
        if (blink) display.drawMsg(pump.getTextStatus().c_str());

        blink=!blink;
        display.print();
        blinkMillis=millis();
        Serial.printf("Amps=%f Bars=%f, PumpStatus=%d, TankStatus=%d\n",amps,bars,pump.getStatus(),tank.getStatus()); 
      }
      
    break;
    case ServerStatus::RESTARTREQUIRED:
      pump.stop(true);
      display.clear();
      display.drawProgressBarValue(100);
      display.drawMsg(String("Rebooting...").c_str());
      display.print();
      Serial.printf("Restarting ESP\n\r");
      delay(500);
      ESP.restart();
    break;
    default:
    break;
  }


}
