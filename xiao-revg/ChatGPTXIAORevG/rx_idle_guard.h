#pragma once
#include <stdint.h>
/* A silent LOW for >=100ms is not a valid 9600-baud stop-and-wait frame.
 * FIFO activity or a recent decoded frame prevents closing on sampled data bits.
 * This detects line loss/break, not a guaranteed physical Casio power state. */
struct RxIdleGuard {
 bool low=false;uint32_t since=0;
 bool lost(uint32_t now,bool high,bool pending,uint32_t lastFrame){
  if(high||pending){low=false;return false;}
  if(!low){low=true;since=now;}
  return (uint32_t)(now-since)>=100&&(uint32_t)(now-lastFrame)>=100;
 }
};
