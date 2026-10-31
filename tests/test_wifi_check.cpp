#include <cassert>
#include <atomic>
#include <cstdio>
#include <initializer_list>
#include "../atom/ChatGPTAtom/wifi_check.h"
struct Hotspot {
 uint32_t &time;bool connected=false,connects=true;int begins=0,signal=-55;uint32_t readyAt=100;uint8_t failure=E_WIFI;
 bool ready(){return connected||(begins&&connects&&(uint32_t)(time-readyAt)<0x80000000u);}
 void begin(){begins++;connected=false;}
 int rssi(){return signal;}
 uint8_t error(){return failure;}
};
int main(){
 for(int signal: {-90,-76,-75,-66,-65,-40}){
  uint32_t time=0;std::atomic_bool cancel{false};Hotspot wifi{time};wifi.connected=true;wifi.signal=signal;
  auto result=check_wifi(wifi,true,false,[&](){return time;},cancel,[&](){time+=10;});
  assert(!result.error&&!result.cancelled&&result.quality==wifi_quality(signal)&&!wifi.begins&&!time);
 }
 uint32_t time=0;std::atomic_bool cancel{false};Hotspot wifi{time};
 auto run=[&](bool configured=true,bool dirty=false){return check_wifi(wifi,configured,dirty,[&](){return time;},cancel,[&](){time+=10;});};
 assert(run().quality==3&&wifi.begins==1&&time==100);
 wifi.connected=true;time=0;assert(run(true,true).quality==3&&wifi.begins==2&&time==100);
 {
  uint32_t ticks=0;std::atomic_bool stop{false};Hotspot missing{ticks};
  auto r=check_wifi(missing,false,false,[&](){return ticks;},stop,[&](){ticks+=10;});assert(r.error==E_CONFIG&&!missing.begins&&!ticks);
  stop=true;r=check_wifi(missing,true,true,[&](){return ticks;},stop,[&](){ticks+=10;});assert(r.cancelled&&!missing.begins);
 }
 for(uint8_t error: {uint8_t(E_WIFI),uint8_t(E_WIFI_NOT_FOUND),uint8_t(E_WIFI_AUTH)}){
  uint32_t ticks=0;std::atomic_bool stop{false};Hotspot bad{ticks};bad.connects=false;bad.failure=error;
  auto r=check_wifi(bad,true,false,[&](){return ticks;},stop,[&](){ticks+=10;});assert(r.error==error&&!r.quality&&ticks==20000&&bad.begins==1);
 }
 {
  uint32_t ticks=0xfffffff0u;std::atomic_bool stop{false};Hotspot bad{ticks};bad.connects=false;
  auto r=check_wifi(bad,true,false,[&](){return ticks;},stop,[&](){ticks+=10;});assert(r.error==E_WIFI&&(uint32_t)(ticks-0xfffffff0u)==20000);
 }
 {
  uint32_t ticks=0;std::atomic_bool stop{false};Hotspot late{ticks};late.readyAt=100;
  auto r=check_wifi(late,true,false,[&](){return ticks;},stop,[&](){ticks+=10;if(ticks==100)stop=true;});assert(r.cancelled&&!r.quality&&!r.error&&late.begins==1);
 }
 puts("PASS wifi diagnosis: existing configured association, RSSI quality boundaries, reconnect, no credentials required except hotspot, missing configuration, missing hotspot/auth errors, 20s budget/wrap and cancellation before/during connect; no HTTP/API work.");
}
