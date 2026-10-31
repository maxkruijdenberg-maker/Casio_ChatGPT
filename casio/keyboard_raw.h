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
/* The editor supports a physical SHIFT+key chord as well as a tapped, one-shot
 * SHIFT. Keep this separate from the legacy scanner used at OS boundaries. */
typedef struct {int down,used,announced,deferred;} KeyboardShiftState;
static void keyboard_shift_reset(KeyboardShiftState *state){
 state->down=state->used=state->announced=state->deferred=0;
}
static void keyboard_shift_consume(KeyboardShiftState *state){state->used=1;}
static int keyboard_decode_modifier(const uint16_t words[6],int previous,KeyboardShiftState *state,int *chordShifted,int modifier){
 int row,col,other=0,count=0,shiftDown=keyboard_down(words,modifier);
 *chordShifted=0;
 if(!shiftDown)keyboard_shift_reset(state);
 else if(!state->down){
  state->down=1;state->used=state->announced=state->deferred=0;
  /* A late modifier must not redispatch an already-held ordinary key. */
  if(previous&&previous!=modifier&&previous!=48&&keyboard_down(words,previous))state->deferred=previous;
 }
 if(keyboard_down(words,48))return 48; /* MENU retains absolute priority. */
 if(!shiftDown)return keyboard_decode(words,previous);
 if(state->deferred){
  if(keyboard_down(words,state->deferred))return previous==state->deferred?previous:0;
  state->deferred=0;
 }
 for(col=0;col<10;col++)for(row=0;row<7;row++){
  int key=(row+1)*10+col;
  if(key!=modifier&&keyboard_valid(row,col)&&((words[col/2]>>(row+8*(col%2)))&1)){other=key;count++;}
 }
 if(count>1){
  /* Preserve a previously held key, but reject every new ambiguous chord. */
  if(previous&&previous!=modifier&&keyboard_down(words,previous))return previous;
  return previous==modifier&&state->announced&&!state->used?modifier:0;
 }
 if(count==1){
  if(previous==other)return other;
  if(!state->used){state->used=1;*chordShifted=modifier==78||!state->announced;}
  return other;
 }
 /* Releasing the function before SHIFT must never create a second SHIFT edge.
  * announced also protects a tapped SHIFT from a temporary rejected chord. */
 if(state->used||(state->announced&&previous!=modifier))return 0;
 state->announced=1;return modifier;
}
static int keyboard_decode_shift(const uint16_t words[6],int previous,KeyboardShiftState *state,int *shifted){
 return keyboard_decode_modifier(words,previous,state,shifted,78);
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
static int keyboardReadShift(int previous,KeyboardShiftState *state,int *chordShifted){
 uint16_t words[6];keyboardReadWords(words);return keyboard_decode_shift(words,previous,state,chordShifted);
}
/* ALPHA toggles a persistent letter lock exactly once per physical press.
 * A chord only needs a synthetic toggle when ALPHA was not announced alone. */
static int keyboardReadModifiers(int previous,KeyboardShiftState *shift,KeyboardShiftState *alpha,int *shifted,int *alphaToggle){
 uint16_t words[6];keyboardReadWords(words);*shifted=*alphaToggle=0;
 if(!keyboard_down(words,78))keyboard_shift_reset(shift);
 if(!keyboard_down(words,77))keyboard_shift_reset(alpha);
 if(keyboard_down(words,77)&&!keyboard_down(words,78))
  return keyboard_decode_modifier(words,previous,alpha,alphaToggle,77);
 return keyboard_decode_shift(words,previous,shift,shifted);
}
#endif
