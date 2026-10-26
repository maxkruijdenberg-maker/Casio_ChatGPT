/* Host-only device stubs exercise the real editor, not OpenAI functionality. */
#include <stdlib.h>
#define main casio_program_main
#include "../casio/main.c"
#undef main
#include <assert.h>
static unsigned short framebuffer[384*216];
static const int *keys;static size_t keyCount,keyAt;static int timeTicks;
void *GetVRAMAddress(void){return framebuffer;}
void Bdisp_AllClr_VRAM(void){for(size_t i=0;i<384*216;i++)framebuffer[i]=0xffff;}
void Bdisp_PutDisp_DD(void){}
void Bdisp_EnableColor(int n){(void)n;}
int PRGM_GetKey(void){if(keyAt>=keyCount){fprintf(stderr,"Editor exceeded key script\n");abort();}return keys[keyAt++];}
int RTC_GetTicks(void){return timeTicks;}
int RTC_Elapsed_ms(int start,int ms){return timeTicks-start>=ms;}
void OS_InnerWait_ms(int ms){timeTicks+=ms;}
int Serial_ClearTX(void){return 0;}int Serial_ClearRX(void){return 0;}
int Serial_Close(int n){(void)n;return 0;}int Serial_IsOpen(void){return 0;}
int Serial_Open(unsigned char *p){(void)p;return 0;}
int Serial_PollTX(void){return 0;}int Serial_PollRX(void){return 0;}
int Serial_ReadSingle(unsigned char*p){(void)p;return 1;}
int Serial_Write(const unsigned char*p,int n){(void)p;(void)n;return 0;}
void SetSetupSetting(unsigned int n,unsigned char v){(void)n;(void)v;}
unsigned char GetSetupSetting(unsigned int n){(void)n;return 0;}
void SetQuitHandler(void(*h)(void)){(void)h;}
void DisplayMainMenu(void){}
static void script(const int *p,size_t n){keys=p;keyCount=n;keyAt=0;lastKey=0;quitting=0;vram=framebuffer;}
#define SCRIPT(a) script(a,sizeof(a)/sizeof(a[0]))
int main(void){
 char b[128]="";
 const int hello[]={78,0,65,0,36,0,25,0,25,0,54,0,79};SCRIPT(hello);
 assert(editor(b,sizeof(b),"Test",0,1,0,1)==79);assert(!strcmp(b,"Hello"));
 const int exact[]={78,0,76,0,39,0,66,0,39,0,56,0,79};b[0]=0;SCRIPT(exact);
 assert(editor(b,sizeof(b),"API",1,0,1,0)==79);assert(!strcmp(b,"aBc"));
 /* Secret editor displays '*' in its first content cell. */
 Bdisp_AllClr_VRAM();glyph(8,40,'*',0);unsigned short expected[14*8];
 for(int y=0;y<14;y++)for(int x=0;x<8;x++)expected[y*8+x]=framebuffer[(40+y)*384+8+x];
 const int done[]={79};SCRIPT(done);editor(b,sizeof(b),"API",1,0,1,0);
 for(int y=0;y<13;y++)for(int x=0;x<8;x++)assert(framebuffer[(40+y)*384+8+x]==expected[y*8+x]);
 /* One SHIFT locks letters for every subsequent key; second restores digits. */
 const int mixed[]={78,0,76,0,66,0,78,0,72,0,79};b[0]=0;SCRIPT(mixed);editor(b,sizeof(b),"Test",0,1,0,1);assert(!strcmp(b,"Ab1"));
 const int edit[]={38,0,44,0,79};strcpy(b,"Aéπ");SCRIPT(edit);editor(b,sizeof(b),"Test",0,0,0,1);assert(!strcmp(b,"Aπ"));
 puts("PASS: actual Casio editor with host device stubs: SHIFT lock, automatic first capital, exact secret case, masked display, digit toggle, UTF-8 cursor/deletion.");
 return 0;
}
