#ifndef CHATGPT_UTF8_H
#define CHATGPT_UTF8_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
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
static int u_valid(const char *s,size_t len){
 size_t p=0,n;uint32_t cp;
 while(p<len){n=u_decode(s+p,len-p,&cp);if(!n||(cp==0xfffd&&n==1&&(unsigned char)s[p]>=128)||cp==0)return 0;p+=n;}
 return 1;
}
#endif
