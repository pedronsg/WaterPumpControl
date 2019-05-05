#include "webServer.h"
#include <sstream>
#include "FS.h"
#include "ArduinoJson.h"
#include <WebAuthentication.h>

WebServer::WebServer()
{
  session_key=millis();
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

void WebServer::redirectToAuthentication(AsyncWebServerRequest &request)
{
  session_key=millis();
  AsyncWebServerResponse *response = request.beginResponse(SPIFFS, "/login.html.gz","text/html");
  response->addHeader("Content-Encoding", "gzip");
  response->addHeader("Cache-Control"," no-cache");
  request.send(response);
}

String WebServer::uint64ToString(uint64_t input) {
  String result = "";
  uint8_t base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c +='0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

bool WebServer::authenticated(AsyncWebServerRequest *request) {
  if (request->hasHeader("Cookie")){
    String cookie = request->header("Cookie");
    String name = "PUMPCONTROL=" + uint64ToString(session_key);
    if (cookie.indexOf(name) != -1) {
      return (true);
    }
  }

  AsyncWebServerResponse *response = request->beginResponse(301); 
  response->addHeader("Location","/login");
  response->addHeader("Cache-Control"," no-cache");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
  return (false);
}


void WebServer::start()
{
  server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
  if(!authenticated(request)) redirectToAuthentication(*request);
    else
    {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz","text/html");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }
  });


  server.on("/login", HTTP_POST, [this](AsyncWebServerRequest *request){
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    
    DynamicJsonDocument root(512);
    deserializeJson(root, (const char*)data);

    if (!root.isNull() && root.containsKey("clientId") && root.containsKey("clientSecret")
    && root["clientId"]==_config->http_username && root["clientSecret"]==_config->http_password)
    {
      AsyncWebServerResponse *response = request->beginResponse(200); 
      session_key = millis();
      response->addHeader("Cache-Control"," no-cache");
      response->addHeader("Set-Cookie", "PUMPCONTROL="+ uint64ToString(session_key));
      request->send(response);
    }
    else
    {
      request->send(401, "text/plain", "Authentication Error!");
      Serial.println("auth error");
    }
  });



  
  server.on("/live", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!authenticated(request)) redirectToAuthentication(*request);
    else
    {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz","text/html");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }
  });

  server.on("/network", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!authenticated(request)) redirectToAuthentication(*request);
    else
    {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/network.html.gz","text/html");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }
  });

  server.on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(!authenticated(request)) redirectToAuthentication(*request);
    else
    {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/settings.html.gz","text/html");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);  
    }
  });

  server.on("/admin", HTTP_GET, [this](AsyncWebServerRequest *request){
   if(!authenticated(request)) redirectToAuthentication(*request);
   else
   {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/admin.html.gz","text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
   }
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

  server.on("/digestAuthRequest.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/digestAuthRequest.js.gz","application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });


  server.on("/getSettings", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!authenticated(request)) redirectToAuthentication(*request);
    else
    {
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
    }
  });

  server.on("/setStatus", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!authenticated(request)) return redirectToAuthentication(*request);
  //  if(!request->authenticate(_config->http_username, _config->http_password,"pump",false))
  //      return redirectToAuthentication(*request);
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
    if(!authenticated(request)) return redirectToAuthentication(*request);
  //  if(!request->authenticate(_config->http_username, _config->http_password,"pump",false))
   //     return redirectToAuthentication(*request);
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
    if(!authenticated(request)) redirectToAuthentication(*request);
    else
    {
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
    }
});


server.on("/getAdmin", HTTP_POST, [this](AsyncWebServerRequest *request){
  if(!authenticated(request)) redirectToAuthentication(*request);
  else
  {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(256);

    root["httpUser"] = _config->http_password;
    root["httpPass"] = _config->http_username;
    root["firmversion"] = FIRMWAREVERSION;

    serializeJson(root,*response);
    request->send(response);
  }
});

 server.on("/getPumpStatus", HTTP_POST, [this](AsyncWebServerRequest *request){
   if(!authenticated(request)) redirectToAuthentication(*request);
   else
   {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      DynamicJsonDocument root(512);

      root["amps"] = _pump->getAmps();
      root["bars"] = _tank->getBars();
      root["maxBars"] = _config->maxBars;
      root["minBars"] = _config->minBars;
      root["pumpStatusValue"] = (int)_pump->getStatus();   

      serializeJson(root,*response);
      request->send(response); 
   }
  });


  server.on("/setNetwork", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!authenticated(request)) return redirectToAuthentication(*request);
   // if(!request->authenticate(_config->http_username, _config->http_password,"pump",false))
   //     return redirectToAuthentication(*request);
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
    if(!authenticated(request)) redirectToAuthentication(*request);
    else
    {
      request->send(200, "text/plain", "Success.");
      Serial.println ( "reboot from web" );
      _pump->stop(true);
      digitalWrite(2, LOW);
      status=ServerStatus::RESTARTREQUIRED;
    }
  });

  server.on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!authenticated(request)) return redirectToAuthentication(*request);
   // if(!request->authenticate(_config->http_username, _config->http_password,"pump",false))
    //      return redirectToAuthentication(*request);
      request->send(200, "text/plain", "Success.");
      Serial.println ( "reset from web" );
      _pump->stop(true);
      digitalWrite(2, LOW);
      _eeprom->reset();
      status=ServerStatus::RESTARTREQUIRED;
  });

  server.on("/logout", HTTP_GET, [this](AsyncWebServerRequest *request){
    
    redirectToAuthentication(*request);
  //request->requestAuthentication();
  //request->send(200, "text/plain", "Success.");
  //request->send(401);
  });

server.on("/login", HTTP_GET, [this](AsyncWebServerRequest *request){
  return redirectToAuthentication(*request);
  });

// handler for the /update form POST (once file upload finishes)
  server.on("/upload", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!authenticated(request)) return redirectToAuthentication(*request);
      request->send(200);
    }, handle_firmwareUpdate);


server.onNotFound([](AsyncWebServerRequest *request) {
	request->send(404);
});

server.on("/setFirmwareSize", HTTP_POST, [this](AsyncWebServerRequest *request){
  if(!authenticated(request)) return redirectToAuthentication(*request);
 //   if(!request->authenticate(_config->http_username, _config->http_password))
  //      return redirectToAuthentication(*request);//request->requestAuthentication();
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
