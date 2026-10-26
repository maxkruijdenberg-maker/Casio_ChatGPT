#ifndef CHATGPT_TEXT_H
#define CHATGPT_TEXT_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* UTF-8 only on the wire and in the editor. Never split a code point. */
static size_t u_decode(const char *s,size_t n,uint32_t *cp){
 uint32_t x,min;size_t len,i;unsigned char c;if(!n)return 0;c=(unsigned char)s[0];
 if(c<0x80){*cp=c;return 1;}
 if(c>=0xc2&&c<=0xdf){len=2;x=c&31;min=0x80;}
 else if(c>=0xe0&&c<=0xef){len=3;x=c&15;min=0x800;}
 else if(c>=0xf0&&c<=0xf4){len=4;x=c&7;min=0x10000;}
 else {*cp=0xfffd;return 1;}
 if(n<len){*cp=0xfffd;return 1;}
 for(i=1;i<len;i++){c=(unsigned char)s[i];if((c&0xc0)!=0x80){*cp=0xfffd;return 1;}x=(x<<6)|(c&63);}
 if(x<min||x>0x10ffff||(x>=0xd800&&x<=0xdfff)){*cp=0xfffd;return 1;}
 *cp=x;return len;
}
static size_t u_encode(uint32_t x,char out[5]){
 size_t n;if(x<0x80){out[0]=(char)x;n=1;}
 else if(x<0x800){out[0]=(char)(0xc0|(x>>6));out[1]=(char)(0x80|(x&63));n=2;}
 else if(x<0x10000&&!(x>=0xd800&&x<=0xdfff)){out[0]=(char)(0xe0|(x>>12));out[1]=(char)(0x80|((x>>6)&63));out[2]=(char)(0x80|(x&63));n=3;}
 else if(x<=0x10ffff&&x>=0x10000){out[0]=(char)(0xf0|(x>>18));out[1]=(char)(0x80|((x>>12)&63));out[2]=(char)(0x80|((x>>6)&63));out[3]=(char)(0x80|(x&63));n=4;}
 else return 0;out[n]=0;return n;
}
static size_t u_prev(const char*s,size_t pos){if(!pos)return 0;--pos;while(pos&&((unsigned char)s[pos]&0xc0)==0x80)--pos;return pos;}
static size_t u_next(const char*s,size_t pos){uint32_t cp;return pos+u_decode(s+pos,strlen(s+pos),&cp);}
static int u_insert(char*s,size_t cap,size_t *cursor,uint32_t cp){char b[5];size_t n=u_encode(cp,b),len=strlen(s);
 if(!n||cp==0||len+n>=cap||*cursor>len)return 0;
 memmove(s+*cursor+n,s+*cursor,len-*cursor+1);memcpy(s+*cursor,b,n);*cursor+=n;return 1;
}
static void u_delete(char*s,size_t *cursor){size_t prev;if(!*cursor)return;prev=u_prev(s,*cursor);memmove(s+prev,s+*cursor,strlen(s+*cursor)+1);*cursor=prev;}
/* Matrix positions are libfxcg PRGM_GetKey values. Letters follow key labels. */
static int alpha_key(int key){
 const int map[26]={76,66,56,46,36,26,75,65,55,45,35,25,74,64,54,73,63,53,43,33,72,62,52,42,32,71};
 int i;for(i=0;i<26;i++)if(key==map[i])return 'a'+i;if(key==61)return ' ';if(key==51)return '"';return 0;
}
static int number_key(int key){
 switch(key){case 71:return '0';case 72:return '1';case 62:return '2';case 52:return '3';case 73:return '4';case 63:return '5';case 53:return '6';case 74:return '7';case 64:return '8';case 54:return '9';case 61:return '.';case 51:return 'e';case 41:return '-';case 42:return '+';case 32:return '-';case 43:return '*';case 33:return '/';case 55:return '(';case 45:return ')';case 35:return ',';default:return 0;}
}
#endif
