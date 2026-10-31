#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#define CONFIG_IDF_TARGET_ESP32C6 1
#define ARDUINO_XIAO_ESP32C6 1
constexpr int LOW=0,HIGH=1,INPUT=1,OUTPUT=2;
static int modes[32],levels[32];
static unsigned rejectedWrites=0,outputHighGlitches=0,pulses=0,lastPulseWidth=0;
static uint64_t us=0,pulseAt=0;
static uint32_t millis(){return uint32_t(us/1000);}
static void pinMode(int p,int mode){
 if(mode==OUTPUT&&levels[p])outputHighGlitches++;
 modes[p]=mode;
}
static void setLevel(int p,int v){
 if(p==1&&levels[p]!=v){if(v){pulseAt=us;pulses++;}else lastPulseWidth=unsigned(us-pulseAt);}
 levels[p]=v;
}
/* Match Arduino ESP32 3.3.0: digitalWrite before GPIO ownership is rejected. */
static void digitalWrite(int p,int v){if(!modes[p]){rejectedWrites++;return;}setLevel(p,v);}
int gpio_set_level(int p,unsigned v){setLevel(p,int(v));return 0;}
static void delayMicroseconds(unsigned n){us+=n;}
int gpio_get_level(int p){return levels[p];}
#include "../xiao-revg/ChatGPTXIAORevG/hardware.h"
static bool receive(const WFrame &f,bool corrupt){
 uint8_t bytes[WIRE_ENCODED];auto n=w_encode(&f,bytes);if(corrupt)bytes[n-2]^=1;
 WParser parser={};WFrame decoded={};bool accepted=false;
 for(size_t i=0;i<n;i++)if(w_feed(&parser,bytes[i],&decoded)==1)accepted=boardAcceptFrame(decoded);
 return accepted;
}
int main(){
 static_assert(RX_PIN==17&&TX_PIN==16&&TX_ARM_PIN==2&&HEARTBEAT_PIN==1,"Rev F GPIOs");
 // Include a stale HIGH latch, not only the usual zeroed cold-boot registers.
 levels[TX_ARM_PIN]=HIGH;levels[HEARTBEAT_PIN]=HIGH;
 levels[17]=HIGH;
 boardInitPins();
 assert(rejectedWrites==0&&outputHighGlitches==0);
 assert(modes[TX_PIN]==INPUT&&modes[TX_ARM_PIN]==OUTPUT&&modes[HEARTBEAT_PIN]==OUTPUT);
 assert(levels[TX_ARM_PIN]==LOW&&levels[HEARTBEAT_PIN]==LOW);
 pulses=0;
 WFrame hello={};hello.type=W_HELLO;
 assert(!receive(hello,false)&&levels[TX_ARM_PIN]==LOW); // UART configuration failed.
 boardUartReady=true;
 assert(!receive(hello,true)&&levels[TX_ARM_PIN]==LOW); // CRC rejected before hook.
 WFrame ask={};ask.type=W_ASK;
 assert(!receive(ask,false)&&levels[TX_ARM_PIN]==LOW); // Paid request cannot open gate.
 auto openingAt=us;assert(receive(hello,false));assert(us-openingAt==205);
 assert(levels[TX_ARM_PIN]==HIGH&&levels[HEARTBEAT_PIN]==LOW&&pulses==1&&lastPulseWidth==5);
 auto first=us;us=first+9000;boardServiceLease();assert(pulses==1);
 us=first+11000;boardServiceLease();assert(pulses==2&&lastPulseWidth==5);
 us=uint64_t(boardLease.lastFrame+30000)*1000;boardServiceLease();
 assert(!boardLease.active&&levels[TX_ARM_PIN]==LOW&&levels[HEARTBEAT_PIN]==LOW);
 assert(!receive(ask,false));assert(receive(hello,false));
 boardUartReady=false;boardServiceLease();assert(levels[TX_ARM_PIN]==LOW&&levels[HEARTBEAT_PIN]==LOW);
 // Casio TX idle is HIGH; LOW/HiZ with no traffic must close the output.
 boardUartReady=true;levels[17]=HIGH;assert(receive(hello,false));
 us+=1000;levels[17]=LOW;boardServiceLease();
 us+=99000;boardServiceLease();assert(boardLease.active);
 us+=2000;boardServiceLease();assert(!boardLease.active&&levels[TX_ARM_PIN]==LOW);
 levels[17]=HIGH;assert(receive(hello,false));
 // Sampling LOW during real buffered UART traffic cannot force a disconnect.
 levels[17]=LOW;us+=200000;boardServiceLease(true);assert(boardLease.active);
 us+=50000;assert(receive(hello,false));boardServiceLease();assert(boardLease.active);
 us+=50000;assert(receive(hello,false));boardServiceLease();assert(boardLease.active);
 puts("PASS Rev G XIAO actual GPIO hooks: 3-wire line-loss guard, pending-data immunity, recent-frame immunity, safe initialization with stale HIGH latches, correct pins, failed-UART lockout, CRC boundary, HELLO-only arming, 5us heartbeat, lease expiry and reopen. Host GPIO model, not a hardware measurement.");
}
