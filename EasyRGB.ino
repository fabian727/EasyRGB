#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPUpdateServer.h>

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print(x)
#else
#define DEBUG_PRINT(x)
#endif

//all you need for LED driving (WS2812 or PWM)
Adafruit_NeoPixel pixels1;
Adafruit_NeoPixel pixels2;
#define NUMPIXELS 1

//following variables without values, will be read of the configuration file
//esp8266-01
uint8_t pin1, //red
        pin2, //green
        pin3; //blue

uint8_t width,
        height;
bool analog = true;
bool ws2812 = false;

//parameters of the HomePage, given to the Output
bool power = false;
uint8_t red = 0,
        green = 0,
        blue = 0;

//all you need for hosting a webserver
char* ssid = nullptr;
char* pwd = nullptr;
char* host = nullptr;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
static const char* update_path = "/update";
static const char* update_username = "admin";
static const char* update_password = "admin";

char ascii2hex(char c)
{
  if ((c >= '0') && (c <= '9'))
    return c - '0' ;
  if ((c >= 'A') && (c <= 'F'))
    return c + 10 - 'A' ;
  if ((c >= 'a') && (c <= 'f'))
    return c + 10 - 'a' ;
  return 0;
}

void loadConfig() {
  String path = "/config.json";
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    StaticJsonBuffer<512> jsonBuffer;

    // Parse the root object
    JsonObject &root = jsonBuffer.parseObject(file);

    if (root.success()) {
      // Copy values from the JsonObject to the Config
      pin1 = root["pin1"];
      pin2 = root["pin2"];
      pin3 = root["pin3"];
      width  = root["width"];
      height = root["height"];

      if (host != nullptr) {
        free(host);
      }
      size_t length = root["host"].as<String>().length();
      host = (char*) malloc(length+1);
      strncpy(host, root["host"], length);
      host[length] = '\0';

      if (ssid != nullptr) {
        free(ssid);
      }
      length = root["ssid"].as<String>().length();
      ssid = (char*) malloc(length+1);
      strncpy(ssid, root["ssid"], length);
      ssid[length] = '\0';

      if (pwd != nullptr) {
        free(pwd);
      }
      length = root["pwd"].as<String>().length();
      pwd = (char*) malloc(length+1);
      strncpy(pwd, root["pwd"], length);
      pwd[length] = '\0';

      DEBUG_PRINT("loaded configuration file with following parameters:\n");
      DEBUG_PRINT("pin1: ");
      DEBUG_PRINT(pin1);
      DEBUG_PRINT("\npin2: ");
      DEBUG_PRINT(pin2);
      DEBUG_PRINT("\npin3: ");
      DEBUG_PRINT(pin3);
      DEBUG_PRINT("\nwidth: ");
      DEBUG_PRINT(width);
      DEBUG_PRINT("\nheight: ");
      DEBUG_PRINT(height);
      DEBUG_PRINT("\nhost: |");
      DEBUG_PRINT(host);
      DEBUG_PRINT("|\nssid: |");
      DEBUG_PRINT(ssid);
      DEBUG_PRINT("|\npwd: |");
      DEBUG_PRINT(pwd);
      DEBUG_PRINT("|\n");
    } else {
      DEBUG_PRINT("failed parsing configuration file\n");
    }
    // Close the file (File's destructor doesn't close the file)
    file.close();
  }
}

void saveConfig() {
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["width"] = width;
  root["height"] = height;
  root["pin1"] = pin1;
  root["pin2"] = pin2;
  root["pin3"] = pin3;
  root["host"] = host;
  root["ssid"] = ssid;
  root["pwd"]  = pwd;

  DEBUG_PRINT("save configuration file with following parameters:\n");
  DEBUG_PRINT("pin1: ");
  DEBUG_PRINT(root["pin1"].as<uint8_t>());
  DEBUG_PRINT("\npin2: ");
  DEBUG_PRINT(root["pin2"].as<uint8_t>());
  DEBUG_PRINT("\npin3: ");
  DEBUG_PRINT(root["pin3"].as<uint8_t>());
  DEBUG_PRINT("\nwidth: ");
  DEBUG_PRINT(root["width"].as<uint8_t>());
  DEBUG_PRINT("\nheight: ");
  DEBUG_PRINT(root["height"].as<uint8_t>());
  DEBUG_PRINT("\nhost: |");
  DEBUG_PRINT(root["host"].as<String>());
  DEBUG_PRINT("|\nssid: |");
  DEBUG_PRINT(root["ssid"].as<String>());
  DEBUG_PRINT("|\npwd: |");
  DEBUG_PRINT(root["pwd"].as<String>());
  DEBUG_PRINT("|\n");

  String path = "/config.json";
  File file = SPIFFS.open(path, "w");
  if (root.printTo(file) == 0) {
    DEBUG_PRINT("Failed to write configuration");
  }
  file.close();
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  else if(filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

bool handleFileRead(String path){
  DEBUG_PRINT("handleFileRead: ");
  DEBUG_PRINT(path);
  if(path.endsWith("/")) {
    path += "index.html";
  } 
  if(!path.endsWith(".json") && !path.endsWith(".html")) {
    path += ".html";
  }
  String contentType = getContentType(path);
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  } else {
    return false;
  }
}

void configPage() {
  String path = "/config.html";
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "FileNotFound");
  }
  if (server.hasArg("pin1")) {
    pin1 = atoi(server.arg("pin1").c_str());
  }
  if (server.hasArg("pin2")) {
    pin2 = atoi(server.arg("pin2").c_str());
  }
  if (server.hasArg("pin3")) {
    pin3 = atoi(server.arg("pin3").c_str());
  }
  if (server.hasArg("width")) {
    width = atoi(server.arg("width").c_str());
  }
  if (server.hasArg("height")) {
    height = atoi(server.arg("height").c_str());
  }
  
  if (server.hasArg("host")) {
    if (host != nullptr) {
      free(host);
    }
    size_t length = server.arg("host").length();
    host = (char*) malloc(length+1);
    strncpy(host, server.arg("host").c_str(), length);
    host[length] = '\0';
  }

  if (server.hasArg("pwd")) {
    if (pwd != nullptr) {
      free(pwd);
    }
    size_t length = server.arg("pwd").length();
    pwd = (char*) malloc(length);
    strncpy(pwd, server.arg("pwd").c_str(), length);
    pwd[length] = '\0';
  }
  
  if (server.hasArg("ssid")) {
    if (ssid != nullptr) {
      free(ssid);
    }
    size_t length = server.arg("ssid").length();
    ssid = (char*) malloc(length);
    strncpy(ssid, server.arg("ssid").c_str(), length);
    ssid[length] = '\0';
  }
  
  saveConfig();
}

void indexPage() {
  //if the request has no args, load the page
  if (server.args() == 0) {
    String path = "/index.html";
    if (SPIFFS.exists(path)) {
      File file = SPIFFS.open(path, "r");
      server.streamFile(file, "text/html");
      file.close();
    } else {
      server.send(404, "text/plain", "FileNotFound");
    }
    //else parse the gotten arguments
  } else {
    if (server.hasArg("power")) {
      String buffer = server.arg("power");
      if (buffer == "on") {
        power = true;
        DEBUG_PRINT("__on__");
      } else {
        power = false;
        DEBUG_PRINT("__off__");
      }
      updateLed();
    }
    if (server.hasArg("colour")) {
      String rgb = server.arg("colour");
      red   = ascii2hex(rgb[0]) << 4 | ascii2hex(rgb[1]);
      green = ascii2hex(rgb[2]) << 4 | ascii2hex(rgb[3]);
      blue  = ascii2hex(rgb[4]) << 4 | ascii2hex(rgb[5]);

      DEBUG_PRINT("__");
      DEBUG_PRINT(server.arg("colour"));
      DEBUG_PRINT("__");
      updateLed();
    }
    server.send(200, "text/plain", "OK");
  }
}

void setupLed() {
  if (analog) {
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);

    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    digitalWrite(pin3, LOW);

    analogWriteRange(255);
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
    analogWrite(pin3, 0);
  }
  if (ws2812) {
    pixels1 = Adafruit_NeoPixel(NUMPIXELS, pin1, NEO_GRBW + NEO_KHZ800);
    pixels2 = Adafruit_NeoPixel(NUMPIXELS, pin2, NEO_GRBW + NEO_KHZ800);
  }
}

void updateLed() {
  if (analog) {
    if (power) {
      analogWrite(pin1, red);
      analogWrite(pin2, green);
      analogWrite(pin3, blue);
    } else {
      analogWrite(pin1, 0);
      analogWrite(pin2, 0);
      analogWrite(pin3, 0);
    }
  }

  if (ws2812) {
    if (power) {
      for (uint8_t i = 0; i < NUMPIXELS; i++) {
        pixels1.setPixelColor(i, red, green, blue, 0);
        pixels2.setPixelColor(i, red, green, blue, 0);
      }
    } else {
      for (uint8_t i = 0; i < NUMPIXELS; i++) {
        pixels1.setPixelColor(i, 0, 0, 0, 0);
        pixels2.setPixelColor(i, red, green, blue, 0);
      }
    }
    pixels1.show();
    pixels2.show();
  }
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.setDebugOutput(true);
#endif

  //SPIFFS INIT
  SPIFFS.begin();
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

  loadConfig();
  setupLed();

  //WIFI INIT
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINT(ssid);
  DEBUG_PRINT("\n");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pwd);

  //30*500ms = 15sec
  uint8_t x = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(x);
    x++;
    if(x>30)
    {
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
/*
  delay(2000);
  MDNS.begin(host);
  DEBUG_PRINT("mDNS started as: |");
  DEBUG_PRINT(host);
  DEBUG_PRINT("|\n");
*/
  //SERVER INIT
  httpUpdater.setup(&server, update_path, update_username, update_password);

  server.on("/", indexPage);
  server.on("/config", configPage);
  server.onNotFound([]() {
    if(!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.begin();
  DEBUG_PRINT("HTTP server started\n");
}

void loop() {
  server.handleClient();
  delay(10);
}
