#ifndef MQTT_ESP_H_
#define MQTT_ESP_H_

#include <AsyncMqttClient.h>
#include "pump.h"
#include "tank.h"


class MQTT_ESP : public AsyncMqttClient
{
    private:
        bool initialized;

        void connectToMqtt();
        void onMqttConnect(bool sessionPresent);
        void onMqttDisconnect(AsyncMqttClientDisconnectReason reason); 
        void onMqttSubscribe(uint16_t packetId, uint8_t qos);
        void onMqttUnsubscribe(uint16_t packetId);
        void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
        void onMqttPublish(uint16_t packetId);
        void sendMsg(const char* topic, const char* msg);

    public:
        MQTT_ESP();
        void init(Pump &pump, Tank &tank);
        void publishAll();
        bool isInitialized();
        void onWifiConnected();
        void onWifiDisconnected();
};

#endif