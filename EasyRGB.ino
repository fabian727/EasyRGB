#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>

const char* host = "EasyRGB";
const char* update_path = "/update";
const char* update_username = "admin";
const char* update_password = "admin";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//RGB LEDs
#define PIN_R 2
#define PIN_G 3
#define PIN_B 0
//Tx = GPIO 1
//Rx = GPIO 3

const String PageOne = "<!DOCTYPE html>\
<html>\
<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\
<meta charset=\"UTF-8\">\
<head>\
  <title>Web RGB</title>\
<style>\
body {\
  background-color: black;\
  margin: 0px;\
  padding: 0px;\
  font-size: 30px;\
}\
form {\
  display: none;\
}\
canvas {\
  width: 100%;\
  max-width: 500px;\
  margin-left: auto;\
  margin-right: auto;\
  display: block;\
}\
#inner {\
  position: absolute;\
  left: 50vw;\
  top: calc(250px - 65px);\
  height: 26vw;\
  width: 26vw;\
  transform:translateX(-50%);\
  max-height: 130px;\
  max-width: 130px;\
  border-radius: 100%;\
  background-color: #838485;\
  border: 5px solid black;\
  cursor: pointer;\
}\
@media screen and (max-width: 500px) {\
  #inner {\
    top: 36vw;    /*works until max size is reached*/\
  }\
}\
#inner > p {\
  text-align: center;\
  padding-top: calc(50% - 20px);\
  margin: 0;\
  text-shadow: 0px 0px 5px #";
const String PageTwo=";}\
\
</style>\
<script>\
function power() {\
  if(document.getElementById(\"power\").checked == false) {\
    document.getElementById(\"inner\").children[0].style.textShadow = \"0px 0px 5px #";
    
const String PageThree= " \";\
  } else {\
    document.getElementById(\"inner\").children[0].style.textShadow = \"unset\";\
  }\
  document.getElementById(\"power\").checked = !document.getElementById(\"power\").checked;\
  document.getElementById(\"Form\").submit();\
}\
function canvas_create() {\
  var canvas = document.getElementById(\"picker\");\
  var width = canvas.offsetWidth;\
  canvas.width = width;\
  canvas.height = width;\
  var context = canvas.getContext(\"2d\");\
  var x = width/2;\
  var radius = width/3;\
  var counterClockwise = false;\
\
  for(var angle=0; angle<=360; angle+=1){\
    var startAngle = (angle-2)*Math.PI/180;\
    var endAngle = angle * Math.PI/180;\
    context.beginPath();\
    context.moveTo(x, x);\
    context.arc(x, x, radius, startAngle, endAngle, counterClockwise);\
    context.closePath();\
    context.fillStyle = 'hsl('+angle+', 100%, 50%)';\
    context.fill();\
  }\
  \
  var gradient = context.createRadialGradient(x, x, x/3.5, x, x, x/2);\
  gradient.addColorStop(0, 'white');\
  gradient.addColorStop(1, 'rgba(255,255,255,0)');\
  context.fillStyle = gradient;\
  context.fillRect(0, 0, width, width);\
  gradient = context.createRadialGradient(x, x, x/2.1, x, x, x/1.4);\
  gradient.addColorStop(0, 'rgba(0,0,0,0)');\
  gradient.addColorStop(1, 'black');\
  context.fillStyle = gradient;\
  context.fillRect(0, 0, width, width);\
  document.getElementById(\"picker\").addEventListener(\"click\",function(event){\
    var eventLocation = getEventLocation(this,event);\
    var context = this.getContext('2d');\
    var pixelData = context.getImageData(eventLocation.x, eventLocation.y, 1, 1).data; \
    var hex = (\"000000\" + rgbToHex(pixelData[0], pixelData[1], pixelData[2])).slice(-6);\
    document.getElementById(\"rgb\").value = hex;\
    document.getElementById(\"Form\").submit();\
  },false);\
}\
function getElementPosition(obj) {\
  var curleft = 0, curtop = 0;\
  if (obj.offsetParent) {\
    do {\
      curleft += obj.offsetLeft;\
      curtop += obj.offsetTop;\
    } while (obj = obj.offsetParent);\
    return { x: curleft, y: curtop };\
  }\
  return undefined;\
}\
function getEventLocation(element,event){\
    var pos = getElementPosition(element);\
    return {\
        x: (event.pageX - pos.x),\
        y: (event.pageY - pos.y)\
    };\
}\
function rgbToHex(r, g, b) {\
  if (r > 255 || g > 255 || b > 255)\
    throw \"Invalid color component\";\
  return ((r << 16) | (g << 8) | b).toString(16);\
}\
</script>\
</head>\
<body>\
  <form id=\"Form\" action=\"\" method=\"GET\">\
    <input id=\"rgb\" type=\"text\" name=\"rgb\" value=\"\"></input>\
    <input id=\"power\" type=\"checkbox\" name=\"on\"";

const String PageFour="></input>\
  </form>\
  <canvas id=\"picker\"></canvas>\
  <div id=\"inner\" onclick=\"power()\"><p>&#x23FB;</p></div>\
</body>\
<footer>\
  <script>\
    canvas_create();\
  </script>\
</footer>\
</html>";

void Ereignis_Index()
{
  uint8_t r=0,g=0,b=0;
  String Page= PageOne;
  String rgb="000";
  if(httpServer.hasArg("rgb")) {
    rgb = httpServer.arg("rgb");
    int number = (int) strtol( &rgb[0], NULL, 16);
    // Split them up into r, g, b values
    r = number >> 16;
    g = number >> 8 & 0xFF;
    b = number & 0xFF;
  }
  Page+=rgb+PageTwo+rgb+PageThree;

  if(httpServer.hasArg("on") && httpServer.arg("on") == "on") {
    Page += " checked";
  } else {
    r=0;
    g=0;
    b=0;
  }
  analogWrite(PIN_R,r);
  analogWrite(PIN_G,g);
  analogWrite(PIN_B,b);
  Page += PageFour;
  httpServer.send(200, "text/html", Page);
}

String FileRead(const char *path) {
  String content;
  File file;
  if (!SPIFFS.open(path, "r")) {
    return "";
  }
  content = file.readStringUntil(EOF);
  file.close();
  return content;
}

bool FileWrite(const char *path, String* content) {
  File file;
  if (!SPIFFS.open(path, "w")) {
    return false;
  }
  file.print(*content);
  file.close();
}
  //-------//
 // SETUP //
//-------//
void setup(void) {
  //Setup File System
  SPIFFS.begin();
  //Setup WiFi (if needed with WPS)
  WiFi.mode(WIFI_STA);
  if (!WiFi.SSID()) { //on empty SSID (saved on SPI flash)
    if (WiFi.beginWPSConfig()) { //start WPS
      if (WiFi.SSID()) {  //re-assure with SSID
        WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); //start WiFi-session
      }
    }
  } else {  //on already known SSID & PWD
    WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); //start WiFi-session
  }

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.disconnect();  //delete SSID
    setup();
  }

  MDNS.begin(host);

  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.on("/", Ereignis_Index);

  httpServer.begin();

  MDNS.addService("http", "tcp", 80);

  //LEDs                                                        //not needed for real, but for sure (security)
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  digitalWrite(PIN_R,LOW);
  digitalWrite(PIN_G,LOW);
  digitalWrite(PIN_B,LOW);

  analogWriteRange(255);

}

void loop(void) {
  httpServer.handleClient();
}
