/* Casio fx-CG50 add-in. libfxcg SDK; no sleep implementation.
 * Raw key polling is intentional: native GetKey consumes SHIFT itself, whereas
 * this app must interpret one SHIFT press as a persistent letter-mode toggle.
 * UTF-8 editor + an explicit bitmap repertoire avoid guessing Casio code pages.
 */
#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/serial.h>
#include <fxcg/rtc.h>
#include <fxcg/system.h>
#include "storage_device.h"
#include "library.h"
#include "viewport.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "../atom/ChatGPTAtom/wire.h"
#include "text.h"
#include "font.h"
#include "logo.h"
#include "layout.h"
#include "../atom/ChatGPTAtom/config_validation.h"
#define COLS 44
#define ROWS 8
#define PROMPT_MAX 1024
#define ANSWER_MAX 8192
static char prompt[PROMPT_MAX+1],answer[ANSWER_MAX+1],field[PROMPT_MAX+1],lastQuestion[PROMPT_MAX+1];
static int quitting=0,serialOpen=0,lastKey=0,errors=0,retries=0;
static unsigned char oldSetup=0;
static int heldTick=0,repeatTick=0;
static uint8_t seq=0;
static WParser parser;
static LibraryEntry savedChats[STORE_SLOTS];
static int libraryReady=0,activeChatSlot=-1,contextDirty=1,viewTurn=0,unsavedReply=0;
enum {UI_CHATS=2000};
static int chatSidebar(void);
static int saveReply(void);
static int restoreChat(void);
static void newLocalChat(void);
static int canLeaveReply(void);
static int loadChatPair(int slot,int turn);

static unsigned short *vram;
static ViewProfile viewProfile={100,0,0};
static ViewMap viewMap;
static int viewReady=0;
static const char *const errNames[]={
 "Geen fout","Vul eerst Instellingen in","Wifi niet verbonden: controleer hotspot",
 "Internet of DNS niet bereikbaar","Tijdsynchronisatie mislukt","TLS/HTTPS-verbinding mislukt",
 "API-sleutel ongeldig of geen toegang","API-limiet of onvoldoende tegoed",
 "Model niet beschikbaar","API gaf een fout terug","Antwoord of invoer te groot",
 "Antwoord niet leesbaar","Antwoord-time-out","Atom is nog bezig",
 "Upload mislukt","Atom herstart; vraag niet opnieuw verstuurd",
 "Geheugen of opslag niet beschikbaar","Ongeldig bericht",
 "De API kon deze vraag niet beantwoorden","Hotspot niet gevonden: controleer naam en 2,4 GHz",
 "Wifi-verbinding mislukt: controleer wachtwoord"};
static const char *const phases[]={"Gereed","Wifi verbinden...","Klok synchroniseren...","Model controleren...","Antwoord ophalen...","Antwoord ontvangen","Fout","Geannuleerd"};
static void erase(void *p,size_t n){volatile unsigned char *v=p;while(n--)*v++=0;}
static int elapsed(int start,int ms){return RTC_Elapsed_ms(start,ms);}
static void closeSerial(void){
 if(serialOpen){Serial_ClearTX();Serial_Close(1);serialOpen=0;}
 SetSetupSetting(0x14,oldSetup);
}
static void cleanup(void){closeSerial();storageClose();EnableStatusArea(0);DrawFrame(0xffff);erase(field,sizeof(field));}
/* The menu launch key and the key which opened a dialog must not become a
 * command in the newly opened screen. Require a stable all-keys-up edge. */
static void waitForKeyRelease(void){
 int quiet=0;
 while(quiet<3){if(PRGM_GetKey()==KEY_PRGM_NONE)quiet++;else quiet=0;OS_InnerWait_ms(10);}
 lastKey=0;heldTick=repeatTick=RTC_GetTicks();
}
static int keyEvent(void){
 int k=PRGM_GetKey(),now=RTC_GetTicks();
 if(k==KEY_PRGM_MENU){quitting=1;return k;}
 if(!k){lastKey=0;return 0;}
 if(k!=lastKey){lastKey=k;heldTick=repeatTick=now;return k;}
 if((k==KEY_PRGM_UP||k==KEY_PRGM_DOWN||k==KEY_PRGM_LEFT||k==KEY_PRGM_RIGHT||k==44)&&elapsed(heldTick,450)&&elapsed(repeatTick,100)){
  repeatTick=now;return k;
 }return 0;
}
static int waitKey(void){int k;while(!quitting){k=keyEvent();if(k)return k;OS_InnerWait_ms(10);}return KEY_PRGM_MENU;}
/* RGB565 palette; all geometry is in actual 384x216 display pixels. */
enum {C_BG=0x10a2,C_PANEL=0x2104,C_RAISED=0x2965,C_TEXT=0xef7d,
 C_MUTED=0xa514,C_LINE=0x39c7,C_MINT=0x7737,C_RED=0xfc10};
static int linkKnown=0,linkOnline=0,linkSeen=0;
static void applyView(ViewProfile p){viewProfile=view_valid(p)?p:view_default();view_build(&viewMap,viewProfile);viewReady=1;}
static void physicalRect(int x,int y,int w,int h,unsigned short color){
 int xx,yy;for(yy=y;yy<y+h;yy++)for(xx=x;xx<x+w;xx++)
  if(xx>=0&&xx<384&&yy>=0&&yy<216)vram[yy*384+xx]=color;
}
static void rect(int x,int y,int w,int h,unsigned short color){
 ViewRect r;if(!viewReady)applyView(view_default());
 if(view_rect(&viewMap,x,y,w,h,&r))physicalRect(r.x,r.y,r.w,r.h,color);
}
static void rounded(int x,int y,int w,int h,unsigned short color){
 int row,inset;for(row=0;row<h;row++){
  int edge=row<h/2?row:h-1-row;inset=edge==0?4:edge==1?2:edge==2?1:0;
  rect(x+inset,y+row,w-2*inset,1,color);
 }
}
static void drawLogo(int x,int y,int size,unsigned short color){
 const uint8_t *bits=size==16?logo16:size==40?logo40:logo24;int i;
 for(i=0;i<size*size;i++)if(bits[i/8]&(1<<(7-i%8)))rect(x+i%size,y+i/size,1,1,color);
}
static int glyphIndex(uint32_t cp){size_t i;for(i=0;i<FONT_COUNT;i++)if(font_glyphs[i].code==cp)return (int)i;return -1;}
static void glyph(int x,int y,uint32_t cp,unsigned short color){
 int i=glyphIndex(cp),dx,dy;if(i<0)i=glyphIndex(0xfffd);
 if(color==0)color=C_TEXT;if(color==0x0280)color=C_MINT;
 for(dy=0;dy<14;dy++)for(dx=0;dx<8;dx++)if(x+dx>=0&&x+dx<384&&y+dy>=0&&y+dy<216&&font_glyphs[i].rows[dy]&(1<<(7-dx)))rect(x+dx,y+dy,1,1,color);
}
static void printAt(int x,int y,const char *s,unsigned short color){uint32_t cp;size_t pos=0,len=strlen(s),n;
 while(pos<len&&x<=376){n=u_decode(s+pos,len-pos,&cp);glyph(x,y,cp,color);pos+=n;x+=8;}
}
static void clipped(int x,int y,const char*s,int width,unsigned short color){
 size_t at=0,len=strlen(s),n;int col=0;uint32_t cp;
 while(at<len&&col<width){n=u_decode(s+at,len-at,&cp);
  if(col==width-1&&at+n<len)cp=0x2026;
  glyph(x+8*col,y,cp,color);col++;at+=n;
 }
}
static void clear(const char *title){
 const char *name=title;if(!strncmp(title,"ChatGPT - ",10))name=title+10;
 physicalRect(0,0,384,216,C_BG);rect(0,0,384,32,C_PANEL);
 drawLogo(8,4,24,C_TEXT);clipped(40,9,name,29,C_TEXT);
 if(!strcmp(name,"ChatGPT")||!strcmp(name,"Vraag"))printAt(112,9,"V7",C_MUTED);
 rect(0,31,384,1,C_LINE);
 {int fresh=linkOnline&&!elapsed(linkSeen,30000);
  const char *status=linkKnown?(linkOnline?(fresh?"Atom OK":"Atom ?"):"Offline"):"Atom ?";
  rounded(304,7,6,6,linkKnown&&!linkOnline?C_RED:fresh?C_MINT:C_MUTED);
  printAt(316,7,status,C_MUTED);
 }
}
static void configureDisplay(void){
 Bdisp_EnableColor(1);EnableStatusArea(3);
 Bdisp_DefineDMARange(0,383,0,215);
}
static void show(void){
 /* Do not inherit a cropped OS transfer window from a dialog or main menu.
  * The normal full-display syscall handles the device's framebuffer transfer. */
 configureDisplay();Bdisp_PutDisp_DD();
 /* The outer LCD frame is not part of VRAM. Zooming the canvas cannot fill
  * it; use the OS frame API as well as transferring the complete canvas. */
 DrawFrame(C_BG);
}
static void footer(const char *a,const char*b,const char*c,const char*d,const char*e,const char*f){
 const char *p[6];int i;p[0]=a;p[1]=b;p[2]=c;p[3]=d;p[4]=e;p[5]=f;
 rect(0,192,384,24,C_PANEL);rect(0,192,384,1,C_LINE);
 for(i=0;i<6;i++){
  int n=(int)strlen(p[i]);if(i)rect(i*64,197,1,14,C_LINE);
  if(i==0&&(!strcmp(a,"Stuur")||!strcmp(a,"Opslaan"))){rounded(3,195,58,19,C_MINT);printAt(32-n*4,197,p[i],C_BG);}
  else printAt(i*64+32-n*4,197,p[i],C_TEXT);
 }
}
static void textPage(const char *s,int first,int hide){
 size_t at=0,len=strlen(s),n;uint32_t cp;int row=0,col=0;
 while(at<len){n=u_decode(s+at,len-at,&cp);at+=n;
  if(cp=='\r')continue;if(cp=='\n'){row++;col=0;continue;}
  if(col==COLS){row++;col=0;}
  if(row>=first&&row<first+ROWS)glyph(12+col*8,52+(row-first)*14,hide?'*':cp,C_TEXT);
  col++;
 }
}
static void paragraph(const char *s,int first,int x,int y,int cols,int visible){
 TextLine line;size_t p=0,n;int row=0,col;uint32_t cp;
 while(line_at(s,p,cols,&line)){
  if(row>=first&&row<first+visible){size_t at=line.start;col=0;
   while(at<line.end){n=u_decode(s+at,line.end-at,&cp);at+=n;if(cp=='\r')continue;
    glyph(x+col*8,y+(row-first)*14,cp,C_TEXT);col++;
   }
  }
  row++;p=line.next;if(row>=first+visible)break;
 }
}
static void scrollbar(int x,int y,int height,int first,int visible,int lines){
 int thumb,pos,max=scroll_limit(lines,visible);if(!max)return;
 thumb=height*visible/lines;if(thumb<8)thumb=8;
 pos=(height-thumb)*first/max;rect(x,y,3,height,C_LINE);rect(x,y+pos,3,thumb,C_MUTED);
}
static void cursorPosition(const char *s,size_t cursor,int *row,int *col){
 size_t p=0,n;uint32_t cp;*row=*col=0;
 while(p<cursor){n=u_decode(s+p,cursor-p,&cp);p+=n;
  if(cp=='\n'){(*row)++;*col=0;continue;}if(cp=='\r')continue;
  if(*col==COLS){(*row)++;*col=0;}(*col)++;
 }
 if(*col==COLS){(*row)++;*col=0;}
}
static void message(const char *title,const char *body){
 int first=0,lines=line_count(body,COLS),k,max=scroll_limit(lines,ROWS);
 while(!quitting){clear(title);paragraph(body,first,12,52,COLS,ROWS);scrollbar(376,52,112,first,ROWS,lines);
  if(max)printAt(12,175,"Pijlen: lezen · EXIT: terug",C_MUTED);
  footer("Terug","","","","","");show();k=waitKey();
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)return;
  if(k==KEY_PRGM_DOWN&&first<max)first++;if(k==KEY_PRGM_UP&&first>0)first--;
 }
}
static int confirm(const char *question){waitForKeyRelease();clear("Bevestigen");textPage(question,0,0);footer("Ja","Nee","","","","");show();
 while(!quitting){int k=waitKey();if(k==KEY_PRGM_F1)return 1;if(k==KEY_PRGM_F2||k==KEY_PRGM_EXIT)return 0;}return 0;
}
static int ensureSerial(void){
 unsigned char mode[6]={0,5,0,0,0,0};
 storageClose();if(serialOpen)return 1;
 if(Serial_IsOpen()==1||Serial_Open(mode)!=0)return 0;
 serialOpen=1;Serial_ClearRX();Serial_ClearTX();memset(&parser,0,sizeof(parser));return 1;
}
/* RPC status: 1 success, 0 device error (reply data[0]), -1 timeout,
 * -2 user cancelled/menu. Identical encoded bytes are retried at most 4 times. */
static int rpc(int type,const uint8_t *data,size_t len,WFrame *reply,int cancellable){
 WFrame req;uint8_t bytes[WIRE_ENCODED];size_t n;int attempt,start,k;
 if(len>WIRE_MAX)return -1;
 if(!ensureSerial()){linkKnown=1;linkOnline=0;return -1;}
 req.type=(uint8_t)type;req.seq=++seq;req.len=(uint8_t)len;if(len)memcpy(req.data,data,len);
 n=w_encode(&req,bytes);
 for(attempt=0;attempt<4&&!quitting;attempt++){
  if(attempt)retries++;
  if(Serial_Write(bytes,(int)n)!=0){OS_InnerWait_ms(50);continue;}
  start=RTC_GetTicks();
  while(!elapsed(start,800)&&!quitting){
   while(Serial_PollRX()>0){unsigned char c;WFrame f;int ok;
    if(Serial_ReadSingle(&c)!=0)break;ok=w_feed(&parser,c,&f);if(ok<0)errors++;
    if(ok==1&&f.seq==req.seq&&f.type==(req.type|0x80)&&f.len){linkKnown=linkOnline=1;linkSeen=RTC_GetTicks();*reply=f;return f.data[0]?0:1;}
   }
   k=keyEvent();if(quitting||(cancellable&&k==KEY_PRGM_EXIT))return -2;
   OS_InnerWait_ms(5);
  }
  memset(&parser,0,sizeof(parser));
 }if(!quitting){linkKnown=1;linkOnline=0;}return quitting?-2:-1;
}
static int checkedRpcEx(int type,const uint8_t *data,size_t len,WFrame *reply,int cancellable){
 int ok=rpc(type,data,len,reply,cancellable);
 if(ok==1)return 1;if(quitting||ok==-2)return 0;
 if(ok==0){uint8_t e=reply->data[0];message("Atom meldt",e<sizeof(errNames)/sizeof(errNames[0])?errNames[e]:"Onbekende fout");}
 else message("Geen verbinding","Atom niet bereikbaar. Controleer USB-voeding, TX/RX, GND en UART-instellingen. Er is niets automatisch opnieuw bij de API aangevraagd.");
 return 0;
}
static int checkedRpc(int type,const uint8_t *data,size_t len,WFrame *reply){return checkedRpcEx(type,data,len,reply,0);}
static int uploadText(int target,const char *s){
 size_t len=strlen(s),pos=0,n;uint8_t b[WIRE_MAX];WFrame r;
 b[0]=(uint8_t)target;w_put16(b+1,(uint16_t)len);
 if(!checkedRpcEx(W_BEGIN,b,3,&r,target>=5))return 0;
 while(pos<len&&!quitting){
  if(target>=5&&pos%512<22){char info[64];clear("Gesprek herstellen");sprintf(info,"Context: %u / %u bytes",(unsigned)pos,(unsigned)len);printAt(12,82,info,C_TEXT);printAt(12,112,"EXIT: annuleren · nog geen API-vraag",C_MUTED);show();}
  n=len-pos;if(n>WIRE_MAX-2)n=WIRE_MAX-2;w_put16(b,(uint16_t)pos);memcpy(b+2,s+pos,n);
  if(!checkedRpcEx(W_APPEND,b,n+2,&r,target>=5))return 0;pos+=n;
 }return checkedRpcEx(W_APPLY,0,0,&r,target>=5);
}
/* All printable ASCII plus a named Unicode repertoire. API keys use ASCII only. */
static uint32_t pickCharacter(int asciiOnly){
 static const char extras[]="éëèêáàâäíïìîóöòôúüùûçñÉËπθαβγδμωφ∞√≤≥≠×÷±∑∫∆→←↑↓²³°€";
 uint32_t choices[180];int count=0,i,sel=0,first=0,k;size_t pos=0,len=strlen(extras);uint32_t cp;
 for(i=32;i<=126;i++)choices[count++]=(uint32_t)i;
 if(!asciiOnly)while(pos<len){pos+=u_decode(extras+pos,len-pos,&cp);choices[count++]=cp;}
 while(!quitting){char label[80];clear("Kies een teken");
  sprintf(label,"Teken %d/%d: U+%04lX",sel+1,count,(unsigned long)choices[sel]);printAt(8,35,label,C_MUTED);
  if(sel<first)first=(sel/12)*12;if(sel>=first+96)first=(sel/12-7)*12;
  for(i=first;i<count&&i<first+96;i++){
   int x=8+((i-first)%12)*30,y=51+((i-first)/12)*15;glyph(x,y,choices[i],i==sel?0x0280:0);
   if(i==sel){int x2;for(x2=x;x2<x+16;x2++)rect(x2,y+14,1,1,C_MINT);}
  }
  printAt(8,175,"Pijlen kiezen · EXE invoegen",C_MUTED);footer("Kies","","","","","");show();k=waitKey();
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)return choices[sel];if(k==KEY_PRGM_EXIT)return 0;
  if(k==KEY_PRGM_LEFT&&sel>0)sel--;if(k==KEY_PRGM_RIGHT&&sel<count-1)sel++;
  if(k==KEY_PRGM_UP&&sel>=12)sel-=12;if(k==KEY_PRGM_DOWN&&sel+12<count)sel+=12;
 }return 0;
}
/* Returns F1/F2/F3/F6/EXIT. Secret input is never revealed or retained on Casio. */
static int editor(char *buf,size_t cap,const char *title,int secret,int autoCapital,int asciiOnly,int promptMode){
 size_t cursor=strlen(buf);int letters=0,upper=0,first=0,row,col,k,cp,firstLetter=autoCapital&&cursor==0;
 char label[100];
 while(!quitting){
  cursorPosition(buf,cursor,&row,&col);if(row<first)first=row;if(row>=first+ROWS)first=row-ROWS+1;
  clear(title);rounded(8,35,152,15,C_RAISED);
  sprintf(label,"%s %s",letters?(upper?"ABC":firstLetter?"Abc":"abc"):"123",letters?"SHIFT-lock":"SHIFT: letters");printAt(12,35,label,C_MUTED);
  sprintf(label,"%u/%u bytes",(unsigned)strlen(buf),(unsigned)(cap-1));printAt(240,35,label,C_MUTED);
  rounded(6,50,368,120,C_LINE);rounded(7,51,366,118,C_PANEL);
  if(promptMode==1&&!buf[0]){
   drawLogo(172,62,40,C_TEXT);printAt(104,109,"Waar wil je hulp bij?",C_TEXT);
   printAt(76,135,"Typ een vraag. F1 verstuurt.",C_MUTED);
  }else textPage(buf,first,secret);
  {int x=12+col*8,y=52+(row-first)*14+13;if((promptMode!=1||buf[0])&&y<170)rect(x,y,8,1,C_MINT);}
  if(promptMode==1)footer("Stuur","Nieuw","Test","Tekens","ABC/abc","Instel");else footer("Opslaan","","","Tekens","ABC/abc","");
  printAt(8,175,promptMode==1?"OPTN: chats · EXIT: terug":"DEL: wissen · Pijlen: cursor · EXIT: terug",C_MUTED);show();k=waitKey();
  if(quitting)return KEY_PRGM_MENU;
  if(k==KEY_PRGM_SHIFT){letters=!letters;continue;}
  if(k==KEY_PRGM_ALPHA||k==KEY_PRGM_F5){upper=!upper;continue;}
  if(promptMode==1&&k==KEY_PRGM_OPTN)return UI_CHATS;
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)return KEY_PRGM_F1;
  if(k==KEY_PRGM_EXIT)return k;
  if(promptMode==1&&(k==KEY_PRGM_F2||k==KEY_PRGM_F3||k==KEY_PRGM_F6))return k;
  if(k==KEY_PRGM_LEFT){cursor=u_prev(buf,cursor);continue;}
  if(k==KEY_PRGM_RIGHT){if(cursor<strlen(buf))cursor=u_next(buf,cursor);continue;}
  if(k==44){u_delete(buf,&cursor);if(!buf[0])firstLetter=autoCapital;continue;}
  if(k==34){if(confirm("Dit invoerveld helemaal wissen?")){erase(buf,cap);cursor=0;firstLetter=autoCapital;}continue;}
  if(k==KEY_PRGM_UP){int i;for(i=0;i<COLS&&cursor;i++)cursor=u_prev(buf,cursor);continue;}
  if(k==KEY_PRGM_DOWN){int i;for(i=0;i<COLS&&cursor<strlen(buf);i++)cursor=u_next(buf,cursor);continue;}
  if(k==KEY_PRGM_F4)cp=(int)pickCharacter(asciiOnly);
  else {
   cp=letters?alpha_key(k):number_key(k);
   if(cp>='a'&&cp<='z'&&(upper||firstLetter))cp-=32;
  }
  if(!cp){if(k!=KEY_PRGM_F4)message("Teken niet ondersteund","Gebruik F4 Tekens voor speciale tekens en formules.");continue;}
  if(asciiOnly&&(cp<32||cp>126)){message("Ongeldig teken","Dit veld ondersteunt alleen ASCII.");continue;}
  if(glyphIndex((uint32_t)cp)<0){message("Teken niet ondersteund","Dit invoerteken kan niet worden weergegeven.");continue;}
  if(!u_insert(buf,cap,&cursor,(uint32_t)cp)){message("Invoer vol","De maximale lengte in bytes is bereikt.");continue;}
  if((cp>='a'&&cp<='z')||(cp>='A'&&cp<='Z'))firstLetter=0;
 }return KEY_PRGM_MENU;
}
static char publicSSID[33],publicModel[81];
static unsigned settingsEpoch=0;
static int atomMajor=0;
static int readPublic(int target,char*out,size_t cap){
 uint8_t b[3];size_t at=0,total=0;WFrame r;out[0]=0;
 do {
  b[0]=(uint8_t)target;w_put16(b+1,(uint16_t)at);
  if(rpc(W_GET_CONFIG,b,3,&r,0)!=1||r.len<2)return 0;
  if(!at)total=r.data[1];
  if(r.data[1]!=total||total>=cap||at+r.len-2>total||(r.len==2&&at<total))return 0;
  memcpy(out+at,r.data+2,r.len-2);at+=r.len-2;
 }while(at<total&&!quitting);
 out[at]=0;return !quitting;
}
static int refreshSettings(uint8_t flags[4]){
 WFrame r;clear("Instellingen");printAt(12,72,"Instellingen ophalen...",C_MUTED);show();
 atomMajor=0;if(rpc(W_INFO,0,0,&r,0)!=1||r.len<5)return 0;
 memcpy(flags,r.data+1,4);publicSSID[0]=publicModel[0]=0;atomMajor=1;
 if(rpc(W_DEVICE,0,0,&r,0)==1&&r.len>=4){atomMajor=r.data[1];
  if(r.data[3]&1){if(!readPublic(1,publicSSID,sizeof(publicSSID)))publicSSID[0]=0;
   if(!readPublic(4,publicModel,sizeof(publicModel)))publicModel[0]=0;}
 }
 return linkOnline;
}
static void screenTest(void){
 int k;physicalRect(0,0,384,216,C_BG);rect(0,0,384,2,C_MINT);rect(0,214,384,2,C_MINT);rect(0,0,2,216,C_MINT);rect(382,0,2,216,C_MINT);
 printAt(5,4,"TL",C_TEXT);printAt(361,4,"TR",C_TEXT);printAt(5,198,"BL",C_TEXT);printAt(361,198,"BR",C_TEXT);
 printAt(108,76,"Schermtest 384x216",C_TEXT);printAt(60,106,"Zijn alle vier hoeken zichtbaar?",C_MUTED);printAt(140,144,"EXIT: terug",C_MUTED);show();
 while(!quitting){k=waitKey();if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)return;}
}
static void screenSettings(void){
 ViewProfile original=viewProfile,draft=viewProfile;int selected=0,k,i;char text[64];
 while(!quitting){
  clear("Scherm");rect(0,0,384,1,C_MINT);rect(0,215,384,1,C_MINT);rect(0,0,1,216,C_MINT);rect(383,0,1,216,C_MINT);
  printAt(80,37,"Live voorbeeld",C_MUTED);
  for(i=0;i<3;i++){
   int y=64+i*29;if(selected==i)rounded(64,y-2,256,22,C_RAISED);
   if(i==0)sprintf(text,"Schaal       %d%%",draft.percent);
   if(i==1)sprintf(text,"Horizontaal  %+d px",draft.x);
   if(i==2)sprintf(text,"Verticaal    %+d px",draft.y);
   printAt(76,y,text,selected==i?C_MINT:C_TEXT);
  }
  printAt(52,157,"↑↓ kiezen · ←→ aanpassen",C_MUTED);printAt(64,175,"EXIT: annuleren",C_MUTED);
  footer("Opslaan","Test","100%","","","");show();k=waitKey();
  if(k==KEY_PRGM_EXIT){applyView(original);return;}
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<2)selected++;
  if(k==KEY_PRGM_F3){draft=view_default();applyView(draft);continue;}
  if(k==KEY_PRGM_F2){screenTest();continue;}
  if(k==KEY_PRGM_LEFT||k==KEY_PRGM_RIGHT){
   int delta=k==KEY_PRGM_LEFT?-1:1;ViewProfile next=draft;
   if(selected==0)next.percent+=delta;else if(selected==1)next.x+=delta;else next.y+=delta;
   if(view_valid(next)){draft=next;applyView(draft);}
  }
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN){
   if(view_save(&disk,draft)){storageClose();return;}
   storageClose();message("Niet opgeslagen","Je scherminstelling kon niet worden opgeslagen. Probeer opnieuw of annuleer met EXIT.");
  }
 }
 applyView(original);
}
static void settings(void){
 int selected=0,k,i,known;uint8_t configured[4]={0};WFrame r;
 const char *names[]={"Hotspotnaam","Hotspotwachtwoord","API-sleutel","Model","Taal","Versie","Instellingen wissen"};
 const size_t caps[]={33,64,257,81};
 known=refreshSettings(configured);
 while(!quitting){clear("Instellingen");
  for(i=0;i<7;i++){
   int y=38+i*19;const char *value="";char version[32];
   if(i==selected)rounded(6,y-1,372,18,C_RAISED);
   printAt(14,y,names[i],i==selected?C_MINT:C_TEXT);
   if(i<4){value=known?(configured[i]?"Ingesteld":"Leeg"):"Onbekend";
    if(known&&i==0&&publicSSID[0])value=publicSSID;
    if(known&&i==3&&publicModel[0])value=publicModel;
   }else if(i==4)value="Nederlands";
   else if(i==5){sprintf(version,"7.0 / Atom %s",atomMajor==3?"3.0":atomMajor==2?"2.0":atomMajor==1?"1.x":"?");value=version;}
   clipped(224,y,value,16,C_MUTED);if(i!=4&&i!=5)glyph(364,y,'>',C_MUTED);
  }
  printAt(8,175,known?"F3: vernieuwen · EXIT: terug":"Atom offline · F3: opnieuw verbinden",known?C_MUTED:C_RED);
  footer("Wijzig","","Test","Scherm","","");show();k=waitKey();
  if(k==KEY_PRGM_EXIT)break;
  if(k==KEY_PRGM_F3){known=refreshSettings(configured);continue;}
  if(k==KEY_PRGM_F4){screenSettings();continue;}
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<6)selected++;
  if(k!=KEY_PRGM_F1&&k!=KEY_PRGM_RETURN)continue;
  if(selected==4){message("Taal","De interface en de standaard antwoordtaal zijn Nederlands.");continue;}
  if(selected==5){message("Versie","Casio ChatGPT V7.0\nCompatibel met Atom-firmware V3.0.\n\nVoor V7 hoef je alleen ChatGPT.g3a bij te werken. Gesprekken en Atom-instellingen blijven behouden.");continue;}
  if(selected==6){if(confirm("Hotspotgegevens en API-sleutel op de Atom wissen?")){
   settingsEpoch++;checkedRpc(W_WIPE,(const uint8_t*)"WIPE",4,&r);known=refreshSettings(configured);}
   continue;
  }
  erase(field,sizeof(field));
  if(selected==0&&known)strcpy(field,publicSSID);
  if(selected==3)strcpy(field,known&&publicModel[0]?publicModel:"gpt-4.1-mini");
  do {
   k=editor(field,caps[selected],names[selected],selected==1||selected==2,0,selected==2||selected==3,0);
   if(k!=KEY_PRGM_F1||quitting)break;
   if(!setting_valid(selected+1,field,strlen(field))){
    message("Controleer de invoer",selected==2?"Een API-sleutel mag geen spaties bevatten. Let op hoofdletters en kleine letters.":selected==3?"Modelnaam: gebruik letters, cijfers, punten, streepjes en underscores. De naam mag niet leeg zijn.":"Deze instelling bevat ongeldige tekst.");continue;
   }
   if(uploadText(selected+1,field)){
    settingsEpoch++;erase(field,sizeof(field));known=refreshSettings(configured);
    message("Opgeslagen","De instelling is op de Atom opgeslagen. Je lokale chats blijven bewaard. Voor een vervolgvraag wordt de benodigde context opnieuw hersteld.");break;
   }
   /* Keep failed edits masked in RAM so a UART error does not force retyping. */
  }while(!quitting);
  erase(field,sizeof(field));
 }
}
static void diagnose(void){WFrame r;char text[420];int beforeR=retries,beforeE=errors;
 clear("UART-diagnose");printAt(8,40,"Atom wordt getest...",0);show();
 if(!checkedRpc(W_HELLO,0,0,&r))return;
 {WFrame version;if(rpc(W_DEVICE,0,0,&version,0)==1&&version.len>=4)atomMajor=version.data[1];else atomMajor=1;}
 sprintf(text,"Atom antwoordt op protocol v%u.\n9600 baud, 8N1, geen inversie.\n\nHerhalingen deze test: %d\nCRC/framefouten deze test: %d\n\nEen geldig antwoord controleert UART-verkeer in beide richtingen. Dit controleert geen wifi of API-toegang.",r.len>1?r.data[1]:0,retries-beforeR,errors-beforeE);
 if(atomMajor<3)strcat(text,"\n\nWerk de Atom-firmware bij naar V3.");
 message("UART-diagnose",text);
}
static int fetchAnswer(unsigned int len){
 size_t pos=0;uint8_t b[2];WFrame r;if(len>ANSWER_MAX)return 0;answer[0]=0;
 while(pos<len&&!quitting){w_put16(b,(uint16_t)pos);if(!checkedRpc(W_READ,b,2,&r))return 0;
  if(r.len<2||pos+r.len-1>len){message("Antwoordfout","Ongeldige antwoordlengte ontvangen.");return 0;}
  memcpy(answer+pos,r.data+1,r.len-1);pos+=r.len-1;
 }answer[pos]=0;return !quitting;
}
static void readAnswer(int contextWasDropped){
 int first=0,k;unsigned epoch=settingsEpoch;
 while(!quitting){
  int lines=line_count(answer,43),max=scroll_limit(lines,6);char position[56];
  clear("ChatGPT");rounded(8,37,368,26,C_RAISED);clipped(16,43,lastQuestion,44,C_TEXT);
  drawLogo(12,70,16,C_TEXT);printAt(36,71,"ChatGPT",C_MINT);
  if(activeChatSlot>=0&&savedChats[activeChatSlot].used){sprintf(position,"%d/%d",viewTurn+1,savedChats[activeChatSlot].turns);printAt(288,71,position,C_MUTED);}
  paragraph(answer,first,12,91,43,6);scrollbar(376,91,84,first,6,lines);
  printAt(8,176,unsavedReply?"Nog niet opgeslagen · F3: bewaren":contextWasDropped?"Oudste API-context gewist":activeChatSlot>=0&&viewTurn+1<savedChats[activeChatSlot].turns?"Oud bericht · F1 vervolgt laatste":"SHIFT: vraag · OPTN: chats",unsavedReply?C_RED:C_MUTED);
  footer("Vraag","Nieuw",unsavedReply?"Bewaar":"Test","Chats",viewTurn>0?"Vorig":"",activeChatSlot>=0&&viewTurn+1<savedChats[activeChatSlot].turns?"Volgend":"Instel");show();k=waitKey();
  if(k==KEY_PRGM_F1){prompt[0]=0;return;}
  if(k==KEY_PRGM_EXIT)return;
  if(k==KEY_PRGM_SHIFT){message("Jouw vraag",lastQuestion);continue;}
  if(k==KEY_PRGM_UP&&first>0)first--;if(k==KEY_PRGM_DOWN&&first<max)first++;
  if(k==KEY_PRGM_LEFT){first-=6;if(first<0)first=0;}if(k==KEY_PRGM_RIGHT){first+=6;if(first>max)first=max;}
  if(k==KEY_PRGM_F2&&canLeaveReply()&&confirm("Nieuw gesprek beginnen?")){newLocalChat();return;}
  if(k==KEY_PRGM_F3){if(unsavedReply)saveReply();else diagnose();}
  if(k==KEY_PRGM_F4||k==KEY_PRGM_OPTN){int changed=chatSidebar();if(changed==2)return;if(changed)first=0;}
  if(k==KEY_PRGM_F5&&viewTurn>0&&!unsavedReply){if(loadChatPair(activeChatSlot,viewTurn-1))first=0;}
  if(k==KEY_PRGM_F6){
   if(activeChatSlot>=0&&viewTurn+1<savedChats[activeChatSlot].turns&&!unsavedReply){if(loadChatPair(activeChatSlot,viewTurn+1))first=0;}
   else {settings();if(settingsEpoch!=epoch){contextDirty=1;epoch=settingsEpoch;}}
  }
 }
}

#include "chats.h"
static void ask(void){
 WFrame r;uint8_t id[4];int start=RTC_GetTicks(),ok;uint8_t s,e;unsigned len;
 if(!prompt[0]){message("Geen vraag","Typ eerst een vraag.");return;}
 loadLibrary();
 if(unsavedReply){message("Bewaar het antwoord","Het vorige antwoord is nog niet opgeslagen. Open het gesprek en probeer F3 Bewaar, of verwerp het bewust via Nieuw.");return;}
 if(activeChatSlot>=0&&savedChats[activeChatSlot].turns>=CHAT_TURNS){message("Gesprek vol","Dit gesprek bevat acht vragen en antwoorden. Begin een nieuw gesprek; de oude berichten blijven lokaal bewaard.");return;}
 if(activeChatSlot<0&&freeSlot(savedChats)<0){message("Chats vol","Er zijn acht opgeslagen gesprekken. Verwijder eerst bewust een oud gesprek via OPTN. Er is nog geen API-vraag gestart.");return;}
 if(!restoreChat())return;
 if(!uploadText(0,prompt))return;
 if(!checkedRpc(W_ALLOC,0,0,&r)||r.len!=5)return;memcpy(id,r.data+1,4);
 if(!checkedRpc(W_ASK,id,4,&r))return;
 while(!quitting){
  ok=rpc(W_STATUS,0,0,&r,1);
  if(ok==-2){WFrame ignored;rpc(W_CANCEL,0,0,&ignored,0);if(!quitting)message("Annulering aangevraagd","Een al verstuurde API-aanvraag kan nog kosten maken. De Atom moet de netwerkbewerking eerst afronden.");return;}
  if(ok!=1||r.len!=10){if(!quitting)message("Verbinding onderbroken","De aanvraag kan al verstuurd zijn. Er wordt niet automatisch een nieuwe API-aanvraag gedaan. Controleer de Atom met F3.");return;}
  s=r.data[1];e=r.data[2];len=w_get16(r.data+3);
  if(w_get32(r.data+6)!=w_get32(id)){message("Atom herstart","Aanvraagstatus hoort niet meer bij deze vraag. Er is geen automatische herhaling bij de API.");return;}
  if(s==S_READY){int dropped=(r.data[5]&0x80)!=0;if(fetchAnswer(len)){strcpy(lastQuestion,prompt);prompt[0]=0;unsavedReply=1;saveReply();readAnswer(dropped);}return;}
  if(s==S_ERROR){message("Aanvraag mislukt",e<sizeof(errNames)/sizeof(errNames[0])?errNames[e]:"Onbekende fout");return;}
  if(s==S_CANCELLED){message("Geannuleerd","Er wordt geen antwoord opgehaald.");return;}
  clear("ChatGPT");rounded(8,37,368,26,C_RAISED);clipped(16,43,prompt,44,C_TEXT);
  drawLogo(172,74,40,C_TEXT);printAt(32,124,s<sizeof(phases)/sizeof(phases[0])?phases[s]:"Status onbekend",C_TEXT);
  {int stage;for(stage=1;stage<=4;stage++)rounded(24+(stage-1)*86,151,74,4,s>=stage?C_MINT:C_LINE);}
  printAt(24,175,"EXIT: annuleren · MENU: afsluiten",C_MUTED);show();
  {int wait=RTC_GetTicks();while(!elapsed(wait,250)&&!quitting){int k=keyEvent();if(k==KEY_PRGM_EXIT){WFrame ignored;rpc(W_CANCEL,0,0,&ignored,0);message("Annulering aangevraagd","Een reeds verstuurde API-aanvraag kan nog kosten maken.");return;}OS_InnerWait_ms(10);}}
  if(elapsed(start,150000)){WFrame ignored;rpc(W_CANCEL,0,0,&ignored,0);message("Time-out","Geen antwoord binnen 150 seconden. Er wordt geen nieuwe API-aanvraag gestart.");return;}
 }
}
/* Reset every launch, including warm OS launches which retain static RAM.
 * Reopening is owned by the OS, not by an internal loop around this session. */
static void beginSession(void){
 if(serialOpen)closeSerial();storageClose();
 oldSetup=GetSetupSetting(0x14);
 quitting=0;lastKey=0;heldTick=repeatTick=0;errors=retries=0;seq=0;
 memset(&parser,0,sizeof(parser));memset(savedChats,0,sizeof(savedChats));
 libraryReady=0;activeChatSlot=-1;contextDirty=1;viewTurn=0;unsavedReply=0;
 linkKnown=linkOnline=linkSeen=0;atomMajor=0;atomBootKnown=0;lastAtomBoot=0;settingsEpoch=0;
 prompt[0]=answer[0]=lastQuestion[0]=publicSSID[0]=publicModel[0]=0;
 erase(field,sizeof(field));Bkey_ClrAllFlags();SetSetupSetting(0x14,0);
 SetQuitHandler(cleanup);waitForKeyRelease();
 /* libfxcg/Utilities documents that color mode may need one OS GetKey pass.
  * Consume a synthetic EXIT here, never dispatch it into our own close dialog. */
 {int discarded;Bdisp_EnableColor(1);Keyboard_PutKeycode(-1,-1,KEY_CTRL_EXIT);do {GetKey(&discarded);}while(discarded!=KEY_CTRL_EXIT);}
 Bkey_ClrAllFlags();SetSetupSetting(0x14,0);
 applyView(view_load(&disk));storageClose();configureDisplay();vram=(unsigned short*)GetVRAMAddress();
 loadLibrary();
}
static void runSession(void){
 int k;unsigned epoch;
 for(;;){
  while(!quitting){
   k=editor(prompt,sizeof(prompt),"ChatGPT - Vraag",0,1,0,1);
   if(k==KEY_PRGM_F1)ask();
   else if(k==KEY_PRGM_F2){if(canLeaveReply()&&confirm("Nieuw gesprek beginnen?"))newLocalChat();}
   else if(k==KEY_PRGM_F3)diagnose();
   else if(k==UI_CHATS){int changed=chatSidebar();if(changed==1)readAnswer(0);}
   else if(k==KEY_PRGM_F6){epoch=settingsEpoch;settings();if(epoch!=settingsEpoch)contextDirty=1;}
   else if(k==KEY_PRGM_EXIT){
    if(answer[0])readAnswer(0);else if(confirm("ChatGPT sluiten en terug naar MENU?"))quitting=1;
   }
  }
  if(!unsavedReply)break;
  resumeAfterMenu();if(saveReply()||canLeaveReply()){quitting=1;break;}
 }
}
static void endSession(void){
 /* Keep UART alive until native MENU takes over. In particular, do not send
  * W_CANCEL or closeSerial here: a failed menu handoff must not drop the link.
  * SetQuitHandler releases it when the OS switches applications. */
 storageClose();erase(field,sizeof(field));waitForKeyRelease();
}
static void nativeMainMenu(void){
 /* Call the native menu directly instead of injecting a translated key and
  * assuming GetKey consumed it as a request to leave this add-in. Explicitly
  * enable main-function return, which can be disabled in the inherited OS state.
  * The call can return after drawing MENU: that is not evidence of re-entry.
  * After cleanup, main must return to the OS without drawing another app frame. */
 Bkey_ClrAllFlags();EnableGetkeyToMainFunctionReturn();
 EnableStatusArea(0);DrawFrame(0xffff);
 DisplayMainMenu();
 cleanup();
}

int main(void){
 beginSession();runSession();endSession();
 nativeMainMenu();
 /* Match V1's return to the OS. Do not restart the app when DisplayMainMenu
  * returns; only a new OS launch may call beginSession again. */
 return 0;
}
