#include "webServer.h"
#include <sstream>
#include "FS.h"
#include "ArduinoJson.h"

WebServer::WebServer()
{
}


void WebServer::init(ConfigData &config, Eeprom &eeprom, WifiPump &wifiPump, Pump &pump, Tank &tank)
{
  _config= &config;
  _eeprom = &eeprom;
  _wifi = &wifiPump;
  _pump = &pump;
  _tank = &tank;
}

void WebServer::handle_firmwareUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

 if(status==ServerStatus::RESTARTREQUIRED) return;

  if (!index){
    Update.runAsync(true);
    if (!Update.begin(free_space)) {
      Update.printError(Serial);
    }
    else
    {
      firmwareProgress=0;
      status=ServerStatus::WRITINGFIRMWARE;
      Serial.println("Start flashing...");
    }
    

  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }
  else
  {
    firmwareProgress = ((float)Update.progress()/(float)newFirmwareSize)*100;
    status=ServerStatus::WRITINGFIRMWARE;
  }
  

  if (final) {
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      firmwareProgress=100;
      status=ServerStatus::RESTARTREQUIRED;
      Serial.println("Update complete");
    }
    
  }
}




void WebServer::start()
{
  server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();

      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz","text/html");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
  });

  
  server.on("/live", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();

      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz","text/html");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
  });
  server.on("/network", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/network.html.gz","text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/settings.html.gz","text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/admin", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/admin.html.gz","text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/bootstrap.css.gz","text/css");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });

  server.on("/jumbotron-narrow.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/jumbotron-narrow.css.gz","text/css");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });

  server.on("/favicon-32x32.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon-32x32.png","image/png");
  });

  server.on("/favicon-16x16.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon-16x16.png","image/png");
  });

  server.on("/apple-touch-icon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apple-touch-icon.png","image/png");
  });

  server.on("/Chart.bundle.js", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/Chart.bundle.js.gz","application/javascript");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);  
  });

  server.on("/utils.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/utils.js.gz","application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });

  server.on("/site.webmanifest", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site.webmanifest","application/manifest+json");
  });

  server.on("/getSettings", HTTP_POST, [this](AsyncWebServerRequest *request){
      if(!request->authenticate(_config->http_username, _config->http_password))
          return request->requestAuthentication();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(512);

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
    DynamicJsonDocument root(64);
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
  DynamicJsonDocument root(256);

  root["httpUser"] = _config->http_password;
  root["httpPass"] = _config->http_username;
  root["firmversion"] = FIRMWAREVERSION;

  serializeJson(root,*response);
  request->send(response);

});

 server.on("/getPumpStatus", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(512);

    root["amps"] = _pump->getAmps();
    root["bars"] = _tank->getBars();
    root["maxBars"] = _config->maxBars;
    root["minBars"] = _config->minBars;
    root["pumpStatusValue"] = (int)_pump->getStatus();   

    serializeJson(root,*response);
    request->send(response); 
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
        else if((int)root["wifiMode"]==WifiMode::ACCESSPOINTCLIENT)
        {
          _config->wifiMode = WifiMode::ACCESSPOINTCLIENT;

          strcpy(_config->wifiAp.network.ssid, root["apModeSSID"].as<char*>());
          strcpy(_config->wifiAp.network.key, root["apModePass"].as<char*>());

          _config->wifiClient.dhcpClient= root["clientDhcpMode"].as<bool>();
          strcpy(_config->wifiClient.network.ssid, root["clientModeSSID"].as<char*>());
          strcpy(_config->wifiClient.network.key, root["clientModePass"].as<char*>());
          if(!_config->wifiClient.dhcpClient)
          {
            strcpy(_config->wifiClient.network.ip, root["clientModeIp"].as<char*>());
            strcpy(_config->wifiClient.network.mask, root["clientModeMask"].as<char*>());
            strcpy(_config->wifiClient.network.gateway, root["clientModeGateway"].as<char*>());
            strcpy(_config->wifiClient.network.dns, root["clientModeDns1"].as<char*>());
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
      status=ServerStatus::RESTARTREQUIRED;
  });

  server.on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
  request->send(200, "text/plain", "Success.");
      Serial.println ( "reset from web" );
      _pump->stop(true);
      digitalWrite(2, LOW);
      _eeprom->reset();
      status=ServerStatus::RESTARTREQUIRED;
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
  request->requestAuthentication();
  request->send(200, "text/plain", "Success.");
  });



// handler for the /update form POST (once file upload finishes)
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
      request->send(200);
    }, handle_firmwareUpdate);

  server.onNotFound([](AsyncWebServerRequest *request){
int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });

server.on("/setFirmwareSize", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonDocument root(64);
    deserializeJson(root, (const char*)data);

    if (!root.isNull() && root.containsKey("firmwareSize")) {
         this->newFirmwareSize = root["firmwareSize"];
         this->firmwareProgress = 0;
         request->send(200, "text/plain", "Success.");
    }
    else
    {
      request->send(404, "text/plain", "Error");
    }

  });

  server.begin();

  Serial.println ( "HTTP server started" );
}
