#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../casio/conversation.h"
static void expect(const ConversationTimeline *timeline,int index,int turn,int part,int row){
 ConversationLine line;assert(conversation_line(timeline,index,&line));
 assert(line.turn==turn&&line.part==part&&line.row==row);
}
int main(void){
 ConversationTimeline timeline;ConversationLine line;
 conversation_init(&timeline);assert(!timeline.count&&!timeline.total);
 assert(!conversation_line(&timeline,0,&line)&&!conversation_line(&timeline,-1,&line));
 assert(conversation_tail(&timeline,8)==0&&conversation_clamp(&timeline,42,8)==0);
 assert(!conversation_add(&timeline,-1,"x","y",43));
 assert(!conversation_add(&timeline,0,0,"y",43)&&!conversation_add(&timeline,0,"x",0,43));
 assert(!conversation_add(&timeline,0,"x","y",0)&&!timeline.count);
 /* Three pairs of varied heights form one continuous row address space. */
 assert(conversation_add(&timeline,0,"éπéπéπ","eerste antwoord",4));
 assert(timeline.entries[0].qLines==2&&timeline.entries[0].aLines==4);
 assert(timeline.total==9);expect(&timeline,0,0,0,0);expect(&timeline,1,0,1,0);expect(&timeline,2,0,1,1);
 expect(&timeline,3,0,2,0);expect(&timeline,4,0,3,0);expect(&timeline,7,0,3,3);expect(&timeline,8,0,4,0);
 assert(conversation_add(&timeline,1,"twee","x = 2",4));
 int second=timeline.entries[1].start;assert(second==9);
 expect(&timeline,second,1,0,0);expect(&timeline,second+2,1,2,0);
 expect(&timeline,second+3,1,3,0);expect(&timeline,second+4,1,3,1);expect(&timeline,second+5,1,4,0);
 assert(conversation_add(&timeline,2,"derde\nvraag","regel één\nregel twee",43));
 int third=timeline.entries[2].start;assert(third==15&&timeline.total==22);
 expect(&timeline,third,2,0,0);expect(&timeline,third+1,2,1,0);expect(&timeline,third+2,2,1,1);
 expect(&timeline,third+3,2,2,0);expect(&timeline,third+4,2,3,0);expect(&timeline,third+5,2,3,1);expect(&timeline,third+6,2,4,0);
 /* Every row is mapped, forward and backward, including both boundaries. */
 for(int row=0;row<timeline.total;row++)assert(conversation_line(&timeline,row,&line));
 for(int row=timeline.total-1;row>=0;row--)assert(conversation_line(&timeline,row,&line));
 assert(!conversation_line(&timeline,timeline.total,&line)&&!conversation_line(&timeline,0,0));
 assert(conversation_tail(&timeline,6)==16&&conversation_tail(&timeline,99)==0);
 assert(conversation_clamp(&timeline,-1,6)==0&&conversation_clamp(&timeline,999,6)==16);
 assert(conversation_clamp(&timeline,10,6)==10&&conversation_tail(&timeline,0)==21);
 /* Three visible rows spanning old/new pairs need no previous/next action. */
 expect(&timeline,7,0,3,3);expect(&timeline,8,0,4,0);expect(&timeline,9,1,0,0);
 /* Eight stored pairs plus one unsaved response remain bounded metadata. */
 conversation_init(&timeline);
 for(int i=0;i<CHAT_TURNS+1;i++)assert(conversation_add(&timeline,i,"q","a",43));
 assert(timeline.count==CHAT_TURNS+1&&timeline.total==(CHAT_TURNS+1)*5);
 expect(&timeline,timeline.total-5,CHAT_TURNS,0,0);expect(&timeline,timeline.total-2,CHAT_TURNS,3,0);
 int before=timeline.total;assert(!conversation_add(&timeline,9,"overflow","ignored",43)&&timeline.total==before);
 assert(sizeof(timeline)<256);
 puts("PASS continuous conversation layout: UTF-8 wrapping, question/answer labels and rows, three-pair boundaries, bidirectional history traversal, tail/clamp, eight saved plus one unsaved pair, bounded metadata without concatenated answers.");
 return 0;
}
