#include "../casio/native_menu.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {uint32_t table,entry;uint16_t code[150];int fail,reads16,reads32;} ROM;
static int read32(void *p,uint32_t a,uint32_t *v){
 ROM *r=p;r->reads32++;if(r->fail==32)return 0;
 if(a==0x8002007c){*v=r->table;return 1;}
 if(a==r->table+0x1e58*4){*v=r->entry;return 1;}return 0;
}
static int read16(void *p,uint32_t a,uint16_t *v){
 ROM *r=p;r->reads16++;assert(a>=r->entry&&a<r->entry+300&&!(a&1));
 if(r->fail==16)return 0;*v=r->code[(a-r->entry)/2];return 1;
}
static ROM fresh(void){ROM r;memset(&r,0,sizeof(r));r.table=0x80040000;r.entry=0x80030000;return r;}
static void pattern(ROM *r,unsigned at,unsigned rx,unsigned ry,int displacement){
 assert(at+6<=150);uint16_t *w=r->code+at;
 w[0]=0xd000+(rx<<8)+7;w[1]=0xe003+(ry<<8);w[2]=0xb000+(displacement&0x0fff);
 w[3]=0x2000+(rx<<8)+(ry<<4);w[4]=0xaff8;w[5]=0x0009;
}
static uint32_t find(ROM *r){NativeMenuROM reader={r,read16,read32};return nm_find_menu(&reader);}
int main(void){
 ROM r=fresh();assert(!find(&r));assert(r.reads16<=150*6);
 for(unsigned rx=0;rx<16;rx++)for(unsigned ry=0;ry<16;ry++){
  r=fresh();pattern(&r,8,rx,ry,100);assert(find(&r)==r.entry+8*2+8+200);
 }
 r=fresh();pattern(&r,90,2,7,-100);assert(find(&r)==r.entry+90*2+8-200);
 r=fresh();pattern(&r,144,1,2,2);assert(find(&r)==r.entry+144*2+12);
 for(unsigned i=0;i<6;i++){
  r=fresh();pattern(&r,8,2,7,100);r.code[8+i]=0;assert(!find(&r));
 }
 r=fresh();pattern(&r,8,2,7,100);r.code[11]^=0x10;assert(!find(&r));
 r=fresh();pattern(&r,8,2,7,100);pattern(&r,40,2,7,200);assert(!find(&r));
 r=fresh();pattern(&r,8,2,7,100);pattern(&r,40,2,7,68);assert(find(&r)==r.entry+224);
 uint32_t badTables[]={0,0x80020071,0x80fffffcu,0xffffffffu,0x7ffffffcu};
 for(unsigned i=0;i<sizeof(badTables)/sizeof(*badTables);i++){r=fresh();r.table=badTables[i];assert(!find(&r)&&!r.reads16);}
 uint32_t badEntries[]={0,0x80020071,0x80fffffcu,0xffffffffu,0x7ffffffcu};
 for(unsigned i=0;i<sizeof(badEntries)/sizeof(*badEntries);i++){r=fresh();r.entry=badEntries[i];assert(!find(&r)&&!r.reads16);}
 r=fresh();r.entry=0x80020070;pattern(&r,0,2,7,-100);assert(!find(&r));
 r=fresh();r.fail=16;assert(!find(&r));r=fresh();r.fail=32;assert(!find(&r));
 assert(!nm_find_menu(NULL));NativeMenuROM empty={0,0,0};assert(!nm_find_menu(&empty));
 assert(nm_rom_range(0x80020070,2)&&nm_rom_range(0x80fffffe,2));
 assert(!nm_rom_range(0x80ffffff,2)&&!nm_rom_range(0,2)&&!nm_rom_range(0x80020070,0xffffffff));
 puts("PASS: native CG menu resolver; all SH register pairs, signed BSR, exact pattern, bounded reads, invalid/misaligned ROM pointers, missing/ambiguous patterns and read errors.");
 return 0;
}
