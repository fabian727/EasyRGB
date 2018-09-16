# EasyRGB
NodeMCU server for background-light

This program has 2 web-sites
"/" or so called: index
you can configure the RGB value with a color-wheel
"/config":
SSID and PWD
pin1 is red
pin2 is green
pin3 is blue

after each change you have to reboot/ restart/ plug out in the board to make it happen else it is only saved in the config.json
If it does not find a secured WiFi-connection, it will create an AccessPoint (IoT) itself and you reach it under this IP: [192.168.4.1](http://192.168.4.1/config)
else you can find it under hostname (default: easyrgb) ending with your local range (normally: .local or .fritz.box)
