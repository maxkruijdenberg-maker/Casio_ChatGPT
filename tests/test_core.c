#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "../atom/ChatGPTAtom/wire.h"
#include "../casio/text.h"
static unsigned checks;
#define CHECK(x) do{checks++;assert(x);}while(0)
static void test_frames(void){
 unsigned len,seed,bit;WFrame a,b;uint8_t enc[WIRE_ENCODED];size_t n,i;WParser parser;
 CHECK(w_crc((const uint8_t*)"123456789",9)==0x29b1);
 for(len=0;len<=WIRE_MAX;len++)for(seed=0;seed<256;seed++){
  memset(&a,0,sizeof(a));a.type=1;a.seq=(uint8_t)seed;a.len=(uint8_t)len;
  for(i=0;i<len;i++)a.data[i]=(uint8_t)(seed+i);n=w_encode(&a,enc);CHECK(n<=WIRE_ENCODED);
  memset(&parser,0,sizeof(parser));int got=0;
  for(i=0;i<n;i++)if(w_feed(&parser,enc[i],&b)==1)got++;
  CHECK(got==1);CHECK(a.type==b.type&&a.seq==b.seq&&a.len==b.len&&!memcmp(a.data,b.data,len));
 }
 memset(&a,0,sizeof(a));a.type=2;a.seq=19;a.len=24;memset(a.data,0x55,24);n=w_encode(&a,enc);
 for(i=1;i<n-1;i++)for(bit=0;bit<8;bit++){
  uint8_t damaged[WIRE_ENCODED];memcpy(damaged,enc,n);damaged[i]^=(uint8_t)(1<<bit);
  memset(&parser,0,sizeof(parser));int got=0;size_t k;
  for(k=0;k<n;k++)if(w_feed(&parser,damaged[k],&b)==1)got++;
  CHECK(got==0);
  for(k=0;k<n;k++)if(w_feed(&parser,enc[k],&b)==1)got++;
  CHECK(got==1);
 }
 /* Overflow and truncated escape recover at the next frame delimiter. */
 memset(&parser,0,sizeof(parser));for(i=0;i<1000;i++)w_feed(&parser,0x55,&b);
 CHECK(w_feed(&parser,0x7e,&b)==-1);CHECK(w_feed(&parser,0x7d,&b)==0);CHECK(w_feed(&parser,0x7e,&b)==-1);
 int got=0;for(i=0;i<n;i++)if(w_feed(&parser,enc[i],&b)==1)got++;CHECK(got==1);
 a.len=25;CHECK(w_encode(&a,enc)==0);
}
static void test_text(void){
 char s[33]="";size_t c=0,n;uint32_t cp;char b[5];uint32_t values[]={32,'A',0xe9,0x3c0,0x1f600,0x10ffff};
 unsigned i;for(i=0;i<sizeof(values)/sizeof(values[0]);i++){
  n=u_encode(values[i],b);CHECK(n>0);CHECK(u_decode(b,n,&cp)==n&&cp==values[i]);
 }
 CHECK(u_insert(s,sizeof(s),&c,'A'));CHECK(u_insert(s,sizeof(s),&c,0xe9));CHECK(u_insert(s,sizeof(s),&c,0x3c0));CHECK(u_insert(s,sizeof(s),&c,0x1f600));
 CHECK(c==9);CHECK(u_prev(s,c)==5);u_delete(s,&c);CHECK(c==5&&!strcmp(s,"Aéπ"));
 c=u_prev(s,c);u_delete(s,&c);CHECK(c==1&&!strcmp(s,"Aπ"));CHECK(u_next(s,c)==3);
 CHECK(u_insert(s,sizeof(s),&c,0xe9));CHECK(!strcmp(s,"Aéπ"));
 CHECK(!u_insert(s,strlen(s)+1,&c,'x'));CHECK(!u_insert(s,sizeof(s),&c,0));
 CHECK(u_encode(0xd800,b)==0);CHECK(u_encode(0x110000,b)==0);
 const char *bad[]={"\xc0\xaf","\xed\xa0\x80","\xf4\x90\x80\x80","\xe2\x82","\xff"};
 for(i=0;i<sizeof(bad)/sizeof(bad[0]);i++)CHECK(u_decode(bad[i],strlen(bad[i]),&cp)==1&&cp==0xfffd);
 CHECK(alpha_key(76)=='a');CHECK(alpha_key(71)=='z');CHECK(alpha_key(61)==' ');CHECK(number_key(72)=='1');
 for(i=0;i<26;i++)CHECK(alpha_key((int[]){76,66,56,46,36,26,75,65,55,45,35,25,74,64,54,73,63,53,43,33,72,62,52,42,32,71}[i])=='a'+(int)i);
}
int main(void){test_frames();test_text();printf("PASS: %u assertions; frame roundtrips, corruption, recovery, bounds, UTF-8, editing, key mapping.\n",checks);return 0;}
