//#define DEBUG_ENABLED

#include "mqtt_ESP.h"
#include <Ticker.h>
#include "config.h"
#include <ESP8266WiFi.h>

#define MQTT_PUMP_STATE_TOPIC           "WATER/PUMP/STATE"
#define MQTT_PUMP_STATE_AMPS_TOPIC      "WATER/PUMP/AMPS/VALUE"
#define MQTT_PUMP_STATE_MIN_AMPS_TOPIC  "WATER/PUMP/AMPS/MIN"
#define MQTT_PUMP_STATE_MAX_AMPS_TOPIC  "WATER/PUMP/AMPS/MAX"
#define MQTT_PUMP_CMD_TOPIC             "WATER/PUMP/CMD"

#define MQTT_TANK_STATE_TOPIC           "WATER/TANK/STATE"
#define MQTT_TANK_STATE_BARS_TOPIC      "WATER/TANK/BARS/VALUE"
#define MQTT_TANK_STATE_MIN_BARS_TOPIC  "WATER/TANK/BARS/MIN"
#define MQTT_TANK_STATE_MAX_BARS_TOPIC  "WATER/TANK/BARS/MAX"

ConfigData *mqtt_config;
Pump *mqtt_pump;
Tank *mqtt_tank;

//Ticker wifiReconnectTimer;
Ticker mqttReconnectTimer;

void MQTT_ESP::connectToMqtt() 
{
  #ifdef DEBUG_ENABLED 
  Serial1.println("INFO: Connecting to MQTT...");
  #endif
  if(initialized)
    connect();
}

void MQTT_ESP::onWifiConnected() 
{
  #ifdef DEBUG_ENABLED 
  Serial1.print("MQTT INFO: Connecting to Wi-Fi.)");
  #endif
  connectToMqtt();
}

void MQTT_ESP::onWifiDisconnected() 
{
  #ifdef DEBUG_ENABLED 
  Serial1.println("MQTT INFO: Disconnected from Wi-Fi.");
  #endif
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}


void MQTT_ESP::onMqttConnect(bool sessionPresent) 
{
  #ifdef DEBUG_ENABLED 
  Serial1.println("MQTT INFO: Connected to MQTT.");
  #endif
  subscribe(MQTT_PUMP_CMD_TOPIC, 2);
  publishAll();
}

void MQTT_ESP::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
  #ifdef DEBUG_ENABLED 
  Serial1.println("MQTT INFO: Disconnected from MQTT.");
  #endif
  if (WiFi.isConnected()) {
    #ifdef DEBUG_ENABLED 
      Serial1.println("MQTT INFO: Enabling MQTT reconnect timer");
    #endif
    mqttReconnectTimer.once(2, std::bind(&MQTT_ESP::connectToMqtt, this));
  }
}

void MQTT_ESP::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  #ifdef DEBUG_ENABLED 
  Serial1.println("Subscribe acknowledged.");
  #endif
}

void MQTT_ESP::onMqttUnsubscribe(uint16_t packetId) {
  #ifdef DEBUG_ENABLED 
  Serial1.println("Unsubscribe acknowledged.");
  Serial1.print("  packetId: ");
  Serial1.println(packetId);
  #endif
}

void MQTT_ESP::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
  if(!mqtt_config->useMQTT)
    return;

   char new_payload[len+1];
   new_payload[len] = '\0';
   strncpy(new_payload, payload, len);

  // handle message topic
  if (!strcmp(MQTT_PUMP_CMD_TOPIC,topic) )
  {
    // test if the payload is equal to "ON" or "OFF"
    if (!strcmp(new_payload, PumpStatusText[PumpStatus::ON]))
    {
        mqtt_pump->init();
        mqtt_pump->start();
        publish(MQTT_PUMP_STATE_TOPIC,1, true, PumpStatusText[PumpStatus::ON]);
        #ifdef DEBUG_ENABLED 
          Serial1.println("MQTT INFO: Remote pump on...");
        #endif
    } 
    else if (!strcmp(new_payload, PumpStatusText[PumpStatus::OFF]))
    {
      mqtt_pump->stop(true);
      publish(MQTT_PUMP_STATE_TOPIC,1, true, PumpStatusText[PumpStatus::OFF]);
      #ifdef DEBUG_ENABLED 
      Serial1.println("MQTT INFO: Remote pump off...");
      #endif
    }
  }
}

void MQTT_ESP::onMqttPublish(uint16_t packetId) {
  #ifdef DEBUG_ENABLED 
  // Serial1.println("Publish acknowledged.");
  #endif
}

MQTT_ESP::MQTT_ESP()
{
  mqtt_config=CConfig::GetInstance();
  initialized =false;
}


void MQTT_ESP::init(Pump &pump, Tank &tank)
{
    mqtt_pump = &pump;
    mqtt_tank = &tank;

    onConnect(std::bind(&MQTT_ESP::onMqttConnect, this,std::placeholders::_1));
    onDisconnect(std::bind(&MQTT_ESP::onMqttDisconnect, this,std::placeholders::_1));
    onSubscribe(std::bind(&MQTT_ESP::onMqttSubscribe, this,std::placeholders::_1,std::placeholders::_2));
    onUnsubscribe(std::bind(&MQTT_ESP::onMqttUnsubscribe, this,std::placeholders::_1));
    onMessage(std::bind(&MQTT_ESP::onMqttMessage, this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6));
    onPublish(std::bind(&MQTT_ESP::onMqttPublish, this,std::placeholders::_1));
    setCredentials(mqtt_config->mqttClient.user, mqtt_config->mqttClient.pass);
    setServer(mqtt_config->mqttClient.address, mqtt_config->mqttClient.port);
    if(!connected())
      mqttReconnectTimer.once(2, std::bind(&MQTT_ESP::connectToMqtt, this));
    initialized =true;
    #ifdef DEBUG_ENABLED 
    Serial1.println("MQTT initialized.");
    #endif
}


void MQTT_ESP::publishAll()
{
    sendMsg(MQTT_PUMP_STATE_TOPIC, mqtt_pump->getTextStatus().c_str());
    sendMsg(MQTT_PUMP_STATE_AMPS_TOPIC,String(mqtt_pump->getAmps()).c_str());
    sendMsg(MQTT_PUMP_STATE_MIN_AMPS_TOPIC, String(mqtt_config->minAmps).c_str());
    sendMsg(MQTT_PUMP_STATE_MAX_AMPS_TOPIC, String(mqtt_config->maxAmps).c_str());

    sendMsg(MQTT_TANK_STATE_TOPIC, mqtt_tank->getTextStatus().c_str());
    sendMsg(MQTT_TANK_STATE_BARS_TOPIC,String(mqtt_tank->getBars()).c_str());
    sendMsg(MQTT_TANK_STATE_MIN_BARS_TOPIC, String(mqtt_config->minBars).c_str());
    sendMsg(MQTT_TANK_STATE_MAX_BARS_TOPIC, String(mqtt_config->maxBars).c_str());
}

bool MQTT_ESP::isInitialized()
{
  return initialized;
}

void MQTT_ESP::sendMsg(const char* topic, const char* msg)
{
  if(mqtt_config->useMQTT)
  {
    #ifdef DEBUG_ENABLED 
      Serial1.printf("Published topic: %s, message: %s to %s:%d (user:%s pass:%s)\n\r", topic,msg, mqtt_config->mqttClient.address, mqtt_config->mqttClient.port, mqtt_config->mqttClient.user, mqtt_config->mqttClient.pass);
    #endif
    publish(topic,1, true, msg);
  }
}
