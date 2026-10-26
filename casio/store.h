#ifndef CG3_STORE_H
#define CG3_STORE_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "../atom/ChatGPTAtom/wire.h"
#define STORE_SLOTS 8
#define CHAT_TURNS 8
#define STORE_META 64
#define STORE_CHAT_MAX (STORE_META+CHAT_TURNS*(4+1024+8192))
#define STORE_HEADER 24
/* Two independent banks. A save replaces only the inactive bank. The new
 * header is written last and the complete result is read back and checked.
 * Existing valid data survives failed writes; this is not a promise about
 * arbitrary hardware/flash-controller failures. All integers are big-endian. */
typedef struct {
 void *ctx;
 int (*read)(void*,const char*,uint32_t,void*,size_t);
 int (*begin)(void*,const char*,uint32_t);
 int (*write)(void*,const char*,uint32_t,const void*,size_t);
 int (*finish)(void*,const char*);
} StoreOps;
typedef struct {char name[12];uint32_t seq,length;uint8_t bank,valid,present;} StoreRef;
typedef int (*StoreReader)(void*,uint32_t,void*,size_t);
static uint32_t s_crc_step(uint32_t crc,const uint8_t*p,size_t n){
 size_t i;int b;for(i=0;i<n;i++){crc^=p[i];for(b=0;b<8;b++)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}return crc;
}
static uint32_t s_crc(const void*p,size_t n){return s_crc_step(0xffffffffu,(const uint8_t*)p,n)^0xffffffffu;}
static uint32_t s_max(int kind){(void)kind;return STORE_CHAT_MAX;}
static void s_name(char*out,int kind,int slot,int bank){sprintf(out,"%c%d%c.dat",kind,slot,bank?'B':'A');}
static StoreRef s_probe(const StoreOps *o,int kind,int slot,int bank){
 StoreRef r;uint8_t h[STORE_HEADER],chunk[256];uint32_t at=0,crc=0xffffffffu;size_t n;
 memset(&r,0,sizeof(r));r.bank=(uint8_t)bank;s_name(r.name,kind,slot,bank);
 if(!o->read(o->ctx,r.name,0,h,sizeof(h)))return r;r.present=1;
 if(memcmp(h,"CG3S",4)||h[4]!=1||h[5]!=kind||h[6]!=slot||h[7]||s_crc(h,20)!=w_get32(h+20))return r;
 r.seq=w_get32(h+8);r.length=w_get32(h+12);if(!r.seq||r.length>s_max(kind))return r;
 while(at<r.length){n=r.length-at;if(n>sizeof(chunk))n=sizeof(chunk);
  if(!o->read(o->ctx,r.name,STORE_HEADER+at,chunk,n))return r;
  crc=s_crc_step(crc,chunk,n);at+=(uint32_t)n;
 }
 if((crc^0xffffffffu)!=w_get32(h+16))return r;r.valid=1;return r;
}
static StoreRef s_latest(const StoreOps*o,int kind,int slot){
 StoreRef a=s_probe(o,kind,slot,0),b=s_probe(o,kind,slot,1);
 if(b.valid&&(!a.valid||b.seq>a.seq))return b;
 if(!a.valid&&!b.valid&&b.present&&!a.present)return b;return a;
}
static int s_read(const StoreOps*o,const StoreRef*r,uint32_t at,void*out,size_t n){
 return r->valid&&at<=r->length&&n<=r->length-at&&o->read(o->ctx,r->name,STORE_HEADER+at,out,n);
}
static int s_save(const StoreOps*o,int kind,int slot,uint32_t length,StoreReader reader,void*ctx,StoreRef*out){
 StoreRef old=s_latest(o,kind,slot),next;uint8_t h[STORE_HEADER]={0},chunk[256];uint32_t at=0,crc=0xffffffffu;size_t n;char name[12];int bank=old.valid?!old.bank:0;
 if(slot<0||slot>=STORE_SLOTS||kind!='C'||length>s_max(kind)||(old.valid&&old.seq==UINT32_MAX))return 0;
 s_name(name,kind,slot,bank);if(!o->begin(o->ctx,name,STORE_HEADER+length))return 0;
 /* An invalid initial header prevents a partial payload from being selected. */
 if(!o->write(o->ctx,name,0,h,sizeof(h))){o->finish(o->ctx,name);return 0;}
 while(at<length){n=length-at;if(n>sizeof(chunk))n=sizeof(chunk);
  if(!reader||!reader(ctx,at,chunk,n)||!o->write(o->ctx,name,STORE_HEADER+at,chunk,n)){o->finish(o->ctx,name);return 0;}
  crc=s_crc_step(crc,chunk,n);at+=(uint32_t)n;
 }
 memcpy(h,"CG3S",4);h[4]=1;h[5]=(uint8_t)kind;h[6]=(uint8_t)slot;
 w_put32(h+8,old.valid?old.seq+1:1);w_put32(h+12,length);w_put32(h+16,crc^0xffffffffu);w_put32(h+20,s_crc(h,20));
 if(!o->write(o->ctx,name,0,h,sizeof(h))){o->finish(o->ctx,name);return 0;}
 if(!o->finish(o->ctx,name))return 0;
 next=s_probe(o,kind,slot,bank);if(!next.valid||next.length!=length||next.seq!=w_get32(h+8))return 0;
 if(out)*out=next;return 1;
}
/* Stream concatenation: old disk data and small in-memory segments. No entire
 * conversation is ever loaded into the calculator's 64KB application RAM. */
typedef struct {const uint8_t*bytes;const StoreRef*ref;uint32_t offset,length;} StorePart;
typedef struct {const StoreOps*ops;StorePart part[5];int count;} StoreJoin;
static int s_join(void*ctx,uint32_t at,void*out,size_t n){
 StoreJoin*j=(StoreJoin*)ctx;uint8_t*dst=(uint8_t*)out;int i;size_t take;
 for(i=0;i<j->count&&n;i++){
  StorePart*p=&j->part[i];if(at>=p->length){at-=p->length;continue;}
  take=p->length-at;if(take>n)take=n;
  if(p->ref){if(!s_read(j->ops,p->ref,p->offset+at,dst,take))return 0;}
  else memcpy(dst,p->bytes+at,take);
  dst+=take;n-=take;at=0;
 }return n==0;
}
#endif
