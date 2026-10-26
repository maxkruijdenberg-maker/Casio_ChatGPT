#ifndef CHATGPT_NATIVE_MENU_DEVICE_H
#define CHATGPT_NATIVE_MENU_DEVICE_H
#include "native_menu.h"
void Native_ClearKeyBuffer(void);
#ifdef CHATGPT_HOST_TEST
static int menuROMRead16(void *,uint32_t,uint16_t *);
static int menuROMRead32(void *,uint32_t,uint32_t *);
static void menuInvoke(uint32_t);
#else
static int menuROMRead16(void *unused,uint32_t address,uint16_t *value){
 (void)unused;if((address&1)||!nm_rom_range(address,2))return 0;
 *value=*(volatile const uint16_t *)(uintptr_t)address;return 1;
}
static int menuROMRead32(void *unused,uint32_t address,uint32_t *value){
 (void)unused;if((address&3)||!nm_rom_range(address,4))return 0;
 *value=*(volatile const uint32_t *)(uintptr_t)address;return 1;
}
static void menuInvoke(uint32_t address){((void (*)(void))(uintptr_t)address)();}
#endif
static uint32_t nativeMenuTarget(void){
 const NativeMenuROM rom={0,menuROMRead16,menuROMRead32};return nm_find_menu(&rom);
}
#endif
