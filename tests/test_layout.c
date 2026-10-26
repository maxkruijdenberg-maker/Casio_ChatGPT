#include <assert.h>
#include <stdio.h>
#include "../casio/layout.h"
int main(void){
 TextLine l;const char *s="Hallo wereld weer";
 assert(line_at(s,0,8,&l)&&l.start==0&&l.end==5&&l.next==6);
 assert(line_at(s,l.next,8,&l)&&l.start==6&&l.end==12&&l.next==13);
 assert(line_count(s,8)==3);assert(line_count("",8)==1);
 assert(line_count("a\n\nb",8)==3);assert(line_count("a\nb",1)==2);
 assert(line_at("éπ😀a",0,2,&l)&&l.end==4&&l.next==4);
 assert(line_at("éπ😀a",4,2,&l)&&l.end==9);
 assert(scroll_limit(30,6)==24&&scroll_limit(3,6)==0);
 /* Every nonempty line makes progress, including narrow Unicode words. */
 const char *cases[]={"xxxxxxxxxxxxxxx","    a   b","éπ😀éπ😀","a\r\nb","\n\n"};
 for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);i++)for(int width=1;width<10;width++){
  size_t at=0;int lines=0;while(line_at(cases[i],at,width,&l)){
   assert(l.next>at);assert(l.end>=l.start);assert(u_valid(cases[i]+l.start,l.end-l.start));at=l.next;
   assert(++lines<100);
  }
 }
 puts("PASS: word wrapping, Unicode boundaries, blank lines, long words and scroll limits.");
}
