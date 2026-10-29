#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../casio/answer_text.h"
#include "../casio/font.h"
static const Glyph *find_glyph(uint32_t cp){
 for(size_t i=0;i<FONT_COUNT;i++)if(font_glyphs[i].code==cp)return &font_glyphs[i];
 return 0;
}
int main(void){
 char out[256];
 assert(answer_display("23 + 42 = 65",out,sizeof(out))&&!strcmp(out,"23 + 42 = 65"));
 assert(answer_display("23 + 42 ＝ 65; x ﹦ 6",out,sizeof(out))&&!strcmp(out,"23 + 42 = 65; x = 6"));
 assert(answer_display("𝑥 − 2 ＝ 4",out,sizeof(out))&&!strcmp(out,"x - 2 = 4"));
 assert(answer_display("\\(2x = 12\\) en \\[x = 6\\]",out,sizeof(out))&&!strcmp(out,"2x = 12 en x = 6"));
 assert(answer_display("x ≠ 4; x ≤ 8; x ≥ 2; x ≡ y",out,sizeof(out))&&!strcmp(out,"x ≠ 4; x ≤ 8; x ≥ 2; x ≡ y"));
 assert(answer_display("Prijs $5; 𝑎＋𝑏＝𝑐",out,sizeof(out))&&!strcmp(out,"Prijs $5; a+b=c"));
 assert(answer_display("ℎ = 4",out,sizeof(out))&&!strcmp(out,"h = 4"));
 /* Previously preserved relation code points still rendered as the replacement
  * '?' because they had no font entry. Verify both text AND the actual bitmap. */
 const char *relations="≈≃≅≔≕≡≝≜≟≢≉";
 assert(answer_display(relations,out,sizeof(out))&&!strcmp(out,relations));
 size_t at=0;uint32_t cp;
 while(relations[at]){
  at+=u_decode(relations+at,strlen(relations+at),&cp);
  const Glyph *g=find_glyph(cp);assert(g);
  assert(memcmp(g->rows,find_glyph(0xfffd)->rows,14));
  assert(memcmp(g->rows,find_glyph('?')->rows,14));
  assert(memcmp(g->rows,find_glyph('=')->rows,14));
  int ink=0;for(int row=0;row<14;row++)ink|=g->rows[row];assert(ink);
 }
 assert(answer_display("x ⁼ 6; x ₌ 6; a ⩵ b; a ⩶ b",out,sizeof(out))&&!strcmp(out,"x = 6; x = 6; a == b; a === b"));
 char pair[3],triple[4];
 assert(answer_display("⩵",pair,sizeof(pair))&&!strcmp(pair,"=="));
 assert(answer_display("⩶",triple,sizeof(triple))&&!strcmp(triple,"==="));
 assert(!answer_display("⩶",pair,sizeof(pair))&&!pair[0]);
 assert(answer_display("Is x = ?; x ≟ 3",out,sizeof(out))&&!strcmp(out,"Is x = ?; x ≟ 3"));
 char original[]="𝑥＝6";assert(answer_display(original,out,sizeof(out))&&!strcmp(original,"𝑥＝6"));
 char small[4];assert(!answer_display("1234",small,sizeof(small))&&!small[0]);
 assert(answer_display("123",small,sizeof(small))&&!strcmp(small,"123"));
 assert(!answer_display("x",out,0));
 puts("PASS answer display: ASCII/full-width/small/super-/subscript equality, native relation glyphs distinct from question/replacement/equality, double/triple equality counts, math letters, Unicode minus/spaces, LaTeX delimiters, relation/currency preservation, unchanged originals and bounded output");
}
