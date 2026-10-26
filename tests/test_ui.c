/* Host-only device stubs exercise the real editor, not OpenAI functionality. */
#include <stdlib.h>
#define main casio_program_main
#include "../casio/main.c"
#undef main
#include <assert.h>
#include "../atom/ChatGPTAtom/upload.h"
static unsigned short framebuffer[384*216];
static unsigned char rx[8192];static size_t rxAt,rxEnd;
static int connected=1,busyUntil,requestCount[32],privateReads;
static char storedSSID[33]="MijnHotspot",storedModel[81]="gpt-4.1-mini";
static Upload fakeUpload;
static const int *keys;static size_t keyCount,keyAt;static int timeTicks;
void *GetVRAMAddress(void){return framebuffer;}
void Bdisp_AllClr_VRAM(void){for(size_t i=0;i<384*216;i++)framebuffer[i]=0xffff;}
void Bdisp_PutDisp_DD(void){}
void Bdisp_EnableColor(int n){(void)n;}
int PRGM_GetKey(void){if(timeTicks<busyUntil)return 0;if(keyAt>=keyCount){fprintf(stderr,"Editor exceeded key script\n");abort();}return keys[keyAt++];}
int RTC_GetTicks(void){return timeTicks;}
int RTC_Elapsed_ms(int start,int ms){return timeTicks-start>=ms;}
void OS_InnerWait_ms(int ms){timeTicks+=ms;}
int Serial_ClearTX(void){return 0;}int Serial_ClearRX(void){rxAt=rxEnd=0;return 0;}
int Serial_Close(int n){(void)n;return 0;}int Serial_IsOpen(void){return 0;}
int Serial_Open(unsigned char *p){(void)p;return 0;}
int Serial_PollTX(void){return 256;}int Serial_PollRX(void){return (int)(rxEnd-rxAt);}
int Serial_ReadSingle(unsigned char*p){if(rxAt>=rxEnd)return 1;*p=rx[rxAt++];return 0;}
int Serial_Write(const unsigned char*p,int n){
 WParser parser={0};WFrame f,r={0};int got=0;for(int i=0;i<n;i++)if(w_feed(&parser,p[i],&f)==1)got=1;
 assert(got&&f.type<32);requestCount[f.type]++;
 if(!connected){busyUntil=timeTicks+800;return 0;}
 r.type=f.type|0x80;r.seq=f.seq;r.len=1;
 switch(f.type){
 case W_INFO:r.len=5;for(int i=1;i<5;i++)r.data[i]=1;break;
 case W_DEVICE:r.len=4;r.data[1]=2;r.data[2]=0;r.data[3]=1;break;
 case W_GET_CONFIG:{
  if(f.data[0]!=1&&f.data[0]!=4){privateReads++;r.data[0]=E_REQUEST;break;}
  const char *value=f.data[0]==1?storedSSID:storedModel;size_t at=w_get16(f.data+1),len=strlen(value),amount=len-at;
  if(amount>22)amount=22;r.len=amount+2;r.data[1]=len;memcpy(r.data+2,value+at,amount);break;
 }
 case W_BEGIN:r.data[0]=upload_begin(&fakeUpload,f.data[0],w_get16(f.data+1));break;
 case W_APPEND:r.data[0]=upload_append(&fakeUpload,w_get16(f.data),f.data+2,f.len-2);break;
 case W_APPLY:
  assert(upload_complete(&fakeUpload));
  if(fakeUpload.target==4)strcpy(storedModel,fakeUpload.bytes);
  if(fakeUpload.target==1)strcpy(storedSSID,fakeUpload.bytes);
  upload_clear(&fakeUpload);break;
 case W_HELLO:r.len=5;r.data[1]=1;break;
 default:r.data[0]=E_REQUEST;
 }
 if(rxAt==rxEnd)rxAt=rxEnd=0;
 rxEnd+=w_encode(&r,rx+rxEnd);assert(rxEnd<=sizeof(rx));return 0;
}
void SetSetupSetting(unsigned int n,unsigned char v){(void)n;(void)v;}
unsigned char GetSetupSetting(unsigned int n){(void)n;return 0;}
void SetQuitHandler(void(*h)(void)){(void)h;}
void DisplayMainMenu(void){}
static void script(const int *p,size_t n){keys=p;keyCount=n;keyAt=0;lastKey=0;quitting=0;vram=framebuffer;busyUntil=0;}
#define SCRIPT(a) script(a,sizeof(a)/sizeof(a[0]))
static void dumpFrame(const char*path){
 FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n384 216\n255\n");
 for(int i=0;i<384*216;i++){unsigned short v=framebuffer[i];unsigned char rgb[3]={(v>>11)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);
}
int main(int argc,char **argv){
 char b[128]="";
 const int hello[]={78,0,65,0,36,0,25,0,25,0,54,0,79};SCRIPT(hello);
 assert(editor(b,sizeof(b),"Test",0,1,0,1)==79);assert(!strcmp(b,"Hello"));
 const int exact[]={78,0,76,0,39,0,66,0,39,0,56,0,79};b[0]=0;SCRIPT(exact);
 assert(editor(b,sizeof(b),"API",1,0,1,0)==79);assert(!strcmp(b,"aBc"));
 /* Secret editor displays '*' in its first content cell. */
 rect(0,0,384,216,C_PANEL);glyph(12,52,'*',C_TEXT);unsigned short expected[14*8];
 for(int y=0;y<14;y++)for(int x=0;x<8;x++)expected[y*8+x]=framebuffer[(52+y)*384+12+x];
 const int done[]={79};SCRIPT(done);editor(b,sizeof(b),"API",1,0,1,0);
 for(int y=0;y<13;y++)for(int x=0;x<8;x++)assert(framebuffer[(52+y)*384+12+x]==expected[y*8+x]);
 /* One SHIFT locks letters for every subsequent key; second restores digits. */
 const int mixed[]={78,0,76,0,66,0,78,0,72,0,79};b[0]=0;SCRIPT(mixed);editor(b,sizeof(b),"Test",0,1,0,1);assert(!strcmp(b,"Ab1"));
 const int edit[]={38,0,44,0,79};strcpy(b,"Aéπ");SCRIPT(edit);editor(b,sizeof(b),"Test",0,0,0,1);assert(!strcmp(b,"Aπ"));
 /* Offline settings must open without a misleading configuration popup. */
 const int leave[]={47};connected=0;SCRIPT(leave);settings();assert(!linkOnline&&requestCount[W_INFO]>0);
 connected=1;strcpy(storedModel,"my-custom-model");
 const int modelEdit[]={37,0,37,0,37,0,79,0,79,0,79,0,47};SCRIPT(modelEdit);settings();
 assert(!strcmp(storedModel,"my-custom-model")&&privateReads==0);
 assert(field[0]==0&&atomMajor==2);
 /* Last page cannot scroll into blank rows. */
 strcpy(answer,"one\ntwo\nthree\nfour\nfive\nsix\nseven");strcpy(lastQuestion,"Test");
 const int scroll[]={27,0,27,0,37,0,47};SCRIPT(scroll);readAnswer(0);
 if(argc==3&&!strcmp(argv[1],"--screens")){
  strcpy(storedModel,"gpt-4.1-mini");
  char path[512];snprintf(path,sizeof(path),"%s/input.ppm",argv[2]);
  strcpy(b,"Hoe los ik 2x + 5 = 17 op?");strcpy(prompt,b);const int shotType[]={78,0,79};SCRIPT(shotType);editor(prompt,sizeof(prompt),"ChatGPT",0,1,0,1);dumpFrame(path);
  strcpy(lastQuestion,b);strcpy(answer,"Trek eerst 5 van beide kanten af.\n2x = 12\n\nDeel daarna door 2: x = 6.");
  SCRIPT(leave);readAnswer(0);snprintf(path,sizeof(path),"%s/answer.ppm",argv[2]);dumpFrame(path);
  SCRIPT(leave);settings();snprintf(path,sizeof(path),"%s/settings.ppm",argv[2]);dumpFrame(path);
  connected=0;SCRIPT(leave);settings();snprintf(path,sizeof(path),"%s/offline.ppm",argv[2]);dumpFrame(path);
 }
 puts("PASS: actual Casio editor with host device stubs: SHIFT lock, automatic first capital, exact secret case, masked display, digit toggle, UTF-8 cursor/deletion, offline settings, secret read restrictions, saved model editing and answer paging.");
 return 0;
}
