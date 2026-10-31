#ifndef CHATGPT_IDLE_POWER_H
#define CHATGPT_IDLE_POWER_H
#include <stdint.h>
/* UART stays awake: the existing RX wiring does not provide a reliable deep-
 * sleep wake/power-state input. Normal app HELLOs arrive every 10 seconds. */
struct IdlePower {
 uint32_t last=0;bool parked=false;
 void begin(uint32_t now){last=now;parked=false;}
 bool contact(uint32_t now){last=now;bool waking=parked;parked=false;return waking;}
 bool park(uint32_t now,bool busy){
  if(parked||busy||(uint32_t)(now-last)<35000)return false;
  parked=true;return true;
 }
};
#endif
