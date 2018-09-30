
#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print(x)
#else
#define DEBUG_PRINT(x)
#endif

#endif  //__DEBUG_H__
