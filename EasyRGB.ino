#include "config.h"

#ifdef WIFI
#include <ESP8266WiFi.h>
//WiFi
#ifndef WLAN_SSID
#define WLAN_SSID "YOUR_SSID"
#endif
#ifndef WLAN_PWD
#define WLAN_PWD "YOUR_PWD"
#endif
#endif

#ifdef PWM_LED
#ifndef PIN_R
#define PIN_R 12
#endif
#ifndef PIN_G
#define PIN_G 13
#endif
#ifndef PIN_B
#define PIN_B 14
#endif
#endif

#ifdef mDNS
//mDNS
#include <ESP8266mDNS.h>
#endif
String host = "easyrgb";
String ssid = "................................................................";
String pwd  = "................................................................";

#include <ESP_EEPROM.h>


#ifdef WebServer
#include "webpage.h"
#include <ESP8266WebServer.h>
//WebServer
ESP8266WebServer httpServer(80);

//UpdateServer
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater;
static const char* update_path = "/update";
static const char* update_username = "admin";
static const char* update_password = "admin";
#endif

#ifdef CONFIG
//#include <ArduinoJson.h>
//#include "FS.h"

int width = 30, height = 10, pin1 = 0, pin2 = 5;

/*
 * load Config
 */
bool loadConfig() {
  /*
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  width = atoi(json["width"]);
  height = atoi(json["height"]);
  pin1 = atoi(json["Pin1"]);
  pin2 = atoi(json["Pin2"]);
  host = (const char *) json["ServerName"];
  ssid = (const char *) json["SSID"];
  pwd  = (const char *) json["PWD"];

  saveConfig();
*/
EEPROM.get(0,width);
EEPROM.get(2,height);
EEPROM.get(3,pin1);
EEPROM.get(4,pin2);

//byte 5-7 is empty

//get bytes 8-63 (all inclusive) as host (as it is saved '\0' terminated, the string will do its rest)
for(uint8_t i=0;i<(64-7);i++){
  EEPROM.get(i+8,host[i]);
}
//same from bytes 64-127 for the ssid
for(uint8_t i=0;i<(128-64);i++){
  EEPROM.get(i+64,ssid[i]);
}
//get bytes 8-64 as host (as it is saved '\0' terminated, the string will do its rest)
for(uint8_t i=0;i<(192-64);i++){
  EEPROM.get(i+128,pwd[i]);
}
  return true;
}

/*
 * save Config
 */
bool saveConfig() {

EEPROM.put(0,width);
EEPROM.put(2,height);
EEPROM.put(3,pin1);
EEPROM.put(4,pin2);

//byte 5-7 is empty

//get bytes 8-63 (all inclusive) as host (as it is saved '\0' terminated, the string will do its rest)
for(uint8_t i=0;i<(64-7);i++){
  EEPROM.put(i+8,host[i]);
}
//same from bytes 64-127 for the ssid
for(uint8_t i=0;i<(128-64);i++){
  EEPROM.put(i+64,ssid[i]);
}
//get bytes 8-64 as host (as it is saved '\0' terminated, the string will do its rest)
for(uint8_t i=0;i<(192-64);i++){
  EEPROM.put(i+128,pwd[i]);
}
return EEPROM.commit();
  /*
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["width"] = width;
  json["height"] = height;
  json["Pin1"] = pin1;
  json["Pin2"] = pin2;
  json["ServerName"] = host;
  json["SSID"] = ssid;
  json["PWD"] = pwd;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
  */
}
#endif


void loadDefault() {
  if(host == "") {
    host = "easyrgb";
  }
  if(ssid == "") {
    ssid = WLAN_SSID;
  }
  if(pwd == "") {
    pwd = WLAN_PWD;
  }
}

#ifdef WS2812
#include <Adafruit_NeoPixel.h>
#ifndef NUMPIXELS
#define NUMPIXELS      4
#endif
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, pin1, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, pin2, NEO_GRBW + NEO_KHZ800);
#endif

#ifdef WebServer

/*
 * ascii2hex
 */
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

/*
 * Ereignis Index
 */
void Ereignis_Index() {
  uint8_t r = 0, g = 0, b = 0;
  String Page = "";
  String rgb = String("000000");
  uint8_t red = 0, green = 0, blue = 0, white = 0;
  if (httpServer.hasArg("rgb") && httpServer.arg("rgb").length() == 6) {
    rgb = httpServer.arg("rgb");
    red   = ascii2hex(rgb[0]) << 4 | ascii2hex(rgb[1]);
    green = ascii2hex(rgb[2]) << 4 | ascii2hex(rgb[3]);
    blue  = ascii2hex(rgb[4]) << 4 | ascii2hex(rgb[5]);
  }

  if (httpServer.hasArg("on") && httpServer.arg("on") == "on") {
    r = red;
    g = green;
    b = blue;
#ifdef WS2812
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels1.setPixelColor(i, r, g, b, 0);
    }
#endif
#ifdef PWM_LED
    analogWrite(PIN_R, r);
    analogWrite(PIN_G, g);
    analogWrite(PIN_B, b);
#endif
  }

  if (!httpServer.hasArg("nosite")) {
    Page = PageOne;
    if (httpServer.hasArg("on") && httpServer.arg("on") == "on") {
      snprintf(&rgb[0], 7, "%06x", (r << 16 | g << 8 | b));
      Page += rgb;
      Page += PageTwo;
      Page += rgb;
      Page += PageThree + "checked";
    } else {
      Page += "000000" + PageTwo + PageThree;
    }
    Page += PageFour;
    httpServer.send(200, "text/html", Page);
  }

#ifdef AMBI
  //if it is used as ambi light
  if (httpServer.hasArg("r")) {
    String clr = httpServer.arg("r");
    if (clr.length() % 8 == 0) {
      for (int j = 0, k = 0; j < clr.length(); k++) {
        red   = clr[j++] << 8 | clr[j++];
        green = clr[j++] << 8 | clr[j++];
        blue  = clr[j++] << 8 | clr[j++];
        white = clr[j++] << 8 | clr[j++];
#ifdef WS2812
        pixels1.setPixelColor(k, red, green, blue, white);
#endif
      }
    }
  }
  if (httpServer.hasArg("t")) {
    String clr = httpServer.arg("t");
    if (clr.length() % 8 == 0) {
      for (int j = 0, k = 0; j < clr.length(); k++) {
        red   = clr[j++] << 8 | clr[j++];
        green = clr[j++] << 8 | clr[j++];
        blue  = clr[j++] << 8 | clr[j++];
        white = clr[j++] << 8 | clr[j++];
#ifdef WS2812
        pixels1.setPixelColor(k + height, red, green, blue, white);
#endif
      }
    }
  }
  if (httpServer.hasArg("b")) {
    String clr = httpServer.arg("b");
    if (clr.length() % 8 == 0) {
      for (int j = 0, k = 0; j < clr.length(); k++) {
        red   = clr[j++] << 8 | clr[j++];
        green = clr[j++] << 8 | clr[j++];
        blue  = clr[j++] << 8 | clr[j++];
        white = clr[j++] << 8 | clr[j++];
#ifdef WS2812
        pixels1.setPixelColor(k, red, green, blue, white);
#endif
      }
    }
  }
  if (httpServer.hasArg("l")) {
    String clr = httpServer.arg("l");
    if (clr.length() % 8 == 0) {
      for (int j = 0, k = 0; j < clr.length(); k++) {
        red   = clr[j++] << 8 | clr[j++];
        green = clr[j++] << 8 | clr[j++];
        blue  = clr[j++] << 8 | clr[j++];
        white = clr[j++] << 8 | clr[j++];
#ifdef WS2812
        pixels1.setPixelColor(k + width, red, green, blue, white);
#endif
      }
    }
  }
#endif
#ifdef WS2812
  pixels1.show();
  pixels2.show();
#endif
}

/*
 * Ereignis Config
 */
#ifdef CONFIG
void Ereignis_Config() {
  String PageConfig;
  PageConfig += PageConfig1;
#ifdef WS2812
  PageConfig += PageConfig9;
  PageConfig += height;
  PageConfig += PageConfig2;
  PageConfig += width;
  PageConfig += PageConfig3;
  PageConfig += pin1;
  PageConfig += PageConfig4;
  PageConfig += pin2;
  PageConfig += PageConfig10;
#endif
  PageConfig += PageConfig5;
  PageConfig += host;
  PageConfig += PageConfig7;
  PageConfig += ssid;
  PageConfig += PageConfig8;
  PageConfig += pwd;
  PageConfig += PageConfig6;
  
  httpServer.send(200, "text/html", PageConfig);
  if (httpServer.hasArg("width")) {
    width = atoi(httpServer.arg("width").c_str());
  }
  if (httpServer.hasArg("height")) {
    height = atoi(httpServer.arg("height").c_str());
  }
#ifdef WS2812
  if (httpServer.hasArg("height") || httpServer.hasArg("width")) {
    pixels1.updateLength(height + width);
    pixels2.updateLength(height + width);
  }
  if (httpServer.hasArg("pin1")) {
    pin1 = atoi(httpServer.arg("pin1").c_str());
    pixels1.setPin(pin1);
  }
  if (httpServer.hasArg("pin2")) {
    pin2 = atoi(httpServer.arg("pin2").c_str());
    pixels2.setPin(pin2);
  }
#endif
  if (httpServer.hasArg("ServerName")) {
    host = httpServer.arg("ServerName");
  }
  if (httpServer.hasArg("ssid")) {
    ssid = httpServer.arg("ssid");
  }
  if (httpServer.hasArg("pwd")) {
    pwd = httpServer.arg("pwd");
  }

  if (httpServer.hasArg("ServerName") ||
#ifdef WS2812
      httpServer.hasArg("height") ||
      httpServer.hasArg("width") ||
      httpServer.hasArg("pin1") ||
      httpServer.hasArg("pin2") ||
#endif
      httpServer.hasArg("ssid") ||
      httpServer.hasArg("pwd")) {
    saveConfig();
  }
}
#endif
#endif

/*
 * Setup
 */
void setup() {
  Serial.begin(115200);
#ifdef CONFIG
  EEPROM.begin(256);
/*
  if(!SPIFFS.begin()) {
    Serial.println("could not mount filesystem");      }
*/
  loadConfig();
  loadDefault();
#endif

#ifdef PWM_LED
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  digitalWrite(PIN_R, LOW);
  digitalWrite(PIN_G, LOW);
  digitalWrite(PIN_B, LOW);

  analogWriteRange(255);
  analogWrite(PIN_R, 50);
  analogWrite(PIN_G, 0);
  analogWrite(PIN_B, 0);
#endif

  //WiFi
  Serial.println();
  Serial.println("Up and Running");
#ifdef WIFI
  Serial.print("Connecting to \"");
  Serial.print(ssid);
  Serial.println("\"");
  Serial.print("with pwd: \"");
  Serial.print(pwd);
  Serial.println("\"");
  //    WiFi.setOutputPower(0);
  if(host == "") {
    host = "easyrgb";
  }
  WiFi.hostname(host);
  WiFi.mode(WIFI_STA);

  int status = WL_IDLE_STATUS;
  analogWrite(PIN_G, 50);
  analogWrite(PIN_R, 50);
  WiFi.begin(ssid.c_str(), pwd.c_str());
  for(uint8_t i=0;i<20;i++) {
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if(WiFi.status() == WL_CONNECTED) {
      Serial.print("!");
    }
      break;
    }
  }
  Serial.println("");
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.print("My IP: ");
    Serial.println(WiFi.localIP());
  } else {
    WiFi.softAP("IoT");
    Serial.println("HotSpot created");
    Serial.print("My IP: ");
    Serial.println(WiFi.softAPIP());
  }

  // Print the IP address
#endif
  //UpdateServer
#ifdef WebServer
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);

  //WebServer
  httpServer.on("/", Ereignis_Index);
#ifdef CONFIG
  httpServer.on("/config", Ereignis_Config);
#endif
  httpServer.begin();
#endif

  //LED strip init black
#ifdef WS2812
  pinMode(pin1, OUTPUT);
  pixels1.begin();
  pixels1.setPixelColor(0, 0, 100, 0, 0);
  pixels1.show();

  pinMode(pin2, OUTPUT);
  pixels2.begin();
  pixels2.show();
#endif

  //MDNS
#ifdef mDNS
  MDNS.begin((const char*) &host);
#ifdef WebServer
  MDNS.addService("http", "tcp", 80);
#endif
#endif
  analogWrite(PIN_R, 0);
  analogWrite(PIN_G, 50);
  analogWrite(PIN_B, 0);
}

void loop() {
#ifdef WebServer
  httpServer.handleClient();
#endif
}


