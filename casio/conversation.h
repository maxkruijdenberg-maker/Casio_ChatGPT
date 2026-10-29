#ifndef CHATGPT_CONVERSATION_H
#define CHATGPT_CONVERSATION_H
#include "layout.h"
#include "store.h"
/* Only row counts and pair offsets are retained. Bodies stay in local storage;
 * callers can load just the pair intersecting the visible viewport. */
#define CONVERSATION_CAPACITY (CHAT_TURNS+1)
typedef struct {int turn,start,qLines,aLines;} ConversationEntry;
typedef struct {
 int count,total;
 ConversationEntry entries[CONVERSATION_CAPACITY];
} ConversationTimeline;
/* part: 0 Jij label, 1 question text, 2 ChatGPT label, 3 answer text, 4 blank.
 * Text row indexes are relative to the respective wrapped message. */
typedef struct {int turn,part,row;} ConversationLine;
static void conversation_init(ConversationTimeline *timeline){
 timeline->count=timeline->total=0;
}
static int conversation_add(ConversationTimeline *timeline,int turn,const char *question,const char *answer,int width){
 ConversationEntry *entry;
 if(timeline->count>=CONVERSATION_CAPACITY||turn<0||!question||!answer||width<1)return 0;
 entry=&timeline->entries[timeline->count];entry->turn=turn;entry->start=timeline->total;
 entry->qLines=line_count(question,width);entry->aLines=line_count(answer,width);
 timeline->total+=entry->qLines+entry->aLines+3;timeline->count++;return 1;
}
static int conversation_line(const ConversationTimeline *timeline,int index,ConversationLine *line){
 int i,row;
 if(index<0||index>=timeline->total||!line)return 0;
 for(i=0;i<timeline->count;i++){
  const ConversationEntry *entry=&timeline->entries[i];
  row=index-entry->start;
  if(row<0||row>=entry->qLines+entry->aLines+3)continue;
  line->turn=entry->turn;line->row=0;
  if(row==0)line->part=0;
  else if(row<=entry->qLines){line->part=1;line->row=row-1;}
  else if(row==entry->qLines+1)line->part=2;
  else if(row<=entry->qLines+entry->aLines+1){line->part=3;line->row=row-entry->qLines-2;}
  else line->part=4;
  return 1;
 }
 return 0;
}
static int conversation_tail(const ConversationTimeline *timeline,int visible){
 if(visible<1)visible=1;
 return timeline->total>visible?timeline->total-visible:0;
}
static int conversation_clamp(const ConversationTimeline *timeline,int first,int visible){
 int max=conversation_tail(timeline,visible);
 if(first<0)return 0;return first>max?max:first;
}
#endif
