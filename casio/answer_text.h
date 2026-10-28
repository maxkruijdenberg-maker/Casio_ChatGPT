#ifndef CHATGPT_ANSWER_TEXT_H
#define CHATGPT_ANSWER_TEXT_H
#include "text.h"
/* Display-only compatibility conversion; original replies/history are untouched.
 * Convert Unicode presentation variants without changing mathematical relations. */
static uint32_t answer_symbol(uint32_t cp){
 switch(cp){
 case 0xff1d:case 0xfe66:return '=';
 case 0xff0b:return '+';case 0xff0d:case 0x2212:return '-';
 case 0xff0a:return '*';case 0xff0f:return '/';
 case 0xa0:case 0x202f:return ' ';case 0x210e:return 'h';
 default:break;
 }
 if(cp>=0x1d400&&cp<=0x1d419)return 'A'+cp-0x1d400;
 if(cp>=0x1d41a&&cp<=0x1d433)return 'a'+cp-0x1d41a;
 if(cp>=0x1d434&&cp<=0x1d44d)return 'A'+cp-0x1d434;
 if(cp>=0x1d44e&&cp<=0x1d467&&cp!=0x1d455)return 'a'+cp-0x1d44e;
 return cp;
}
static int answer_display(const char *source,char *out,size_t cap){
 size_t at=0,used=0,len=strlen(source),n,m;uint32_t cp;char b[5];
 if(!cap)return 0;
 while(at<len){
  /* Remove only explicit LaTeX math delimiters, never mathematical operators. */
  if(source[at]=='\\'&&at+1<len&&(source[at+1]=='('||source[at+1]==')'||source[at+1]=='['||source[at+1]==']')){at+=2;continue;}
  n=u_decode(source+at,len-at,&cp);cp=answer_symbol(cp);m=u_encode(cp,b);
  if(!m||used+m>=cap){out[0]=0;return 0;}
  memcpy(out+used,b,m);used+=m;at+=n;
 }
 out[used]=0;return 1;
}
#endif
