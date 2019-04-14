#include "webServer.h"
#include <sstream>
#include "FS.h"
#include "ArduinoJson.h"



void WebServer::init(ConfigData &config, Eeprom &eeprom, WifiPump &wifiPump, Pump &pump, Tank &tank)
{
  _config= &config;
  _eeprom = &eeprom;
  _wifi = &wifiPump;
  _pump = &pump;
  _tank = &tank;
}


void WebServer::start()
{
  server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
     request->send(SPIFFS,  "/index.html","text/html");
  });
  server.on("/live", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
    request->send(SPIFFS,  "/index.html","text/html");
  });
  server.on("/network", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
    request->send(SPIFFS,  "/network.html","text/html");
  });
  server.on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
    request->send(SPIFFS,  "/settings.html","text/html");
  });
  server.on("/admin", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
    request->send(SPIFFS,  "/admin.html","text/html");
  });
  server.on("/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS,  "/bootstrap.css","text/css");
  });

  server.on("/jquery-2.1.0.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS,  "/jquery-2.1.0.js","application/javascript");
  });
  server.on("/utils.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS,  "/utils.js","application/javascript");
  });
  server.on("/jumbotron-narrow.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jumbotron-narrow.css","text/css");
  });
  server.on("/favicon-32x32.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon-32x32.png","image/png");
  });
  server.on("/favicon-16x16.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon-16x16.png","image/png");
  });


  server.on("/getSettings", HTTP_POST, [this](AsyncWebServerRequest *request){
      if(!request->authenticate(_config->http_username, _config->http_password))
          return request->requestAuthentication();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(128);

    root["maxBars"] = _config->maxBars;
    root["minBars"] = _config->minBars;
    root["maxAmps"] = _config->maxAmps;
    root["minAmps"] = _config->minAmps;
    root["maxRunTime"] = _config->maxRunningtime;
    root["noWater"] = _config->noWaterTime;
    root["unprotectedDelay"] = _config->unprotectedStartDelay;

    serializeJson(root,*response);
    request->send(response);

  });


  server.on("/setStatus", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonDocument root(32);
    deserializeJson(root, (const char*)data);
    
    if (!root.isNull()) {
      if (root.containsKey("startStop")) {
        if(root["startStop"])
        { //start pump only if tank isn't full
          if(_tank->getStatus()!=TankStatus::FULL)
          {
            _pump->init();
            _pump->start();
          }
        }
        else
        {
          _pump->stop(true);
        }
        request->send(200, "text/plain", "Success.");
      }
      else
      {
        request->send(404, "text/plain", "Error");
      }

    } else {
      request->send(404, "text/plain", "Error");
    }
  });

  server.on("/setSettings", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonDocument root(512);
    deserializeJson(root, (const char*)data);

    if (!root.isNull()) {
      if (root.containsKey("maxBars")) {
        _config->maxBars = root["maxBars"];
      }
      if (root.containsKey("minBars")) {
        _config->minBars = root["minBars"];
      }
      if (root.containsKey("maxAmps")) {
        _config->maxAmps = root["maxAmps"];
      }
      if (root.containsKey("minAmps")) {
        _config->minAmps = root["minAmps"];
      }
      if (root.containsKey("maxRunTime")) {
        _config->maxRunningtime = root["maxRunTime"];
      }
      if (root.containsKey("noWater")) {
        _config->noWaterTime = root["noWater"];
      }
      if (root.containsKey("unprotectedDelay")) {
        _config->unprotectedStartDelay = root["unprotectedDelay"];
      }

      if(_eeprom->save(*_config))
      {
        request->send(200, "text/plain", "Success.");
      }
      else
      {
        request->send(404, "text/plain", "Error");
      }

    } else {
      request->send(404, "text/plain", "Error");
    }
  });


  server.on("/getNetwork", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument root(512);

  root["wifiMode"] =(int)_config->wifiMode;
  root["apModeSSID"] = _config->wifiAp.network.ssid;
  root["apModePass"] = _config->wifiAp.network.key;

  root["clientModeSSID"] = _config->wifiClient.network.ssid;
  root["clientModePass"] = _config->wifiClient.network.key;
  root["clientDhcpMode"] =  _config->wifiClient.dhcpClient;
  root["clientModeIp"] = _config->wifiClient.network.ip;
  root["clientModeMask"] = _config->wifiClient.network.mask;
  root["clientModeGateway"] =_config->wifiClient.network.gateway;
	root["clientModeDns1"] = _config->wifiClient.network.dns;

  serializeJson(root,*response);
  request->send(response);

});


server.on("/getAdmin", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument root(128);

  root["httpUser"] = _config->http_password;
  root["httpPass"] = _config->http_username;

  serializeJson(root,*response);
  request->send(response);

});


 server.on("/getPumpStatus", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonDocument root(64);
    deserializeJson(root, (const char*)data);

    if (!root.isNull()) {
      if (root.containsKey("numberOfItems")) {
        int nitens=root["numberOfItems"];

        AsyncResponseStream *response = request->beginResponseStream("application/json");
        DynamicJsonDocument doc(512);
        JsonObject root2 = doc.to<JsonObject>();

        root2["pumpStatus"] = (int)_pump->getStatus();

        
        JsonArray array = root2.createNestedArray("values");

        for (int i=0; i<nitens; i++)
        {
          JsonObject object = array.createNestedObject();
          object["amps"] = _pump->getAmps();
          object["bars"] = _tank->getBars();
          object["maxBars"] = _config->maxBars;
          object["minBars"] = _config->minBars;
          object["pumpStatusValue"] = (int)_pump->getStatus();
        }       

        serializeJson(root2,*response);
        request->send(response);
        
      }
      else 
      {
        request->send(404, "text/plain", "Error");
      }
    } 
    else 
    {  
      request->send(404, "text/plain", "Error");
    }
  });


  server.on("/setNetwork", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonDocument root(512);
    deserializeJson(root, (const char*)data);
    if (!root.isNull()) {
      if (root.containsKey("wifiMode")) {
        if((int)root["wifiMode"]==WifiMode::ACCESSPOINT)
        {
          _config->wifiMode = ACCESSPOINT;
          strcpy(_config->wifiAp.network.ssid, root["apModeSSID"].as<char*>());
          strcpy(_config->wifiAp.network.key, root["apModePass"].as<char*>());
        }
        else if((int)root["wifiMode"]==WifiMode::CLIENT)
        {
          _config->wifiMode = CLIENT;
          _config->wifiClient.dhcpClient= root["clientDhcpMode"].as<bool>();
          strcpy(_config->wifiClient.network.ssid, root["clientModeSSID"].as<char*>());
          strcpy(_config->wifiClient.network.key, root["clientModePass"].as<char*>());
          if(!_config->wifiClient.dhcpClient)
          {
            strcpy(_config->wifiClient.network.ip, root["clientModeIp"].as<char*>());
            strcpy(_config->wifiClient.network.mask, root["clientModeMask"].as<char*>());
            strcpy(_config->wifiClient.network.gateway, root["clientModeGateway"].as<char*>());
            strcpy(_config->wifiClient.network.dns, root["clientModeDns1"].as<char*>());
            Serial.println(_config->wifiClient.network.ip);
          }
        }
        else if((int)root["wifiMode"]==WifiMode::DISABLED) 
        {
          _config->wifiMode = DISABLED;
        }
        if(_eeprom->save(*_config))
        {
          request->send(200, "text/plain", "Success.");
          _wifi->start();
        }
        else
        {
          request->send(404, "text/plain", "Error");
        }
      }
    } else {
      request->send(404, "text/plain", "Error");
    }
  });


  server.on("/reboot", HTTP_POST, [this](AsyncWebServerRequest *request){
  request->send(200, "text/plain", "Success.");
      Serial.println ( "reboot from web" );
      _pump->stop(true);
      digitalWrite(2, LOW);
      ESP.restart();
  });

  server.on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
  request->send(200, "text/plain", "Success.");
      Serial.println ( "reset from web" );
      _pump->stop(true);
      digitalWrite(2, LOW);
      _eeprom->reset();
      ESP.restart();
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
  request->requestAuthentication();
  request->send(200, "text/plain", "Success.");
  });

  server.on("/Chart.bundle.js", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS,  "/Chart.bundle.js","application/javascript");
  });

  server.on("/utils.js", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS,  "/utils.js","application/javascript");
  });

  server.begin();
  Serial.println ( "HTTP server started" );
}
