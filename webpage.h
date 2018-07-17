#include "config.h"

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
  max-width: 1024px;\
  margin-left: auto;\
  margin-right: auto;\
  display: block;\
}\
#inner {\
  position: absolute;\
  left: 50vw;\
  top: 36vw;\
  height: 26vw;\
  width: 26vw;\
  transform:translateX(-50%);\
  border-radius: 100%;\
  background-color: #838485;\
  border: 5px solid black;\
  cursor: pointer;\
  box-shadow: inset 0 0 15px #";

const String PageTwo =";\
}\
#inner > svg {\
  margin-left: 25%;\
  width:50%;\
  height:100%;\
}\
@media screen and (min-width: 1025px) {\
  canvas {\
    max-width: 500px;\
  }\
  #inner {\
    top: calc(250px - 65px);\
    height: 130px;\
    width: 130px;\
  }\
}\
</style>\
<script>\
function power() {\
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
\
  document.getElementById(\"picker\").addEventListener(\"click\",function(event){\
    var eventLocation = getEventLocation(this,event);\
    var context = this.getContext('2d');\
    var pixelData = context.getImageData(eventLocation.x, eventLocation.y, 1, 1).data; \
    var hex = (\"000000\" + rgbToHex(pixelData[0], pixelData[1], pixelData[2])).slice(-6);\
    document.getElementById(\"rgb\").value = hex;\
    document.getElementById(\"Form\").submit();\
  },false);\
}\
\
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
    <input id=\"rgb\" type=\"text\" name=\"rgb\" value=\"";
const String PageThree = "\"></input>\
    <input id=\"power\" type=\"checkbox\" name=\"on\"";
      
const String PageFour = "></input>\
  </form>\
  <canvas id=\"picker\"></canvas>\
  <div id=\"inner\" onclick=\"power()\">\
    <svg viewBox=\"0 0 200 185\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\">\
      <path style=\"stroke:#000;stroke-width:22;stroke-linecap:round;\" d=\"m 100,20 0,75\"/>\
      <path style=\"fill:none;stroke:#000;stroke-width:22;stroke-linecap:round\" d=\"M 65 30 A 75 75 0 1 0 135 30\"/>\
    </svg>\
  </div>\
</body>\
<footer>\
  <script>\
    canvas_create();\
  </script>\
</footer>\
</html>";

const String PageConfig1 = "<!DOCTYPE html>\
<html>\
<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\
<meta charset=\"UTF-8\">\
<head>\
  <title>Web RGB</title>\
  <style>body{\
    background-color:grey;\
  }</style>\
</head>\
<body>\
  <form method=\"GET\" action=\"\">";
const String PageConfig9 = "\
    Height:\
    <input autofocus type=\"number\" name=\"height\" min=\"1\" max=\"60\" value=\"";
const String PageConfig2 = "\"><br>\
    Width:\
    <input type=\"number\" name=\"width\" min=\"1\" max=\"60\" value=\"";
const String PageConfig3 = "\"><br>\
    Pin1:\
    <input type=\"number\" name=\"pin1\" min=\"0\" max=\"20\" value=\"";
const String PageConfig4 = "\"><br>\
    Pin2:\
    <input type=\"number\" name=\"pin2\" min=\"0\" max=\"20\" value=\"";
const String PageConfig10 = "\"><br>";
const String PageConfig5 = "\
    Servername:\
    <input type=\"text\" name=\"ServerName\" value=\"";
const String PageConfig7 = "\"><br>\
  SSID:\
    <input type=\"text\" name=\"ssid\" min=\"0\" max=\"20\" value=\"";
const String PageConfig8 = "\"><br>\
    PWD:\
    <input type=\"password\" name=\"pwd\" min=\"0\" max=\"20\" value=\"";
const String PageConfig6 = "\"><br>\
    <input type=\"submit\" value=\"Set\">\
  </form>\
</body>\
</html>";
