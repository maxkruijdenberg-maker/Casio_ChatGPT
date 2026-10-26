#ifndef CG3_LIBRARY_H
#define CG3_LIBRARY_H
#include "store.h"
#include "../atom/ChatGPTAtom/utf8.h"
typedef struct {StoreRef ref;char title[48];uint8_t turns,used,damaged;} LibraryEntry;
static int meta_valid(const uint8_t *meta,int kind){
 const char *title=(const char*)meta+8;const char *end=memchr(title,0,48);
 return meta[0]==kind&&meta[1]==1&&kind=='C'&&meta[2]>0&&meta[2]<=CHAT_TURNS&&end&&u_valid(title,(size_t)(end-title));
}
static LibraryEntry lib_entry(const StoreOps*o,int kind,int slot){
 LibraryEntry e;uint8_t meta[STORE_META];memset(&e,0,sizeof(e));e.ref=s_latest(o,kind,slot);
 if(!e.ref.valid){e.damaged=e.ref.present;return e;}
 if(!e.ref.length)return e;
 if(e.ref.length<STORE_META||!s_read(o,&e.ref,0,meta,sizeof(meta))||!meta_valid(meta,kind)){
  e.damaged=1;return e;
 }
 e.used=1;e.turns=meta[2];memcpy(e.title,meta+8,48);return e;
}
static void lib_title(char*out,const char *text){
 size_t at=0,len=strlen(text),n,used=0;uint32_t cp;
 while(at<len&&used<47){n=u_decode(text+at,len-at,&cp);if(cp=='\n'||cp=='\r')break;if(used+n>47)break;memcpy(out+used,text+at,n);used+=n;at+=n;}
 out[used]=0;if(!used)strcpy(out,"Nieuw gesprek");
}
static void lib_meta(uint8_t*out,int kind,const char*title,int turns){
 char clean[48];memset(out,0,STORE_META);lib_title(clean,title);out[0]=(uint8_t)kind;out[1]=1;out[2]=(uint8_t)turns;memcpy(out+8,clean,strlen(clean));
}
static int lib_pair(const StoreOps*o,const StoreRef*r,int index,char*q,size_t qcap,char*a,size_t acap){
 uint8_t lengths[4],meta[STORE_META];uint32_t at=STORE_META;int i;size_t qn,an;
 if(!s_read(o,r,0,meta,sizeof(meta))||!meta_valid(meta,'C')||index<0||index>=meta[2])return 0;
 for(i=0;i<=index;i++){
  if(!s_read(o,r,at,lengths,4))return 0;qn=w_get16(lengths);an=w_get16(lengths+2);at+=4;
  if(!qn||!an||qn>1024||an>8192||at>r->length||qn+an>r->length-at)return 0;
  if(i==index){
   if(qn>=qcap||an>=acap||!s_read(o,r,at,q,qn)||!s_read(o,r,at+qn,a,an)||!u_valid(q,qn)||!u_valid(a,an))return 0;
   q[qn]=a[an]=0;return 1;
  }
  at+=(uint32_t)(qn+an);
 }return 0;
}
static int lib_append(const StoreOps*o,int slot,const LibraryEntry*entry,const char*q,const char*a,LibraryEntry*out){
 uint8_t meta[STORE_META],lengths[4];StoreJoin j;StoreRef ref;size_t qn=strlen(q),an=strlen(a);uint32_t old=entry->used?entry->ref.length-STORE_META:0;
 if(entry->damaged||entry->turns>=CHAT_TURNS||!qn||qn>1024||!an||an>8192||!u_valid(q,qn)||!u_valid(a,an))return 0;
 lib_meta(meta,'C',entry->used?entry->title:q,entry->turns+1);w_put16(lengths,(uint16_t)qn);w_put16(lengths+2,(uint16_t)an);
 memset(&j,0,sizeof(j));j.ops=o;j.count=5;
 j.part[0]=(StorePart){meta,0,0,STORE_META};j.part[1]=(StorePart){0,&entry->ref,STORE_META,old};
 j.part[2]=(StorePart){lengths,0,0,4};j.part[3]=(StorePart){(const uint8_t*)q,0,0,qn};j.part[4]=(StorePart){(const uint8_t*)a,0,0,an};
 if(!s_save(o,'C',slot,STORE_META+old+4+qn+an,s_join,&j,&ref))return 0;
 if(out){memset(out,0,sizeof(*out));out->ref=ref;out->used=1;out->turns=meta[2];memcpy(out->title,meta+8,48);}return 1;
}
static int lib_delete(const StoreOps*o,int kind,int slot){return s_save(o,kind,slot,0,0,0,0);}
#endif
