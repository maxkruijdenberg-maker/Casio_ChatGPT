#include <assert.h>
#include <stdio.h>
#include <string.h>
#define CHATGPT_HOST_TEST 1
#include "../casio/keyboard_raw.h"
static void keyboardReadWords(uint16_t words[6]){memset(words,0,12);}
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
 assert(!keyboardRead(0));
 puts("PASS raw CG50 keyboard: 50 physical keys, word/bit mapping, held keys, MENU priority, chord/diagnostic rejection; read-only KEYSC path without OS key calls");
}
