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
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "../atom/ChatGPTAtom/wire.h"
#include "text.h"
#include "font.h"
#define COLS 46
#define ROWS 9
#define PROMPT_MAX 1024
#define ANSWER_MAX 8192
static char prompt[PROMPT_MAX+1],answer[ANSWER_MAX+1],field[PROMPT_MAX+1];
static int quitting=0,serialOpen=0,lastKey=0,errors=0,retries=0;
static unsigned char oldSetup=0;
static int heldTick=0,repeatTick=0;
static uint8_t seq=0;
static WParser parser;
static unsigned short *vram;
static const char *const errNames[]={
 "Geen fout","Vul eerst Instellingen in","Wifi niet verbonden: controleer hotspot",
 "Internet of DNS niet bereikbaar","Tijdsynchronisatie mislukt","TLS/HTTPS-verbinding mislukt",
 "API-sleutel ongeldig of geen toegang","API-limiet of onvoldoende tegoed",
 "Model niet beschikbaar","API gaf een fout terug","Antwoord of invoer te groot",
 "Antwoord niet leesbaar","Antwoord-time-out","Atom is nog bezig",
 "Upload mislukt","Atom herstart; vraag niet opnieuw verstuurd",
 "Geheugen of opslag niet beschikbaar","Ongeldig bericht"};
static const char *const phases[]={"Gereed","Wifi verbinden...","Klok synchroniseren...","Model controleren...","Antwoord ophalen...","Antwoord ontvangen","Fout","Geannuleerd"};
static void erase(void *p,size_t n){volatile unsigned char *v=p;while(n--)*v++=0;}
static int elapsed(int start,int ms){return RTC_Elapsed_ms(start,ms);}
static void closeSerial(void){
 if(serialOpen){Serial_ClearTX();Serial_Close(1);serialOpen=0;}
 SetSetupSetting(0x14,oldSetup);
}
static void cleanup(void){closeSerial();erase(field,sizeof(field));}
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
static int glyphIndex(uint32_t cp){size_t i;for(i=0;i<FONT_COUNT;i++)if(font_glyphs[i].code==cp)return (int)i;return -1;}
static void glyph(int x,int y,uint32_t cp,unsigned short color){
 int i=glyphIndex(cp),dx,dy;if(i<0)i=glyphIndex(0xfffd);
 for(dy=0;dy<14;dy++)for(dx=0;dx<8;dx++)if(x+dx>=0&&x+dx<384&&y+dy>=0&&y+dy<216&&font_glyphs[i].rows[dy]&(1<<(7-dx)))vram[(y+dy)*384+x+dx]=color;
}
static void printAt(int x,int y,const char *s,unsigned short color){uint32_t cp;size_t pos=0,len=strlen(s),n;
 while(pos<len&&x<384){n=u_decode(s+pos,len-pos,&cp);glyph(x,y,cp,color);pos+=n;x+=8;}
}
static void clear(const char *title){Bdisp_AllClr_VRAM();printAt(8,3,title,0x0280);}
static void show(void){Bdisp_PutDisp_DD();}
static void footer(const char *a,const char*b,const char*c,const char*d,const char*e,const char*f){
 const char *p[6];int i;p[0]=a;p[1]=b;p[2]=c;p[3]=d;p[4]=e;p[5]=f;
 for(i=0;i<6;i++)printAt(i*64,198,p[i],0x0280);
}
static void textPage(const char *s,int first,int hide){
 size_t at=0,len=strlen(s),n;uint32_t cp;int row=0,col=0;
 while(at<len){n=u_decode(s+at,len-at,&cp);at+=n;
  if(cp=='\r')continue;
  if(cp=='\n'){row++;col=0;continue;}
  if(col==COLS){row++;col=0;}
  if(row>=first&&row<first+ROWS)glyph(8+col*8,40+(row-first)*14,hide?'*':cp,0);
  col++;
 }
}
static void cursorPosition(const char *s,size_t cursor,int *row,int *col){
 size_t p=0,n;uint32_t cp;*row=*col=0;
 while(p<cursor){n=u_decode(s+p,cursor-p,&cp);p+=n;
  if(cp=='\n'){(*row)++;*col=0;continue;}if(cp=='\r')continue;
  if(*col==COLS){(*row)++;*col=0;}(*col)++;
 }
 if(*col==COLS){(*row)++;*col=0;}
}
static void message(const char *title,const char *body){clear(title);textPage(body,0,0);footer("Terug","","","","","");show();waitKey();}
static int confirm(const char *question){clear("Bevestigen");textPage(question,0,0);footer("Ja","Nee","","","","");show();
 while(!quitting){int k=waitKey();if(k==KEY_PRGM_F1)return 1;if(k==KEY_PRGM_F2||k==KEY_PRGM_EXIT)return 0;}return 0;
}
/* RPC status: 1 success, 0 device error (reply data[0]), -1 timeout,
 * -2 user cancelled/menu. Identical encoded bytes are retried at most 4 times. */
static int rpc(int type,const uint8_t *data,size_t len,WFrame *reply,int cancellable){
 WFrame req;uint8_t bytes[WIRE_ENCODED];size_t n;int attempt,start,k;
 if(len>WIRE_MAX)return -1;
 req.type=(uint8_t)type;req.seq=++seq;req.len=(uint8_t)len;if(len)memcpy(req.data,data,len);
 n=w_encode(&req,bytes);
 for(attempt=0;attempt<4&&!quitting;attempt++){
  if(attempt)retries++;
  if(Serial_Write(bytes,(int)n)!=0){OS_InnerWait_ms(50);continue;}
  start=RTC_GetTicks();
  while(!elapsed(start,800)&&!quitting){
   while(Serial_PollRX()>0){unsigned char c;WFrame f;int ok;
    if(Serial_ReadSingle(&c)!=0)break;ok=w_feed(&parser,c,&f);if(ok<0)errors++;
    if(ok==1&&f.seq==req.seq&&f.type==(req.type|0x80)&&f.len){*reply=f;return f.data[0]?0:1;}
   }
   k=keyEvent();if(quitting||(cancellable&&k==KEY_PRGM_EXIT))return -2;
   OS_InnerWait_ms(5);
  }
  memset(&parser,0,sizeof(parser));
 }return quitting?-2:-1;
}
static int checkedRpc(int type,const uint8_t *data,size_t len,WFrame *reply){
 int ok=rpc(type,data,len,reply,0);
 if(ok==1)return 1;if(quitting)return 0;
 if(ok==0){uint8_t e=reply->data[0];message("Atom meldt",e<sizeof(errNames)/sizeof(errNames[0])?errNames[e]:"Onbekende fout");}
 else message("Geen verbinding","Atom niet bereikbaar. Controleer USB-voeding, TX/RX, GND en UART-instellingen. Er is niets automatisch opnieuw bij de API aangevraagd.");
 return 0;
}
static int uploadText(int target,const char *s){
 size_t len=strlen(s),pos=0,n;uint8_t b[WIRE_MAX];WFrame r;
 b[0]=(uint8_t)target;w_put16(b+1,(uint16_t)len);
 if(!checkedRpc(W_BEGIN,b,3,&r))return 0;
 while(pos<len&&!quitting){n=len-pos;if(n>WIRE_MAX-2)n=WIRE_MAX-2;w_put16(b,(uint16_t)pos);memcpy(b+2,s+pos,n);
  if(!checkedRpc(W_APPEND,b,n+2,&r))return 0;pos+=n;
 }return checkedRpc(W_APPLY,0,0,&r);
}
/* All printable ASCII plus a named Unicode repertoire. API keys use ASCII only. */
static uint32_t pickCharacter(int asciiOnly){
 static const char extras[]="éëèêáàâäíïìîóöòôúüùûçñÉËπθαβγδμωφ∞√≤≥≠×÷±∑∫∆→←↑↓²³°€";
 uint32_t choices[180];int count=0,i,sel=0,first=0,k;size_t pos=0,len=strlen(extras);uint32_t cp;
 for(i=32;i<=126;i++)choices[count++]=(uint32_t)i;
 if(!asciiOnly)while(pos<len){pos+=u_decode(extras+pos,len-pos,&cp);choices[count++]=cp;}
 while(!quitting){char label[80];clear("Kies een teken");
  sprintf(label,"Teken %d/%d: U+%04lX",sel+1,count,(unsigned long)choices[sel]);printAt(8,20,label,0);
  if(sel<first)first=(sel/12)*12;if(sel>=first+96)first=(sel/12-7)*12;
  for(i=first;i<count&&i<first+96;i++){
   int x=8+((i-first)%12)*30,y=44+((i-first)/12)*16;glyph(x,y,choices[i],i==sel?0x0280:0);
   if(i==sel){int x2;for(x2=x;x2<x+16;x2++)vram[(y+14)*384+x2]=0x0280;}
  }
  printAt(8,178,"Pijlen kiezen; EXE invoegen",0);footer("Kies","","","","","");show();k=waitKey();
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
  clear(title);sprintf(label,"%s | %u/%u bytes | SHIFT: letters",letters?(upper?"ABC":firstLetter?"Abc":"abc"):"123",(unsigned)strlen(buf),(unsigned)(cap-1));printAt(8,20,label,0);
  textPage(buf,first,secret);
  {int x=8+col*8,y=40+(row-first)*14+13,i;for(i=0;i<8;i++)if(y<174)vram[y*384+x+i]=0x0280;}
  if(promptMode)footer("Stuur","Nieuw","Test","Tekens","ABC/abc","Instel");else footer("Opslaan","","","Tekens","ABC/abc","");
  printAt(8,178,"DEL wist; pijlen verplaatsen; EXIT terug",0);show();k=waitKey();
  if(quitting)return KEY_PRGM_MENU;
  if(k==KEY_PRGM_SHIFT){letters=!letters;continue;}
  if(k==KEY_PRGM_ALPHA||k==KEY_PRGM_F5){upper=!upper;continue;}
  if(k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)return KEY_PRGM_F1;
  if(k==KEY_PRGM_EXIT)return k;
  if(promptMode&&(k==KEY_PRGM_F2||k==KEY_PRGM_F3||k==KEY_PRGM_F6))return k;
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
static void settings(void){
 int selected=0,k,i;WFrame r;uint8_t configured[4]={0};
 const char *names[]={"Hotspotnaam","Hotspotwachtwoord","OpenAI API-sleutel","Model","Taal: Nederlands","Alle instellingen wissen"};
 const size_t caps[]={33,64,257,81};
 if(checkedRpc(W_INFO,0,0,&r)&&r.len==5)memcpy(configured,r.data+1,4);
 while(!quitting){clear("ChatGPT - Instellingen");
  for(i=0;i<6;i++){char s[70];sprintf(s,"%c %s%s",i==selected?'>':' ',names[i],i<4?(configured[i]?" [ingesteld]":" [leeg]"):"");printAt(8,40+i*20,s,0);}
  printAt(8,174,"Geheimen worden niet teruggelezen.",0);footer("Wijzig","","","","","");show();k=waitKey();
  if(k==KEY_PRGM_EXIT)break;if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<5)selected++;
  if(k!=KEY_PRGM_F1&&k!=KEY_PRGM_RETURN)continue;
  if(selected==4){message("Taal","Deze versie gebruikt Nederlands voor menu's en als standaard antwoordtaal.");continue;}
  if(selected==5){if(confirm("Hotspotgegevens en API-sleutel op de Atom wissen?")){
    if(checkedRpc(W_WIPE,(const uint8_t*)"WIPE",4,&r))memset(configured,0,4);
   }continue;}
  erase(field,sizeof(field));
  if(selected==3)strcpy(field,"gpt-4.1-mini");
  k=editor(field,caps[selected],names[selected],selected==1||selected==2,0,selected==2||selected==3,0);
  if(k==KEY_PRGM_F1){if(uploadText(selected+1,field)){configured[selected]=field[0]!=0;message("Opgeslagen","Instelling is op de Atom opgeslagen.");}}
  erase(field,sizeof(field));
 }
}
static void diagnose(void){WFrame r;char text[350];int beforeR=retries,beforeE=errors;
 clear("UART-diagnose");printAt(8,40,"Atom wordt getest...",0);show();
 if(!checkedRpc(W_HELLO,0,0,&r))return;
 sprintf(text,"Atom antwoordt op protocol v%u.\n9600 baud, 8N1, geen inversie.\n\nHerhalingen deze test: %d\nCRC/framefouten deze test: %d\n\nEen geldig antwoord controleert UART-verkeer in beide richtingen. Dit controleert geen wifi of API-toegang.",r.len>1?r.data[1]:0,retries-beforeR,errors-beforeE);
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
 int first=0,k,lines,col;size_t cursor=strlen(answer);char status[80];
 cursorPosition(answer,cursor,&lines,&col);lines++;
 while(!quitting){clear("ChatGPT - Antwoord");sprintf(status,"Regel %d/%d%s",first+1,lines,contextWasDropped?" | Oudste context gewist":"");printAt(8,20,status,0);
  textPage(answer,first,0);printAt(8,174,"Pijlen scrollen; F1 vervolgvraag",0);footer("Vraag","Nieuw","Test","","","Instel");show();k=waitKey();
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F1){prompt[0]=0;return;}
  if(k==KEY_PRGM_UP&&first>0)first--;if(k==KEY_PRGM_DOWN&&first+ROWS<lines)first++;
  if(k==KEY_PRGM_LEFT){first-=ROWS;if(first<0)first=0;}
  if(k==KEY_PRGM_RIGHT){first+=ROWS;if(first>lines-1)first=lines-1;}
  if(k==KEY_PRGM_F2&&confirm("Nieuw gesprek beginnen? De oude context wordt gewist.")){
   WFrame r;if(checkedRpc(W_NEW,0,0,&r)){prompt[0]=answer[0]=0;return;}}
  if(k==KEY_PRGM_F3)diagnose();if(k==KEY_PRGM_F6)settings();
 }
}
static void ask(void){
 WFrame r;uint8_t id[4];int start=RTC_GetTicks(),ok;uint8_t s,e;unsigned len;
 if(!prompt[0]){message("Geen vraag","Typ eerst een vraag.");return;}
 if(!uploadText(0,prompt))return;
 if(!checkedRpc(W_ALLOC,0,0,&r)||r.len!=5)return;memcpy(id,r.data+1,4);
 if(!checkedRpc(W_ASK,id,4,&r))return;
 while(!quitting){
  ok=rpc(W_STATUS,0,0,&r,1);
  if(ok==-2){WFrame ignored;rpc(W_CANCEL,0,0,&ignored,0);if(!quitting)message("Annulering aangevraagd","Een al verstuurde API-aanvraag kan nog kosten maken. De Atom moet de netwerkbewerking eerst afronden.");return;}
  if(ok!=1||r.len!=10){if(!quitting)message("Verbinding onderbroken","De aanvraag kan al verstuurd zijn. Er wordt niet automatisch een nieuwe API-aanvraag gedaan. Controleer de Atom met F3.");return;}
  s=r.data[1];e=r.data[2];len=w_get16(r.data+3);
  if(w_get32(r.data+6)!=w_get32(id)){message("Atom herstart","Aanvraagstatus hoort niet meer bij deze vraag. Er is geen automatische herhaling bij de API.");return;}
  if(s==S_READY){if(fetchAnswer(len))readAnswer((r.data[5]&0x80)!=0);return;}
  if(s==S_ERROR){message("Aanvraag mislukt",e<sizeof(errNames)/sizeof(errNames[0])?errNames[e]:"Onbekende fout");return;}
  if(s==S_CANCELLED){message("Geannuleerd","Er wordt geen antwoord opgehaald.");return;}
  clear("ChatGPT - Bezig");printAt(8,40,s<sizeof(phases)/sizeof(phases[0])?phases[s]:"Status onbekend",0);
  printAt(8,80,"EXIT: annuleren; MENU: afsluiten",0);show();
  {int wait=RTC_GetTicks();while(!elapsed(wait,250)&&!quitting){int k=keyEvent();if(k==KEY_PRGM_EXIT){WFrame ignored;rpc(W_CANCEL,0,0,&ignored,0);message("Annulering aangevraagd","Een reeds verstuurde API-aanvraag kan nog kosten maken.");return;}OS_InnerWait_ms(10);}}
  if(elapsed(start,150000)){WFrame ignored;rpc(W_CANCEL,0,0,&ignored,0);message("Time-out","Geen antwoord binnen 150 seconden. Er wordt geen nieuwe API-aanvraag gestart.");return;}
 }
}
int main(void){
 unsigned char mode[6]={0,5,0,0,0,0};int k;WFrame r;
 vram=(unsigned short*)GetVRAMAddress();oldSetup=GetSetupSetting(0x14);SetQuitHandler(cleanup);
 if(Serial_IsOpen()==1){message("UART bezet","Sluit een andere seriele toepassing eerst af.");return 0;}
 if(Serial_Open(mode)!=0){message("UART-fout","De seriele poort kan niet worden geopend.");return 0;}
 serialOpen=1;Serial_ClearRX();Serial_ClearTX();Bdisp_EnableColor(1);
 while(!quitting){
  k=editor(prompt,sizeof(prompt),"ChatGPT - Vraag",0,1,0,1);
  if(k==KEY_PRGM_F1)ask();
  else if(k==KEY_PRGM_F2){if(confirm("Nieuw gesprek beginnen?")){if(checkedRpc(W_NEW,0,0,&r))prompt[0]=answer[0]=0;}}
  else if(k==KEY_PRGM_F3)diagnose();else if(k==KEY_PRGM_F6)settings();
  else if(k==KEY_PRGM_EXIT){if(confirm("ChatGPT sluiten en terug naar MENU?"))quitting=1;}
 }
 /* Best-effort cancellation. PollTX reports free space, not queued bytes.
  * At 9600 baud, 150ms covers our short frame plus one pending max frame. */
 {WFrame cancel={0};uint8_t bytes[WIRE_ENCODED];size_t n;
  cancel.type=W_CANCEL;cancel.seq=++seq;n=w_encode(&cancel,bytes);
  Serial_Write(bytes,(int)n);OS_InnerWait_ms(150);
 }
 cleanup();DisplayMainMenu();return 0;
}
