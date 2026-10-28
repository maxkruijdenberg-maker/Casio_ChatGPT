/* Casio fx-CG50 add-in. libfxcg SDK; no sleep implementation.
 * Raw key polling is intentional: native GetKey handles modifiers itself, whereas
 * this editor uses ALPHA lock and one-shot SHIFT for math text.
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
#include "native_menu_device.h"
#include "keyboard_raw.h"
#include "models.h"
#include "answer_text.h"
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
static unsigned uartOpens=0,uartCloses=0;
static int rpcDepth=0;
static int autoConnectTick(void);
static void animateLoading(void);
static void paintLoading(int phase);
static int loadingActive=0,loadingPhase=0,loadingAt=0,loadingPurpose=0;
static unsigned char oldSetup=0;
static int heldTick=0,repeatTick=0;
static uint8_t seq=0;
static WParser parser;
static LibraryEntry savedChats[STORE_SLOTS];
static int libraryReady=0,activeChatSlot=-1,contextDirty=1,viewTurn=0,unsavedReply=0;
enum {UI_CHATS=2000,UI_LINK_REFRESH=2001};
static int chatSidebar(void);
static int saveReply(void);
static int restoreChat(void);
static void newLocalChat(void);
static int canLeaveReply(void);
static int loadChatPair(int slot,int turn);
static int requestClose(void);
static void diagnose(void);
static int closePromptActive=0;

static unsigned short *vram;
static ViewProfile viewProfile={100,0,0};
static ViewMap viewMap;
static int viewReady=0;
static const char *const errNames[]={
 "Geen fout","Stel Wifi en account eerst in","Wifi niet verbonden: controleer hotspot",
 "Internet of DNS niet bereikbaar","De klok kan niet worden bijgewerkt","Beveiligde internetverbinding mislukt",
 "API-sleutel ongeldig of geen toegang","Je API-tegoed of gebruikslimiet is bereikt",
 "Kies een ander model via Instellingen","API gaf een fout terug","Antwoord of invoer te groot",
 "Antwoord niet leesbaar","Antwoord-time-out","Atom is nog bezig",
 "Upload mislukt","Atom herstart; vraag niet opnieuw verstuurd",
 "Geheugen of opslag niet beschikbaar","Ongeldig bericht",
 "De API kon deze vraag niet beantwoorden","Hotspot niet gevonden: controleer naam en 2,4 GHz",
 "Wifi-verbinding mislukt: controleer wachtwoord"};

static void erase(void *p,size_t n){volatile unsigned char *v=p;while(n--)*v++=0;}
static int elapsed(int start,int ms){return RTC_Elapsed_ms(start,ms);}
static void resetSerialTransport(void){
 if(serialOpen&&Serial_IsOpen()==1){Serial_ClearTX();Serial_Close(1);uartCloses++;}
 serialOpen=0;memset(&parser,0,sizeof(parser));
}
static void closeSerial(void){
 resetSerialTransport();SetSetupSetting(0x14,oldSetup);
}
static void stopMenuTimer(void);
static void stopAutoProbe(void);
static void cleanup(void){stopAutoProbe();stopMenuTimer();closeSerial();storageClose();EnableStatusArea(0);DrawFrame(0xffff);erase(field,sizeof(field));}
/* The menu launch key and the key which opened a dialog must not become a
 * command in the newly opened screen. Require a stable all-keys-up edge. */
static void waitForKeyRelease(void){
 int quiet=0;
 while(quiet<3){if(keyboardRead(lastKey)==KEY_PRGM_NONE)quiet++;else quiet=0;OS_InnerWait_ms(10);}
 lastKey=0;heldTick=repeatTick=RTC_GetTicks();
}
/* Raw PRGM polling does not consume the OS event FIFO. Drain it only at
 * session boundaries, with native MENU processing disabled for these stale
 * events. Keys are already physically released, so this is bounded work. */
static void drainNativeKeys(void){
 int column,row,n;unsigned short code;
 for(n=0;n<32;n++)if(GetKeyWait_OS(&column,&row,KEYWAIT_HALTOFF_TIMEROFF,0,1,&code)!=KEYREP_KEYEVENT)break;
}
static int keyEvent(void){
 int k=keyboardRead(lastKey),now=RTC_GetTicks();

 if(!k){lastKey=0;return 0;}
 if(k!=lastKey){lastKey=k;heldTick=repeatTick=now;
  if(k==KEY_PRGM_MENU){if(closePromptActive)return 0;return requestClose()?KEY_PRGM_MENU:UI_LINK_REFRESH;}
  return k;
 }
 if((k==KEY_PRGM_UP||k==KEY_PRGM_DOWN||k==KEY_PRGM_LEFT||k==KEY_PRGM_RIGHT||k==44)&&elapsed(heldTick,450)&&elapsed(repeatTick,100)){
  repeatTick=now;return k;
 }return 0;
}
static int waitKey(void){int k;while(!quitting){k=keyEvent();if(k)return k;OS_InnerWait_ms(10);}return KEY_PRGM_MENU;}
/* RGB565 palette; all geometry is in actual 384x216 display pixels. */
enum {C_BG=0xf7be,C_PANEL=0xffff,C_RAISED=0xdf7c,C_TEXT=0x1925,
 C_MUTED=0x5b8d,C_LINE=0xe73c,C_MINT=0x03ce,C_RED=0xb166,
 C_BLUE=0x2b39,C_PURPLE=0x7255,C_GOLD=0x9be2,C_SHADOW=0xe71c,
 C_BLUE_SOFT=0xe75f,C_PURPLE_SOFT=0xef1f,C_GOLD_SOFT=0xffba};
static int linkKnown=0,linkOnline=0,linkSeen=0,linkMisses=0;
static void linkSucceeded(void){linkKnown=linkOnline=1;linkSeen=RTC_GetTicks();linkMisses=0;}
static void linkFailed(void){
 linkKnown=1;if(linkMisses<3){linkMisses++;if(linkMisses==3)resetSerialTransport();}
 if(linkMisses>=3)linkOnline=0;
}
static int autoProbeActive=0,autoProbeStarted=0,autoProbeAt=0;
static uint8_t autoProbeSeq=0;
static WParser autoProbeParser;
static void stopAutoProbe(void){autoProbeActive=0;memset(&autoProbeParser,0,sizeof(autoProbeParser));}
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
  if(inset>(w-1)/2)inset=(w-1)/2;if(inset<0)inset=0;
  rect(x+inset,y+row,w-2*inset,1,color);
 }
}
/* One card language across all selectable lists: shadow, colored focus ring,
 * tinted interior. Geometry stays inside the native 384x216 canvas. */
static void choiceCard(int x,int y,int w,int h,int selected,unsigned short accent,unsigned short tint){
 rounded(x+1,y+2,w,h,C_SHADOW);
 rounded(x,y,w,h,selected?accent:C_LINE);
 rounded(x+1+(selected!=0),y+1+(selected!=0),w-2-2*(selected!=0),h-2-2*(selected!=0),selected?tint:C_PANEL);
}
static void drawLogo(int x,int y,int size,unsigned short color){
 int source=size==16?16:size>=40?40:24,i;
 const uint8_t *bits=source==16?logo16:source==40?logo40:logo24;
 for(i=0;i<source*source;i++)if(bits[i/8]&(1<<(7-i%8))){
  int sx=i%source,sy=i/source;rect(x+sx*size/source,y+sy*size/source,(sx+1)*size/source-sx*size/source,(sy+1)*size/source-sy*size/source,color);
 }
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
static void centered(int y,const char *text,unsigned short color){
 size_t at=0,len=strlen(text);int chars=0;uint32_t cp;
 while(at<len){at+=u_decode(text+at,len-at,&cp);chars++;}
 if(chars>46)chars=46;clipped((384-chars*8)/2,y,text,46,color);
}
static void settingIcon(int x,int y,int type,unsigned short color){
 if(type==0){rect(x+3,y+12,3,5,color);rect(x+9,y+8,3,9,color);rect(x+15,y+3,3,14,color);}
 else if(type==1){rect(x+9,y+2,3,17,color);rect(x+2,y+9,17,3,color);rect(x+5,y+6,11,3,color);rect(x+5,y+12,11,3,color);}
 else if(type==2)glyph(x+6,y+3,'?',color);
 else {rounded(x+2,y+8,4,4,color);rounded(x+9,y+8,4,4,color);rounded(x+16,y+8,4,4,color);}
}
static void clear(const char *title){
 const char *name=title;if(!strncmp(title,"ChatGPT - ",10))name=title+10;
 physicalRect(0,0,384,216,C_BG);rect(0,0,384,32,C_PANEL);
 drawLogo(8,4,24,C_MINT);clipped(40,9,name,29,C_TEXT);
 rect(0,30,384,2,C_LINE);rect(8,30,40,2,C_MINT);
 {
  const char *status=linkOnline?"Verbonden":"Verbroken";
  rounded(292,9,5,5,linkOnline?C_MINT:C_RED);
  printAt(306,7,status,C_MUTED);
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
  int n=(int)strlen(p[i]),step=n>7?7:8,j;unsigned short color=C_TEXT;
  if(n>8)n=8;if(n){rounded(i*64+3,195,58,19,i==5?C_BLUE:C_LINE);rounded(i*64+4,196,56,17,C_PANEL);}
  if(i==0&&(!strcmp(a,"Stuur")||!strcmp(a,"Opslaan")||!strcmp(a,"Verder")||!strcmp(a,"Kies")||!strcmp(a,"Open")||!strcmp(a,"Opnieuw"))){rounded(3,195,58,19,C_MINT);color=C_BG;}
  for(j=0;j<n;j++)glyph(i*64+32-n*step/2+j*step,197,(unsigned char)p[i][j],color);
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
static int confirm(const char *question){
 waitForKeyRelease();
 while(!quitting){
  int k;clear(closePromptActive?"ChatGPT afsluiten?":"Bevestigen");
  paragraph(question,0,12,52,COLS,ROWS);
  footer(closePromptActive?"Sluiten":"Ja",closePromptActive?"Blijven":"Nee","","","","");show();
  k=waitKey();if(k==KEY_PRGM_F1)return 1;if(k==KEY_PRGM_F2||k==KEY_PRGM_EXIT)return 0;
 }return 0;
}
/* MENU is the sole application exit route. Cancelling keeps the caller's
 * screen, typed draft, selection and UART state; the held key is consumed. */
static int requestClose(void){
 int yes;closePromptActive=1;
 yes=confirm("ChatGPT sluiten en naar het Casio-menu? Opgeslagen chats blijven bewaard. Niet-verstuurde invoer wordt bij sluiten gewist. Een verstuurde API-vraag kan nog kosten maken.");
 closePromptActive=0;waitForKeyRelease();if(yes)quitting=1;return yes;
}
static int ensureSerial(void){
 unsigned char mode[6]={0,5,0,0,0,0};
 storageClose();
 if(serialOpen&&Serial_IsOpen()==1)return 1;
 /* The OS may keep the port open across menu/USB transitions, or close it
  * while our cached flag still says open. Reclaim and configure our own UART. */
 serialOpen=0;
 if(Serial_IsOpen()==1){Serial_ClearTX();Serial_Close(1);uartCloses++;OS_InnerWait_ms(20);}
 if(Serial_Open(mode)!=0)return 0;
 uartOpens++;serialOpen=1;Serial_ClearRX();Serial_ClearTX();memset(&parser,0,sizeof(parser));return 1;
}
/* V17 retains the V13 transport and keyboard polling policy. */
/* RPC status: 1 success, 0 device error (reply data[0]), -1 timeout,
 * -2 user cancelled/menu. Identical encoded bytes; metadata tolerates 3 x 1200ms, other commands 4 x 800ms. */
static int rpcInner(int type,const uint8_t *data,size_t len,WFrame *reply,int cancellable){
 WFrame req;uint8_t bytes[WIRE_ENCODED];size_t n;int attempt,start,k;
 int readOnly=type==W_HELLO||type==W_INFO||type==W_DEVICE||type==W_GET_CONFIG||type==W_MODELS;
 int attempts=readOnly?3:4,timeout=readOnly?1200:800;
 stopAutoProbe();
 if(len>WIRE_MAX)return -1;
 if(!ensureSerial()){linkFailed();return -1;}
 req.type=(uint8_t)type;req.seq=++seq;req.len=(uint8_t)len;if(len)memcpy(req.data,data,len);
 n=w_encode(&req,bytes);
 for(attempt=0;attempt<attempts&&!quitting;attempt++){
  if(attempt)retries++;
  if(!ensureSerial()){OS_InnerWait_ms(20);continue;}
  if(Serial_Write(bytes,(int)n)!=0){resetSerialTransport();OS_InnerWait_ms(20);ensureSerial();continue;}
  start=RTC_GetTicks();
  while(!elapsed(start,timeout)&&!quitting){
   while(Serial_PollRX()>0){unsigned char c;WFrame f;int ok;
    if(Serial_ReadSingle(&c)!=0)break;ok=w_feed(&parser,c,&f);if(ok<0)errors++;
    if(ok==1&&f.seq==req.seq&&f.type==(req.type|0x80)&&f.len){linkSucceeded();*reply=f;return f.data[0]?0:1;}
   }
   if(loadingActive&&!closePromptActive)animateLoading();
   k=keyEvent();if(quitting||(cancellable&&(k==KEY_PRGM_EXIT||k==KEY_PRGM_F1)))return -2;
   OS_InnerWait_ms(5);
  }
  memset(&parser,0,sizeof(parser));
 }if(!quitting)linkFailed();return quitting?-2:-1;
}
static int rpc(int type,const uint8_t *data,size_t len,WFrame *reply,int cancellable){int result;rpcDepth++;result=rpcInner(type,data,len,reply,cancellable);rpcDepth--;return result;}
static int checkedRpcEx(int type,const uint8_t *data,size_t len,WFrame *reply,int cancellable){
 int ok=rpc(type,data,len,reply,cancellable);
 if(ok==1)return 1;if(quitting||ok==-2)return 0;
 if(ok==0){uint8_t e=reply->data[0];message("Atom meldt",e<sizeof(errNames)/sizeof(errNames[0])?errNames[e]:"Onbekende fout");}
 else message("Geen verbinding","Controleer of de Atom stroom krijgt en goed is aangesloten. Probeer daarna opnieuw. Er is geen nieuwe betaalde vraag verstuurd.");
 return 0;
}
static int checkedRpc(int type,const uint8_t *data,size_t len,WFrame *reply){return checkedRpcEx(type,data,len,reply,0);}
static int uploadText(int target,const char *s){
 size_t len=strlen(s),pos=0,n;uint8_t b[WIRE_MAX];WFrame r;
 b[0]=(uint8_t)target;w_put16(b+1,(uint16_t)len);
 if(!checkedRpcEx(W_BEGIN,b,3,&r,target>=5))return 0;
 while(pos<len&&!quitting){
  if(target>=5&&pos%512<22&&!loadingActive){clear("Gesprek openen");printAt(12,82,"Je gesprek wordt klaargezet...",C_TEXT);printAt(12,112,"EXIT: annuleren · nog geen API-vraag",C_MUTED);show();}
  n=len-pos;if(n>WIRE_MAX-2)n=WIRE_MAX-2;w_put16(b,(uint16_t)pos);memcpy(b+2,s+pos,n);
  if(!checkedRpcEx(W_APPEND,b,n+2,&r,target>=5))return 0;pos+=n;
 }return checkedRpcEx(W_APPLY,0,0,&r,target>=5);
}
/* Nonblocking, free UART heartbeat. It never calls keyEvent or an API command.
 * Foreground RPC takes ownership by stopping this probe before sending. */
static int autoConnectTick(void){
 WFrame frame;unsigned char bytes[WIRE_ENCODED];size_t n;
 if(quitting)return 0;
 if(autoProbeActive){
  int limit=2*WIRE_ENCODED;
  while(limit--&&Serial_PollRX()>0){
   unsigned char c;int ok;if(Serial_ReadSingle(&c)!=0)break;
   ok=w_feed(&autoProbeParser,c,&frame);if(ok<0)errors++;
   if(ok==1&&frame.type==(W_HELLO|0x80)&&frame.seq==autoProbeSeq&&frame.len>=2){
    if(frame.data[0]==E_NONE&&frame.data[1]==1)linkSucceeded();else linkFailed();
    stopAutoProbe();return 1;
   }
  }
  if(!elapsed(autoProbeAt,1200))return 0;
  stopAutoProbe();linkFailed();return 1;
 }
 if(autoProbeStarted&&!elapsed(autoProbeAt,linkOnline&&!linkMisses?10000:1000))return 0;
 autoProbeStarted=1;autoProbeAt=RTC_GetTicks();
 if(!ensureSerial()){linkFailed();return 1;}
 memset(&frame,0,sizeof(frame));frame.type=W_HELLO;frame.seq=autoProbeSeq=++seq;
 memset(&autoProbeParser,0,sizeof(autoProbeParser));n=w_encode(&frame,bytes);
 autoProbeActive=1;
 if(Serial_Write(bytes,(int)n)!=0){stopAutoProbe();resetSerialTransport();linkFailed();return 1;}
 return 1;
}
static int waitEditorKey(int promptMode){
 int k;while(!quitting){
  int online=linkOnline;
  if(promptMode==1&&!rpcDepth&&!closePromptActive)autoConnectTick();
  int refresh=online!=linkOnline;
  k=keyEvent();if(k)return k;
  if(refresh)return UI_LINK_REFRESH;
  OS_InnerWait_ms(10);
 }return KEY_PRGM_MENU;
}
/* All printable ASCII plus a named Unicode repertoire. API keys use ASCII only. */
static uint32_t pickCharacter(int asciiOnly){
 static const char extras[]="éëèêáàâäíïìîóöòôúüùûçñÉËπθαβγδμωφ∞√≤≥≠×÷±∑∫∆→←↑↓²³°€";
 uint32_t choices[180];int count=0,i,sel=0,first=0,k;size_t pos=0,len=strlen(extras);uint32_t cp;
 choices[count++]='=';
 for(i=32;i<=126;i++)if(i!='=')choices[count++]=(uint32_t)i;
 if(!asciiOnly)while(pos<len){pos+=u_decode(extras+pos,len-pos,&cp);choices[count++]=cp;}
 while(!quitting){char label[80];clear("Kies een teken");
  sprintf(label,"Teken %d van %d",sel+1,count);printAt(8,35,label,C_MUTED);
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
/* Shared paint path: the OS saves this neutral input page before opening MENU,
 * so resuming cannot first restore the old "Ja/Nee" closing dialog. */
static void renderEditor(const char *buf,size_t cap,size_t cursor,const char *title,int secret,int promptMode,
 int letters,int upper,int shifted,int firstLetter,int first){
 int row,col;char label[100];cursorPosition(buf,cursor,&row,&col);
 clear(title);rounded(8,35,152,15,C_RAISED);
 sprintf(label,"%s %s%s",letters?(upper?"ABC":firstLetter?"Abc":"abc"):"123",letters?"ALPHA aan":"ALPHA: abc",shifted?" S":"");printAt(12,35,label,C_MUTED);
 (void)cap;
 choiceCard(6,50,368,120,0,C_MINT,C_RAISED);
 if(promptMode==1&&!buf[0]){
  rounded(160,54,64,48,C_RAISED);drawLogo(172,58,40,C_MINT);
  centered(107,activeChatSlot>=0?"We gaan verder.":"Waar wil je hulp bij?",C_TEXT);
  centered(128,activeChatSlot>=0?"Vraag door, op jouw tempo.":"Een goed idee begint met een vraag.",C_MUTED);
  centered(149,"Typ je vraag en kies Stuur.",C_MINT);
 }else textPage(buf,first,secret);
 {int x=12+col*8,y=52+(row-first)*14+13;if((promptMode!=1||buf[0])&&y<170)rect(x,y,8,1,C_MINT);}
 if(promptMode==1)footer("Stuur","Nieuw","Chats","Tekens","ABC/abc","Instel");else footer("Opslaan","","","Tekens","ABC/abc","");
 printAt(8,175,promptMode==1?"OPTN: chats · MENU: afsluiten":"DEL: wissen · Pijlen: cursor · EXIT: terug",C_MUTED);show();
}
static void prepareMenuSnapshot(void){
 int known=linkKnown,online=linkOnline,seen=linkSeen,active=autoProbeActive;
 linkKnown=1;linkOnline=0;linkSeen=0;autoProbeActive=1;
 renderEditor("",PROMPT_MAX+1,0,"ChatGPT - Vraag",0,1,0,0,0,1,0);
 linkKnown=known;linkOnline=online;linkSeen=seen;autoProbeActive=active;
}
/* Returns F1/F2/F6/EXIT. Secret input is never revealed or retained on Casio. */
static int editor(char *buf,size_t cap,const char *title,int secret,int autoCapital,int asciiOnly,int promptMode){
 size_t cursor=strlen(buf);int letters=0,upper=0,shifted=0,first=0,row,col,k,cp,useShift,firstLetter=autoCapital&&cursor==0;
 while(!quitting){
  const char *token=0;
  cursorPosition(buf,cursor,&row,&col);if(row<first)first=row;if(row>=first+ROWS)first=row-ROWS+1;
  renderEditor(buf,cap,cursor,title,secret,promptMode,letters,upper,shifted,firstLetter,first);
  k=waitEditorKey(promptMode);if(quitting)return KEY_PRGM_MENU;if(k==UI_LINK_REFRESH)continue;
  if(k==KEY_PRGM_SHIFT){shifted=!shifted;continue;}
  if(k==KEY_PRGM_ALPHA){letters=!letters;shifted=0;continue;}
  useShift=shifted;shifted=0;
  if(k==KEY_PRGM_F5){upper=!upper;continue;}
  if(promptMode==1&&(k==KEY_PRGM_OPTN||k==KEY_PRGM_F3))return UI_CHATS;
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)return KEY_PRGM_F1;
  if(k==KEY_PRGM_EXIT)return k;
  if(promptMode==1&&(k==KEY_PRGM_F2||k==KEY_PRGM_F6))return k;
  if(k==KEY_PRGM_LEFT){cursor=useShift?0:u_prev(buf,cursor);continue;}
  if(k==KEY_PRGM_RIGHT){if(useShift)cursor=strlen(buf);else if(cursor<strlen(buf))cursor=u_next(buf,cursor);continue;}
  if(k==44){u_delete(buf,&cursor);if(!buf[0])firstLetter=autoCapital;continue;}
  if(k==KEY_PRGM_ACON){if(confirm("Dit invoerveld helemaal wissen?")){erase(buf,cap);cursor=0;firstLetter=autoCapital;}continue;}
  if(k==KEY_PRGM_UP){int i;for(i=0;i<COLS&&cursor;i++)cursor=u_prev(buf,cursor);continue;}
  if(k==KEY_PRGM_DOWN){int i;for(i=0;i<COLS&&cursor<strlen(buf);i++)cursor=u_next(buf,cursor);continue;}
  if(useShift&&k==61)cp='=';
  else if(k==KEY_PRGM_F4)cp=(int)pickCharacter(asciiOnly);
  else {
   if(useShift||(!letters&&promptMode==1))token=math_key(k,useShift);
   if(token){
    size_t at=0,len=strlen(token),n;uint32_t ch;int valid=1;
    while(at<len){n=u_decode(token+at,len-at,&ch);if((asciiOnly&&(ch<32||ch>126))||glyphIndex(ch)<0){valid=0;break;}at+=n;}
    if(!valid){message("Ongeldig teken","Dit veld ondersteunt alleen de beschikbare ASCII-tekens.");continue;}
    if(!u_insert_text(buf,cap,&cursor,token)){message("Invoer vol","De functie past niet meer in dit invoerveld.");continue;}
    for(at=0;at<len;at++)if((token[at]>='a'&&token[at]<='z')||(token[at]>='A'&&token[at]<='Z'))firstLetter=0;
    continue;
   }
   cp=letters&&!useShift?alpha_key(k):number_key(k);
   if(cp>='a'&&cp<='z'&&(upper||firstLetter))cp-=32;
  }
  if(!cp){if(k!=KEY_PRGM_F4)message("Teken niet ondersteund","Gebruik F4 Tekens voor speciale tekens en formules.");continue;}
  if(asciiOnly&&(cp<32||cp>126)){message("Ongeldig teken","Dit veld ondersteunt alleen ASCII.");continue;}
  if(glyphIndex((uint32_t)cp)<0){message("Teken niet ondersteund","Dit invoerteken kan niet worden weergegeven.");continue;}
  if(!u_insert(buf,cap,&cursor,(uint32_t)cp)){message("Invoer vol","Dit veld is vol. Verwijder tekst om verder te typen.");continue;}
  if((cp>='a'&&cp<='z')||(cp>='A'&&cp<='Z'))firstLetter=0;
 }return KEY_PRGM_MENU;
}
static char publicSSID[33],publicModel[81];
static unsigned settingsEpoch=0;
static char remoteModels[32768];
static unsigned short modelOffsets[384];
static int remoteModelCount=0;
static int fetchModels(void);
static int atomMajor=0,settingsKnown=0;
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
 if(rpc(W_INFO,0,0,&r,0)!=1||r.len<5)return 0;
 settingsKnown=1;memcpy(flags,r.data+1,4);if(!atomMajor)atomMajor=1;
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
  footer("Opslaan","Hoeken","100%","","","");show();k=waitKey();
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
/* Phone setup is opt-in. Status polling never sends a paid request. */
static void phoneSetup(void){
 WFrame r;uint8_t action=1;int tick,k;char network[32],password[13];
 if(!linkOnline&&!checkedRpc(W_HELLO,0,0,&r))return;
 if(!atomMajor){if(rpc(W_DEVICE,0,0,&r,0)!=1||r.len<4){message("Geen verbinding","De Atom reageert nog niet. Controleer de aansluiting en probeer opnieuw.");return;}atomMajor=r.data[1];}
 if(atomMajor<11){message("Atom-update nodig","Werk de Atom bij met de meegeleverde firmware om via je telefoon in te stellen. De bestaande instellingen blijven behouden.");return;}
 if(!checkedRpc(W_SETUP,&action,1,&r)||r.len!=17)return;
 settingsEpoch++;tick=RTC_GetTicks();
 while(!quitting){
  if(r.data[1]==1){
   sprintf(network,"ChatGPT-%04X",w_get16(r.data+3));memcpy(password,r.data+5,12);password[12]=0;
   clear("Installeren via telefoon");
   printAt(12,40,"1. Verbind je telefoon met wifi:",C_TEXT);printAt(24,60,network,C_MINT);
   printAt(12,84,"Wachtwoord:",C_MUTED);printAt(144,84,password,C_TEXT);
   printAt(12,108,"2. Open in je browser:",C_TEXT);printAt(24,128,"http://192.168.4.1",C_MINT);
   printAt(12,152,"3. Vul wifi in en plak je API-sleutel.",C_TEXT);
   printAt(12,176,"Geen internet op dit netwerk is normaal.",C_MUTED);
  }else{
   clear("Installeren via telefoon");
   printAt(12,60,r.data[1]==3?"Verbonden - klaar voor gebruik":r.data[1]==2?"Opgeslagen - verbinden met wifi...":r.data[1]==4?"Wifi verbinden niet gelukt":"Installatie gesloten",r.data[1]==4?C_RED:C_MINT);
   printAt(12,100,r.data[1]==3?"API-sleutel wordt bij je vraag getest.":r.data[1]==4?"Controleer wifi-naam en wachtwoord.":"Je bestaande chats blijven bewaard.",C_MUTED);
   printAt(12,128,"Opslaan verstuurt geen API-vraag.",C_MUTED);
  }
  footer("Terug","","","","","");show();
  do{k=keyEvent();if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT||k==KEY_PRGM_F1||quitting)break;OS_InnerWait_ms(10);}while(!elapsed(tick,1000));
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT||k==KEY_PRGM_F1||quitting)break;
  action=0;if(rpc(W_SETUP,&action,1,&r,0)!=1||r.len!=17)break;tick=RTC_GetTicks();
 }
 {int leaving=quitting;quitting=0;action=2;rpc(W_SETUP,&action,1,&r,0);quitting=leaving;}erase(password,sizeof(password));
}
static int choiceCount(void){return remoteModelCount?remoteModelCount:MODEL_COUNT;}
static const char *choiceId(int index,char out[81]){
 if(!remoteModelCount)return modelChoices[index].id;
 size_t at=modelOffsets[index],n=0;while(remoteModels[at]&&remoteModels[at]!='\n'&&n<80)out[n++]=remoteModels[at++];out[n]=0;return out;
}
static int parseModelList(unsigned total){
 unsigned at=0,n=0,start=0;int count=0;remoteModelCount=0;
 if(!total||total>=sizeof(remoteModels))return 0;
 while(at<total){unsigned char c=remoteModels[at++];
  if(c=='\n'){if(!n||count>=384||!setting_valid(4,remoteModels+start,n))return 0;modelOffsets[count++]=(unsigned short)start;start=at;n=0;}
  else if(c<33||c>126||++n>80)return 0;
 }
 if(n||!count)return 0;remoteModels[total]=0;remoteModelCount=count;return 1;
}
static int modelFetchCancelled=0,modelFetchError=0;
static int modelRpc(const uint8_t *b,size_t n,WFrame *r){
 int ok=rpc(W_MODELS,b,n,r,1);if(ok==-2)modelFetchCancelled=1;else if(ok!=1)modelFetchError=ok==0?r->data[0]:E_TIMEOUT;return ok;
}
static int fetchModelsWork(void){
 WFrame r;uint8_t b[3]={0};int started=RTC_GetTicks();unsigned total,pos=0;
 if(modelRpc(b,1,&r)!=1||r.len!=5)return 0;
 if(r.data[1]!=1){b[0]=1;if(modelRpc(b,1,&r)!=1||r.len!=5)return 0;}
 while(r.data[1]==1&&!quitting){
  int at=RTC_GetTicks();while(!elapsed(at,200)&&!quitting){
   animateLoading();int k=keyEvent();if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F1){modelFetchCancelled=1;return 0;}OS_InnerWait_ms(10);
  }
  if(elapsed(started,90000))return 0;b[0]=0;if(modelRpc(b,1,&r)!=1||r.len!=5)return 0;
 }
 if(quitting)return 0;
 if(r.data[1]!=2){modelFetchError=r.data[2];return 0;}
 total=w_get16(r.data+3);if(!total||total>=sizeof(remoteModels))return 0;
 /* Do not retain partial lists after a failed refresh. */
 remoteModelCount=0;
 while(pos<total&&!quitting){
  b[0]=2;w_put16(b+1,(uint16_t)pos);
  if(modelRpc(b,3,&r)!=1||r.len<2||pos+r.len-1>total)return 0;
  memcpy(remoteModels+pos,r.data+1,r.len-1);pos+=r.len-1;
 }
 return !quitting&&parseModelList(total);
}
static int fetchModels(void){
 int ok;modelFetchCancelled=modelFetchError=0;loadingPurpose=1;loadingActive=1;loadingPhase=0;loadingAt=RTC_GetTicks();paintLoading(0);
 ok=fetchModelsWork();loadingActive=0;
 if(!ok&&!quitting&&!modelFetchCancelled)message("Modellen niet geladen",modelFetchError>0&&modelFetchError<(int)(sizeof(errNames)/sizeof(errNames[0]))?errNames[modelFetchError]:"Controleer Wifi en account en probeer opnieuw. Je huidige model is niet gewijzigd.");
 return ok;
}
static void paintModels(int selected){
 int i,first=(selected/5)*5,count=choiceCount();char id[81],label[64];clear("Jouw modellen");
 sprintf(label,"%d modellen · %s",count,remoteModelCount?"jouw account":"startkeuzes");printAt(14,38,label,C_MUTED);
 for(i=first;i<count&&i<first+5;i++){
  int y=61+(i-first)*23;const char *name=choiceId(i,id);
  choiceCard(8,y-3,368,21,i==selected,C_BLUE,C_BLUE_SOFT);
  clipped(18,y,name,40,i==selected?C_BLUE:C_TEXT);
  if(!strcmp(publicModel,name))rounded(362,y+4,5,5,C_MINT);
 }
 printAt(14,177,"↑↓ kiezen · EXE opslaan",C_MUTED);
 footer("Kies","Ververs","Vorig","Info","Volgend","Terug");show();
}
static int modelPicker(void){
 int selected=0,k;char id[81],details[520];
 if(!atomMajor){WFrame r;if(rpc(W_DEVICE,0,0,&r,0)==1&&r.len>=4)atomMajor=r.data[1];}
 if(atomMajor>=17&&!remoteModelCount&&!fetchModels()&&modelFetchCancelled)return 0;
 for(int i=0;i<choiceCount();i++)if(!strcmp(publicModel,choiceId(i,id)))selected=i;
 while(!quitting){
  if(selected>=choiceCount())selected=0;
  paintModels(selected);k=waitEditorKey(1);if(quitting)return 0;
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT||k==KEY_PRGM_F6)return 0;
  if(k==KEY_PRGM_UP&&selected>0)selected--;
  if(k==KEY_PRGM_DOWN&&selected<choiceCount()-1)selected++;
  if(k==KEY_PRGM_F5){selected+=5;if(selected>=choiceCount())selected=choiceCount()-1;}
  if(k==KEY_PRGM_F3){selected-=5;if(selected<0)selected=0;}
  if(k==KEY_PRGM_F2){
   if(atomMajor<17)message("Atom bijwerken","Upload de meegeleverde Atom-firmware om de modellen uit je eigen account op te halen. Je wifi en sleutel blijven bewaard.");
   else if(!fetchModels()&&modelFetchCancelled)return 0;
   continue;
  }
  if(k==KEY_PRGM_F4){
   const char *name=choiceId(selected,id);int preset=modelChoiceIndex(name);
   sprintf(details,"%s\n\n%s\n\n%s",name,preset>=0?modelChoices[preset].description:"Dit model staat in je API-account.",preset>=0?"Kosten zijn relatief; controleer actuele API-prijzen.":"Kosten verschillen per model. Controleer de API-prijzen. Niet elk model werkt met deze app. Kies bij een fout een ander model.");
   message("Over dit model",details);
  }
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN||k==KEY_PRGM_RIGHT){
   const char *name=choiceId(selected,id);if(!strcmp(publicModel,name))return 1;
   if(uploadText(4,name)){strcpy(publicModel,name);settingsEpoch++;return 1;}
  }
 }
 return 0;
}
static void deviceSettings(void){
 int selected=0,k,i;WFrame reply;const char *names[]={"Scherm aanpassen","Over deze app","Wifi en sleutel wissen"};
 while(!quitting){
  clear("Meer instellingen");
  for(i=0;i<3;i++){int y=58+i*34;choiceCard(8,y-4,368,28,i==selected,C_GOLD,C_GOLD_SOFT);printAt(18,y,names[i],i==selected?C_GOLD:C_TEXT);glyph(362,y,'>',C_MUTED);}
  footer("Open","","","","","Terug");show();k=waitKey();
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F6||k==KEY_PRGM_LEFT)return;
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<2)selected++;
  if(k!=KEY_PRGM_F1&&k!=KEY_PRGM_RETURN&&k!=KEY_PRGM_RIGHT)continue;
  if(selected==0)screenSettings();
  if(selected==1)message("Over deze app","ChatGPT voor Casio fx-CG50\nVersie 18.0\n\nChats staan op je Casio. Wifi en je sleutel staan op de Atom.\n\nEigen API-tegoed en internet zijn nodig. AI kan fouten maken.");
  if(selected==2&&confirm("Wifi en API-sleutel wissen? Je opgeslagen chats blijven behouden.")){
   if(checkedRpc(W_WIPE,(const uint8_t*)"WIPE",4,&reply)){settingsEpoch++;settingsKnown=0;remoteModelCount=0;publicSSID[0]=publicModel[0]=0;message("Gegevens gewist","Open Wifi en account om opnieuw in te stellen.");}
  }
 }
}
static void helpAndDevice(void){
 int selected=0,k,i;const char *names[]={"Aan de slag","Typen en toetsen","Antwoorden en kosten","Verbinding controleren"};
 const char *body[]={
 "1. Open Wifi en account.\n2. Volg de stappen op je telefoon.\n3. Kies een model.\n4. Typ je vraag en kies Stuur.\n\nJe hebt eigen OpenAI API-tegoed nodig. Een ChatGPT-abonnement bevat dit niet.",
 "ALPHA zet letters aan of uit. F5 wisselt hoofdletters. SHIFT werkt een keer voor wiskundefuncties. F4 opent speciale tekens.\n\nEXIT gaat terug. MENU vraagt om afsluiten. Tijdens laden annuleert EXIT of F1 het wachten.",
 "F1 Verder stelt een vervolgvraag. Chats blijven lokaal op je Casio.\n\nAPI-gebruik kost geld via je eigen account. Controleer prijzen en antwoorden. Een al verstuurde vraag kan ook na annuleren kosten maken."
 };
 while(!quitting){
  clear("Hulp");
  for(i=0;i<4;i++){int y=48+i*31;choiceCard(8,y-4,368,27,i==selected,C_PURPLE,C_PURPLE_SOFT);printAt(18,y,names[i],i==selected?C_PURPLE:C_TEXT);glyph(362,y,'>',C_MUTED);}
  footer("Open","","","","","Terug");show();k=waitKey();
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT||k==KEY_PRGM_F6)return;
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<3)selected++;
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN||k==KEY_PRGM_RIGHT){if(selected<3)message(names[selected],body[selected]);else diagnose();}
 }
}
static void paintSettings(int selected){
 int i;const char *names[]={"Wifi en account","Model","Hulp","Meer"};
 const char *notes[]={publicSSID[0]?publicSSID:"Instellen via telefoon",publicModel[0]?publicModel:"Kies je model","Zo werkt de app","Scherm en appinformatie"};
 clear("Instellingen");
 for(i=0;i<4;i++){
  static const unsigned short accent[4]={C_MINT,C_BLUE,C_PURPLE,C_GOLD};
  static const unsigned short tint[4]={C_RAISED,C_BLUE_SOFT,C_PURPLE_SOFT,C_GOLD_SOFT};
  int y=43+i*35;choiceCard(8,y-3,368,32,i==selected,accent[i],tint[i]);
  rounded(17,y+1,29,25,tint[i]);settingIcon(20,y+3,i,accent[i]);
  printAt(56,y,names[i],i==selected?accent[i]:C_TEXT);clipped(56,y+14,notes[i],36,C_MUTED);glyph(362,y+7,'>',accent[i]);
 }
 footer("Open","","","","Hulp","Terug");show();
}
static void settings(void){
 int selected=0,k;uint8_t configured[4]={0};
 if(!settingsKnown&&linkOnline)refreshSettings(configured);
 while(!quitting){
  paintSettings(selected);k=waitEditorKey(1);
  if(k==UI_LINK_REFRESH){if(linkOnline&&!settingsKnown)refreshSettings(configured);continue;}
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F6||k==KEY_PRGM_LEFT)return;
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<3)selected++;
  if(k==KEY_PRGM_F5){helpAndDevice();continue;}
  if(k!=KEY_PRGM_F1&&k!=KEY_PRGM_RETURN&&k!=KEY_PRGM_RIGHT)continue;
  if(selected==0){phoneSetup();remoteModelCount=0;if(!quitting)refreshSettings(configured);}
  if(selected==1)modelPicker();
  if(selected==2)helpAndDevice();
  if(selected==3){deviceSettings();if(!settingsKnown&&linkOnline)refreshSettings(configured);}
 }
}
static void paintDiagnosis(int successes,int checked){
 const char *title=successes==checked&&successes?"Verbinding goed":successes?"Verbinding wisselt":"Geen verbinding";
 const char *body=successes==checked&&successes?"Je Casio en Atom reageren goed op elkaar. Je kunt verder met je vraag.":successes?"Het contact valt soms weg. Controleer of je Atom goed is aangesloten en probeer opnieuw.":"Controleer of de Atom stroom krijgt en goed is aangesloten. Kies Opnieuw om nog eens te controleren.";
 clear("Verbinding controleren");rounded(8,44,368,36,C_RAISED);printAt(18,55,title,successes==checked&&successes?C_MINT:C_RED);
 paragraph(body,0,14,92,44,3);
 paragraph("Deze controle kijkt naar de aansluiting, niet naar wifi of je API-tegoed.",0,14,148,44,3);
 footer("Opnieuw","","","","","Terug");show();
}
static void diagnose(void){
 int k,successes,checked,i,beforeRetries,beforeErrors;WFrame reply;
 do {
  successes=checked=0;beforeRetries=retries;beforeErrors=errors;clear("Verbinding controleren");rounded(160,53,64,58,C_RAISED);drawLogo(172,61,40,C_MINT);centered(127,"Even controleren...",C_TEXT);centered(149,"We kijken of alles goed is aangesloten.",C_MUTED);show();
  for(i=0;i<3&&!quitting;i++){
   int result=rpc(W_HELLO,0,0,&reply,1);if(quitting||result==-2)return;checked++;
   if(result==1&&reply.len>=2&&reply.data[1]==1)successes++;else if(!successes)break;
   OS_InnerWait_ms(60);
  }
  if(quitting)return;
  if(!successes){while(linkMisses<3)linkFailed();linkOnline=0;}
  if(successes==checked&&successes&&(retries>beforeRetries||errors>beforeErrors))successes--;
  while(!quitting){
   paintDiagnosis(!linkOnline?0:successes,checked);k=waitKey();
   if(k==UI_LINK_REFRESH&&linkOnline&&!successes)break;
   if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F6||k==KEY_PRGM_LEFT)return;
   if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN||k==KEY_PRGM_RIGHT)break;
  }
 }while(!quitting);
}
static int fetchAnswer(unsigned int len){
 size_t pos=0;uint8_t b[2];WFrame r;if(len>ANSWER_MAX)return 0;answer[0]=0;
 while(pos<len&&!quitting){w_put16(b,(uint16_t)pos);if(!checkedRpc(W_READ,b,2,&r))return 0;
  if(r.len<2||pos+r.len-1>len){message("Antwoordfout","Ongeldige antwoordlengte ontvangen.");return 0;}
  memcpy(answer+pos,r.data+1,r.len-1);pos+=r.len-1;
 }answer[pos]=0;return !quitting;
}
static char displayedAnswer[ANSWER_MAX+1];
static void readAnswer(int contextWasDropped){
 int first=0,k;unsigned epoch=settingsEpoch;
 while(!quitting){
  int lines,max;char position[56];
  if(!answer_display(answer,displayedAnswer,sizeof(displayedAnswer)))strcpy(displayedAnswer,answer);
  lines=line_count(displayedAnswer,43);max=scroll_limit(lines,6);
  clear("ChatGPT");rounded(8,37,368,26,C_RAISED);clipped(16,43,lastQuestion,44,C_TEXT);
  drawLogo(12,70,16,C_TEXT);printAt(36,71,"ChatGPT",C_MINT);
  if(activeChatSlot>=0&&savedChats[activeChatSlot].used){sprintf(position,"%d/%d",viewTurn+1,savedChats[activeChatSlot].turns);printAt(288,71,position,C_MUTED);}
  paragraph(displayedAnswer,first,12,91,43,6);scrollbar(376,91,84,first,6,lines);
  printAt(8,176,unsavedReply?"Nog niet opgeslagen · F3: bewaren":contextWasDropped?"Oudste API-context gewist":activeChatSlot>=0&&viewTurn+1<savedChats[activeChatSlot].turns?"Oud bericht · F1 vervolgt laatste":"SHIFT: vraag · OPTN: chats",unsavedReply?C_RED:C_MUTED);
  footer("Verder","Nieuw",unsavedReply?"Bewaar":"Vraag","Chats",viewTurn>0?"Vorig":"",activeChatSlot>=0&&viewTurn+1<savedChats[activeChatSlot].turns?"Volgend":"Instel");show();k=waitKey();
  if(k==KEY_PRGM_F1){prompt[0]=0;return;}
  if(k==KEY_PRGM_EXIT)return;
  if(k==KEY_PRGM_SHIFT){message("Jouw vraag",lastQuestion);continue;}
  if(k==KEY_PRGM_UP&&first>0)first--;if(k==KEY_PRGM_DOWN&&first<max)first++;
  if(k==KEY_PRGM_LEFT){first-=6;if(first<0)first=0;}if(k==KEY_PRGM_RIGHT){first+=6;if(first>max)first=max;}
  if(k==KEY_PRGM_F2&&canLeaveReply()){newLocalChat();return;}
  if(k==KEY_PRGM_F3){if(unsavedReply)saveReply();else message("Jouw vraag",lastQuestion);}
  if(k==KEY_PRGM_F4||k==KEY_PRGM_OPTN){int changed=chatSidebar();if(changed==2)return;if(changed)first=0;}
  if(k==KEY_PRGM_F5&&viewTurn>0&&!unsavedReply){if(loadChatPair(activeChatSlot,viewTurn-1))first=0;}
  if(k==KEY_PRGM_F6){
   if(activeChatSlot>=0&&viewTurn+1<savedChats[activeChatSlot].turns&&!unsavedReply){if(loadChatPair(activeChatSlot,viewTurn+1))first=0;}
   else {settings();if(settingsEpoch!=epoch){contextDirty=1;epoch=settingsEpoch;}}
  }
 }
}

#include "chats.h"
static void paintLoading(int phase){
 static const int orbit[8][2]={{0,-43},{30,-30},{43,0},{30,30},{0,43},{-30,30},{-43,0},{-30,-30}};
 int i;clear("ChatGPT");choiceCard(136,40,112,98,0,C_MINT,C_RAISED);
 rounded(151,53,82,72,C_RAISED);drawLogo(160,57,64,C_MINT);
 for(i=0;i<8;i++){int active=(phase%8)==i;rounded(190+orbit[i][0],87+orbit[i][1],active?6:3,active?6:3,active?C_BLUE:C_LINE);}
 centered(147,loadingPurpose?"Jouw modellen, op een rij.":"Even voor je aan de slag.",C_TEXT);
 centered(167,loadingPurpose?"Je modellen worden opgehaald.":"Je vraag wordt verwerkt.",C_MUTED);
 centered(196,"EXIT of F1: annuleren",C_MINT);show();
}
static void animateLoading(void){if(elapsed(loadingAt,120)){loadingAt=RTC_GetTicks();paintLoading(++loadingPhase);}}
static void askWork(void){
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
  if(ok!=1||r.len!=10){if(!quitting)message("Verbinding onderbroken","De aanvraag kan al verstuurd zijn. Er wordt niet automatisch een nieuwe API-aanvraag gedaan. Open Instellingen, Hulp en Verbinding controleren.");return;}
  s=r.data[1];e=r.data[2];len=w_get16(r.data+3);
  if(w_get32(r.data+6)!=w_get32(id)){message("Atom herstart","Aanvraagstatus hoort niet meer bij deze vraag. Er is geen automatische herhaling bij de API.");return;}
  if(s==S_READY){int dropped=(r.data[5]&0x80)!=0;if(fetchAnswer(len)){loadingActive=0;strcpy(lastQuestion,prompt);prompt[0]=0;unsavedReply=1;saveReply();readAnswer(dropped);}return;}
  if(s==S_ERROR){message("Aanvraag mislukt",e<sizeof(errNames)/sizeof(errNames[0])?errNames[e]:"Onbekende fout");return;}
  if(s==S_CANCELLED){message("Geannuleerd","Er wordt geen antwoord opgehaald.");return;}
  paintLoading(loadingPhase);
  {int wait=RTC_GetTicks();while(!elapsed(wait,250)&&!quitting){
   animateLoading();int k=keyEvent();if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F1){WFrame ignored;rpc(W_CANCEL,0,0,&ignored,0);loadingActive=0;message("Geannuleerd","Een al verstuurde vraag kan nog kosten maken.");return;}OS_InnerWait_ms(10);
  }}
  if(elapsed(start,150000)){WFrame ignored;rpc(W_CANCEL,0,0,&ignored,0);message("Time-out","Geen antwoord binnen 150 seconden. Er wordt geen nieuwe API-aanvraag gestart.");return;}
 }
}
static void ask(void){
 loadingPurpose=0;loadingActive=prompt[0]!=0;loadingPhase=0;loadingAt=RTC_GetTicks();if(loadingActive)paintLoading(0);
 askWork();loadingActive=0;
}
/* Fresh state after a cold launch or a return from native GetKey/MENU. */
static void beginSession(void){
 int warm=serialOpen&&Serial_IsOpen()==1;storageClose();
 if(!warm){serialOpen=0;linkKnown=linkOnline=linkSeen=0;}
 oldSetup=GetSetupSetting(0x14);
 quitting=0;closePromptActive=0;lastKey=0;heldTick=repeatTick=0;errors=retries=0;seq=0;
 memset(&parser,0,sizeof(parser));memset(savedChats,0,sizeof(savedChats));
 libraryReady=0;activeChatSlot=-1;contextDirty=1;viewTurn=0;unsavedReply=0;
 linkMisses=0;settingsKnown=0;remoteModelCount=0;loadingActive=0;rpcDepth=0;atomMajor=0;atomBootKnown=0;lastAtomBoot=0;settingsEpoch=0;
 prompt[0]=answer[0]=lastQuestion[0]=publicSSID[0]=publicModel[0]=0;
 erase(field,sizeof(field));Bkey_ClrAllFlags();SetSetupSetting(0x14,0);
 SetQuitHandler(0);waitForKeyRelease();drainNativeKeys();Native_ClearKeyBuffer();
 /* libfxcg/Utilities documents that color mode may need one OS GetKey pass.
  * Consume a synthetic EXIT here, never dispatch it into our own close dialog. */
 {int discarded;Bdisp_EnableColor(1);Keyboard_PutKeycode(-1,-1,KEY_CTRL_EXIT);do {GetKey(&discarded);}while(discarded!=KEY_CTRL_EXIT);}
 Bkey_ClrAllFlags();SetSetupSetting(0x14,0);
 applyView(view_load(&disk));storageClose();configureDisplay();vram=(unsigned short*)GetVRAMAddress();
 loadLibrary();if(serialOpen&&Serial_IsOpen()==1)Serial_ClearRX();autoProbeStarted=0;stopAutoProbe();autoConnectTick();
}
static void runSession(void){
 int k;unsigned epoch;
 for(;;){
  while(!quitting){
   k=editor(prompt,sizeof(prompt),"ChatGPT - Vraag",0,1,0,1);
   if(k==KEY_PRGM_F1)ask();
   else if(k==KEY_PRGM_F2){if(canLeaveReply()&&(!prompt[0]||confirm("De niet-verstuurde vraag wissen en opnieuw beginnen?")))newLocalChat();}
   else if(k==UI_CHATS){int changed=chatSidebar();if(changed==1)readAnswer(0);}
   else if(k==KEY_PRGM_F6){epoch=settingsEpoch;settings();if(epoch!=settingsEpoch)contextDirty=1;}
   else if(k==KEY_PRGM_EXIT){
    /* Root EXIT leaves the app and typed draft exactly as they are. */
   }
  }
  if(!unsavedReply)break;
  resumeAfterMenu();if(saveReply()||canLeaveReply()){quitting=1;break;}
 }
}
static void endSession(void){
 /* Finish local work; keep the OS-owned UART driver open through native MENU.
  * No app timer, interrupt handler or quit callback remains active in MENU. */
 storageClose();erase(field,sizeof(field));waitForKeyRelease();stopAutoProbe();
}
/* The OS key-wait must be active when a fallback MENU event is injected.
 * This ordering follows gint_osmenu_native; pre-queueing then calling GetKey
 * (V8) leaves this CG50 on the old dialog, even with physical MENU. */
static volatile int menuTimer=-1,menuTimerInjected=0;
static void stopMenuTimer(void){
 int timer=menuTimer;menuTimer=-1;
 if(timer>0){Timer_Stop(timer);Timer_Deinstall(timer);}
}
static void menuTimerHandler(void){
 menuTimerInjected=1;Keyboard_PutKeycode(4,9,0);stopMenuTimer();
}
static int nativeMainMenu(void){
 uint32_t target;int column=0,row=0,result;unsigned short key=0;
 Bkey_ClrAllFlags();drainNativeKeys();Native_ClearKeyBuffer();
 target=nativeMenuTarget();prepareMenuSnapshot();
 if(target){
  stopAutoProbe();storageClose();SetSetupSetting(0x14,oldSetup);SetQuitHandler(0);
  Bkey_ClrAllFlags();EnableStatusArea(0);DrawFrame(0xffff);
  menuInvoke(target); /* Saves app continuation; returns on actual re-selection. */
  stopAutoProbe();storageClose();return 1;
 }
 menuTimerInjected=0;
 menuTimer=Timer_Install(0,menuTimerHandler,20);
 if(menuTimer<=0){menuTimer=-1;return 0;}
 stopAutoProbe();storageClose();SetSetupSetting(0x14,oldSetup);SetQuitHandler(0);
 Bkey_ClrAllFlags();EnableStatusArea(0);DrawFrame(0xffff);
 if(Timer_Start(menuTimer)<0){stopMenuTimer();ensureSerial();return 0;}
 result=GetKeyWait_OS(&column,&row,KEYWAIT_HALTON_TIMEROFF,1,0,&key);
 (void)result;stopMenuTimer();
 if(!menuTimerInjected){ensureSerial();return 0;}
 stopAutoProbe();storageClose();return 1;
}

int main(void){
 for(;;){
  beginSession();
  for(;;){
   runSession();endSession();
   if(nativeMainMenu())break;
   /* Do not strand a white-rimmed closing dialog on an unsupported path. */
   quitting=0;
   message("MENU niet geopend","De OS-menuovergang kon niet worden gestart. Je sessie en Atom-verbinding blijven beschikbaar. Probeer MENU opnieuw.");
  }
  /* Restart UI only after SaveAndOpenMainMenu/native key-wait returns. */
 }
}
