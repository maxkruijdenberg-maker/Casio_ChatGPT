#ifndef CHATGPT_LAYOUT_H
#define CHATGPT_LAYOUT_H
#include "text.h"
typedef struct {size_t start,end,next;} TextLine;
/* Word-wrap with a UTF-8-safe hard break for words wider than the display. */
static int line_at(const char*s,size_t start,int width,TextLine *line){
 size_t len=strlen(s),p=start,lastSpace=(size_t)-1,afterSpace=0,n;int col=0;uint32_t cp;
 if(start>=len||width<1)return 0;
 line->start=start;
 while(p<len){
  n=u_decode(s+p,len-p,&cp);
  if(cp=='\n'){line->end=p;line->next=p+n;return 1;}
  if(cp=='\r'){p+=n;continue;}
  if(col>=width){
   if(lastSpace!=(size_t)-1&&lastSpace>start){line->end=lastSpace;line->next=afterSpace;}
   else {line->end=p;line->next=p;}
   return 1;
  }
  if(cp==' '){lastSpace=p;afterSpace=p+n;}
  p+=n;col++;
 }
 line->end=p;line->next=p;return 1;
}
static int line_count(const char*s,int width){
 size_t p=0;TextLine l;int count=0;
 while(line_at(s,p,width,&l)){count++;p=l.next;}
 if(!count)return 1;
 return count;
}
static int scroll_limit(int lines,int visible){return lines>visible?lines-visible:0;}
#endif
