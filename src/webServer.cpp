//#define DEBUG_ENABLED
#include "webServer.h"
#include <sstream>
#include <FS.h>
#include <ArduinoJson.h>
#include <WebAuthentication.h>
#include <uptime.h>
#include "config.h"

ConfigData *_config;


WebServer::WebServer()
{
  _config= CConfig::GetInstance();
}


void WebServer::init(Eeprom &eeprom, CWIFI &wifiPump, Pump &pump, Tank &tank)
{
  _eeprom = &eeprom;
  _wifi = &wifiPump;
  _pump = &pump;
  _tank = &tank;
}

void setMQTT(JsonDocument &root)
{
  _config->useMQTT = root["mqttCheck"].as<bool>();
  if(_config->useMQTT)
  {
    if(root["mqttAddress"].as<char*>() == NULL)
    {
      strcpy(_config->mqttClient.address, "192.168.1.1");
    }
    else
    {
      strcpy(_config->mqttClient.address, root["mqttAddress"].as<char*>());
    }
    _config->mqttClient.port= root["mqttPort"].as<int>();
    if(root["mqttUser"].as<char*>() == NULL)
    {
      strcpy(_config->mqttClient.user, "");
    }
    else
    {
      strcpy(_config->mqttClient.user, root["mqttUser"].as<char*>());
    }
    if(root["mqttPass"].as<char*>() == NULL)
    {
      strcpy(_config->mqttClient.pass, "");
    }
    else
    {
      strcpy(_config->mqttClient.pass, root["mqttPass"].as<char*>()); 
    }
  }

  #ifdef DEBUG_ENABLED 
    Serial1.print("MQTT Enabled="); Serial1.println(_config->useMQTT);
    Serial1.print("MQTT address="); Serial1.println(_config->mqttClient.address);
    Serial1.print("MQTT port="); Serial1.println(_config->mqttClient.port);
    Serial1.print("MQTT User="); Serial1.println(_config->mqttClient.user);
    Serial1.print("MQTT Pass="); Serial1.println(_config->mqttClient.pass);
  #endif
};

void setAPmode(JsonDocument &root)
{
    #ifdef DEBUG_ENABLED 
    Serial1.println("Changed to AP mode");
    #endif
    _config->wifiMode = WifiMode::ACCESSPOINT;
    if(root["apModePass"].as<char*>() == NULL)
    {
      strcpy(_config->wifiAp.network.key, "");
    }
    else
    {
      strcpy(_config->wifiAp.network.key, root["apModePass"].as<char*>());
    }
    strcpy(_config->wifiAp.network.ssid, root["apModeSSID"].as<char*>());
    #ifdef DEBUG_ENABLED 
    Serial1.printf("SSID:%s\n",_config->wifiAp.network.ssid);
    Serial1.printf("WifiKey:%s\n",_config->wifiAp.network.key);
    #endif
};

void setClientmode(JsonDocument &root)
{
  #ifdef DEBUG_ENABLED 
  Serial1.println("Changed to Client mode");
  Serial1.printf("dhcpClient:%i\n",root["clientDhcpMode"].as<bool>());
  Serial1.printf("SSID:%s\n",root["clientModeSSID"].as<char*>());
  Serial1.printf("key:%s\n",root["clientModePass"].as<char*>());
  #endif
  _config->wifiMode = WifiMode::CLIENT;
  _config->wifiClient.dhcpClient= root["clientDhcpMode"].as<bool>();
  strcpy(_config->wifiClient.network.ssid, root["clientModeSSID"].as<char*>());
  strcpy(_config->wifiClient.network.key, root["clientModePass"].as<char*>());
  if(!_config->wifiClient.dhcpClient)
  {
    strcpy(_config->wifiClient.network.ip, root["clientModeIp"].as<char*>());
    strcpy(_config->wifiClient.network.mask, root["clientModeMask"].as<char*>());
    strcpy(_config->wifiClient.network.gateway, root["clientModeGateway"].as<char*>());
    strcpy(_config->wifiClient.network.dns, root["clientModeDns1"].as<char*>());
    #ifdef DEBUG_ENABLED 
    Serial1.println(_config->wifiClient.network.ip);
    #endif
  }
}

void setAPClientmode(JsonDocument &root)
{
    #ifdef DEBUG_ENABLED 
    Serial1.println("Changed to AP/Client mode");
    #endif
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

void WebServer::handle_firmwareUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

 if(status==ServerStatus::RESTARTREQUIRED) return;

  if (!index)
  {
    Update.runAsync(true);
    if (!Update.begin(free_space)) {
      Update.printError(Serial1);
    }
    else
    {
      firmwareProgress=0;
      status=ServerStatus::WRITINGFIRMWARE;
      #ifdef DEBUG_ENABLED 
      Serial1.println("Start flashing...");
      #endif
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial1);
  }
  else
  {
    firmwareProgress = ((float)Update.progress()/(float)newFirmwareSize)*100;
    status=ServerStatus::WRITINGFIRMWARE;
  }
  

  if (final) {
    if (!Update.end(true)){
      #ifdef DEBUG_ENABLED 
      Update.printError(Serial1);
      #endif
    } else {
      firmwareProgress=100;
      status=ServerStatus::RESTARTREQUIRED;
      #ifdef DEBUG_ENABLED 
      Serial1.println("Update complete");
      #endif
    }
    
  }
}

void WebServer::start()
{
  server.onNotFound([](AsyncWebServerRequest *request) {
	  request->send(404);
  });

  server.on("/login", HTTP_POST, [this](AsyncWebServerRequest *request){
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    
    static String s;

    if(index==0) s="";
    for(size_t i=0; i<len;i++)
      s = s + String((char)data[i]);
    if(s.length()<total) 
      return;
      #ifdef DEBUG_ENABLED 
    Serial1.println(String("data=") + s.c_str());  
    #endif

    DynamicJsonDocument root(512);
    deserializeJson(root, s.c_str());

    if (!root.isNull() && root.containsKey("clientId") && root.containsKey("clientSecret")
    && root["clientId"]==_config->http_username && root["clientSecret"]==_config->http_password)
    {
      AsyncWebServerResponse *response = request->beginResponse(200); 
      response->addHeader("Cache-Control"," no-cache");
      request->send(response);
    }
    else
    {
      request->send(401, "text/plain", "Authentication Error!");
      #ifdef DEBUG_ENABLED 
      Serial1.println("auth error");
      #endif
    }
  });


#pragma region HTTP_FILES_REGION

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

  server.on("/bootstrap4.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/bootstrap4.css.gz","text/css");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });
  server.on("/bootstrap.min.css.map", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/bootstrap.min.css.map.gz","text/css");
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

  server.on("/digestAuthRequest.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/digestAuthRequest.js.gz","application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });

 server.on("/spinner.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/spinner.js.gz","application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });

  server.on("/spinner.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/spinner.css.gz","text/css");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });

#pragma endregion HTTP_FILES_REGION

#pragma region GET_REGION

  server.on("/getNetwork", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
      return request->requestAuthentication();

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(1024);

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
    root["mqttCheck"] = _config->useMQTT;
    root["mqttAddress"] = _config->mqttClient.address;
    root["mqttPort"] = _config->mqttClient.port;
    root["mqttUser"] = _config->mqttClient.user;
    root["mqttPass"] = _config->mqttClient.pass;

    serializeJson(root,*response);
    request->send(response);
});

server.on("/getAdmin", HTTP_POST, [this](AsyncWebServerRequest *request){
  if(!request->authenticate(_config->http_username, _config->http_password))
    return request->requestAuthentication();
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument root(256);

  root["httpUser"] = _config->http_username;
  root["httpPass"] = _config->http_password;
  root["firmversion"] = FIRMWAREVERSION;

  serializeJson(root,*response);
  request->send(response);
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


 server.on("/getPumpStatus", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
      return request->requestAuthentication();

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(700);

    root["amps"] = _pump->getAmps();
    root["bars"] = _tank->getBars();
    root["maxBars"] = _config->maxBars;
    root["minBars"] = _config->minBars;
    root["pumpStatusValue"] = (int)_pump->getStatus();
    root["tankStatusValue"] = (int)_tank->getStatus(); 
    uptime::calculateUptime();
    root["upTimeD"] = (short)uptime::getDays();
    root["upTimeH"] = (unsigned char)uptime::getHours();
    root["upTimeM"] = (unsigned char)uptime::getMinutes();

    serializeJson(root,*response);
    request->send(response); 
  });

#pragma endregion GET_REGION

#pragma region SET_REGION

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

  static String s;

  if(index==0) s="";
  for(size_t i=0; i<len;i++)
    s = s + String((char)data[i]);
  if(s.length()<total) 
    return;
  #ifdef DEBUG_ENABLED 
  Serial1.println(String("data=") + s.c_str());  
  #endif

  DynamicJsonDocument root(512);
  deserializeJson(root, s.c_str());

  if (!root.isNull()) {
    if (root.containsKey("maxBars")) {
      _config->maxBars = root["maxBars"].as<float>();
    }
    if (root.containsKey("minBars")) {
      _config->minBars = root["minBars"].as<float>();
    }
    if (root.containsKey("maxAmps")) {
      _config->maxAmps = root["maxAmps"].as<float>();
    }
    if (root.containsKey("minAmps")) {
      _config->minAmps = root["minAmps"].as<float>();
    }
    if (root.containsKey("maxRunTime")) {
      _config->maxRunningtime = root["maxRunTime"].as<uint8_t>();
    }
    if (root.containsKey("noWater")) {
      _config->noWaterTime = root["noWater"].as<uint8_t>();
    }
    if (root.containsKey("unprotectedDelay")) {
      _config->unprotectedStartDelay = root["unprotectedDelay"].as<uint16_t>();
    }
    
    _eeprom->save();
    request->send(200, "text/plain", "Success.");

    } else {
      request->send(404, "text/plain", "Error");
    }
    
  });


 server.on("/setAdmin", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
      return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

 static String s;

  if(index==0) s="";
  for(size_t i=0; i<len;i++)
    s = s + String((char)data[i]);
  if(s.length()<total) 
    return;
    #ifdef DEBUG_ENABLED 
  Serial1.println(String("data=") + s.c_str());  
  #endif

  DynamicJsonDocument root(512);
  deserializeJson(root, s.c_str());

  if (!root.isNull()) {
    if (root.containsKey("httpUser")) {
      strcpy(_config->http_username , root["httpUser"].as<char*>());
    }
    if (root.containsKey("httpPass")) {
        strcpy(_config->http_password, root["httpPass"].as<char*>());
    }
    
    _eeprom->save();
    request->send(200, "text/plain", "Success.");

    } else {
      request->send(404, "text/plain", "Error");
    }
    
  });

  server.on("/setNetwork", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
      return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

    static String s;
    if(index==0) s="";
    for(size_t i=0; i<len;i++)
      s = s + String((char)data[i]);
    if(s.length()<total) 
      return;
      #ifdef DEBUG_ENABLED 
    Serial1.println(String("data=") + s.c_str());  
    #endif

    DynamicJsonDocument root(1264);
    deserializeJson(root, s.c_str());
    
    #ifdef DEBUG_ENABLED 
    Serial1.println("Requesting network mode change...");
    #endif

    if (!root.isNull()) {
      if (root.containsKey("wifiMode")) {

        if((int)root["wifiMode"]==WifiMode::ACCESSPOINT)
        {
          setAPmode(root);
          setMQTT(root);
        }
        else if((int)root["wifiMode"]==WifiMode::CLIENT)
        {
          setClientmode(root);
          setMQTT(root);
        }
        else if((int)root["wifiMode"]==WifiMode::ACCESSPOINTCLIENT)
        {
          setAPClientmode(root);
          setMQTT(root);
        }
        else if((int)root["wifiMode"]==WifiMode::DISABLED) 
        {
          #ifdef DEBUG_ENABLED 
          Serial1.println("Wifi Disabled");
          #endif
          _config->wifiMode = DISABLED;
        }

          _eeprom->save();
          #ifdef DEBUG_ENABLED 
          Serial1.println("Changes Saved to EEprom");
          #endif
          request->send(200, "text/plain", "Success.");
          _wifi->start();         
       }
    } else {
      #ifdef DEBUG_ENABLED 
      Serial1.println("Unknwon error setting Network values");
      #endif
      request->send(404, "text/plain", "Error");
    }
  });

server.on("/setFirmwareSize", HTTP_POST, [this](AsyncWebServerRequest *request){
  if(!request->authenticate(_config->http_username, _config->http_password))
    return request->requestAuthentication();
}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

  static String s;
  if(index==0) s="";
  for(size_t i=0; i<len;i++)
    s = s + String((char)data[i]);
  if(s.length()<total) 
    return;
  #ifdef DEBUG_ENABLED 
    Serial1.println(String("data=") + s.c_str());  
  #endif
  DynamicJsonDocument root(64);
  deserializeJson(root,s.c_str());

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

#pragma endregion SET_REGION

#pragma region COMMANDS_REGION
  server.on("/reboot", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
      return request->requestAuthentication();
   
    request->send(200, "text/plain", "Success.");
    #ifdef DEBUG_ENABLED 
      Serial1.println ( "reboot from web" );
    #endif
    _pump->stop(true);
    digitalWrite(2, LOW);
    status=ServerStatus::RESTARTREQUIRED;

  });

  server.on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
      return request->requestAuthentication();

    request->send(200, "text/plain", "Success.");
    #ifdef DEBUG_ENABLED 
    Serial1.println ( "reset from web" );
    #endif
    _pump->stop(true);
    digitalWrite(2, LOW);
    _eeprom->reset();
    status=ServerStatus::RESTARTREQUIRED;
  });

   server.on("/logout", HTTP_GET, [this](AsyncWebServerRequest *request){
    request->requestAuthentication();
  });

// server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
//   request->send(401);
// });


server.on("/login", HTTP_GET, [this](AsyncWebServerRequest *request){
  return request->requestAuthentication();
});

// handler for the /update form POST (once file upload finishes)
server.on("/upload", HTTP_POST, [this](AsyncWebServerRequest *request){
  if(!request->authenticate(_config->http_username, _config->http_password))
    return request->requestAuthentication();
  request->send(200);
}, handle_firmwareUpdate);

#pragma endregion COMMANDS_REGION

  server.begin();
  #ifdef DEBUG_ENABLED 
  Serial1.println ( "HTTP server started" );
  #endif
}
