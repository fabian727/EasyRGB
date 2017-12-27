#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

//RGB LEDs
#define PIN_R 12  //D6
#define PIN_G 13  //D7
#define PIN_B 14  //D5

//EEPROM
#define SSID_OFFSET 0
#define PWD_OFFSET 128

void EEPROM_save(int offset, char *data) {
  int addr=0;
  while(data[addr] != '\0'){
    EEPROM.write(offset+addr,data[addr]);
    addr++;
  }
}

//Wifi
#define HOSTNAME "Easy RGB"

void WiFi_start_AP() {
  WiFi.mode(WIFI_AP);
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
  WiFi.softAP("EasyRGB");
}
void WiFi_connect(char *ssid, char *pwd) {
    WiFi.hostname(HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(&ssid[0], &pwd[0]);
}

//WebServer 
ESP8266WebServer server(80);

const String Page_Header = "<!DOCTYPE html>\
<html>\
<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\
<head>\
  <title>Web RGB</title>\
<script>\
function canvas_create() {\
var canvas = document.getElementById(\"picker\");\
var context = canvas.getContext(\"2d\");\
var x = canvas.width / 2;\
var y = canvas.height / 2;\
var radius = 100;\
var counterClockwise = false;\
\
for(var angle=0; angle<=360; angle+=1){\
    var startAngle = (angle-2)*Math.PI/180;\
    var endAngle = angle * Math.PI/180;\
    context.beginPath();\
    context.moveTo(x, y);\
    context.arc(x, y, radius, startAngle, endAngle, counterClockwise);\
    context.closePath();\
    context.fillStyle = 'hsl('+angle+', 100%, 50%)';\
    context.fill();\
}\
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
function canvas_listen() {\
var canvas = document.getElementById(\"picker\");\
canvas.addEventListener(\"click\",function(event){\
    var eventLocation = getEventLocation(this,event);\
    var context = this.getContext('2d');\
    var pixelData = context.getImageData(eventLocation.x, eventLocation.y, 1, 1).data; \
    var hex = (\"000000\" + rgbToHex(pixelData[0], pixelData[1], pixelData[2])).slice(-6);\
    document.getElementById(\"rgb\").value = hex;\
    document.getElementById(\"Form\").submit();\
},false);\
}\
</script>\
</head>\
<body style=\"background-color:#222;\">\
  <form id=\"Form\" action=\"\" method=\"GET\">\
    <input type=\"checkbox\" name=\"on\" onChange=\"this.form.submit()\"";
    
const String Page_Footer = ">\
    <input id=\"rgb\" type=\"text\" name=\"rgb\" value=\"\" style=\"visibility:hidden;\">\
  </form>\
  <canvas id=\"picker\" width=\"300\" height=\"300\"></canvas>\
</body>\
<foot>\
  <script>\
    canvas_create();\
    canvas_listen();\
  </script>\
</foot>\
</html>\
";

const String Config_Page = "<!DOCTYPE html>\
<html>\
  <meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\
  <head>\
    </script>\
    <style>\
      input[type=text] {\
        width:200%;\
      }\
      form {\
        margin-left:auto;\
        margin-right:auto;\
        width:100px;\
      }\
    </style>\
    <title>Web RGB</title>\
  </head>\
  <body style=\"background-color:#222;\">\
    <form action=\"\" method=\"POST\" style=\"\">\
      <input type=\"text\" name=\"ssid\" placeholder=\"SSID\"><br>\
      <input type=\"password\" name=\"pwd\" placeholder=\"Password\"><br>\
      <input type=\"submit\" name=\"action\" value=\"OK\">\
    </form>\
  </body>\
</html>\
";

void Ereignis_Index()
{
  String Page = Page_Header;
  int r=0,g=0,b=0;
  if(server.arg("on") == "on") {
    String rgb = server.arg("rgb");
    int number = (int) strtol( &rgb[0], NULL, 16);
    // Split them up into r, g, b values
    r = number >> 16;
    g = number >> 8 & 0xFF;
    b = number & 0xFF;
    Page += "checked";
  }
  Page += Page_Footer;
  analogWrite(PIN_R,r);
  analogWrite(PIN_G,g);
  analogWrite(PIN_B,b);
  server.send(200, "text/html", Page);
}

void Ereignis_Config()
{
  if(server.hasArg("ssid") && server.hasArg("pwd")) {
    String ssid = server.arg("ssid");
    String pwd = server.arg("pwd");
    EEPROM_save(SSID_OFFSET,&ssid[0]);
    EEPROM_save(PWD_OFFSET,&pwd[0]);
    WiFi_connect(&ssid[0], &pwd[0]);
  }
    server.send(200, "text/html", Config_Page);
}

  //-------//
 // SETUP //
//-------//
void setup()
{
  //LEDs                                                        //not needed for real, but for sure (security)
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  //WiFi
  char ssid[128];
  char pwd[128];
 
  EEPROM.begin(256);                                            //EEPROM init with 256 Bytes/Chars

  if(EEPROM.read(SSID_OFFSET) == 255) {                         //if SSID saved in EEPROM is empty, setup AccessPoint
    WiFi_start_AP();
  } else {                                                      //else
    int addr=0;
    do{                                                         //read ssid
      addr++;
      ssid[addr] = EEPROM.read(SSID_OFFSET+addr);
    } while(ssid[addr] != '\0');
    addr=0;
    do{                                                         //read pwd
      addr++;
      pwd[addr] = EEPROM.read(PWD_OFFSET+addr);
    } while(pwd[addr] != '\0');

    WiFi_connect(&ssid[0],&pwd[0]);                             //check if connection was successfull, else start AccesPoint
    int tmr = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      tmr++;
      if(tmr > 12) {//>6sec
        EEPROM.write(0,255);
        WiFi_start_AP();
        break;
      }
    }
  }
 
  server.on("/", Ereignis_Index);
  server.on("/config", Ereignis_Config);
  server.begin();
}

  //------//
 // LOOP //
//------//
void loop()
{
  server.handleClient();
}
