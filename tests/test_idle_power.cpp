#include <cassert>
#include <cstdio>
#include "../atom/ChatGPTAtom/idle_power.h"
int main(){
 IdlePower p;p.begin(100);assert(!p.park(35099,false)&&!p.park(35100,true));
 assert(p.park(35100,false)&&p.parked&&!p.park(90000,false));
 assert(p.contact(90001)&&!p.parked&&!p.contact(90002));
 for(uint32_t t=100000;t<900000;t+=10000){assert(!p.park(t,false));assert(!p.contact(t));}
 assert(!p.park(924999,false)&&p.park(925000,false));
 p.begin(0xfffffff0u);assert(!p.park(uint32_t(0xfffffff0u+34999u),false));assert(p.park(uint32_t(0xfffffff0u+35000u),false));
 assert(p.contact(60000)&&!p.parked);
 p.begin(0);assert(!p.park(120000,true)&&p.park(120001,false));
 puts("PASS idle radio policy: 35s peer-loss delay, 10s reading keepalives never park, active-worker gate, one transition per idle period, UART-contact wake and uint32 clock wrap. Physical current/reconnect still needs device testing.");
}
