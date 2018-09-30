
#include <ESP8266WebServer.h>
#include <FS.h>

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();


#endif //__FILESYSTEM_H__
