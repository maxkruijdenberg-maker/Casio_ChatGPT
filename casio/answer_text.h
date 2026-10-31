#ifndef CHATGPT_ANSWER_TEXT_H
#define CHATGPT_ANSWER_TEXT_H
#include "text.h"
/* Display-only compatibility conversion; original replies/history are untouched.
 * Convert Unicode presentation variants without changing mathematical relations. */
static uint32_t answer_symbol(uint32_t cp){
 switch(cp){
 case 0xff1d:case 0xfe66:case 0x207c:case 0x208c:return '=';
 case 0xff0b:return '+';case 0xff0d:case 0x2212:return '-';
 case 0xff0a:return '*';case 0xff0f:return '/';
 case 0xa0:case 0x2007:case 0x2009:case 0x200a:case 0x202f:return ' ';case 0x210e:return 'h';
 case 0x2705:case 0x1f44d:return 0x2713; /* check / thumbs-up */
 case 0x274c:case 0x1f44e:return 0x2717; /* cross / thumbs-down */
 case 0x1f642:case 0x1f60a:case 0x1f600:return 0x263a;
 case 0x2764:return 0x2665;
 case 0x1f539:case 0x1f538:case 0x1f537:case 0x1f536:case 0x1f4cc:return 0x2022;
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
  n=u_decode(source+at,len-at,&cp);
  /* These format/emoji modifiers have no standalone ink. Rendering them as
   * missing glyphs creates a spurious diamond after otherwise valid text. */
  if(cp==0x200b||cp==0x200c||cp==0x200d||cp==0x2060||cp==0xfeff||cp==0xfe0e||cp==0xfe0f||cp==0xad||(cp>=0x1f3fb&&cp<=0x1f3ff)){at+=n;continue;}
  cp=answer_symbol(cp);
  /* Preserve the count of equal signs in the uncommon double/triple forms.
   * The font has no native glyphs for these, while ASCII is unambiguous. */
  if(cp==0x2a75||cp==0x2a76){m=cp==0x2a75?2:3;memcpy(b,"===",m);}
  else m=u_encode(cp,b);
  if(!m||used+m>=cap){out[0]=0;return 0;}
  memcpy(out+used,b,m);used+=m;at+=n;
 }
 out[used]=0;return 1;
}
#endif
