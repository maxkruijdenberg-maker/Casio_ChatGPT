#ifndef CHATGPT_INPUT_EDIT_H
#define CHATGPT_INPUT_EDIT_H
#include "text.h"
/* Bounded local text tools. Never store secret-field contents here. */
#define EDIT_CAP 1025
static char editObserved[EDIT_CAP],editUndo[EDIT_CAP],editClipboard[EDIT_CAP];
static size_t editObservedCursor,editUndoCursor;
static int editUndoReady,editOverwrite;
static void edit_begin(const char *text,size_t cursor,int secret){
 editObserved[0]=editUndo[0]=0;editUndoReady=editOverwrite=0;
 editObservedCursor=cursor;if(!secret&&strlen(text)<EDIT_CAP)strcpy(editObserved,text);
}
static void edit_observe(const char *text,size_t cursor,int secret){
 if(secret||strlen(text)>=EDIT_CAP)return;
 if(strcmp(text,editObserved)){
  strcpy(editUndo,editObserved);editUndoCursor=editObservedCursor;editUndoReady=1;strcpy(editObserved,text);
 }
 editObservedCursor=cursor;
}
static int edit_undo(char *text,size_t cap,size_t *cursor){
 if(!editUndoReady||strlen(editUndo)>=cap)return 0;
 /* Swap in place: one-level Undo can also redo without another 1KB buffer. */
 size_t oldLen=strlen(text),undoLen=strlen(editUndo),last=oldLen>undoLen?oldLen:undoLen;
 for(size_t i=0;i<=last;i++){char c=i<=oldLen?text[i]:0;text[i]=i<=undoLen?editUndo[i]:0;editUndo[i]=c;}
 {size_t c=*cursor;*cursor=editUndoCursor;editUndoCursor=c;}
 strcpy(editObserved,text);editObservedCursor=*cursor;return 1;
}
static int edit_copy(const char *text,size_t from,size_t to){
 size_t len=strlen(text),n;if(from>to){size_t p=from;from=to;to=p;}
 if(to>len||from==to||from>=len||(from&&((unsigned char)text[from]&0xc0)==0x80)||(to<len&&((unsigned char)text[to]&0xc0)==0x80))return 0;
 n=to-from;if(n>=EDIT_CAP)return 0;memcpy(editClipboard,text+from,n);editClipboard[n]=0;return 1;
}
static int edit_insert(char *text,size_t cap,size_t *cursor,const char *token,int overwrite){
 size_t len=strlen(text),n=strlen(token),end=*cursor,at=0;
 if(*cursor>len||!n||!u_valid(token,n)||(*cursor<len&&((unsigned char)text[*cursor]&0xc0)==0x80))return 0;
 if(overwrite)while(at<n&&end<len){at=u_next(token,at);end=u_next(text,end);}
 if(len-(end-*cursor)+n>=cap)return 0;
 memmove(text+*cursor+n,text+end,len-end+1);memcpy(text+*cursor,token,n);*cursor+=n;return 1;
}
#endif
