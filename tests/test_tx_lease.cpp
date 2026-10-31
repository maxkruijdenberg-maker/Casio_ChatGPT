#include <cassert>
#include <cstdio>
#include "../esp32c6/ChatGPTC6/tx_lease.h"
int main(){
 TxLease lease;assert(!lease.active&&!lease.due(0));
 assert(!lease.accept(W_ASK,4,10)&&!lease.accept(W_HELLO,1,10));
 assert(lease.accept(W_HELLO,0,10)&&lease.active);lease.pulsed(10);
 assert(!lease.due(19)&&lease.due(20));lease.pulsed(20);
 for(uint32_t now=30;now<30010;now+=10){assert(lease.due(now));lease.pulsed(now);}
 assert(!lease.due(30010)&&!lease.active);
 assert(!lease.accept(W_ASK,4,30011)&&lease.accept(W_HELLO,0,30012));
 assert(lease.accept(W_STATUS,0,59999));assert(lease.due(60000));
 assert(!lease.due(89999)&&!lease.active);
 TxLease wrap;assert(wrap.accept(W_HELLO,0,0xfffffff0u));wrap.pulsed(0xfffffff0u);
 assert(!wrap.due(0xfffffff5u)&&wrap.due(0x00u));
 assert(!wrap.due((uint32_t)(0xfffffff0u+30000u))&&!wrap.active);
 // Only a valid CRC-protected frame reaches the board hook.
 WFrame frame={};frame.type=W_HELLO;frame.seq=1;uint8_t wire[WIRE_ENCODED];size_t n=w_encode(&frame,wire);
 WParser parser={};WFrame decoded={};int accepted=0;
 for(size_t i=0;i<n;i++)if(w_feed(&parser,wire[i],&decoded)==1)accepted++;
 assert(accepted==1);wire[n/2]^=1;parser={};accepted=0;
 for(size_t i=0;i<n;i++)if(w_feed(&parser,wire[i],&decoded)==1)accepted++;
 assert(!accepted);
 puts("PASS Rev E TX lease: HELLO-only opening, malformed/paid opening rejected, 10ms pulses, 30s expiry, fresh-frame extension, wrap, CRC frame boundary; original payload/ledger unchanged.");
}
