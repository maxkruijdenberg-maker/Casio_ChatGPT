#ifndef CHATGPT_UPLOAD_H
#define CHATGPT_UPLOAD_H
#include "wire.h"
#define UPLOAD_MAX 8192
typedef struct {char bytes[UPLOAD_MAX+1];uint16_t expected,used;uint8_t target,active;} Upload;
static void upload_clear(Upload*u){volatile uint8_t*p=(volatile uint8_t*)u;size_t n=sizeof(*u);while(n--)*p++=0;}
static int upload_begin(Upload*u,uint8_t target,uint16_t expected){
 static const uint16_t caps[]={1024,32,63,256,80,1024,8192};upload_clear(u);
 if(target>6||expected>caps[target])return E_BIG;
 u->target=target;u->expected=expected;u->active=1;return E_NONE;
}
static int upload_append(Upload*u,uint16_t offset,const uint8_t*p,size_t n){
 if(!u->active||!n||offset!=u->used||u->used>u->expected||n>(size_t)(u->expected-u->used)||memchr(p,0,n))return E_UPLOAD;
 memcpy(u->bytes+u->used,p,n);u->used+=(uint16_t)n;u->bytes[u->used]=0;return E_NONE;
}
static int upload_complete(const Upload*u){return u->active&&u->used==u->expected;}
#endif
