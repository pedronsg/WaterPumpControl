//#define DEBUG_ENABLED

#include "webServer.h"
#include "display.h"
#include "spiffs.h"
//#include "littlefs.h"
#include <Bounce2.h>
#include "config.h"
#include "mqtt_ESP.h"

Bounce resetButton = Bounce(); 

MQTT_ESP mqtt;
CWIFI wifi(mqtt);
Tank tank;
Pump pump;
Eeprom eeprom;
WebServer webServer;

ConfigData *config;

ServerStatus WebServer::status = ServerStatus::RUNNING;
uint8_t WebServer::firmwareProgress = 0;
uint32_t WebServer::newFirmwareSize = 0;
Display display;
//LITTLE_FS littlefs;
Spiffs littlefs;
uint64_t analogReadMillis=0;
uint64_t mqttMillis=0;



void setup() {
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(ANALOGPIN, INPUT);

  resetButton.attach(RESET_BUTTON);
  resetButton.interval(LONG_PRESS_TIME); // interval in ms
  //PUMP off
  digitalWrite(PUMP_PIN, HIGH);

  #ifdef DEBUG_ENABLED 
    Serial1.begin(115200);
  #endif

  display.init();
  display.printFirmwareVersion();
  display.printProgressValue(10,"loading...");
 
  //SPIFFS files initialization
  littlefs.begin();
  display.printProgressValue(30,"loading...");
  delay ( 500 );
  // read eeprom data
  eeprom.begin();
  display.printProgressValue(50,"loading...");
  //eeprom.reset();
  eeprom.read();
  config = CConfig::GetInstance();
  display.printProgressValue(70,"loading...");
  delay ( 500 );
  
  wifi.start();
  display.printProgressValue(80,"loading...");
  webServer.init(eeprom, wifi, pump, tank);
  webServer.start();
  display.printProgressValue(100,"loading...");
  delay ( 500 );
  display.printInitIp(wifi.getIpAddress().c_str());

  #ifdef DEBUG_ENABLED 
  Serial1.println(config->wifiAp.network.ssid);
  #endif

  if(config->useMQTT)
    mqtt.init(pump, tank);
}

int lastState=-1;


void runMqtt()
{
  if(!config->useMQTT)
    return;

  if(lastState != pump.getStatus())
  {
    if(!mqtt.isInitialized())
      mqtt.init(pump, tank);
    mqtt.publishAll();        
  }
  lastState=pump.getStatus();
}


void readSensors()
{
  tank.update();
  pump.update();

  runMqtt();

  if(tank.getStatus()==TankStatus::FULL)
  {
    pump.stop(false);
  }

  if(pump.getStatus() != PumpStatus::NOWATER && 
    pump.getStatus() != PumpStatus::FLOODPROTECTION && 
    pump.getStatus() != PumpStatus::OVERLOAD &&
    webServer.status == ServerStatus::RUNNING &&
    tank.getStatus()==TankStatus::EMPTY)
  {
    pump.start();
  }
  



  switch (webServer.status)
  {
    case ServerStatus::WRITINGFIRMWARE:
      pump.stop(true);
      display.printFlashing(webServer.firmwareProgress);
      #ifdef DEBUG_ENABLED 
        Serial1.printf("Flashing %d%%\n",webServer.firmwareProgress);
      #endif   
    break;
    case ServerStatus::RUNNING:
        display.printRunning(pump, tank);
        #ifdef DEBUG_ENABLED 
       //   Serial1.printf("Amps=%f Bars=%f, PumpStatus=%d, TankStatus=%d\n",pump.getAmps(),
      //    tank.getBars(),pump.getStatus(),tank.getStatus()); 
        #endif
    break;
    case ServerStatus::RESTARTREQUIRED:
      pump.stop(true);
      display.printRebooting();
      #ifdef DEBUG_ENABLED 
        Serial1.printf("Restarting ESP\n\r");
      #endif
      delay(500);
      ESP.restart();
    break;
    default:
    break;
  }
}


void loop() {

  resetButton.update();

  //reset button
  if (resetButton.fell()) 
  {
    pump.stop(false);
    display.printReset();
    eeprom.reset();
    delay(1000);
    ESP.restart();
  }

   //read sensors each ms interval
 // if(millis()-analogReadMillis>READSENSORSINTERVAL)
 // {
    readSensors();
  //  analogReadMillis=millis();
 // }

  //mqtt update interval
  if(config->useMQTT && millis()-mqttMillis>MQTTUPDATEINTERVAL)
  {
   // Serial1.printf("Checking mqtt...\n\r");
    if(!mqtt.isInitialized())
      mqtt.init(pump, tank);

    mqtt.publishAll();  
    mqttMillis=millis();
  }
}
