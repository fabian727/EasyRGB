#include "debug.h"
#include "config.h"
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>

extern ESP8266WebServer server;

char ascii2hex(char c) {
  if ((c >= '0') && (c <= '9'))
  {
    return (c - '0') ;
  }
  else if ((c >= 'A') && (c <= 'F'))
  {
    return (c + 10 - 'A') ;
  }
  else if ((c >= 'a') && (c <= 'f'))
  {
    return (c + 10 - 'a') ;
  }
  else
  {
    return 0;
  }
}

conf::conf() {
	clr.red   = 0;
	clr.green = 0;
	clr.blue  = 0;
	pin.red   = 0;
	pin.green = 0;
	pin.blue  = 0;
	wifiData.ssid = nullptr;
	wifiData.pwd  = nullptr;
	wifiData.host = nullptr;
	load();
}

void conf::load() {
  DEBUG_PRINT("conf::load\n");
	String path = CONFIG_FILE;
  if(SPIFFS.exists(path)) {
    DEBUG_PRINT("conf::load: config file exists\n");
    File file = SPIFFS.open(path, "r");
    StaticJsonBuffer<512> jsonBuffer;

    // Parse the root object
    JsonObject &root = jsonBuffer.parseObject(file);

    if(root.success()) {
      DEBUG_PRINT("conf::load: parsing success\n");
      
      // Copy values from the JsonObject to the Config
      config.pin.red   = root["pin1"].as<uint8_t>();
      config.pin.green = root["pin2"].as<uint8_t>();
      config.pin.blue  = root["pin3"].as<uint8_t>();

      size_t length = root["bootupcolor"].as<String>().length();
      //go for sure, it is a hex html notation
      //else it does not count as colour
      if(length >= 6 && length <= 9)
      {
        DEBUG_PRINT("conf::load: got bootup color\n");
        char* bootupcolor = (char*) malloc(length+1);
        strncpy(bootupcolor, root["bootupcolor"], length);
        bootupcolor[length] = '\0';
        clr.red   = ascii2hex(bootupcolor[0]) << 4 | ascii2hex(bootupcolor[1]);
        clr.green = ascii2hex(bootupcolor[2]) << 4 | ascii2hex(bootupcolor[3]);
        clr.blue  = ascii2hex(bootupcolor[4]) << 4 | ascii2hex(bootupcolor[5]);
      } else {
        DEBUG_PRINT("conf::load: got NO bootup color\n");
  	    clr.red   = 0;
  	    clr.green = 0;
  	    clr.blue  = 0;
      }

      if (wifiData.host != nullptr) {
        free(wifiData.host);
      }
      length = root["host"].as<String>().length();
      wifiData.host = (char*) malloc(length+1);
      strncpy(wifiData.host, root["host"], length);
      wifiData.host[length] = '\0';

      if (wifiData.ssid != nullptr) {
        free(wifiData.ssid);
      }
      length = root["ssid"].as<String>().length();
      wifiData.ssid = (char*) malloc(length+1);
      strncpy(wifiData.ssid, root["ssid"], length);
      wifiData.ssid[length] = '\0';

      if (wifiData.pwd != nullptr) {
        free(wifiData.pwd);
      }
      length = root["pwd"].as<String>().length();
      wifiData.pwd = (char*) malloc(length+1);
      strncpy(wifiData.pwd, root["pwd"], length);
      wifiData.pwd[length] = '\0';

      DEBUG_PRINT("loaded configuration file with following parameters:\n");
      DEBUG_PRINT("pin1: ");
      DEBUG_PRINT(config.pin.red);
      DEBUG_PRINT("\npin2: ");
      DEBUG_PRINT(config.pin.green);
      DEBUG_PRINT("\npin3: ");
      DEBUG_PRINT(config.pin.blue);
      DEBUG_PRINT("\nhost: |");
      DEBUG_PRINT(wifiData.host);
      DEBUG_PRINT("|\nssid: |");
      DEBUG_PRINT(wifiData.ssid);
      DEBUG_PRINT("|\npwd: |");
      DEBUG_PRINT(wifiData.pwd);
      DEBUG_PRINT("|\n");
    } else {
      DEBUG_PRINT("failed parsing configuration file\n");
    }
    // Close the file (File's destructor doesn't close the file)
    file.close();
  }
}

void conf::save(char* bootupcolor) {
  DEBUG_PRINT("conf::save\n");
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["pin1"] = config.pin.red;
  root["pin2"] = config.pin.green;
  root["pin3"] = config.pin.blue;
  root["host"] = config.wifiData.host;
  root["ssid"] = config.wifiData.ssid;
  root["pwd"]  = config.wifiData.pwd;
  root["bootupcolor"] = bootupcolor;

  DEBUG_PRINT("save configuration file with following parameters:\n");
  DEBUG_PRINT("pin1: ");
  DEBUG_PRINT(root["pin1"].as<uint8_t>());
  DEBUG_PRINT("\npin2: ");
  DEBUG_PRINT(root["pin2"].as<uint8_t>());
  DEBUG_PRINT("\npin3: ");
  DEBUG_PRINT(root["pin3"].as<uint8_t>());
  DEBUG_PRINT("\nbootUpColor: |");
  DEBUG_PRINT(root["bootupcolor"].as<String>());
  DEBUG_PRINT("\nhost: |");
  DEBUG_PRINT(root["host"].as<String>());
  DEBUG_PRINT("|\nssid: |");
  DEBUG_PRINT(root["ssid"].as<String>());
  DEBUG_PRINT("|\npwd: |");
  DEBUG_PRINT(root["pwd"].as<String>());
  DEBUG_PRINT("|\n");

  File file = SPIFFS.open(CONFIG_FILE, "w");
  if (root.printTo(file) == 0) {
    DEBUG_PRINT("Failed to write configuration");
  }
  file.close();
}

void configPage() {
  DEBUG_PRINT("configPage\n");
  char* bootupcolor = nullptr;
  if(SPIFFS.exists(CONFIGPAGE_FILE)) {
    File file = SPIFFS.open(CONFIGPAGE_FILE, "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "FileNotFound");
  }



  if (server.hasArg("pin1")) {
	  config.pin.red = atoi(server.arg("pin1").c_str());
  }
  if (server.hasArg("pin2")) {
	  config.pin.green = atoi(server.arg("pin2").c_str());
  }
  if (server.hasArg("pin3")) {
	  config.pin.blue = atoi(server.arg("pin3").c_str());
  }

  if (server.hasArg("bootupcolor")) {
    size_t length = server.arg("bootupcolor").length();
    bootupcolor = (char*) malloc(length+1);
    strncpy(bootupcolor, server.arg("bootupcolor").c_str(), length);
    bootupcolor[length] = '\0';
  }

  if (server.hasArg("host")) {
    if (config.wifiData.host != nullptr) {
      free(config.wifiData.host);
    }
    size_t length = server.arg("host").length();
    config.wifiData.host = (char*) malloc(length+1);
    strncpy(config.wifiData.host, server.arg("host").c_str(), length);
    config.wifiData.host[length] = '\0';
  }

  if (server.hasArg("pwd")) {
    if (config.wifiData.pwd != nullptr) {
      free(config.wifiData.pwd);
    }
    size_t length = server.arg("pwd").length();
    config.wifiData.pwd = (char*) malloc(length);
    strncpy(config.wifiData.pwd, server.arg("pwd").c_str(), length);
    config.wifiData.pwd[length] = '\0';
  }
  
  if (server.hasArg("ssid")) {
    if (config.wifiData.ssid != nullptr) {
      free(config.wifiData.ssid);
    }
    size_t length = server.arg("ssid").length();
    config.wifiData.ssid = (char*) malloc(length);
    strncpy(config.wifiData.ssid, server.arg("ssid").c_str(), length);
    config.wifiData.ssid[length] = '\0';
  }
  
  config.save(bootupcolor);
}

//save the received  new favourit color in the array as json
void saveFavourites() {
  DEBUG_PRINT("saveFavourites\n");
  String path = "/colors.json";
  StaticJsonBuffer<512> jsonBuffer;

  //if not yet existing, create new default
  if (!SPIFFS.exists(path)) {
	JsonObject &root = jsonBuffer.createObject();
	//if the file does not exist, create it with some default colors;
	root["0"] = "0000FF";
	root["1"] = "00FF00";
	root["2"] = "FF0000";
	root["3"] = "FF00FF";
	root["4"] = "FFFF00";
	root["5"] = "00FFFF";
	root["6"] = "FFFFFF";
	root["7"] = "555555";
	root["8"] = "888888";
	root["9"] = "CCCCCC";
	//save the new colors
	File file = SPIFFS.open(path, "w");
	if(file) {
		if(root.printTo(file) == 0) {
		  DEBUG_PRINT("Failed to write default favourite Colors");
		}
	file.close();
	}
  }

  //load the colors
  File file = SPIFFS.open(path, "r");
  JsonObject &root = jsonBuffer.parseObject(file);
  file.close();

  //update the one new color
  root[server.argName(0)] = server.arg(0);

  //save the new colors
  file = SPIFFS.open(path, "w");
  if(file) {
    if(root.printTo(file) == 0) {
      DEBUG_PRINT("Failed to write favourite Colors");
    }
    file.close();
  }

  DEBUG_PRINT(path + " parsed with new color: ");
  DEBUG_PRINT("|"+server.argName(0) +"| |"+server.arg(0)+"|");
}

