#include <assert.h>
#include <stdio.h>
#include <string.h>
#define CHATGPT_HOST_TEST 1
#include "../casio/keyboard_raw.h"
static uint16_t testWords[6];
static void keyboardReadWords(uint16_t words[6]){memcpy(words,testWords,sizeof(testWords));}
static void expectShift(const uint16_t words[6],int previous,KeyboardShiftState *state,int expected,int expectedShift){
 int shifted=99;assert(keyboard_decode_shift(words,previous,state,&shifted)==expected);assert(shifted==expectedShift);
}
static void testShiftChords(void){
 uint16_t words[6]={0};KeyboardShiftState state={0};
 /* SHIFT alone can be held without creating repeated modifier presses. */
 words[4]=1u<<6;expectShift(words,0,&state,78,0);expectShift(words,78,&state,78,0);
 /* The inverse function is dispatched before either key is released. */
 words[3]=1u<<3;expectShift(words,78,&state,46,1);expectShift(words,46,&state,46,0);
 /* SIN released first: holding SHIFT cannot re-arm the one-shot modifier. */
 words[3]=0;expectShift(words,46,&state,0,0);expectShift(words,0,&state,0,0);
 words[4]=0;expectShift(words,0,&state,0,0);words[3]=1u<<3;expectShift(words,0,&state,46,0);
 /* Simultaneous presses and SHIFT-first release give the same single event. */
 keyboard_shift_reset(&state);memset(words,0,sizeof(words));words[4]=1u<<6;words[3]=1u<<3;
 expectShift(words,0,&state,46,1);words[4]=0;expectShift(words,46,&state,46,0);
 memset(words,0,sizeof(words));expectShift(words,46,&state,0,0);
 words[3]=1u<<3;expectShift(words,0,&state,46,0);
 /* Repeated functions while the same physical SHIFT remains down are ordinary. */
 keyboard_shift_reset(&state);words[4]=1u<<6;words[3]=1u<<3;expectShift(words,0,&state,46,1);
 words[3]=0;expectShift(words,46,&state,0,0);words[3]=1u<<2;expectShift(words,0,&state,36,0);
 /* Tapping SHIFT still emits its own event; a new physical press is fresh. */
 memset(words,0,sizeof(words));expectShift(words,36,&state,0,0);words[4]=1u<<6;expectShift(words,0,&state,78,0);
 memset(words,0,sizeof(words));expectShift(words,78,&state,0,0);words[4]=1u<<6;expectShift(words,0,&state,78,0);
 /* The editor cancels its software latch on the second tap and consumes it. */
 keyboard_shift_consume(&state);expectShift(words,78,&state,0,0);words[3]=1u<<3;expectShift(words,0,&state,46,0);
 words[3]=0;expectShift(words,46,&state,0,0);memset(words,0,sizeof(words));expectShift(words,0,&state,0,0);
 /* SHIFT pressed after a held primary key is deferred, never a duplicate SIN. */
 words[3]=1u<<3;expectShift(words,0,&state,46,0);words[4]=1u<<6;expectShift(words,46,&state,46,0);
 expectShift(words,46,&state,46,0);words[3]=0;expectShift(words,46,&state,78,0);
 words[3]=1u<<2;expectShift(words,78,&state,36,1);
 /* Releasing a late SHIFT first simply leaves the original key held. */
 keyboard_shift_reset(&state);words[4]=0;words[3]=1u<<3;expectShift(words,0,&state,46,0);
 words[4]=1u<<6;expectShift(words,46,&state,46,0);words[4]=0;expectShift(words,46,&state,46,0);
 /* Unknown three-key chords are rejected; they do not create another SHIFT. */
 keyboard_shift_reset(&state);memset(words,0,sizeof(words));words[4]=1u<<6;expectShift(words,0,&state,78,0);
 words[3]=(1u<<3)|(1u<<2);expectShift(words,78,&state,78,0);expectShift(words,0,&state,0,0);
 words[3]=0;expectShift(words,0,&state,0,0);words[3]=1u<<3;expectShift(words,0,&state,46,1);
 /* MENU wins even over a recognized chord, then a full release resets it. */
 words[4]|=1u<<3;expectShift(words,46,&state,48,0);memset(words,0,sizeof(words));expectShift(words,48,&state,0,0);
 assert(!state.down&&!state.used&&!state.announced&&!state.deferred);
 /* The stateful reader takes the same single six-word snapshot. */
 testWords[4]=1u<<6;testWords[3]=1u<<3;int shifted=0;
 assert(keyboardReadShift(0,&state,&shifted)==46&&shifted);memset(testWords,0,sizeof(testWords));
 assert(!keyboardReadShift(46,&state,&shifted)&&!shifted);
}
int main(void){
 /* Independent physical fixtures: code, KEYSC word, bit. */
 static const int fixtures[][3]={
 {10,0,0},{71,0,14},{61,0,13},{51,0,12},{41,0,11},{31,0,10},
 {72,1,6},{62,1,5},{52,1,4},{42,1,3},{32,1,2},
 {73,1,14},{63,1,13},{53,1,12},{43,1,11},{33,1,10},
 {74,2,6},{64,2,5},{54,2,4},{44,2,3},
 {75,2,14},{65,2,13},{55,2,12},{45,2,11},{35,2,10},{25,2,9},
 {76,3,6},{66,3,5},{56,3,4},{46,3,3},{36,3,2},{26,3,1},
 {77,3,14},{67,3,13},{57,3,12},{47,3,11},{37,3,10},{27,3,9},
 {78,4,6},{68,4,5},{58,4,4},{48,4,3},{38,4,2},{28,4,1},
 {79,4,14},{69,4,13},{59,4,12},{49,4,11},{39,4,10},{29,4,9}};
 assert(sizeof(fixtures)/sizeof(fixtures[0])==50);
 uint16_t words[6]={0};assert(!keyboard_decode(words,0));
 for(unsigned i=0;i<sizeof(fixtures)/sizeof(fixtures[0]);i++){
  memset(words,0,12);words[fixtures[i][1]]=(uint16_t)(1u<<fixtures[i][2]);
  assert(keyboard_decode(words,0)==fixtures[i][0]);assert(keyboard_decode(words,fixtures[i][0])==fixtures[i][0]);
 }
 memset(words,0,12);words[3]=(1<<5)|(1<<3);assert(!keyboard_decode(words,0));assert(keyboard_decode(words,66)==66);
 words[4]=1<<3;assert(keyboard_decode(words,66)==48); /* MENU over a held key. */
 memset(words,0,12);words[5]=1<<11;assert(!keyboard_decode(words,0)); /* No diagnostic keys. */
 memset(words,0,12);words[2]=1<<2;assert(!keyboard_decode(words,34)); /* Unpopulated physical key. */
 /* With no SHIFT, every two-key combination preserves legacy decoding. */
 for(unsigned i=0;i<sizeof(fixtures)/sizeof(fixtures[0]);i++)for(unsigned j=i+1;j<sizeof(fixtures)/sizeof(fixtures[0]);j++){
  if(fixtures[i][0]==78||fixtures[j][0]==78)continue;
  memset(words,0,sizeof(words));words[fixtures[i][1]]|=(uint16_t)(1u<<fixtures[i][2]);words[fixtures[j][1]]|=(uint16_t)(1u<<fixtures[j][2]);
  const int previous[]={0,fixtures[i][0],fixtures[j][0]};
  for(int k=0;k<3;k++){KeyboardShiftState state={0};int shifted=99;assert(keyboard_decode_shift(words,previous[k],&state,&shifted)==keyboard_decode(words,previous[k]));assert(!shifted);}
 }
 testShiftChords();assert(!keyboardRead(0));
 puts("PASS shift-aware CG50 keyboard: simultaneous/held SHIFT chords, both release orders, one-shot consumption, deferred late modifier, toggle cancellation, MENU and legacy two-key equivalence; raw CG50 keyboard: 50 physical keys, word/bit mapping, held keys, MENU priority, chord/diagnostic rejection; read-only KEYSC path without OS key calls");
}
