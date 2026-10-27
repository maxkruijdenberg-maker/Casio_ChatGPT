#ifndef CHATGPT_KEYBOARD_RAW_H
#define CHATGPT_KEYBOARD_RAW_H
#include <stdint.h>
/* CG50 SH7305 KEYSC, read-only. Same six-word layout as gint's keysc_scan.
 * No PRGM_GetKey_OS calls, no controller configuration or register writes.
 * Matrix rows/columns cross-checked with the CG50 physical key map. */
static int keyboard_valid(int row,int col){
 return row>=0&&row<7&&col>=0&&col<10&&(col==0?row==0:col<5?row>=2:row>=1)&&!(col==4&&row==2);
}
static int keyboard_down(const uint16_t words[6],int key){
 int row=key/10-1,col=key%10;
 if(!keyboard_valid(row,col))return 0;
 return (words[col/2]>>(row+8*(col%2)))&1;
}
static int keyboard_decode(const uint16_t words[6],int previous){
 int row,col,result=0,count=0;
 if(keyboard_down(words,48))return 48; /* MENU always wins. */
 if(previous&&keyboard_down(words,previous))return previous;
 for(col=0;col<10;col++)for(row=0;row<7;row++){
  int valid=keyboard_valid(row,col);
  if(valid&&((words[col/2]>>(row+8*(col%2)))&1)){result=(row+1)*10+col;count++;}
 }
 return count==1?result:0; /* Reject new multi-key chords/ghosts. */
}
#ifdef CHATGPT_HOST_TEST
static void keyboardReadWords(uint16_t words[6]);
#else
static void keyboardReadWords(uint16_t words[6]){
 volatile const uint16_t *data=(volatile const uint16_t *)(uintptr_t)0xa44b0000u;
 int i;for(i=0;i<6;i++)words[i]=data[i];
}
#endif
static int keyboardRead(int previous){uint16_t words[6];keyboardReadWords(words);return keyboard_decode(words,previous);}
#endif
