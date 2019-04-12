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
  //starting http server
  //filling html, bootstrap and css
//  css = loadFile("jumbotron-narrow.css");
//  bootstrap = loadFile("bootstrap.css");
//  html = loadFile("index.html");
//  server.on ("/", std::bind(&WebServer::handleRoot, this) );

//server.serveStatic("/index.html", SPIFFS, "/www/");
//server.serveStatic("/", SPIFFS, "/www/index.html");
//server.serveStatic("/control/", SPIFFS, "/www/control.html");
  //


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
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root["maxBars"] = _config->maxBars;
    root["minBars"] = _config->minBars;
    root["maxAmps"] = _config->maxAmps;
    root["minAmps"] = _config->minAmps;
    root["maxRunTime"] = _config->maxRunningtime;
    root["noWater"] = _config->noWaterTime;
    root["unprotectedDelay"] = _config->unprotectedStartDelay;

    //root["ssid"] = WiFi.SSID();
    root.printTo(*response);
    request->send(response);

  });


  server.on("/setStatus", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    
    if (root.success()) {
      if (root.containsKey("startStop")) {
        if(root["startStop"])
        {
          _pump->start();
        }
        else
        {
          _pump->stop();
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
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (root.success()) {
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
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();


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



  //root["ssid"] = WiFi.SSID();
  root.printTo(*response);
  request->send(response);

});


server.on("/getAdmin", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["httpUser"] = _config->http_password;
  root["httpPass"] = _config->http_username;

  root.printTo(*response);
  request->send(response);

});


 server.on("/getPumpStatus", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);

    if (root.success()) {
      if (root.containsKey("numberOfItems")) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        DynamicJsonBuffer jsonBuffer;
        int nitens=root["numberOfItems"];
        JsonObject &root2 = jsonBuffer.createObject();

        root2["pumpStatus"] = (int)_pump->getStatus();
        JsonArray& array = root2.createNestedArray("values");

        for (int i=0; i<nitens; i++)
        {
          JsonObject& object = array.createNestedObject();
          object["amps"] = _pump->getAmps();
          object["bars"] = _tank->getBars();
          object["maxBars"] = _config->maxBars;
          object["minBars"] = _config->minBars;
        }       

        root2.printTo(*response);
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


  server.on("/startStop", HTTP_POST, [this](AsyncWebServerRequest *request){
    if(!request->authenticate(_config->http_username, _config->http_password))
        return request->requestAuthentication();
  }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (root.success()) {
      if (root.containsKey("startStop")) {
        if(root["startStop"]==0)
        {
          _config->wifiMode = ACCESSPOINT;
          strcpy(_config->wifiAp.network.ssid, root["apModeSSID"].as<char*>());
          strcpy(_config->wifiAp.network.key, root["apModePass"].as<char*>());
          _wifi->start();
        }

        AsyncResponseStream *response = request->beginResponseStream("application/json");
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();

        root["startStop"] = _config->http_password;
        root.printTo(*response);
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
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (root.success()) {
      if (root.containsKey("wifiMode")) {
        if(root["wifiMode"]==0)
        {
          _config->wifiMode = ACCESSPOINT;
          strcpy(_config->wifiAp.network.ssid, root["apModeSSID"].as<char*>());
          strcpy(_config->wifiAp.network.key, root["apModePass"].as<char*>());
          _wifi->start();
        }
        else if(root["wifiMode"]==1)
        {
          //_wifi->disconnect();
          _config->wifiMode = CLIENT;
          _config->wifiClient.dhcpClient= root["clientDhcpMode"].as<bool>();
           Serial.println("Client mode");
           Serial.print("DHCP mode="); Serial.println(_config->wifiClient.dhcpClient);
          //
           Serial.println(root["clientModeSSID"].as<char*>());
           Serial.println(root["clientModePass"].as<char*>());
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
          _wifi->start();
        }

        if(_eeprom->save(*_config))
        {
          request->send(200, "text/plain", "Success.");
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


  server.on("/cmdReboot", HTTP_POST, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", "Success.");
      Serial.println ( "reboot web" );
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









//
// bool loadFromSpiffs(String path){
//   String dataType = "text/plain";
//   if(path.endsWith("/")) path += "index.htm";
//
//   if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
//   else if(path.endsWith(".htm")) dataType = "text/html";
//   else if(path.endsWith(".css")) dataType = "text/css";
//   else if(path.endsWith(".js")) dataType = "application/javascript";
//   else if(path.endsWith(".png")) dataType = "image/png";
//   else if(path.endsWith(".gif")) dataType = "image/gif";
//   else if(path.endsWith(".jpg")) dataType = "image/jpeg";
//   else if(path.endsWith(".ico")) dataType = "image/x-icon";
//   else if(path.endsWith(".xml")) dataType = "text/xml";
//   else if(path.endsWith(".pdf")) dataType = "application/pdf";
//   else if(path.endsWith(".zip")) dataType = "application/zip";
//   File dataFile = SPIFFS.open(path.c_str(), "r");
//   if (server.hasArg("download")) dataType = "application/octet-stream";
//   if (server.streamFile(dataFile, dataType) != dataFile.size()) {
//   }
//
//   dataFile.close();
//   return true;
// }
