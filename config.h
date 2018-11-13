#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

//NodeMCU v1.0 esp12-e
//RGB 14 12 13


#define CONFIG_FILE "/config.json"
#define CONFIGPAGE_FILE "/config.html"

struct color
{
  uint8_t red;
	uint8_t green;
	uint8_t blue;
};

//all you need for connecting to WiFi with an own defined name
struct wifidata
{
	char *ssid;
	char *pwd;
	char *host;
};

class conf
{
  public:

  color clr, pin;
  wifidata wifiData;

  conf();

  void load();
  void save(char* bootupcolor);
};

extern conf config;

char ascii2hex(char c);

void saveFavourites();
void configPage();

#endif //__CONFIG_H__
