#include <cassert>
#include <cstdint>
#include <cstdio>
#include "../xiao-revg/ChatGPTXIAORevG/rx_idle_guard.h"
int main(){
 RxIdleGuard g;
 assert(!g.lost(0,true,false,0));assert(!g.lost(0,false,false,0));
 assert(!g.lost(99,false,false,0));assert(g.lost(100,false,false,0));
 assert(!g.lost(101,true,false,0));assert(!g.low);
 assert(!g.lost(200,false,false,0));assert(!g.lost(350,false,true,0));assert(!g.low);
 assert(!g.lost(400,false,false,0));assert(!g.lost(500,false,false,450));
 assert(g.lost(550,false,false,450));
 RxIdleGuard wrap;assert(!wrap.lost(0xfffffff0u,false,false,0xffffff00u));
 assert(!wrap.lost(0x53u,false,false,0xffffff00u));assert(wrap.lost(0x54u,false,false,0xffffff00u));
 // Worst-case 62-byte escaped frame at 9600 baud takes <65ms, shorter than guard.
 static_assert(62*10*1000/9600<100,"guard longer than legal frame");
 puts("PASS Rev G RX idle guard: 100ms line-LOW boundary, pending bytes, recent frames, HIGH reset, clock wrap and maximum encoded-frame duration. Not a power-state sensor.");
}
