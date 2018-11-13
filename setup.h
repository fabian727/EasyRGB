#ifndef __SETUP_H__
#define __SETUP_H__

#include "config.h"

void connectWiFi(wifidata wifiData);
void setOutPutPins(color pin);
void serverInit();
void spiffsInit();


//EasyRGB.ino

void indexPage();

#endif //__SETUP_H__
