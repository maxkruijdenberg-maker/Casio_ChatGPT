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
#include <stdlib.h>
#include "../atom/ChatGPTAtom/wire.h"
#include "text.h"
#include "font.h"
#include "logo_aa.h"
#include "layout.h"
#include "conversation.h"
#include "../atom/ChatGPTAtom/config_validation.h"
#define COLS 44
#define MESSAGE_COLS 33
#define ROWS 8
#define PROMPT_MAX 1024
#define ANSWER_MAX 8192
static char prompt[PROMPT_MAX+1],answer[ANSWER_MAX+1],field[PROMPT_MAX+1],lastQuestion[PROMPT_MAX+1];
static int quitting=0,serialOpen=0,lastKey=0,errors=0,retries=0;
static unsigned uartOpens=0,uartCloses=0;
static int rpcDepth=0;
static int autoConnectTick(void);
static int ensureModelStopped(void);
static int ensureQuestionStopped(void);
static void cancelQuestion(void);
static void animateLoading(void);
static void paintLoading(int phase);
static int loadingActive=0,loadingPhase=0,loadingAt=0;
static int questionStopPending=0;
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
static void releaseModels(void);

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
static void cleanup(void){releaseModels();stopAutoProbe();stopMenuTimer();closeSerial();storageClose();EnableStatusArea(0);DrawFrame(0xffff);erase(field,sizeof(field));}
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
enum {C_BG=0x08a4,
 C_PANEL=0x1106,
 C_RAISED=0x19c7,
 C_TEXT=0xf7bf,
 C_MUTED=0xa5b9,
 C_LINE=0x29ca,
 C_MINT=0x6f18,
 C_RED=0xfc12,
 C_BLUE=0x85bf,
 C_PURPLE=0xc53f,
 C_GOLD=0xf670,
 C_SHADOW=0x0862,
 C_BLUE_SOFT=0x1989,
 C_PURPLE_SOFT=0x3128,
 C_GOLD_SOFT=0x39a4,
 C_GREEN=0x8735,
 C_GREEN_SOFT=0x11c5,
 C_STATUS_RED=0xfcb4,
 C_RED_SOFT=0x4105};
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
/* One card language across all selectable lists: quiet surfaces, colored focus ring,
 * tinted interior. Geometry stays inside the native 384x216 canvas. */
static void choiceCard(int x,int y,int w,int h,int selected,unsigned short accent,unsigned short tint){
 rounded(x,y,w,h,selected?accent:C_PANEL);
 rounded(x+1+(selected!=0),y+1+(selected!=0),w-2-2*(selected!=0),h-2-2*(selected!=0),selected?tint:C_PANEL);
}
static unsigned short mix565(unsigned short front,unsigned short back,int alpha){
 int r=(((front>>11)&31)*alpha+((back>>11)&31)*(15-alpha)+7)/15;
 int g=(((front>>5)&63)*alpha+((back>>5)&63)*(15-alpha)+7)/15;
 int b=((front&31)*alpha+(back&31)*(15-alpha)+7)/15;
 return (unsigned short)((r<<11)|(g<<5)|b);
}
static void logoPaint(int x,int y,int size){
 if(!viewReady)applyView(view_default());
 int source=size==16?16:size==24?24:size==40?40:size==48?48:size==56?56:64;
 const uint8_t *bits=source==16?logo_aa16:source==24?logo_aa24:source==40?logo_aa40:source==48?logo_aa48:source==56?logo_aa56:logo_aa64;
 int px,py;
 for(py=0;py<size;py++)for(px=0;px<size;px++){
  int index=(py*source/size)*source+px*source/size;
  int a=(bits[index/2]>>(index%2?0:4))&15;ViewRect r;
  if(a&&view_rect(&viewMap,x+px,y+py,1,1,&r)){
   unsigned short ink=mix565(C_PURPLE,C_BLUE,py*15/(size>1?size-1:1));
   unsigned short background=vram[r.y*384+r.x];
   physicalRect(r.x,r.y,r.w,r.h,mix565(ink,background,a));
  }
 }
}
static void drawLogo(int x,int y,int size){logoPaint(x,y,size);}
/* Abstract orbit motif, painted with integer-only primitives. No timers,
 * driver state or input handling are involved in these visual accents. */
static void ellipse(int cx,int cy,int rx,int ry,unsigned short color){
 int y,x;long bound=(long)rx*rx*ry*ry;
 for(y=-ry;y<=ry;y++){
  x=rx;while(x>0&&(long)x*x*ry*ry+(long)y*y*rx*rx>bound)x--;
  rect(cx-x,cy+y,2*x+1,1,color);
 }
}
/* One centered, sheared ellipse makes both ring halves exact opposites.
 * 4*dy+dx gives a 1:4 tilt without floating point or rounding asymmetry. */
static void planetRing(int cx,int cy,int rx,int ry,unsigned short color){
 int x,y,innerX=rx-5,innerY=ry-3;
 long outer=(long)16*rx*rx*ry*ry,inner=(long)16*innerX*innerX*innerY*innerY;
 for(y=-(ry+rx/4+2);y<=ry+rx/4+2;y++)for(x=-rx;x<=rx;x++){
  long v=4*y+x;
  long outside=(long)16*x*x*ry*ry+v*v*rx*rx;
  long hole=(long)16*x*x*innerY*innerY+v*v*innerX*innerX;
  if(outside<=outer&&hole>=inner)rect(cx+x,cy+y,1,1,color);
 }
}
static void abstractLogo(int cx,int cy,int size,int phase){
 static const int orbit[8][2]={{1024,0},{724,724},{0,1024},{-724,724},{-1024,0},{-724,-724},{0,-1024},{724,-724}};
 int radius=size/2,rx=radius+29,ry=14;
 planetRing(cx,cy,rx,ry,mix565(C_PURPLE,C_BG,6));
 if(phase>=0){
  int n;for(n=0;n<2;n++){
   int i=(phase+4*n)%8,x=rx*orbit[i][0]/1024,y=ry*orbit[i][1]/1024-x/4;
   rounded(cx+x-2,cy+y-2,n?3:5,n?3:5,n?C_BLUE:C_PURPLE);
  }
 }
 ellipse(cx,cy,radius+10,radius+10,C_PURPLE_SOFT);
 ellipse(cx,cy,radius+8,radius+8,C_BLUE_SOFT);
 ellipse(cx,cy,radius+6,radius+6,C_PANEL);
 logoPaint(cx-radius,cy-radius,size);
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
 drawLogo(8,4,24);clipped(40,9,name,29,C_TEXT);
 rect(0,31,384,1,C_LINE);
 {
  const char *status=linkOnline?"Verbonden":"Verbroken";
  unsigned short color=linkOnline?C_GREEN:C_STATUS_RED;
  ellipse(294,16,3,3,color);printAt(302,9,status,C_TEXT);
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
  if(n>8)n=8;if(i==5&&n)color=C_PURPLE;
  if(i==0&&(!strcmp(a,"Stuur")||!strcmp(a,"Opslaan")||!strcmp(a,"Verder")||!strcmp(a,"Kies")||!strcmp(a,"Open")||!strcmp(a,"Opnieuw"))){rounded(3,195,58,19,C_MINT);color=C_BG;}
  for(j=0;j<n;j++)glyph(i*64+32-n*step/2+j*step,197,(unsigned char)p[i][j],color);
 }
}
/* Navigation uses the physical arrows; these labels are not function keys. */
static void navigationFooter(int open){
 footer("","","","","","");printAt(12,197,"← Terug",C_MUTED);
 if(open)printAt(310,197,"Open →",C_MUTED);
}
static void textPageAt(const char *s,int first,int hide,int left,int top,int visible,int columns){
 size_t at=0,len=strlen(s),n;uint32_t cp;int row=0,col=0;
 while(at<len){n=u_decode(s+at,len-at,&cp);at+=n;
  if(cp=='\r')continue;if(cp=='\n'){row++;col=0;continue;}
  if(col==columns){row++;col=0;}
  if(row>=first&&row<first+visible)glyph(left+col*8,top+(row-first)*14,hide?'*':cp,C_TEXT);
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
static void cursorPosition(const char *s,size_t cursor,int columns,int *row,int *col){
 size_t p=0,n;uint32_t cp;*row=*col=0;
 while(p<cursor){n=u_decode(s+p,cursor-p,&cp);p+=n;
  if(cp=='\n'){(*row)++;*col=0;continue;}if(cp=='\r')continue;
  if(*col==columns){(*row)++;*col=0;}(*col)++;
 }
 if(*col==columns){(*row)++;*col=0;}
}
static void message(const char *title,const char *body){
 int first=0,lines=line_count(body,COLS),k,max=scroll_limit(lines,ROWS);
 while(!quitting){clear(title);paragraph(body,first,12,52,COLS,ROWS);scrollbar(376,52,112,first,ROWS,lines);
  if(max)printAt(12,175,"Pijlen: lezen · EXIT: terug",C_MUTED);
  navigationFooter(0);show();k=waitKey();
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT||k==KEY_PRGM_RETURN)return;
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
/* Everyday symbols only; letters and digits use the physical keyboard. */
static const uint32_t basicCharacters[]={
 '=', '+', '-', 0xd7, 0xf7, '/', '(', ')',
 '.', ',', ':', ';', '!', '?', '%', 0xb0,
 '<', '>', 0x2264, 0x2265, 0x2260, 0x2248, 0xb1, '^',
 0x221a, 0xb2, 0xb3, '[', ']', '{', '}', '|',
 '_', '@', '#', '&', '\'', '\"', 0x20ac, ' '
};
static int characterChoices(uint32_t *choices,int asciiOnly){
 int i,count=0;
 for(i=0;i<(int)(sizeof(basicCharacters)/sizeof(basicCharacters[0]));i++){
  uint32_t cp=basicCharacters[i];if(!asciiOnly||cp<127)choices[count++]=cp;
 }return count;
}
static uint32_t pickCharacter(int asciiOnly){
 uint32_t choices[40];int count=characterChoices(choices,asciiOnly),i,sel=0,k;
 while(!quitting){clear("Kies een teken");
  if(choices[sel]==' ')printAt(12,35,"Spatie",C_MUTED);
  else {printAt(12,35,"Invoegen:",C_MUTED);glyph(92,35,choices[sel],C_TEXT);}
  for(i=0;i<count;i++){
   int x=14+(i%8)*45,y=53+(i/8)*23;
   rounded(x,y,40,20,i==sel?C_MINT:C_LINE);
   rounded(x+1,y+1,38,18,i==sel?C_RAISED:C_PANEL);
   if(choices[i]==' ')printAt(x+12,y+3,"SP",i==sel?C_MINT:C_TEXT);
   else glyph(x+16,y+3,choices[i],i==sel?C_MINT:C_TEXT);
  }
  printAt(12,175,"Pijlen kiezen · EXE invoegen",C_MUTED);footer("Kies","","","","","");show();k=waitKey();
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)return choices[sel];if(k==KEY_PRGM_EXIT)return 0;
  if(k==KEY_PRGM_LEFT&&sel>0)sel--;if(k==KEY_PRGM_RIGHT&&sel<count-1)sel++;
  if(k==KEY_PRGM_UP&&sel>=8)sel-=8;
  if(k==KEY_PRGM_DOWN&&sel/8<(count-1)/8){sel+=8;if(sel>=count)sel=count-1;}
 }return 0;
}
/* Shared paint path: the OS saves this neutral input page before opening MENU,
 * so resuming cannot first restore the old "Ja/Nee" closing dialog. */
static int composerRows(const char *buf);
/* One local transcript, one composer. Only row metadata is cached; message
 * bodies are loaded one visible pair at a time without changing live answers. */
static char displayedAnswer[ANSWER_MAX+1];
static ConversationTimeline chatTimeline;
static int chatTimelineReady=0,chatScroll=0,chatFollowTail=1,chatPaintTurn=-1;
static int chatLiveTurn=-1,chatReadError=0,chatInputLetters=0;
static int hasConversation(void){return (activeChatSlot>=0&&savedChats[activeChatSlot].used)||(answer[0]&&lastQuestion[0]);}
static void invalidateConversation(void){chatTimelineReady=0;chatPaintTurn=-1;chatReadError=0;}
static int loadConversationTurn(int turn){
 if(chatPaintTurn==turn)return 1;
 if(turn==chatLiveTurn){strcpy(field,lastQuestion);if(!answer_display(answer,displayedAnswer,sizeof(displayedAnswer)))return 0;}
 else {
  if(activeChatSlot<0||!lib_pair(&disk,&savedChats[activeChatSlot].ref,turn,field,sizeof(field),displayedAnswer,sizeof(displayedAnswer))){storageClose();return 0;}
  storageClose();if(!answer_display(displayedAnswer,displayedAnswer,sizeof(displayedAnswer)))return 0;
 }
 chatPaintTurn=turn;return 1;
}
static void buildConversation(void){
 if(chatTimelineReady)return;
 int stored=activeChatSlot>=0&&savedChats[activeChatSlot].used?savedChats[activeChatSlot].turns:0;
 chatLiveTurn=unsavedReply||(!stored&&answer[0]&&lastQuestion[0])?stored:-1;
 conversation_init(&chatTimeline);chatPaintTurn=-1;chatReadError=0;
 for(int i=0;i<stored;i++){
  if(!loadConversationTurn(i)||!conversation_add(&chatTimeline,i,field,displayedAnswer,43)){conversation_init(&chatTimeline);chatReadError=1;break;}
 }
 if(chatLiveTurn>=0){if(!loadConversationTurn(chatLiveTurn)||!conversation_add(&chatTimeline,chatLiveTurn,field,displayedAnswer,43))chatReadError=1;}
 chatTimelineReady=1;storageClose();
}
#define CHAT_TEXT_TOP 40
static int conversationRows(const char *buf){int rows=composerRows(buf);return rows>3?3:rows;}
static void paintConversation(int visible){
 buildConversation();if(visible<1)return;
 chatScroll=chatFollowTail?conversation_tail(&chatTimeline,visible):conversation_clamp(&chatTimeline,chatScroll,visible);
 for(int row=0;row<visible;){
  ConversationLine line;if(!conversation_line(&chatTimeline,chatScroll+row,&line))break;
  int y=CHAT_TEXT_TOP+row*14;
  if(line.part==0){
   rounded(10,y,64,14,C_PURPLE_SOFT);ellipse(21,y+4,2,2,C_PURPLE);rounded(17,y+8,9,4,C_PURPLE);printAt(34,y,"Jij",C_TEXT);
  }
  else if(line.part==2){rounded(10,y,96,14,C_BLUE_SOFT);drawLogo(15,y+1,12);printAt(34,y,"ChatGPT",C_TEXT);}
  else if(line.part==1||line.part==3){
   int take=1;ConversationLine next;
   while(row+take<visible&&conversation_line(&chatTimeline,chatScroll+row+take,&next)&&next.turn==line.turn&&next.part==line.part&&next.row==line.row+take)take++;
   if(loadConversationTurn(line.turn))paragraph(line.part==1?field:displayedAnswer,line.row,12,y,43,take);else chatReadError=1;
   row+=take;continue;
  }
  row++;
 }
 scrollbar(376,CHAT_TEXT_TOP,visible*14,chatScroll,visible,chatTimeline.total);storageClose();
}
static int composerRows(const char *buf){
 int row,col;cursorPosition(buf,strlen(buf),MESSAGE_COLS,&row,&col);row++;return row>ROWS?ROWS:row;
}
static void renderEditor(const char *buf,size_t cap,size_t cursor,const char *title,int secret,int promptMode,
 int letters,int shifted,int firstLetter,int first){
 int row,col,top=52,left=12,visible=ROWS;char mode[8];cursorPosition(buf,cursor,promptMode==1?MESSAGE_COLS:COLS,&row,&col);
 clear(title);
 (void)cap;
 if(promptMode==1){
  visible=hasConversation()?conversationRows(buf):composerRows(buf);top=167-visible*14;left=16;
  if(hasConversation())paintConversation((top-6-CHAT_TEXT_TOP)/14);
  else if(visible<=2){
   abstractLogo(192,76,48,-1);
   centered(117,activeChatSlot>=0?"Wat wil je nog weten?":"Waar kan ik je mee helpen?",C_TEXT);
  }
  rounded(8,top-5,368,visible*14+10,C_LINE);
  rounded(9,top-4,366,visible*14+8,C_PANEL);
  if(!buf[0]){
   printAt(16,top,"Stuur een bericht",C_MUTED);
  }else textPageAt(buf,first,secret,left,top,visible,MESSAGE_COLS);
 }else {choiceCard(6,50,368,120,0,C_MINT,C_RAISED);textPageAt(buf,first,secret,left,top,visible,COLS);}
 {int x=left+col*8,y=top+(row-first)*14+13;if((promptMode!=1||buf[0])&&y<172)rect(x,y,8,1,C_MINT);}
 if(promptMode==1)footer("Stuur","Nieuw",unsavedReply?"Bewaar":"Chats","Tekens","","Instel");else footer("Opslaan","","","Tekens","","");
 if(promptMode==1){
  if(chatReadError)printAt(8,175,"Een deel van je chat is niet leesbaar.",C_RED);
  /* Reserved control rail: input and caret stop before the mode and send arrow. */
  sprintf(mode,"%s%s",letters?(firstLetter?"Abc":"abc"):"123",shifted?" S":"");
  printAt(336-(int)strlen(mode)*8,153,mode,C_MUTED);
  rounded(346,152,20,17,C_RAISED);
  for(int dy=0;dy<4;dy++)rect(355-dy,155+dy,2+dy*2,1,C_MUTED);rect(355,159,2,7,C_MUTED);
 }else {
  printAt(8,175,"DEL wissen · ←→ cursor · EXIT terug",C_MUTED);
  sprintf(mode,"%s%s",letters?"abc":"123",shifted?" S":"");
  {int width=(int)strlen(mode)*8+12;rounded(376-width,174,width,15,C_PANEL);printAt(382-width,175,mode,C_MUTED);}
 }show();
}
static void prepareMenuSnapshot(void){
 int known=linkKnown,online=linkOnline,seen=linkSeen,active=autoProbeActive;
 linkKnown=1;linkOnline=0;linkSeen=0;autoProbeActive=1;invalidateConversation();
 renderEditor("",PROMPT_MAX+1,0,"ChatGPT",0,1,0,0,1,0);
 linkKnown=known;linkOnline=online;linkSeen=seen;autoProbeActive=active;
}
/* Returns F1/F2/F6/EXIT. Secret input is never revealed or retained on Casio. */
static int editor(char *buf,size_t cap,const char *title,int secret,int autoCapital,int asciiOnly,int promptMode){
 size_t cursor=strlen(buf);int keepMode=promptMode==1&&buf==prompt;int letters=keepMode?chatInputLetters:0,shifted=0,first=0,row,col,k,cp,useShift,firstLetter=autoCapital&&cursor==0;
 int columns=promptMode==1?MESSAGE_COLS:COLS;
 if(promptMode==1)invalidateConversation();
 while(!quitting){
  const char *token=0;
  firstLetter=autoCapital&&!secret&&sentence_capital(buf,cursor);
  cursorPosition(buf,cursor,columns,&row,&col);
  int visible=promptMode==1?(hasConversation()?conversationRows(buf):composerRows(buf)):ROWS;
  if(promptMode==1){int endRow,endCol,maxFirst;cursorPosition(buf,strlen(buf),columns,&endRow,&endCol);maxFirst=endRow+1-visible;if(maxFirst<0)maxFirst=0;if(first>maxFirst)first=maxFirst;}
  if(row<first)first=row;if(row>=first+visible)first=row-visible+1;
  renderEditor(buf,cap,cursor,title,secret,promptMode,letters,shifted,firstLetter,first);
  k=waitEditorKey(promptMode);if(quitting)return KEY_PRGM_MENU;if(k==UI_LINK_REFRESH)continue;
  if(k==KEY_PRGM_SHIFT){shifted=!shifted;continue;}
  if(k==KEY_PRGM_ALPHA){letters=!letters;if(keepMode)chatInputLetters=letters;shifted=0;continue;}
  useShift=shifted;shifted=0;
  if(k==KEY_PRGM_F5)continue;
  if(promptMode==1&&k==KEY_PRGM_F3&&unsavedReply){saveReply();invalidateConversation();continue;}
  if(promptMode==1&&(k==KEY_PRGM_OPTN||k==KEY_PRGM_F3))return UI_CHATS;
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)return KEY_PRGM_F1;
  if(k==KEY_PRGM_EXIT)return k;
  if(promptMode==1&&(k==KEY_PRGM_F2||k==KEY_PRGM_F6))return k;
  if(k==KEY_PRGM_LEFT){cursor=useShift?0:u_prev(buf,cursor);continue;}
  if(k==KEY_PRGM_RIGHT){if(useShift)cursor=strlen(buf);else if(cursor<strlen(buf))cursor=u_next(buf,cursor);continue;}
  if(k==44){u_delete(buf,&cursor);if(!buf[0])firstLetter=autoCapital;continue;}
  if(k==KEY_PRGM_ACON){if(confirm("Dit invoerveld helemaal wissen?")){erase(buf,cap);cursor=0;firstLetter=autoCapital;}continue;}
  if(promptMode==1&&hasConversation()&&(k==KEY_PRGM_UP||k==KEY_PRGM_DOWN)){
   buildConversation();int max=conversation_tail(&chatTimeline,(167-visible*14-6-CHAT_TEXT_TOP)/14);
   chatScroll=conversation_clamp(&chatTimeline,chatScroll+(k==KEY_PRGM_UP?-1:1),(167-visible*14-6-CHAT_TEXT_TOP)/14);chatFollowTail=chatScroll==max;continue;
  }
  if(k==KEY_PRGM_UP){int i;for(i=0;i<columns&&cursor;i++)cursor=u_prev(buf,cursor);continue;}
  if(k==KEY_PRGM_DOWN){int i;for(i=0;i<columns&&cursor<strlen(buf);i++)cursor=u_next(buf,cursor);continue;}
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
  }
  if(firstLetter){if(cp>='a'&&cp<='z')cp-=32;else if(cp>=0xe0&&cp<=0xfe&&cp!=0xf7&&glyphIndex(cp-32)>=0)cp-=32;}
  if(!cp){if(k!=KEY_PRGM_F4)message("Teken niet ondersteund","Gebruik F4 Tekens voor speciale tekens en formules.");continue;}
  if(asciiOnly&&(cp<32||cp>126)){message("Ongeldig teken","Dit veld ondersteunt alleen ASCII.");continue;}
  if(glyphIndex((uint32_t)cp)<0){message("Teken niet ondersteund","Dit invoerteken kan niet worden weergegeven.");continue;}
  if(!u_insert(buf,cap,&cursor,(uint32_t)cp)){message("Invoer vol","Dit veld is vol. Verwijder tekst om verder te typen.");continue;}
  if((cp>='a'&&cp<='z')||(cp>='A'&&cp<='Z'))firstLetter=0;
 }return KEY_PRGM_MENU;
}
static char publicSSID[33],publicModel[256];
static unsigned settingsEpoch=0;
#define REMOTE_MODELS_CAP 4096
static char *remoteModels=0;
static unsigned short modelOffsets[1024];
static int remoteModelCount=0,modelAccountChecked=0,modelCheckFailed=0;
static int modelShowAll=0,modelStopPending=0;
static void releaseModels(void){if(remoteModels)free(remoteModels);remoteModels=0;remoteModelCount=0;modelAccountChecked=0;modelCheckFailed=0;}
static int fetchModels(void);
static int atomMajor=0,atomMinor=0,settingsKnown=0;
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
 if(rpc(W_DEVICE,0,0,&r,0)==1&&r.len>=4){atomMajor=r.data[1];atomMinor=r.data[2];
  if(r.data[3]&1){if(!readPublic(1,publicSSID,sizeof(publicSSID)))publicSSID[0]=0;
   if(!readPublic(4,publicModel,sizeof(publicModel)))publicModel[0]=0;}
 }
 return linkOnline;
}
static void screenTest(void){
 int k;physicalRect(0,0,384,216,C_BG);rect(0,0,384,2,C_MINT);rect(0,214,384,2,C_MINT);rect(0,0,2,216,C_MINT);rect(382,0,2,216,C_MINT);
 printAt(5,4,"TL",C_TEXT);printAt(361,4,"TR",C_TEXT);printAt(5,198,"BL",C_TEXT);printAt(361,198,"BR",C_TEXT);
 printAt(108,76,"Schermtest 384x216",C_TEXT);printAt(60,106,"Zijn alle vier hoeken zichtbaar?",C_MUTED);printAt(140,144,"EXIT: terug",C_MUTED);show();
 while(!quitting){k=waitKey();if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT||k==KEY_PRGM_RETURN)return;}
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
 if(!ensureModelStopped()||!ensureQuestionStopped())return;
 WFrame r;uint8_t action=1;int tick,k;char network[32],password[13];
 if(!linkOnline&&!checkedRpc(W_HELLO,0,0,&r))return;
 if(!atomMajor){if(rpc(W_DEVICE,0,0,&r,0)!=1||r.len<4){message("Geen verbinding","De Atom reageert nog niet. Controleer de aansluiting en probeer opnieuw.");return;}atomMajor=r.data[1];atomMinor=r.data[2];}
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
  navigationFooter(0);show();
  do{k=keyEvent();if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT||quitting)break;OS_InnerWait_ms(10);}while(!elapsed(tick,1000));
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT||quitting)break;
  action=0;if(rpc(W_SETUP,&action,1,&r,0)!=1||r.len!=17)break;tick=RTC_GetTicks();
 }
 {int leaving=quitting;quitting=0;action=2;rpc(W_SETUP,&action,1,&r,0);quitting=leaving;}erase(password,sizeof(password));
}
/* Keep the transport catalogue intact; expose only maintained text-chat aliases.
 * Zero compatible entries is an empty account view, never an unverified fallback. */
static int accountHasModel(const char *id){
 if(!modelAccountChecked)return 1;
 for(int i=0;i<remoteModelCount;i++){
  const char *p=remoteModels+modelOffsets[i];size_t n=strlen(id);
  if(!strncmp(p,id,n)&&p[n]=='\n')return 1;
 }
 return 0;
}
static int choiceCount(void){int n=0;for(int i=0;i<MODEL_COUNT;i++)n+=accountHasModel(modelChoices[i].id);return n;}
static int choicePreset(int index){
 for(int i=0;i<MODEL_COUNT;i++)if(accountHasModel(modelChoices[i].id)&&index--==0)return i;
 return -1;
}
static const char *choiceId(int index,char out[256]){
 (void)out;int i=choicePreset(index);return i>=0?modelChoices[i].id:"";
}
/* App advice, not a price/performance benchmark. All known choices stay available. */
static const char *const recommendedModels[]={"gpt-6.1-sol","gpt-6-luna","gpt-6-astra"};
static int modelViewCount(void){
 if(modelShowAll)return choiceCount();int count=0;
 for(int i=0;i<3;i++)count+=accountHasModel(recommendedModels[i]);return count;
}
static int modelViewPreset(int index){
 if(modelShowAll)return choicePreset(index);
 for(int i=0;i<3;i++)if(accountHasModel(recommendedModels[i])&&index--==0)return modelChoiceIndex(recommendedModels[i]);
 return -1;
}
static const char *modelViewId(int index){int i=modelViewPreset(index);return i>=0?modelChoices[i].id:"";}
static int modelSelection(const char *id){
 for(int i=0;i<modelViewCount();i++)if(!strcmp(id,modelViewId(i)))return i;
 return 0;
}
static void modelDetails(int preset,char *out,size_t cap){
 const ModelChoice *m=&modelChoices[preset];
 const char *access=modelAccountChecked?"Dit model staat in je account. Dat garandeert nog geen geslaagde chatvraag.":"Toegang met jouw sleutel is nog niet gecontroleerd.";
 const char *format="%s\n\n%s\n\n%s\n\nKosten: %s. Bekijk de API-prijzen bij OpenAI.\n\n%s\nAPI-naam: %s";
 /* The device SDK has sprintf, not snprintf. Bound all six fixed-source fields
  * before formatting; format length overestimates the substitution overhead. */
 if(cap<=strlen(format)+strlen(m->label)+strlen(m->description)+strlen(m->advice)+strlen(m->cost)+strlen(access)+strlen(m->id)){if(cap)out[0]=0;return;}
 sprintf(out,format,m->label,m->description,m->advice,m->cost,access,m->id);
}
static int parseModelList(unsigned total){
 unsigned at=0,n=0,start=0;int count=0;remoteModelCount=0;modelAccountChecked=0;
 if(!remoteModels||total>=REMOTE_MODELS_CAP)return 0;
 while(at<total){unsigned char c=remoteModels[at++];
  if(c=='\n'){if(!n||count>=1024)return 0;modelOffsets[count++]=(unsigned short)start;start=at;n=0;}
  else if(c<33||c>126||++n>255)return 0;
 }
 if(n||(total&&!count))return 0;remoteModels[total]=0;remoteModelCount=count;modelAccountChecked=1;return 1;
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
 total=w_get16(r.data+3);if(!remoteModels||total>=REMOTE_MODELS_CAP)return 0;
 /* Do not retain partial lists after a failed refresh. */
 remoteModelCount=0;modelAccountChecked=0;
 while(pos<total&&!quitting){
  b[0]=2;w_put16(b+1,(uint16_t)pos);
  if(modelRpc(b,3,&r)!=1||r.len<2||pos+r.len-1>total)return 0;
  memcpy(remoteModels+pos,r.data+1,r.len-1);pos+=r.len-1;
 }
 return !quitting&&parseModelList(total);
}
/* A stopped spinner is not proof that the background task has stopped. */
static void paintModelStopping(void){
 clear("Accountcontrole stoppen");centered(85,"De controle wordt gestopt.",C_TEXT);
 centered(111,"Je gegevens blijven bewaard.",C_MUTED);show();
}
static int readModelStopStatus(void){
 WFrame r;uint8_t action=0;
 if(rpc(W_MODELS,&action,1,&r,0)==1&&r.len==5&&r.data[1]!=1&&r.data[1]<=4){modelStopPending=0;return 1;}
 return 0;
}
static int stopModelCheck(void){
 WFrame r;uint8_t action=3;int start=RTC_GetTicks(),leaving=quitting,modal=closePromptActive;
 loadingActive=0;modelStopPending=1;
 paintModelStopping();
 /* Complete this bounded cleanup even after MENU was confirmed. No nested MENU. */
 quitting=0;closePromptActive=1;
 if(rpc(W_MODELS,&action,1,&r,0)==1&&r.len==5){
  if(r.data[1]!=1&&r.data[1]<=4)modelStopPending=0;
  while(modelStopPending&&!elapsed(start,8000)){
   OS_InnerWait_ms(50);if(readModelStopStatus())break;
  }
 }
 closePromptActive=modal;quitting=leaving;
 return !modelStopPending;
}
static int ensureModelStopped(void){
 if(!modelStopPending)return 1;
 loadingActive=0;
 if(readModelStopStatus()||stopModelCheck())return 1;
 message("Controle wordt gestopt","De Atom heeft het stoppen van de accountcontrole nog niet bevestigd. Controleer de verbinding en probeer straks opnieuw. Er is geen chatvraag verstuurd.");
 return 0;
}
static const char *modelCheckReason(int error){
 switch(error){
 case E_JSON:return atomMajor<22?"De modellenlijst van je account kon niet worden gelezen. Werk ook de Atom bij met de meegeleverde firmware.":"De modellenlijst van je account is onvolledig of heeft een onverwacht formaat. Controleer internet en probeer later opnieuw.";
 case E_BIG:return "De modellenlijst van je account is te groot voor dit apparaat.";
 case E_MEMORY:return "Er is onvoldoende vrij geheugen om de accountlijst te verwerken.";
 case E_TIMEOUT:return "Het ophalen van de accountlijst duurde te lang. Controleer je internetverbinding.";
 case E_MODEL:return "OpenAI kon de modellenlijst van je account niet leveren.";
 default:return error>0&&error<(int)(sizeof(errNames)/sizeof(errNames[0]))?errNames[error]:"Controleer wifi en je API-account.";
 }
}
static int fetchModels(void){
 if(!ensureModelStopped()||!ensureQuestionStopped())return 0;
 int ok;modelFetchCancelled=modelFetchError=0;remoteModelCount=0;modelAccountChecked=0;modelCheckFailed=0;
 if(!remoteModels){remoteModels=(char*)malloc(REMOTE_MODELS_CAP);if(!remoteModels){modelCheckFailed=1;message("Account niet gecontroleerd","Er is onvoldoende vrij geheugen. Je ziet de vaste modelkeuzes; je huidige model blijft bewaard.");return 0;}}
 loadingActive=1;loadingPhase=0;loadingAt=RTC_GetTicks();paintLoading(0);
 ok=fetchModelsWork();loadingActive=0;
 if(!ok)stopModelCheck();
 if(!ok){
  remoteModelCount=0;modelAccountChecked=0;remoteModels[0]=0;modelCheckFailed=!modelFetchCancelled;
  if(!quitting&&modelFetchCancelled&&modelStopPending)message("Stoppen niet bevestigd","Het stoppen van de accountcontrole is nog niet bevestigd. Controleer de Atom-verbinding. Een nieuwe chatvraag wordt pas verstuurd zodra de Atom weer beschikbaar is.");
  if(!quitting&&!modelFetchCancelled){
   char reason[512];const char *why=modelCheckReason(modelFetchError);
   sprintf(reason,"%s\n\nJe ziet de vaste modelkeuzes, niet je accountlijst. Je huidige model blijft bewaard. F2 Account probeert opnieuw.",why);
   message("Account niet gecontroleerd",reason);
  }
 }
 return ok;
}
static void paintModels(int selected){
 int i,count=modelViewCount(),first=selected-2;char label[80];if(first>count-5)first=count-5;if(first<0)first=0;clear("Model kiezen");
 sprintf(label,"%s · %d modellen",modelShowAll?"Alle modellen":"Aanbevolen",count);printAt(14,38,label,C_MUTED);
 for(i=first;i<count&&i<first+5;i++){
  int y=61+(i-first)*23;const char *name=modelViewId(i);
  choiceCard(8,y-3,368,21,i==selected,C_BLUE,C_BLUE_SOFT);
  clipped(18,y,modelDisplayName(name),33,i==selected?C_BLUE:C_TEXT);
  if(!strcmp(publicModel,name))printAt(310,y,"Actief",C_MINT);
 }
 if(!count){printAt(18,70,"Geen passende chatmodellen gevonden.",C_TEXT);printAt(18,96,"Controleer je account en vernieuw.",C_MUTED);}
 if(!modelShowAll&&publicModel[0]){
  int shown=0;for(i=0;i<count;i++)if(!strcmp(publicModel,modelViewId(i)))shown=1;
  if(!shown){printAt(14,145,"Actief:",C_MUTED);clipped(78,145,modelDisplayName(publicModel),36,C_TEXT);}
 }
 printAt(14,177,modelStopPending?"Stoppen accountcontrole niet bevestigd":modelAccountChecked?(count?"Account gecontroleerd · kies je model":"Account gecontroleerd · geen keuzes"):!count?"Je huidige keuze blijft bewaard.":modelCheckFailed?"Account niet gecontroleerd · vaste lijst":"Vaste keuzes · F2 controleert je account",C_MUTED);
 scrollbar(378,58,114,first,5,count);footer(count?"Kies":"","Account",modelShowAll?"Advies":"Alles",count?"Info":"","","");show();
}
static int modelPicker(void){
 int selected,k;char details[1024];
 if(!modelViewCount()&&choiceCount())modelShowAll=1;
 selected=modelSelection(publicModel);
 while(!quitting){
  int count=modelViewCount();if(selected>=count)selected=0;
  paintModels(selected);k=waitEditorKey(1);if(quitting)return 0;
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT)return 0;
  if(k==KEY_PRGM_UP&&selected>0)selected--;
  if(k==KEY_PRGM_DOWN&&selected<count-1)selected++;
  if(k==KEY_PRGM_F3){
   const char *highlighted=modelViewId(selected);modelShowAll=!modelShowAll;
   if(!modelViewCount()&&choiceCount())modelShowAll=1;
   selected=modelSelection(highlighted);continue;
  }
  if(k==KEY_PRGM_F2){
   if(!atomMajor){WFrame r;if(rpc(W_DEVICE,0,0,&r,0)==1&&r.len>=4){atomMajor=r.data[1];atomMinor=r.data[2];}}
   if(atomMajor<31)message("Atom bijwerken","Werk ook de Atom bij naar V31 met de meegeleverde firmware. Deze versie kan de accountcontrole echt stoppen. Wifi en sleutel blijven bewaard.");
   else if(!fetchModels()&&modelFetchCancelled)return 0;
   if(!modelViewCount()&&choiceCount())modelShowAll=1;
   selected=modelSelection(publicModel);continue;
  }
  if(k==KEY_PRGM_F4&&count){modelDetails(modelViewPreset(selected),details,sizeof(details));message("Over dit model",details);}
  if(count&&(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN||k==KEY_PRGM_RIGHT)){
   const char *name=modelViewId(selected);
   if(!setting_valid(4,name,strlen(name)))continue;
   if(!strcmp(publicModel,name))return 1;
   if(!ensureModelStopped()||!ensureQuestionStopped())continue;
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
  navigationFooter(1);show();k=waitKey();
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT)return;
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<2)selected++;
  if(k!=KEY_PRGM_RETURN&&k!=KEY_PRGM_RIGHT)continue;
  if(selected==0)screenSettings();
  if(selected==1)message("Over deze app","ChatGPT voor Casio fx-CG50\nVersie 31\n\nChats staan op je Casio. Wifi en je sleutel staan op de Atom.\n\nEigen API-tegoed en internet zijn nodig. AI kan fouten maken.");
  if(selected==2&&confirm("Wifi en API-sleutel wissen? Je opgeslagen chats blijven behouden.")){
   if(checkedRpc(W_WIPE,(const uint8_t*)"WIPE",4,&reply)){settingsEpoch++;settingsKnown=0;remoteModelCount=0;modelAccountChecked=0;publicSSID[0]=publicModel[0]=0;message("Gegevens gewist","Open Wifi en account om opnieuw in te stellen.");}
  }
 }
}
static void helpAndDevice(void){
 int selected=0,k,i;const char *names[]={"Aan de slag","Typen en toetsen","Antwoorden en kosten","Verbinding controleren"};
 const char *body[]={
 "1. Open Wifi en account.\n2. Volg de stappen op je telefoon.\n3. Kies een model.\n4. Typ je vraag en kies Stuur.\n\nJe hebt eigen OpenAI API-tegoed nodig. Een ChatGPT-abonnement bevat dit niet.",
 "ALPHA zet letters aan of uit en blijft actief na versturen. SHIFT werkt een keer voor wiskundefuncties. F4 opent speciale tekens. Links/rechts verplaatst de cursor; omhoog/omlaag leest je gesprek.\n\nEXIT gaat vanuit een chat terug naar het beginscherm. Rechts of EXE opent een keuze; links of EXIT gaat terug. MENU vraagt om afsluiten. Tijdens laden annuleert EXIT of F1 het wachten.",
 "Typ onderaan verder in hetzelfde gesprek. Omhoog/omlaag leest eerdere berichten. F1 Stuur of EXE verstuurt je bericht. Chats blijven lokaal op je Casio.\n\nAPI-gebruik kost geld via je eigen account. Controleer prijzen en antwoorden. Een al verstuurde vraag kan ook na annuleren kosten maken."
 };
 while(!quitting){
  clear("Hulp");
  for(i=0;i<4;i++){int y=48+i*31;choiceCard(8,y-4,368,27,i==selected,C_PURPLE,C_PURPLE_SOFT);printAt(18,y,names[i],i==selected?C_PURPLE:C_TEXT);glyph(362,y,'>',C_MUTED);}
  navigationFooter(1);show();k=waitKey();
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT)return;
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<3)selected++;
  if(k==KEY_PRGM_RETURN||k==KEY_PRGM_RIGHT){if(selected<3)message(names[selected],body[selected]);else diagnose();}
 }
}
static void paintSettings(int selected){
 int i;const char *names[]={"Wifi en account","Model","Hulp","Meer"};
 const char *notes[]={publicSSID[0]?publicSSID:"Instellen via telefoon",publicModel[0]?modelDisplayName(publicModel):"Kies je model","Zo werkt de app","Scherm en appinformatie"};
 clear("Instellingen");
 for(i=0;i<4;i++){
  static const unsigned short accent[4]={C_MINT,C_BLUE,C_PURPLE,C_GOLD};
  static const unsigned short tint[4]={C_RAISED,C_BLUE_SOFT,C_PURPLE_SOFT,C_GOLD_SOFT};
  int y=43+i*35;choiceCard(8,y-3,368,32,i==selected,accent[i],tint[i]);
  rounded(17,y+1,29,25,tint[i]);settingIcon(20,y+3,i,accent[i]);
  printAt(56,y,names[i],i==selected?accent[i]:C_TEXT);clipped(56,y+14,notes[i],36,C_MUTED);glyph(362,y+7,'>',accent[i]);
 }
 navigationFooter(1);show();
}
static void settings(void){
 int selected=0,k;uint8_t configured[4]={0};
 if(!settingsKnown&&linkOnline)refreshSettings(configured);
 while(!quitting){
  paintSettings(selected);k=waitEditorKey(1);
  if(k==UI_LINK_REFRESH){if(linkOnline&&!settingsKnown)refreshSettings(configured);continue;}
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT)return;
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<3)selected++;
  if(k!=KEY_PRGM_RETURN&&k!=KEY_PRGM_RIGHT)continue;
  if(selected==0){phoneSetup();remoteModelCount=0;modelAccountChecked=0;if(!quitting)refreshSettings(configured);}
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
 footer("Opnieuw","","","","","");show();
}
static void diagnose(void){
 int k,successes,checked,i,beforeRetries,beforeErrors;WFrame reply;
 do {
  successes=checked=0;beforeRetries=retries;beforeErrors=errors;clear("Verbinding controleren");rounded(160,53,64,58,C_RAISED);drawLogo(172,61,40);centered(127,"Even controleren...",C_TEXT);centered(149,"We kijken of alles goed is aangesloten.",C_MUTED);show();
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
   if(k==KEY_PRGM_EXIT||k==KEY_PRGM_LEFT)return;
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

#include "chats.h"
static void paintLoading(int phase){
 clear("ChatGPT");abstractLogo(192,108,56,phase);
 centered(196,"EXIT of F1: annuleren",C_MINT);show();
}
static void animateLoading(void){if(elapsed(loadingAt,120)){loadingAt=RTC_GetTicks();paintLoading(++loadingPhase);}}
/* Hiding the spinner and stopping the worker are separate steps. */
static int readQuestionStopStatus(void){
 WFrame r;
 if(rpc(W_STATUS,0,0,&r,0)==1&&r.len==10&&(r.data[1]==S_IDLE||r.data[1]==S_READY||r.data[1]==S_ERROR||r.data[1]==S_CANCELLED)){
  questionStopPending=0;return 1;
 }return 0;
}
static int ensureQuestionStopped(void){
 if(!questionStopPending)return 1;
 loadingActive=0;if(readQuestionStopStatus())return 1;
 cancelQuestion();if(!questionStopPending)return 1;
 message("Vorige vraag wordt gestopt","De Atom heeft het stoppen nog niet bevestigd. Controleer de verbinding en probeer straks opnieuw. Er wordt geen nieuwe vraag verstuurd. Een eerdere API-aanvraag kan nog kosten maken.");return 0;
}
static void cancelQuestion(void){
 WFrame ignored;int start=RTC_GetTicks(),leaving=quitting,modal=closePromptActive;
 loadingActive=0;questionStopPending=1;contextDirty=1;
 {clear("Vraag stoppen");centered(85,"Je vraag wordt gestopt.",C_TEXT);centered(111,"Een eerdere aanvraag kan geld kosten.",C_MUTED);show();}
 quitting=0;closePromptActive=1;
 if(rpc(W_CANCEL,0,0,&ignored,0)==1){
  while(questionStopPending&&!elapsed(start,8000)){
   if(readQuestionStopStatus())break;OS_InnerWait_ms(50);
  }
 }
 closePromptActive=modal;quitting=leaving;
}
static void questionCancelledMessage(void){
 if(quitting)return;
 message(questionStopPending?"Stoppen niet bevestigd":"Geannuleerd",questionStopPending?
  "De Atom heeft het stoppen nog niet bevestigd. Controleer de verbinding. Een eerdere aanvraag kan geld kosten; er wordt geen nieuwe vraag verstuurd.":
  "De Atom is gestopt met wachten op deze vraag. Een al verstuurde API-aanvraag kan nog kosten maken.");
}
static void askWork(void){
 WFrame r;uint8_t id[4];int start=RTC_GetTicks(),ok;uint8_t s,e;unsigned len;
 if(!ensureModelStopped()||!ensureQuestionStopped())return;
 if(!prompt[0]){message("Geen vraag","Typ eerst een vraag.");return;}
 loadLibrary();
 if(unsavedReply){message("Bewaar het antwoord","Het vorige antwoord is nog niet opgeslagen. Open het gesprek en probeer F3 Bewaar, of verwerp het bewust via Nieuw.");return;}
 if(activeChatSlot>=0&&savedChats[activeChatSlot].turns>=CHAT_TURNS){message("Gesprek vol","Dit gesprek bevat acht vragen en antwoorden. Begin een nieuw gesprek; de oude berichten blijven lokaal bewaard.");return;}
 if(activeChatSlot<0&&freeSlot(savedChats)<0){message("Chats vol","Er zijn acht opgeslagen gesprekken. Verwijder eerst bewust een oud gesprek via OPTN. Er is nog geen API-vraag gestart.");return;}
 if(!restoreChat())return;
 if(!uploadText(0,prompt))return;
 if(!checkedRpc(W_ALLOC,0,0,&r)||r.len!=5)return;memcpy(id,r.data+1,4);
 ok=rpc(W_ASK,id,4,&r,0);
 if(ok!=1){
  uint8_t error=ok==0?r.data[0]:0;cancelQuestion();
  if(!quitting){
   if(error)message("Vraag niet gestart",error<sizeof(errNames)/sizeof(errNames[0])?errNames[error]:"Onbekende fout");
   else message("Versturen niet bevestigd","De vraag kan al verstuurd zijn. Er is om stoppen gevraagd en er wordt niet automatisch opnieuw betaald aangevraagd.");
  }return;
 }
 while(!quitting){
  ok=rpc(W_STATUS,0,0,&r,1);
  if(ok==-2){cancelQuestion();questionCancelledMessage();return;}
  if(ok!=1||r.len!=10){cancelQuestion();if(!quitting)message("Verbinding onderbroken","De aanvraag kan al verstuurd zijn. Er wordt niet automatisch een nieuwe API-aanvraag gedaan. Open Instellingen, Hulp en Verbinding controleren.");return;}
  s=r.data[1];e=r.data[2];len=w_get16(r.data+3);
  if(w_get32(r.data+6)!=w_get32(id)){message("Atom herstart","Aanvraagstatus hoort niet meer bij deze vraag. Er is geen automatische herhaling bij de API.");return;}
  if(s==S_READY){int dropped=(r.data[5]&0x80)!=0;if(fetchAnswer(len)){loadingActive=0;strcpy(lastQuestion,prompt);prompt[0]=0;unsavedReply=1;saveReply();chatFollowTail=1;invalidateConversation();(void)dropped;}return;}
  if(s==S_ERROR){message("Aanvraag mislukt",e<sizeof(errNames)/sizeof(errNames[0])?errNames[e]:"Onbekende fout");return;}
  if(s==S_CANCELLED){message("Geannuleerd","Er wordt geen antwoord opgehaald.");return;}
  paintLoading(loadingPhase);
  {int wait=RTC_GetTicks();while(!elapsed(wait,250)&&!quitting){
   animateLoading();int k=keyEvent();if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F1){cancelQuestion();questionCancelledMessage();return;}OS_InnerWait_ms(10);
  }}
  if(quitting){cancelQuestion();return;}
  if(elapsed(start,150000)){cancelQuestion();message("Time-out","Geen antwoord binnen 150 seconden. Er wordt geen nieuwe API-aanvraag gestart.");return;}
 }
}
static void ask(void){
 loadingActive=prompt[0]!=0;loadingPhase=0;loadingAt=RTC_GetTicks();if(loadingActive)paintLoading(0);
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
 linkMisses=0;settingsKnown=0;remoteModelCount=0;modelAccountChecked=0;modelShowAll=0;loadingActive=0;rpcDepth=0;atomMajor=0;atomMinor=0;atomBootKnown=0;lastAtomBoot=0;settingsEpoch=0;
 prompt[0]=answer[0]=lastQuestion[0]=publicSSID[0]=publicModel[0]=0;chatInputLetters=0;chatFollowTail=1;invalidateConversation();
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
   k=editor(prompt,sizeof(prompt),"ChatGPT",0,1,0,1);
   if(k==KEY_PRGM_F1)ask();
   else if(k==KEY_PRGM_F2){if((!prompt[0]||confirm("De niet-verstuurde vraag wissen en opnieuw beginnen?"))&&canLeaveReply())newLocalChat();}
   else if(k==UI_CHATS){int changed=chatSidebar();if(changed){chatFollowTail=1;invalidateConversation();}}
   else if(k==KEY_PRGM_F6){epoch=settingsEpoch;settings();if(epoch!=settingsEpoch)contextDirty=1;}
   else if(k==KEY_PRGM_EXIT)chatHome();
  }
  if(!unsavedReply)break;
  resumeAfterMenu();if(saveReply()||canLeaveReply()){quitting=1;break;}
 }
}
static void endSession(void){
 /* Finish local work; keep the OS-owned UART driver open through native MENU.
  * No app timer, interrupt handler or quit callback remains active in MENU. */
 storageClose();releaseModels();erase(field,sizeof(field));waitForKeyRelease();stopAutoProbe();
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
