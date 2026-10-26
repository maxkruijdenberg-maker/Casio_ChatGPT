#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "../casio/library.h"
typedef struct {uint8_t*p;uint32_t len;} MemoryFile;
typedef struct {MemoryFile file[16];int failAfter,writes,denyBegin;} MemoryDisk;
static int fileIndex(const char*n){assert(n[0]=='C');return (n[1]-'0')*2+(n[2]=='B');}
static int rd(void*v,const char*n,uint32_t at,void*out,size_t len){MemoryFile*f=&((MemoryDisk*)v)->file[fileIndex(n)];if(!f->p||at>f->len||len>f->len-at)return 0;memcpy(out,f->p+at,len);return 1;}
static int begin(void*v,const char*n,uint32_t len){MemoryDisk*d=v;MemoryFile*f=&d->file[fileIndex(n)];if(d->denyBegin)return 0;free(f->p);f->p=calloc(1,len);f->len=len;return f->p!=0;}
static int wr(void*v,const char*n,uint32_t at,const void*data,size_t len){MemoryDisk*d=v;MemoryFile*f=&d->file[fileIndex(n)];if(d->failAfter>=0&&d->writes++==d->failAfter){if(len>1)memcpy(f->p+at,data,len/2);return 0;}assert(at+len<=f->len);memcpy(f->p+at,data,len);return 1;}
static int finish(void*v,const char*n){(void)v;(void)n;return 1;}
static void clear(MemoryDisk*d){for(int i=0;i<16;i++)free(d->file[i].p);memset(d,0,sizeof(*d));d->failAfter=-1;}
int main(void){
 MemoryDisk m={0};m.failAfter=-1;StoreOps ops={&m,rd,begin,wr,finish};LibraryEntry e={0},previous;char q[1025],a[8193];
 assert(s_crc("123456789",9)==0xcbf43926u);
 assert(lib_append(&ops,0,&e,"Vraag é","Hallo π",&e));previous=e;
 assert(lib_append(&ops,0,&e,"Volgende","Tweede antwoord",&e));
 m.file[fileIndex(e.ref.name)].p[STORE_HEADER+STORE_META]^=0x55;
 e=lib_entry(&ops,'C',0);assert(e.ref.seq==previous.ref.seq&&lib_pair(&ops,&e.ref,0,q,sizeof(q),a,sizeof(a))&&!strcmp(a,"Hallo π"));
 m.file[fileIndex(previous.ref.name)].p[STORE_HEADER+STORE_META]^=0x55;
 e=lib_entry(&ops,'C',0);assert(e.damaged&&!e.used);
 assert(lib_delete(&ops,'C',0));e=lib_entry(&ops,'C',0);assert(!e.used&&!e.damaged);
 clear(&m);
 /* An interrupted append retains the complete original thread. */
 for(int fail=0;fail<19;fail++){
  memset(&e,0,sizeof(e));assert(lib_append(&ops,0,&e,"Eerst","Bewaren",&e));previous=e;
  memset(a,'x',4096);a[4096]=0;m.failAfter=fail;m.writes=0;
  assert(!lib_append(&ops,0,&e,"Andere",a,&e));
  e=lib_entry(&ops,'C',0);assert(e.used&&e.ref.seq==previous.ref.seq&&e.turns==1);
  assert(lib_pair(&ops,&e.ref,0,q,sizeof(q),a,sizeof(a))&&!strcmp(a,"Bewaren"));
  clear(&m);
 }
 memset(&e,0,sizeof(e));assert(lib_append(&ops,0,&e,"Bewaard","Antwoord",&e));
 m.denyBegin=1;assert(!lib_append(&ops,0,&e,"Fout","Niet opgeslagen",&e));assert(lib_entry(&ops,'C',0).turns==1);m.denyBegin=0;
 clear(&m);memset(&e,0,sizeof(e));
 for(int i=0;i<CHAT_TURNS;i++){
  sprintf(q,"Vraag %d: éπ",i);sprintf(a,"Antwoord %d: 65",i);
  assert(lib_append(&ops,1,&e,q,a,&e));assert(e.turns==i+1);
 }
 assert(!lib_append(&ops,1,&e,"Vol","Meer",&e));
 for(int i=0;i<CHAT_TURNS;i++){
  char expected[64];assert(lib_pair(&ops,&e.ref,i,q,sizeof(q),a,sizeof(a)));
  sprintf(expected,"Vraag %d: éπ",i);assert(!strcmp(expected,q));
 }
 previous=lib_entry(&ops,'C',1);assert(previous.turns==CHAT_TURNS);
 assert(lib_delete(&ops,'C',1));assert(!lib_entry(&ops,'C',1).used);
 clear(&m);memset(&e,0,sizeof(e));memset(q,'q',1024);q[1024]=0;memset(a,'a',8192);a[8192]=0;
 assert(lib_append(&ops,0,&e,q,a,&e));assert(lib_pair(&ops,&e.ref,0,q,sizeof(q),a,sizeof(a)));
 assert(strlen(q)==1024&&strlen(a)==8192);
 clear(&m);
 puts("PASS: actual V3 chat library; dual banks, checksums, corruption fallback, interrupted appends, tombstones, no-space errors, chat limits, Unicode, in-place metadata updates and full-size records.");
}
