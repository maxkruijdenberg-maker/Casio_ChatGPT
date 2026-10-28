/* Host-only device stubs exercise the real editor, not OpenAI functionality. */
#include <stdlib.h>
#include <setjmp.h>
#define CHATGPT_HOST_TEST 1
#define main casio_program_main
#include "../casio/main.c"
#undef main
#include <assert.h>
#include "../atom/ChatGPTAtom/upload.h"
static unsigned short framebuffer[384*216],otherFramebuffer[384*216],lcd[384*216],startupSnapshot[384*216];
static unsigned short *deviceFramebuffer=framebuffer;
static int statusMode,stripeCalls=0,fullCopies=0,colorMode=0,keysCleared=0,initKeys=0;
static unsigned short lcdFrame=0xffff;
static int dma[4]={24,300,24,191},failCreate=0;
static int nativeQueued=0,lifecycleTesting=0,nativeMenus=0;
static int menuVisible=0,menuReturnEnabled=0,serialCloses=0;
static int copiesAtMenu=0,rawMenuQueued=0,drainedEvents=0;
static int staleEvents=0,nativeResolverReady=1,nativeDirectCalls=0,nativeTimerCalls=0;
static int inNativeWait=0,timerArmed=0,timerInstalled=0;
static int failTimerInstall=0,failTimerStart=0,interruptNativeWait=0,timerStops=0,timerDeletes=0;
static void (*timerCallback)(void);
static jmp_buf lifecycleDone;
static void (*quitCallback)(void);
static int fakeAtomMajor=3,fakePortal=0;
static void menuReturned(void);
typedef struct {unsigned char*data;size_t size,pos;} TestFile;
static TestFile testFiles[32];
static int pathIndex(const unsigned short *wide){
 char name[80];size_t i=0;while(wide[i]&&i<79){name[i]=(char)wide[i];i++;}name[i]=0;
 const char *root="\\\\fls0\\CGPT3\\";assert(!strncmp(name,root,strlen(root)));
 char *base=strrchr(name,'\\');assert(base);base++;
 assert((base[0]=='C'||base[0]=='D')&&base[1]>='0'&&base[1]<='7'&&(base[2]=='A'||base[2]=='B'));
 return (base[0]=='D'?16:0)+(base[1]-'0')*2+(base[2]=='B');
}
int Bfile_CreateEntry_OS(const unsigned short*p,int mode,size_t*size){
 if(mode==CREATEMODE_FOLDER)return 0;if(failCreate)return -1;int i=pathIndex(p);if(testFiles[i].data)return -13;
 testFiles[i].data=calloc(1,*size);testFiles[i].size=*size;testFiles[i].pos=0;return testFiles[i].data?0:-1;
}
int Bfile_DeleteEntry(const unsigned short*p){int i=pathIndex(p);free(testFiles[i].data);memset(&testFiles[i],0,sizeof(testFiles[i]));return 0;}
int Bfile_OpenFile_OS(const unsigned short*p,int mode,int zero){(void)mode;(void)zero;int i=pathIndex(p);if(!testFiles[i].data)return -1;testFiles[i].pos=0;return i+1;}
int Bfile_CloseFile_OS(int h){(void)h;return 0;}
int Bfile_ReadFile_OS(int h,void*out,int size,int pos){
 TestFile*f=&testFiles[h-1];size_t at=pos<0?f->pos:(size_t)pos,n=size;if(at>f->size)return -1;
 if(n>f->size-at)n=f->size-at;memcpy(out,f->data+at,n);f->pos=at+n;return (int)n;
}
int Bfile_SeekFile_OS(int h,int pos){TestFile*f=&testFiles[h-1];if(pos<0||(size_t)pos>f->size)return -1;f->pos=pos;return 0;}
int Bfile_WriteFile_OS(int h,const void*data,int size){TestFile*f=&testFiles[h-1];if((size_t)size>f->size-f->pos)return -1;memcpy(f->data+f->pos,data,size);f->pos+=size;return size;}
void EnableStatusArea(int mode){statusMode=mode;}
void Bdisp_PutDisp_DD_stripe(int first,int last){(void)first;(void)last;stripeCalls++;}
void Bdisp_DefineDMARange(int x1,int x2,int y1,int y2){dma[0]=x1;dma[1]=x2;dma[2]=y1;dma[3]=y2;}

static unsigned char rx[8192];static size_t rxAt,rxEnd;
static int osSerialOpen=0,serialOpens=0,openFailures=0,writeFailures=0,dropPortOnWriteFailure=0;
static unsigned char failedWrite[WIRE_ENCODED];static int failedWriteLength=0,expectSameWrite=0;
static int connected=1,busyUntil,requestCount[32],privateReads,replyDelay=0,replyReadyAt=0;
static int keyAdvanceMs=0,connectAt=-1;
static const char *servedAnswer=0;
static uint32_t fakeBoot=100;
static char storedSSID[33]="MijnHotspot",storedModel[81]="gpt-4.1-mini";
static Upload fakeUpload;
static const int *keys;static size_t keyCount,keyAt;static int timeTicks;
void *GetVRAMAddress(void){return deviceFramebuffer;}
void Bdisp_AllClr_VRAM(void){for(size_t i=0;i<384*216;i++)framebuffer[i]=0xffff;}
void Bdisp_PutDisp_DD(void){
 assert(colorMode==1&&statusMode==3);assert(dma[0]>=0&&dma[1]<384&&dma[2]>=0&&dma[3]<216);
 for(int y=dma[2];y<=dma[3];y++)memcpy(lcd+y*384+dma[0],deviceFramebuffer+y*384+dma[0],(dma[1]-dma[0]+1)*sizeof(unsigned short));fullCopies++;
 assert(!memcmp(lcd,deviceFramebuffer,sizeof(lcd)));
}
void DrawFrame(int color){lcdFrame=(unsigned short)color;}
void Bdisp_EnableColor(int n){colorMode=n;}
void Bkey_ClrAllFlags(void){keysCleared++;}
int Keyboard_PutKeycode(int x,int y,int code){
 assert(!nativeQueued&&!rawMenuQueued);
 if(x==4&&y==9&&code==0){assert(!staleEvents&&inNativeWait);rawMenuQueued=1;}
 else {assert(x==-1&&y==-1&&code==KEY_CTRL_EXIT);nativeQueued=code;}
 return 0;
}
int GetKey(int*key){
 assert(!rawMenuQueued&&nativeQueued==KEY_CTRL_EXIT);*key=nativeQueued;nativeQueued=0;initKeys++;
 if(lifecycleTesting){menuVisible=0;}
 return 0;
}
void Native_ClearKeyBuffer(void){assert(!rawMenuQueued);drainedEvents+=staleEvents;staleEvents=0;nativeQueued=0;}
void EnableGetkeyToMainFunctionReturn(void){menuReturnEnabled=1;}
static int menuROMRead32(void *unused,uint32_t a,uint32_t *v){
 (void)unused;if(a==0x8002007c){*v=nativeResolverReady?0x80040000:0;return 1;}
 if(a==0x80040000+0x1e58*4){*v=0x80030000;return 1;}return 0;
}
static int menuROMRead16(void *unused,uint32_t a,uint16_t *v){
 (void)unused;assert(a>=0x80030000&&a<0x80030000+300&&!(a&1));
 static const uint16_t pattern[]={0xd207,0xe703,0xb064,0x2270,0xaff8,0x0009};
 unsigned i=(a-0x80030000)/2;*v=i>=8&&i<14?pattern[i-8]:0;return 1;
}
static void menuInvoke(uint32_t address){
 assert(address==0x800300e0&&nativeResolverReady&&!timerInstalled&&!staleEvents);
 nativeDirectCalls++;menuReturned();
}
int Timer_Install(int id,void (*handler)(void),int delay){
 assert(id==0&&delay==20&&!timerInstalled);
 if(failTimerInstall)return -1;timerInstalled=42;timerCallback=handler;return 42;
}
int Timer_Start(int id){assert(id==timerInstalled);if(failTimerStart)return -1;timerArmed=1;return 0;}
int Timer_Stop(int id){assert(id==timerInstalled);timerArmed=0;timerStops++;return 0;}
int Timer_Deinstall(int id){assert(id==timerInstalled&&!timerArmed);timerInstalled=0;timerCallback=0;timerDeletes++;return 0;}
int GetKeyWait_OS(int*c,int*r,int mode,int timeout,int menu,unsigned short*code){
 if(mode==KEYWAIT_HALTOFF_TIMEROFF){
  assert(timeout==0&&menu==1);
  if(staleEvents){*c=4;*r=9;*code=KEY_CTRL_MENU;staleEvents--;drainedEvents++;return KEYREP_KEYEVENT;}
  *c=*r=0;*code=0;return KEYREP_NOEVENT;
 }
 assert(mode==KEYWAIT_HALTON_TIMEROFF&&timeout==1&&menu==0&&timerArmed&&timerCallback);
 if(interruptNativeWait)return KEYREP_NOEVENT;
 inNativeWait=1;timerCallback();
 assert(rawMenuQueued&&!timerInstalled&&!timerArmed&&menuTimer==-1);
 rawMenuQueued=0;inNativeWait=0;nativeTimerCalls++;menuReturned();
 *c=2;*r=1;*code=KEY_CTRL_EXE;return KEYREP_KEYEVENT;
}
static int scriptedKeyNext(void){timeTicks+=keyAdvanceMs;if(connectAt>=0&&timeTicks>=connectAt){connected=1;connectAt=-1;}if(timeTicks<busyUntil)return 0;if(keyAt>=keyCount){fprintf(stderr,"Editor exceeded key script at t=%d, link=%d misses=%d\n",timeTicks,linkOnline,linkMisses);abort();}int key=keys[keyAt++];if(lifecycleTesting&&key)staleEvents=5;return key;}
int PRGM_GetKey(void){assert(!"OS keyboard syscall must never run during application polling");return 0;}
static void keyboardReadWords(uint16_t words[6]){
 memset(words,0,12);int key=scriptedKeyNext();if(key){int row=key/10-1,col=key%10;assert(row>=0&&row<7&&col<10);words[col/2]=(uint16_t)(1u<<(row+8*(col%2)));}
}
int RTC_GetTicks(void){return timeTicks;}
int RTC_Elapsed_ms(int start,int ms){return timeTicks-start>=ms;}
void OS_InnerWait_ms(int ms){timeTicks+=ms;}
int Serial_ClearTX(void){return 0;}int Serial_ClearRX(void){rxAt=rxEnd=0;return 0;}
int Serial_Close(int n){(void)n;if(lifecycleTesting&&quitting)assert(!quitCallback);serialCloses++;osSerialOpen=0;return 0;}int Serial_IsOpen(void){return osSerialOpen;}
int Serial_Open(unsigned char *p){assert(p[0]==0&&p[1]==5&&p[2]==0&&p[3]==0&&p[4]==0&&p[5]==0);serialOpens++;if(openFailures){openFailures--;return 1;}osSerialOpen=1;return 0;}
int Serial_PollTX(void){return 256;}int Serial_PollRX(void){return timeTicks<replyReadyAt?0:(int)(rxEnd-rxAt);}
int Serial_ReadSingle(unsigned char*p){if(rxAt>=rxEnd)return 1;*p=rx[rxAt++];return 0;}
int Serial_Write(const unsigned char*p,int n){
 assert(osSerialOpen);if(writeFailures){writeFailures--;if(dropPortOnWriteFailure){osSerialOpen=0;dropPortOnWriteFailure=0;}if(expectSameWrite){memcpy(failedWrite,p,n);failedWriteLength=n;}return 1;}
 if(expectSameWrite&&failedWriteLength){assert(n==failedWriteLength&&!memcmp(p,failedWrite,n));expectSameWrite=0;failedWriteLength=0;}
 WParser parser={0};WFrame f,r={0};int got=0;for(int i=0;i<n;i++)if(w_feed(&parser,p[i],&f)==1)got=1;
 assert(got&&f.type<32);requestCount[f.type]++;
 if(!connected){if(!autoProbeActive)busyUntil=timeTicks+((f.type==W_HELLO||f.type==W_INFO||f.type==W_DEVICE||f.type==W_GET_CONFIG)?1200:800);return 0;}
 r.type=f.type|0x80;r.seq=f.seq;r.len=1;
 switch(f.type){
 case W_READ:{
  if(!servedAnswer){r.data[0]=E_REQUEST;break;}
  size_t offset=w_get16(f.data),length=strlen(servedAnswer);assert(offset<=length);size_t amount=length-offset;
  if(amount>WIRE_MAX-1)amount=WIRE_MAX-1;r.len=amount+1;memcpy(r.data+1,servedAnswer+offset,amount);break;
 }
 case W_INFO:r.len=5;for(int i=1;i<5;i++)r.data[i]=1;break;
 case W_DEVICE:r.len=8;r.data[1]=fakeAtomMajor;r.data[2]=0;r.data[3]=15;w_put32(r.data+4,fakeBoot);break;
 case W_SETUP:
  assert(f.len==1);fakePortal=f.data[0]==1?1:f.data[0]==2?0:fakePortal;
  r.len=17;r.data[1]=fakePortal;w_put16(r.data+3,0x1234);if(fakePortal)memcpy(r.data+5,"EXAMPLE23456",12);break;
 case W_CONTEXT:case W_NEW:case W_CANCEL:break;
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
 rxEnd+=w_encode(&r,rx+rxEnd);replyReadyAt=timeTicks+replyDelay;assert(rxEnd<=sizeof(rx));return 0;
}
void SetSetupSetting(unsigned int n,unsigned char v){(void)n;(void)v;}
unsigned char GetSetupSetting(unsigned int n){(void)n;return 0;}
void SetQuitHandler(void(*h)(void)){quitCallback=h;}
void DisplayMainMenu(void){assert(!"drawing MENU or returning main is not the native GetKey dispatcher");}
static void script(const int *p,size_t n){keys=p;keyCount=n;keyAt=0;lastKey=0;quitting=0;vram=deviceFramebuffer;busyUntil=0;}
#define SCRIPT(a) script(a,sizeof(a)/sizeof(a[0]))
static void dumpFrame(const char*path){
 FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n384 216\n255\n");
 for(int i=0;i<384*216;i++){unsigned short v=vram[i];unsigned char rgb[3]={(v>>11)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);
}
static const int lifeOne[]={31,31,0,0,0,0,47,0,48,48,0,0,0,79,79,0,0,0,0,0,0};
static const int lifeTwo[]={31,31,0,0,0,0,77,0,65,0,48,48,0,0,0,79,79,0,0,0,0,0,0};
static const int lifeThree[]={31,31,0,0,0,0,47,0,48,48,0,0,0,69,69,0,0,0,47,0,48,48,0,0,0,79,79,0,0,0,0,0,0};
static void menuReturned(void){
 assert(lifecycleTesting&&quitting&&serialOpen==1&&storageReadHandle<0&&storageWriteHandle<0);
 assert(statusMode==0&&lcdFrame==0xffff&&fullCopies>copiesAtMenu&&linkOnline);
 assert(!memcmp(deviceFramebuffer,startupSnapshot,sizeof(startupSnapshot)));
 /* OS SaveAndOpenMainMenu (or its timer fallback) suspends the app.
  * Return only after a simulated re-selection; count no app draws in MENU. */
 menuVisible=1;copiesAtMenu=fullCopies;
 /* Switch through unrelated native icons before returning. OS must never
  * retain an add-in callback or file resource. UART is an OS-owned driver. */
 assert(!quitCallback&&serialOpen&&osSerialOpen&&menuTimer==-1);
 for(int otherApp=0;otherApp<8;otherApp++){assert(!quitCallback);assert(storageReadHandle<0&&storageWriteHandle<0);}
 nativeMenus++;
 if(nativeMenus==2)assert(!strcmp(prompt,"H"));
 if(nativeMenus==20)longjmp(lifecycleDone,1);
 assert(fullCopies==copiesAtMenu);
 if(nativeMenus==2)osSerialOpen=0; /* OS leaves port open before re-selection. */
 deviceFramebuffer=nativeMenus%2?otherFramebuffer:framebuffer;
 for(size_t i=0;i<384*216;i++)deviceFramebuffer[i]=0xffff;
 lcdFrame=0xffff;dma[0]=52;dma[1]=300;dma[2]=24;dma[3]=191;
 /* OS launch events must not become commands in the re-opened editor. */
 staleEvents=5;
 if(nativeMenus==1){keys=lifeTwo;keyCount=sizeof(lifeTwo)/sizeof(lifeTwo[0]);}
 else {keys=lifeThree;keyCount=sizeof(lifeThree)/sizeof(lifeThree[0]);}keyAt=0;
}
static void testLifecycle(int direct){
 lcdFrame=0xffff;clear("Schermrand");show();assert(lcdFrame==C_BG);
 applyView((ViewProfile){80,24,-24});clear("Schermrand");show();assert(lcdFrame==C_BG);
 applyView(view_default());cleanup();
 autoProbeStarted=1;autoProbeAt=RTC_GetTicks();autoProbeActive=1;linkKnown=1;linkOnline=0;
 {char empty[PROMPT_MAX+1]="";const int done[]={79};SCRIPT(done);editor(empty,sizeof(empty),"ChatGPT - Vraag",0,1,0,1);}
 memcpy(startupSnapshot,framebuffer,sizeof(startupSnapshot));cleanup();
 int paid=requestCount[W_ASK],cancels=requestCount[W_CANCEL],closes=serialCloses,drains=drainedEvents,hellos=requestCount[W_HELLO];
 nativeResolverReady=direct;
 int directBefore=nativeDirectCalls,timerBefore=nativeTimerCalls;
 lifecycleTesting=1;nativeMenus=0;initKeys=0;menuReturnEnabled=0;menuVisible=0;copiesAtMenu=0;
 SCRIPT(lifeOne);staleEvents=5;
 quitting=1;lastKey=KEY_PRGM_EXIT;libraryReady=1;contextDirty=0;strcpy(field,"discard this stale value");
 if(!setjmp(lifecycleDone)){casio_program_main();assert(!"main must remain available for native MENU re-entry");}
 assert(nativeMenus==20&&initKeys==20&&requestCount[W_ASK]==paid);
 assert(requestCount[W_HELLO]==hellos+20);
 assert(nativeDirectCalls==directBefore+(direct?20:0)&&nativeTimerCalls==timerBefore+(direct?0:20));
 assert(!timerInstalled&&!timerArmed&&menuTimer==-1);
 assert(serialCloses==closes&&requestCount[W_CANCEL]==cancels&&drainedEvents>=drains+15);
 lifecycleTesting=0;deviceFramebuffer=framebuffer;vram=framebuffer;quitting=0;lastKey=0;
 assert(statusMode==0&&field[0]==0);applyView(view_default());
}

static void testMenuFailures(void){
 nativeResolverReady=0;serialOpen=1;osSerialOpen=1;quitting=1;vram=framebuffer;
 clear("MENU-test");show();int closes=serialCloses,stops=timerStops,deletes=timerDeletes;SetQuitHandler(0);
 failTimerInstall=1;assert(!nativeMainMenu());failTimerInstall=0;
 assert(serialOpen&&serialCloses==closes&&!timerInstalled&&lcdFrame==C_BG);
 failTimerStart=1;assert(!nativeMainMenu());failTimerStart=0;
 assert(serialOpen&&serialCloses==closes&&!timerInstalled&&timerStops==stops+1&&timerDeletes==deletes+1);
 interruptNativeWait=1;assert(!nativeMainMenu());interruptNativeWait=0;
 assert(serialOpen&&serialCloses==closes&&!timerInstalled&&menuTimer==-1);
 assert(timerStops==stops+2&&timerDeletes==deletes+2);
 quitting=0;clear("ChatGPT");show();assert(lcdFrame==C_BG&&statusMode==3);cleanup();
 nativeResolverReady=1;
}

static void testAutoConnect(void){
 int counts[32];char text[128]="";WFrame r;memcpy(counts,requestCount,sizeof(counts));
 cleanup();connected=1;replyDelay=150;Serial_ClearRX();autoProbeStarted=0;quitting=0;linkKnown=linkOnline=linkMisses=0;
 int start=timeTicks;assert(autoConnectTick()&&autoProbeActive&&!linkOnline);
 const int typing[]={77,0,66,0,66,0,79};SCRIPT(typing);editor(text,sizeof(text),"Test",0,1,0,1);
 assert(!strcmp(text,"Bb")&&timeTicks-start<150&&autoProbeActive);
 size_t at=keyAt;timeTicks+=150;assert(autoConnectTick()&&linkOnline&&!autoProbeActive&&keyAt==at);
 /* A pending heartbeat cannot hijack a foreground RPC or consume its reply. */
 timeTicks+=10000;assert(autoConnectTick()&&autoProbeActive);replyDelay=0;
 assert(rpc(W_INFO,0,0,&r,0)==1&&r.type==(W_INFO|0x80)&&!autoProbeActive);
 connected=0;linkOnline=0;autoProbeStarted=0;busyUntil=0;Serial_ClearRX();
 assert(autoConnectTick()&&autoProbeActive);timeTicks+=1201;
 assert(autoConnectTick()&&!linkOnline&&!autoProbeActive);
 connected=1;timeTicks+=1000;assert(autoConnectTick()&&autoProbeActive);
 assert(autoConnectTick()&&linkOnline&&!autoProbeActive);
 for(int i=0;i<32;i++)if(i!=W_HELLO&&i!=W_INFO)assert(requestCount[i]==counts[i]);
 cleanup();replyDelay=0;autoProbeStarted=0;
}
static void testUARTRecovery(void){
 int counts[32];WFrame r;memcpy(counts,requestCount,sizeof(counts));
 cleanup();quitting=0;connected=1;replyDelay=0;autoProbeStarted=0;
 /* Regression: V11 permanently rejected Serial_IsOpen()==1 if local flag=0. */
 osSerialOpen=1;serialOpen=0;int closed=serialCloses,opened=serialOpens;
 assert(ensureSerial()&&serialOpen&&osSerialOpen&&serialCloses==closed+1&&serialOpens==opened+1);
 osSerialOpen=0;assert(ensureSerial()&&serialOpen&&osSerialOpen&&serialOpens==opened+2);
 /* A driver write failure is recovered using exactly the same RPC sequence. */
 const int quiet[32]={0};SCRIPT(quiet);writeFailures=1;openFailures=0;expectSameWrite=1;
 assert(rpc(W_INFO,0,0,&r,0)==1&&writeFailures==0&&osSerialOpen&&!expectSameWrite&&!openFailures);
 /* Failed opens retry in the background in <1s, without typing a Test command. */
 cleanup();autoProbeStarted=0;openFailures=1;linkOnline=0;linkKnown=0;linkMisses=0;
 int started=timeTicks;assert(autoConnectTick()&&!autoProbeActive&&!serialOpen);
 timeTicks+=999;assert(!autoConnectTick());timeTicks++;
 assert(autoConnectTick()&&autoProbeActive);assert(autoConnectTick()&&linkOnline&&timeTicks-started<=1000);
 /* A single lost heartbeat must leave a healthy UART open and not mark offline. */
 connected=0;autoProbeStarted=0;Serial_ClearRX();assert(autoConnectTick()&&autoProbeActive);
 closed=serialCloses;timeTicks+=1201;assert(autoConnectTick()&&linkOnline&&osSerialOpen&&linkMisses==1&&serialCloses==closed);
 /* Confirmed silence changes status, but must not close a working driver. */
 for(int miss=2;miss<=3;miss++){
  assert(autoConnectTick()&&autoProbeActive);timeTicks+=1201;assert(autoConnectTick()&&linkMisses==miss);
 }
 assert(!linkOnline&&osSerialOpen&&serialCloses==closed);
 connected=1;assert(autoConnectTick()&&autoProbeActive&&osSerialOpen);assert(autoConnectTick()&&linkOnline&&!linkMisses);
 /* Slow metadata (800ms each) must not close a working port or flash Offline. */
 int quietSlow[2048]={0};SCRIPT(quietSlow);replyDelay=800;closed=serialCloses;
 uint8_t flags[4]={0};assert(refreshSettings(flags)&&linkOnline&&!linkMisses&&serialCloses==closed);
 for(int i=0;i<4;i++)assert(flags[i]);replyDelay=0;
 /* Settings recovers automatically if the Atom becomes available mid-screen. */
 int waitThenExit[400]={0};waitThenExit[399]=47;SCRIPT(waitThenExit);connected=0;
 connectAt=timeTicks+1800;autoProbeStarted=0;int infos=requestCount[W_INFO];settings();
 assert(connected&&connectAt==-1&&linkOnline&&requestCount[W_INFO]==infos);
 for(int i=0;i<32;i++)if(i!=W_HELLO&&i!=W_INFO&&i!=W_DEVICE&&i!=W_GET_CONFIG)assert(requestCount[i]==counts[i]);
 cleanup();autoProbeStarted=0;
}
static void testTypingConnection(void){
 cleanup();quitting=0;connected=1;linkKnown=linkOnline=linkMisses=0;autoProbeStarted=0;replyDelay=200;
 int closed=serialCloses,paid=requestCount[W_ASK];
 int typing[64],n=0;typing[n++]=77;typing[n++]=0;
 for(int i=0;i<30;i++){typing[n++]=76;typing[n++]=0;}typing[n++]=79;
 SCRIPT(typing);keyAdvanceMs=25;char text[128]="";
 assert(editor(text,sizeof(text),"Test",0,1,0,1)==79);
 assert(strlen(text)==30&&linkOnline&&!linkMisses&&serialCloses==closed&&requestCount[W_ASK]==paid);
 keyAdvanceMs=0;replyDelay=0;const int back[]={47};SCRIPT(back);diagnose();cleanup();autoProbeStarted=0;
}
static void testMathInput(void){
 char text[256]="";
 const int trigs[]={46,0,36,0,26,0,66,0,56,0,67,0,57,0,79};SCRIPT(trigs);
 editor(text,sizeof(text),"Test",0,1,0,1);assert(!strcmp(text,"sin(cos(tan(log(ln(^2^("));
 const int shifted[]={78,0,46,0,36,0,78,0,26,0,78,0,66,0,78,0,56,0,78,0,67,0,78,0,57,0,78,0,51,0,78,0,61,0,79};
 text[0]=0;SCRIPT(shifted);editor(text,sizeof(text),"Test",0,1,0,1);
 assert(!strcmp(text,"asin(cos(atan(10^(e^(sqrt(root(π="));
 const int mixedMath[]={77,0,66,0,78,0,46,0,66,0,77,0,26,0,79};
 text[0]=0;SCRIPT(mixedMath);editor(text,sizeof(text),"Test",0,1,0,1);assert(!strcmp(text,"Basin(btan("));
 const int cancelShift[]={78,0,78,0,46,0,79};text[0]=0;SCRIPT(cancelShift);editor(text,sizeof(text),"Test",0,1,0,1);assert(!strcmp(text,"sin("));
 int alphabet[55],n=0;const int map[]={76,66,56,46,36,26,75,65,55,45,35,25,74,64,54,73,63,53,43,33,72,62,52,42,32,71};
 alphabet[n++]=77;alphabet[n++]=0;for(int i=0;i<26;i++){alphabet[n++]=map[i];alphabet[n++]=0;}alphabet[n++]=79;
 text[0]=0;script(alphabet,n);editor(text,sizeof(text),"Test",0,1,0,1);assert(!strcmp(text,"Abcdefghijklmnopqrstuvwxyz"));
 /* Too-small capacity must not leave a half function token. */
 char tiny[5]="a";const int noSpace[]={78,0,46,0,47,0,79};SCRIPT(noSpace);editor(tiny,sizeof(tiny),"Test",0,0,0,1);assert(!strcmp(tiny,"a"));
}

static void testV14NavigationAndModels(void){
 int paid=requestCount[W_ASK],allocated=requestCount[W_ALLOC],devices=requestCount[W_DEVICE];
 char text[128]="";connected=1;replyDelay=0;quitting=0;activeChatSlot=-1;unsavedReply=0;
 /* F3 and the physical SHIFT-dot route both insert a real printable '='. */
 const int equals[]={78,0,61,0,77,0,78,0,61,0,79};SCRIPT(equals);
 assert(editor(text,sizeof(text),"Test",0,1,0,1)==79&&!strcmp(text,"=="));
 assert(glyphIndex('=')>=0&&glyphIndex('=')!=glyphIndex('?')&&requestCount[W_DEVICE]==devices);
 const int symbolEquals[]={49,0,79,0,79};text[0]=0;SCRIPT(symbolEquals);
 assert(editor(text,sizeof(text),"Test",0,1,0,1)==79&&!strcmp(text,"="));
 const int clearCancel[]={10,0,0,0,69,0,79};strcpy(text,"Blijft");SCRIPT(clearCancel);
 assert(editor(text,sizeof(text),"Test",0,1,0,1)==79&&!strcmp(text,"Blijft"));
 const int clearYes[]={10,0,0,0,79,0,79};SCRIPT(clearYes);
 assert(editor(text,sizeof(text),"Test",0,1,0,1)==79&&!text[0]);
 /* MENU is modal and consumes held keys. Repeating MENU inside the modal
  * cannot recurse, close UART, or discard the text when No is chosen. */
 assert(ensureSerial());int closes=serialCloses,opens=serialOpens;
 strcpy(prompt,"Bewaar mijn concept = 42");
 const int cancel[]={48,48,0,0,0,48,48,0,69,69,0,0,0};SCRIPT(cancel);
 assert(keyEvent()==UI_LINK_REFRESH&&!quitting&&!closePromptActive);
 assert(!strcmp(prompt,"Bewaar mijn concept = 42")&&serialCloses==closes&&serialOpens==opens);
 /* MENU-No while an RPC is waiting leaves its parser and UART intact. */
 int rpcCancel[150]={48,0,0,0,69,0,0,0};SCRIPT(rpcCancel);replyDelay=500;WFrame reply;
 assert(rpc(W_INFO,0,0,&reply,0)==1&&!quitting&&serialCloses==closes);replyDelay=0;
 /* Root EXIT does not request exit; MENU-No stays; only MENU-Yes leaves. */
 const int rootExit[]={47,0,48,0,0,0,69,0,0,0,48,0,0,0,79,0,0,0};
 answer[0]=0;SCRIPT(rootExit);runSession();assert(quitting&&!strcmp(prompt,"Bewaar mijn concept = 42"));
 assert(serialCloses==closes&&serialOpens==opens);
 /* Secret drafts also survive MENU-No and remain masked in the editor. */
 strcpy(text,"aBc-123");const int secretCancel[]={48,0,0,0,69,0,0,0,78,0,61,0,79};SCRIPT(secretCancel);
 assert(editor(text,sizeof(text),"API",1,0,1,0)==79&&!strcmp(text,"aBc-123="));
 /* A legacy custom model is retained when cancelling the new picker. */
 strcpy(publicModel,"my-custom-model");strcpy(storedModel,publicModel);
 int uploads=requestCount[W_APPLY];const int back[]={47};SCRIPT(back);assert(!modelPicker());
 assert(!strcmp(storedModel,"my-custom-model")&&requestCount[W_APPLY]==uploads);
 /* Every catalogue model saves through the real chunked config transport. */
 for(int i=0;i<MODEL_COUNT;i++){
  strcpy(publicModel,modelChoices[i].id);strcpy(storedModel,modelChoices[i].id);
  int other=(i+1)%MODEL_COUNT,keysToPick[20],n=0;
  for(int step=i;step!=other;step+=step<other?1:-1){keysToPick[n++]=step<other?37:28;keysToPick[n++]=0;}
  keysToPick[n++]=79;keysToPick[n++]=0;keysToPick[n++]=47;
  script(keysToPick,n);assert(modelPicker());assert(!strcmp(storedModel,modelChoices[other].id));
 }
 /* MENU-No in the picker retains its pending row before choosing it. */
 strcpy(publicModel,modelChoices[0].id);strcpy(storedModel,publicModel);
 const int pickerCancel[]={37,0,48,0,0,0,69,0,0,0,79,0,47};SCRIPT(pickerCancel);
 assert(modelPicker()&&!strcmp(storedModel,modelChoices[1].id)&&!quitting);
 const int explain[]={49,0,47,0,47};SCRIPT(explain);assert(!modelPicker());
 /* Exactly one diagnostics entry lives in Settings. Normal answer F3
  * opens the question without issuing diagnosis/network requests. */
 strcpy(lastQuestion,"23 + 42 = ?");strcpy(answer,"23 + 42 = 65");activeChatSlot=-1;
 devices=requestCount[W_DEVICE];const int fullQuestion[]={59,0,47,0,47};SCRIPT(fullQuestion);readAnswer(0);
 assert(requestCount[W_DEVICE]==devices);
 int hellosBefore=requestCount[W_HELLO];const int diagSettings[]={59,0,47,0,47};SCRIPT(diagSettings);settings();assert(requestCount[W_HELLO]>hellosBefore);
 assert(requestCount[W_ASK]==paid&&requestCount[W_ALLOC]==allocated);
 prompt[0]=answer[0]=lastQuestion[0]=0;quitting=0;
}

static void testV15BuyerInterface(void){
 int paid=requestCount[W_ASK],alloc=requestCount[W_ALLOC];WFrame reply;
 connected=1;replyDelay=0;quitting=0;loadingActive=0;linkOnline=0;settingsKnown=0;atomMajor=11;fakeAtomMajor=11;
 /* Right arrow opens every buyer row. EXE uses the same action. */
 int setups=requestCount[W_SETUP];const int wifiRight[]={27,0,38,0,47};SCRIPT(wifiRight);settings();assert(requestCount[W_SETUP]==setups+2);
 setups=requestCount[W_SETUP];const int wifiExe[]={31,0,47,0,47};SCRIPT(wifiExe);settings();assert(requestCount[W_SETUP]==setups+2);
 const int modelRight[]={37,0,27,0,47,0,47};SCRIPT(modelRight);settings();
 int hello=requestCount[W_HELLO];const int diagRight[]={37,0,37,0,27,0,47,0,47};SCRIPT(diagRight);settings();assert(requestCount[W_HELLO]>=hello+3);
 const int screenRight[]={37,0,37,0,37,0,27,0,47,0,47};SCRIPT(screenRight);settings();
 const int helpRight[]={37,0,37,0,37,0,37,0,27,0,27,0,47,0,47,0,47};SCRIPT(helpRight);settings();
 /* Short queue congestion does not close/reopen the driver. */
 assert(ensureSerial());int closes=serialCloses,opens=serialOpens;
 const int quiet[512]={0};SCRIPT(quiet);writeFailures=3;expectSameWrite=1;
 assert(rpc(W_INFO,0,0,&reply,0)==1&&!expectSameWrite&&serialCloses==closes&&serialOpens==opens);
 /* A real OS-port loss still recovers using the identical request bytes. */
 SCRIPT(quiet);writeFailures=1;dropPortOnWriteFailure=1;openFailures=1;expectSameWrite=1;
 assert(rpc(W_INFO,0,0,&reply,0)==1&&!expectSameWrite&&!openFailures&&osSerialOpen);
 /* Preserve a half-read heartbeat across a foreground handoff: no fake CRC error. */
 Serial_ClearRX();memset(&parser,0,sizeof(parser));autoProbeActive=1;autoProbeSeq=201;autoProbeAt=timeTicks;
 WFrame late={0};late.type=W_HELLO|0x80;late.seq=201;late.len=5;late.data[1]=1;
 unsigned char bytes[WIRE_ENCODED];size_t n=w_encode(&late,bytes),cut=n/2;
 memcpy(rx,bytes,cut);rxAt=0;rxEnd=cut;replyReadyAt=timeTicks;int beforeErrors=errors;
 autoConnectTick();assert(parser.n>0&&autoProbeActive);
 rxAt=0;rxEnd=n-cut;memcpy(rx,bytes+cut,n-cut);
 SCRIPT(quiet);assert(rpc(W_INFO,0,0,&reply,0)==1&&errors==beforeErrors&&!parser.n);
 /* Sustained absence gets one delayed repair, never a reset loop. */
 connected=0;autoProbeStarted=0;linkMisses=0;failureSince=0;repairStarted=0;Serial_ClearRX();
 closes=serialCloses;
 for(int miss=0;miss<3;miss++){autoConnectTick();timeTicks+=1201;autoConnectTick();}
 assert(!linkOnline&&serialCloses==closes);
 timeTicks+=15001;autoConnectTick();timeTicks+=1201;autoConnectTick();assert(serialCloses==closes+1);
 autoConnectTick();timeTicks+=1201;autoConnectTick();assert(serialCloses==closes+1);
 connected=1;autoConnectTick();autoConnectTick();assert(linkOnline&&!linkMisses);
 /* Received answers are rendered compatibly, never rewritten in history. */
 servedAnswer="𝑥 − 2 ＝ 4; x ≠ 3";SCRIPT(quiet);assert(fetchAnswer((unsigned)strlen(servedAnswer)));strcpy(lastQuestion,"Los op");activeChatSlot=-1;
 const int back[]={47};SCRIPT(back);readAnswer(0);
 assert(!strcmp(displayedAnswer,"x - 2 = 4; x ≠ 3")&&!strcmp(answer,"𝑥 − 2 ＝ 4; x ≠ 3"));servedAnswer=0;
 const int chatsKey[]={59};char input[32]="";SCRIPT(chatsKey);assert(editor(input,sizeof(input),"Vraag",0,1,0,1)==UI_CHATS&&!input[0]);
 /* Loader frames are genuine different rendered states and send no request. */
 paintLoading(0);memcpy(otherFramebuffer,framebuffer,sizeof(framebuffer));paintLoading(1);assert(memcmp(otherFramebuffer,framebuffer,sizeof(framebuffer)));
 replyDelay=1000;const int cancelLoader[]={79};SCRIPT(cancelLoader);loadingActive=1;loadingAt=timeTicks;
 assert(rpc(W_STATUS,0,0,&reply,1)==-2&&!quitting);loadingActive=0;replyDelay=0;Serial_ClearRX();
 assert(requestCount[W_ASK]==paid&&requestCount[W_ALLOC]==alloc);
 prompt[0]=answer[0]=lastQuestion[0]=0;quitting=0;
}

int main(int argc,char **argv){
 char b[128]="";
 const int hello[]={77,0,65,0,36,0,25,0,25,0,54,0,79};SCRIPT(hello);
 assert(editor(b,sizeof(b),"Test",0,1,0,1)==79);assert(!strcmp(b,"Hello"));
 const int exact[]={77,0,76,0,39,0,66,0,39,0,56,0,79};b[0]=0;SCRIPT(exact);
 assert(editor(b,sizeof(b),"API",1,0,1,0)==79);assert(!strcmp(b,"aBc"));
 /* Secret editor displays '*' in its first content cell. */
 rect(0,0,384,216,C_PANEL);glyph(12,52,'*',C_TEXT);unsigned short expected[14*8];
 for(int y=0;y<14;y++)for(int x=0;x<8;x++)expected[y*8+x]=framebuffer[(52+y)*384+12+x];
 const int done[]={79};SCRIPT(done);editor(b,sizeof(b),"API",1,0,1,0);
 for(int y=0;y<13;y++)for(int x=0;x<8;x++)assert(framebuffer[(52+y)*384+12+x]==expected[y*8+x]);
 /* One ALPHA locks letters; second restores numeric/math input. */
 const int mixed[]={77,0,76,0,66,0,77,0,72,0,79};b[0]=0;SCRIPT(mixed);editor(b,sizeof(b),"Test",0,1,0,1);assert(!strcmp(b,"Ab1"));
 const int edit[]={38,0,44,0,79};strcpy(b,"Aéπ");SCRIPT(edit);editor(b,sizeof(b),"Test",0,0,0,1);assert(!strcmp(b,"Aπ"));
 /* Offline settings must open without a misleading configuration popup. */
 const int leave[]={47};connected=0;SCRIPT(leave);settings();assert(requestCount[W_INFO]>0);
 connected=1;settingsKnown=0;strcpy(storedModel,"my-custom-model");
 const int modelEdit[]={37,0,79,0,47,0,47};SCRIPT(modelEdit);settings();
 assert(!strcmp(storedModel,"my-custom-model")&&privateReads==0);
 assert(field[0]==0&&atomMajor==3);
 /* Last page cannot scroll into blank rows. */
 strcpy(answer,"one\ntwo\nthree\nfour\nfive\nsix\nseven");strcpy(lastQuestion,"Test");
 const int scroll[]={27,0,27,0,37,0,47};SCRIPT(scroll);readAnswer(0);
 clear("Schermtest");show();assert(statusMode==3&&dma[0]==0&&dma[1]==383&&dma[2]==0&&dma[3]==215&&fullCopies>0&&stripeCalls==0);
 connected=1;loadLibrary();
 /* Local records are reopened and their context is restored without POST. */
 assert(lib_append(&disk,0,&savedChats[0],"Vorige vraag","Antwoord één",&savedChats[0]));
 assert(lib_append(&disk,0,&savedChats[0],"Volgende vraag","Antwoord twee",&savedChats[0]));
 activeChatSlot=0;contextDirty=1;strcpy(prompt,"Nieuwe vraag");int paid=requestCount[W_ASK];SCRIPT(done);
 assert(restoreChat()&&requestCount[W_ASK]==paid&&!strcmp(prompt,"Nieuwe vraag")&&field[0]==0);
 assert(requestCount[W_CONTEXT]==2);
 assert(restoreChat()&&requestCount[W_CONTEXT]==2); /* Same boot: no re-upload. */
 fakeBoot++;assert(restoreChat()&&requestCount[W_CONTEXT]==4&&requestCount[W_ASK]==paid);
 assert(loadChatPair(0,0)&&!strcmp(lastQuestion,"Vorige vraag"));
 assert(loadChatPair(0,1)&&!strcmp(answer,"Antwoord twee"));
 /* Opening and reading saved threads is offline and sends no RPC. */
 connected=0;prompt[0]=0;int beforeOpen=requestCount[W_ASK]+requestCount[W_CONTEXT]+requestCount[W_INFO];
 const int chooseChat[]={37,0,31};SCRIPT(chooseChat);assert(chatSidebar()==1&&activeChatSlot==0&&viewTurn==1);
 const int prev[]={39,0,47};SCRIPT(prev);readAnswer(0);assert(viewTurn==0&&!strcmp(lastQuestion,"Vorige vraag"));
 const int next[]={29,0,47};SCRIPT(next);readAnswer(0);assert(viewTurn==1&&!strcmp(answer,"Antwoord twee"));
 assert(beforeOpen==requestCount[W_ASK]+requestCount[W_CONTEXT]+requestCount[W_INFO]);
 const int questionView[]={78,0,47,0,47};SCRIPT(questionView);readAnswer(0);
 SCRIPT(leave);screenTest();assert(framebuffer[0]==C_MINT&&framebuffer[384*216-1]==C_MINT);
 const int deleteChat[]={37,0,69,0,0,0,79,0,47};SCRIPT(deleteChat);chatSidebar();assert(!savedChats[0].used&&!lib_entry(&disk,'C',0).used);
 connected=1;
 /* Scale changes affect only this app, persist, can be cancelled and fail safely. */
 const int tune[]={38,0,37,0,27,0,37,0,27,0,79};SCRIPT(tune);screenSettings();
 ViewProfile stored=view_load(&disk);assert(stored.percent==99&&stored.x==1&&stored.y==1);
 const int cancelTune[]={27,0,47};SCRIPT(cancelTune);screenSettings();assert(viewProfile.percent==99&&view_load(&disk).percent==99);
 const int resetTune[]={59,0,79};SCRIPT(resetTune);screenSettings();assert(viewProfile.percent==100&&view_load(&disk).percent==100);
 failCreate=1;const int failedTune[]={38,0,79,0,47,0,47};SCRIPT(failedTune);screenSettings();failCreate=0;
 assert(viewProfile.percent==100&&view_load(&disk).percent==100);
 {int before=requestCount[W_SETUP],paidBefore=requestCount[W_ASK];
  const int back[]={47};connected=1;atomMajor=3;SCRIPT(back);phoneSetup();assert(requestCount[W_SETUP]==before);
  atomMajor=11;SCRIPT(back);phoneSetup();assert(requestCount[W_SETUP]==before+2&&!fakePortal);
  const int menuBack[]={48,0,0,0,79,0,0,0};SCRIPT(menuBack);phoneSetup();assert(requestCount[W_SETUP]==before+4&&!fakePortal&&quitting);
  assert(requestCount[W_ASK]==paidBefore);quitting=0;
 }
 testMathInput();testAutoConnect();testUARTRecovery();testTypingConnection();
 testLifecycle(1);testLifecycle(0);
 testMenuFailures();testV14NavigationAndModels();testV15BuyerInterface();
 if(argc==3&&!strcmp(argv[1],"--screens")){
  strcpy(storedModel,"gpt-4.1-mini");
  assert(lib_append(&disk,0,&savedChats[0],"Hoe los ik 2x + 5 = 17 op?","Trek eerst 5 van beide kanten af.\n2x = 12\n\nDeel daarna door 2: x = 6.",&savedChats[0]));
  assert(lib_append(&disk,1,&savedChats[1],"Leg breuken eenvoudig uit.","Een breuk is een deel van een geheel.",&savedChats[1]));
  activeChatSlot=0;viewTurn=0;
  char path[512];snprintf(path,sizeof(path),"%s/input.ppm",argv[2]);
  strcpy(b,"Hoe los ik 2x + 5 = 17 op?");strcpy(prompt,b);const int shotType[]={77,0,79};SCRIPT(shotType);editor(prompt,sizeof(prompt),"ChatGPT",0,1,0,1);dumpFrame(path);
  strcpy(lastQuestion,b);strcpy(answer,"Trek eerst 5 van beide kanten af.\n2x = 12\n\nDeel daarna door 2: x = 6.");
  SCRIPT(leave);readAnswer(0);snprintf(path,sizeof(path),"%s/answer.ppm",argv[2]);dumpFrame(path);
  prompt[0]=0;SCRIPT(leave);chatSidebar();snprintf(path,sizeof(path),"%s/sidebar.ppm",argv[2]);dumpFrame(path);
  SCRIPT(leave);screenTest();snprintf(path,sizeof(path),"%s/screen.ppm",argv[2]);dumpFrame(path);
  SCRIPT(leave);screenSettings();snprintf(path,sizeof(path),"%s/calibration.ppm",argv[2]);dumpFrame(path);
  fakeAtomMajor=11;settingsKnown=0;SCRIPT(leave);settings();snprintf(path,sizeof(path),"%s/settings.ppm",argv[2]);dumpFrame(path);
  atomMajor=11;SCRIPT(leave);phoneSetup();snprintf(path,sizeof(path),"%s/phone.ppm",argv[2]);dumpFrame(path);
  paintModels(1);snprintf(path,sizeof(path),"%s/models.ppm",argv[2]);dumpFrame(path);
  linkOnline=1;paintDiagnosis(3,3);snprintf(path,sizeof(path),"%s/diagnosis.ppm",argv[2]);dumpFrame(path);
  for(int phase=0;phase<8;phase++){paintLoading(phase);snprintf(path,sizeof(path),"%s/loading-%d.ppm",argv[2],phase);dumpFrame(path);}
  paintLoading(0);snprintf(path,sizeof(path),"%s/loading.ppm",argv[2]);dumpFrame(path);
  connected=0;SCRIPT(leave);settings();snprintf(path,sizeof(path),"%s/offline.ppm",argv[2]);dumpFrame(path);
 }
 storageClose();for(int i=0;i<32;i++)free(testFiles[i].data);
 puts("PASS: V15 buyer settings with Right/EXE, customer-only diagnosis, persistent UART on missed replies, transient TX backpressure without driver reset, shared partial-frame handoff, answer equality rendering with originals preserved, animated logo and cancellation without paid requests; V14 MENU-only modal exit and cancellation preserving drafts/UART, root EXIT stays, SHIFT/symbol equality input, all model presets saved without paid requests, legacy model cancellation, single Settings diagnosis; actual Casio editor with host device stubs: phone setup start/stop/old firmware guard/physical MENU stop without paid requests, ALPHA lock, one-shot SHIFT math, atomic function tokens, automatic first capital, exact secret case, masked display, digit toggle, UTF-8 cursor/deletion, offline settings, secret read restrictions, saved model editing, answer paging, offline sidebar, saved turn navigation/deletion, full question reading, no-POST context restore and full-screen corner test, persistent scale/pan settings, cancel/reset/no-space handling and UART recovery from OS-open/local-closed and OS-closed/local-open states, write failure and failed open, 800ms delayed metadata without closing UART, three-miss status hysteresis with no reset on short silence, cooldown-limited repair on prolonged failure, automatic settings reconnect and continuous typing with delayed heartbeat ACK and no UART close, no saved closing-dialog flash across forty sessions through the resolved native SaveAndOpenMainMenu and timer/key-wait fallback, strict injection-after-wait ordering, stale events, held keys, OS-owned UART preserved across MENU/re-entry, no residual quit callback while selecting eight other icons per session, allocation/start/interruption failures without dropped UART or leaked timer, changed VRAM/DMA and the LCD frame.");
 return 0;
}
