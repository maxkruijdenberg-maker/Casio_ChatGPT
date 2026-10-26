/* Native CG menu resolver, based on gint's osmenu.c (Lephe/Dr-Carlos).
 * See docs/V9-NATIVE-MENU.md for pinned source and attribution.
 * No OS RAM is patched. Read-only, bounded instruction recognition. */
#ifndef CHATGPT_NATIVE_MENU_H
#define CHATGPT_NATIVE_MENU_H
#include <stdint.h>
#include <stddef.h>
typedef struct {
 void *context;
 int (*read16)(void *,uint32_t,uint16_t *);
 int (*read32)(void *,uint32_t,uint32_t *);
} NativeMenuROM;
static int nm_rom_range(uint32_t address,uint32_t bytes){
 return address>=0x80020070u&&bytes<=0x00fdff90u&&address<=0x81000000u-bytes;
}
static uint32_t nm_find_menu(const NativeMenuROM *rom){
 uint32_t table,entry,target=0;unsigned i;uint16_t w[6];
 if(!rom||!rom->read16||!rom->read32)return 0;
 if(!rom->read32(rom->context,0x8002007cu,&table)||(table&3)||!nm_rom_range(table,0x1e59u*4))return 0;
 if(!rom->read32(rom->context,table+0x1e58u*4,&entry)||(entry&1)||!nm_rom_range(entry,150*2))return 0;
 for(i=0;i+6<=150;i++){
  unsigned j,rx,ry;int displacement;uint32_t candidate;
  for(j=0;j<6;j++)if(!rom->read16(rom->context,entry+(i+j)*2,&w[j]))return 0;
  if((w[0]&0xf000)!=0xd000||(w[1]&0xf0ff)!=0xe003||(w[2]&0xf000)!=0xb000||
     (w[4]&0xf000)!=0xa000||w[5]!=0x0009)continue;
  rx=(w[0]>>8)&15;ry=(w[1]>>8)&15;
  if(w[3]!=(uint16_t)(0x2000+(rx<<8)+(ry<<4)))continue;
  displacement=w[2]&0x0fff;if(displacement&0x800)displacement-=0x1000;
  candidate=entry+(i+2)*2+4+displacement*2;
  if(!nm_rom_range(candidate,2))continue;
  if(target&&candidate!=target)return 0; /* ambiguous: use timer fallback */
  target=candidate;
 }
 return target;
}
#endif
