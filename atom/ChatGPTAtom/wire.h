#ifndef CHATGPT_WIRE_H
#define CHATGPT_WIRE_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* V1: 7e [version,type,sequence,length,payload,CRC16-CCITT-BE] 7e.
 * Escape 7e/7d with 7d and XOR 20. At most 24 payload bytes; stop-and-wait. */
#define WIRE_MAX 24
#define WIRE_RAW (WIRE_MAX+6)
#define WIRE_ENCODED (2*WIRE_RAW+2)
enum { W_HELLO=1, W_BEGIN, W_APPEND, W_APPLY, W_ASK, W_STATUS, W_READ,
 W_CANCEL, W_NEW, W_INFO, W_WIPE, W_ALLOC, W_GET_CONFIG, W_DEVICE };
enum { S_IDLE, S_WIFI, S_TIME, S_MODEL, S_API, S_READY, S_ERROR, S_CANCELLED };
enum { E_NONE, E_CONFIG, E_WIFI, E_INTERNET, E_TIME, E_TLS, E_AUTH, E_LIMIT,
 E_MODEL, E_HTTP, E_BIG, E_JSON, E_TIMEOUT, E_BUSY, E_UPLOAD, E_RESET,
 E_MEMORY, E_REQUEST, E_REFUSED, E_WIFI_NOT_FOUND, E_WIFI_AUTH };
typedef struct { uint8_t type,seq,len,data[WIRE_MAX]; } WFrame;
typedef struct { uint8_t buf[WIRE_RAW],n,escaped,overflow; } WParser;
static uint16_t w_crc(const uint8_t *p,size_t n) {
 uint16_t c=0xffff; size_t i; int b;
 for(i=0;i<n;i++){c^=(uint16_t)p[i]<<8;for(b=0;b<8;b++)c=(uint16_t)((c<<1)^((c&0x8000)?0x1021:0));}return c;
}
static void w_put16(uint8_t *p,uint16_t x){p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;}
static uint16_t w_get16(const uint8_t *p){return (uint16_t)(((uint16_t)p[0]<<8)|p[1]);}
static void w_put32(uint8_t *p,uint32_t x){p[0]=(uint8_t)(x>>24);p[1]=(uint8_t)(x>>16);p[2]=(uint8_t)(x>>8);p[3]=(uint8_t)x;}
static uint32_t w_get32(const uint8_t *p){return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static size_t w_encode(const WFrame *f,uint8_t *out) {
 uint8_t b[WIRE_RAW];size_t i,n=0;uint16_t crc;
 if(f->len>WIRE_MAX)return 0;
 b[0]=1;b[1]=f->type;b[2]=f->seq;b[3]=f->len;memcpy(b+4,f->data,f->len);
 crc=w_crc(b,f->len+4);w_put16(b+f->len+4,crc);out[n++]=0x7e;
 for(i=0;i<(size_t)f->len+6;i++){if(b[i]==0x7e||b[i]==0x7d){out[n++]=0x7d;out[n++]=b[i]^0x20;}else out[n++]=b[i];}
 out[n++]=0x7e;return n;
}
/* 1 frame, 0 incomplete, -1 damaged. A delimiter always resynchronizes. */
static int w_feed(WParser *p,uint8_t c,WFrame *f) {
 if(c==0x7e){int ok=0;
  if(p->n||p->overflow||p->escaped){ok=-1;
   if(!p->overflow&&!p->escaped&&p->n>=6&&p->buf[0]==1&&p->buf[3]<=WIRE_MAX&&p->n==p->buf[3]+6&&w_crc(p->buf,p->n-2)==w_get16(p->buf+p->n-2)){
    f->type=p->buf[1];f->seq=p->buf[2];f->len=p->buf[3];memcpy(f->data,p->buf+4,f->len);ok=1;
   }
  }memset(p,0,sizeof(*p));return ok;
 }
 if(p->overflow)return 0;
 if(c==0x7d&&!p->escaped){p->escaped=1;return 0;}
 if(p->escaped){c^=0x20;p->escaped=0;}
 if(p->n>=WIRE_RAW){p->overflow=1;return 0;}p->buf[p->n++]=c;return 0;
}
#endif
