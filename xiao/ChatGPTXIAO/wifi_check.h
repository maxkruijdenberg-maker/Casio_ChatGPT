#ifndef CHATGPT_WIFI_CHECK_H
#define CHATGPT_WIFI_CHECK_H
#include <stdint.h>
#include "wire.h"
struct WifiCheckResult {uint8_t quality=0,error=E_NONE;bool cancelled=false;};
/* Only association with the configured hotspot is checked. No HTTP/API work. */
static uint8_t wifi_quality(int rssi){return rssi>=-65?3:rssi>=-75?2:1;}
template<class Wifi,class Clock,class Cancel,class Pause>
static WifiCheckResult check_wifi(Wifi &wifi,bool configured,bool reconnect,Clock now,Cancel &cancel,Pause pause){
 WifiCheckResult result;
 if(cancel.load()){result.cancelled=true;return result;}
 if(!configured){result.error=E_CONFIG;return result;}
 uint32_t started=now();
 if(reconnect||!wifi.ready())wifi.begin();
 while(!cancel.load()&&!wifi.ready()&&(uint32_t)(now()-started)<20000)pause();
 if(cancel.load()){result.cancelled=true;return result;}
 if(!wifi.ready()){result.error=wifi.error();return result;}
 result.quality=wifi_quality(wifi.rssi());return result;
}
#endif
