#include "debug.h"
#include "setup.h"
//#include "config.h"
#include "filesystem.h"

#include <FS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

extern ESP8266WebServer server;
extern ESP8266HTTPUpdateServer httpUpdater;

static const char* update_path = "/update";
static const char* update_username = "admin";
static const char* update_password = "admin";



//connect to given SSID with PWD as HOST (name). On Failure (after 15s), create own network "IoT" with no pwd
void connectWiFi(wifidata wifiData) {
  DEBUG_PRINT("connectWifi\n");
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINT(wifiData.ssid);
  DEBUG_PRINT("\n");
  WiFi.mode(WIFI_STA);
  WiFi.hostname(wifiData.host);
  WiFi.begin(wifiData.ssid, wifiData.pwd);

  //30*500ms = 15sec
  uint8_t x = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(x);
    x++;
    if(x > 30) {
      break;
    }
 }

  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINT("\n");
    DEBUG_PRINT("Connected! IP address: ");
    DEBUG_PRINT(WiFi.localIP());
    DEBUG_PRINT("\n");
  } else {
    WiFi.disconnect(true);

    //open a HotSpot with the given name and without a password
    //ssid,pwd,channel,hidden
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("IoT", NULL, 8, false);
    DEBUG_PRINT("HotSpot created");
    DEBUG_PRINT("\n");
    DEBUG_PRINT("My IP: ");
    DEBUG_PRINT(WiFi.softAPIP());
    DEBUG_PRINT("\n");
  }
}

//set output and PULL-UP-Resistors
void setOutPutPins(color pin) {
  DEBUG_PRINT("setOutPutPins\n");

	pinMode(pin.red, OUTPUT);
	pinMode(pin.green, OUTPUT);
	pinMode(pin.blue, OUTPUT);

	digitalWrite(pin.red, LOW);
	digitalWrite(pin.green, LOW);
	digitalWrite(pin.blue, LOW);

	analogWriteRange(255);
  
	analogWrite(pin.red, 0);
	analogWrite(pin.green, 0);
	analogWrite(pin.blue, 0);
}

void serverInit() {
    DEBUG_PRINT("serverInit\n");

httpUpdater.setup(&server, update_path, update_username, update_password);

  server.on("/", indexPage);
  server.on("/config", configPage);
  server.on("/colors", saveFavourites);

  //FileHandling

  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);

  server.onNotFound([]() {
    if(!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.begin();
  DEBUG_PRINT("HTTP server started\n");
}

void spiffsInit() {
  DEBUG_PRINT("spifssInit\n");
  SPIFFS.begin();
  #ifdef DEBUG TRUE
  DEBUG_PRINT("\n");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      DEBUG_PRINT("FS File: ");
      DEBUG_PRINT(fileName.c_str());
      DEBUG_PRINT("\n");
    }
    DEBUG_PRINT("\n");
  }
  #endif
}
