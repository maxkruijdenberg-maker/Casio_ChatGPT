#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../casio/answer_text.h"
int main(void){
 char out[256];
 assert(answer_display("23 + 42 = 65",out,sizeof(out))&&!strcmp(out,"23 + 42 = 65"));
 assert(answer_display("23 + 42 ＝ 65; x ﹦ 6",out,sizeof(out))&&!strcmp(out,"23 + 42 = 65; x = 6"));
 assert(answer_display("𝑥 − 2 ＝ 4",out,sizeof(out))&&!strcmp(out,"x - 2 = 4"));
 assert(answer_display("\\(2x = 12\\) en \\[x = 6\\]",out,sizeof(out))&&!strcmp(out,"2x = 12 en x = 6"));
 assert(answer_display("x ≠ 4; x ≤ 8; x ≥ 2; x ≡ y",out,sizeof(out))&&!strcmp(out,"x ≠ 4; x ≤ 8; x ≥ 2; x ≡ y"));
 assert(answer_display("Prijs $5; 𝑎＋𝑏＝𝑐",out,sizeof(out))&&!strcmp(out,"Prijs $5; a+b=c"));
 assert(answer_display("ℎ = 4",out,sizeof(out))&&!strcmp(out,"h = 4"));
 char original[]="𝑥＝6";assert(answer_display(original,out,sizeof(out))&&!strcmp(original,"𝑥＝6"));
 char small[4];assert(!answer_display("1234",small,sizeof(small))&&!small[0]);
 assert(answer_display("123",small,sizeof(small))&&!strcmp(small,"123"));
 assert(!answer_display("x",out,0));
 puts("PASS answer display: ASCII/full-width/small equality, math letters, Unicode minus/spaces, LaTeX delimiters, relation/currency preservation, unchanged originals and bounded output");
}
