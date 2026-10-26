#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../atom/ChatGPTAtom/ledger.h"
static uint32_t diskNext,diskLast;static int storageWorks=1;
static int store(const char *name,uint32_t v){if(!storageWorks)return 0;if(!strcmp(name,"next"))diskNext=v;else diskLast=v;return 1;}
static RequestLedger reset(void){RequestLedger g={diskNext,diskLast,0,0};return g;}
int main(void){
 RequestLedger g=reset();unsigned i;int starts=0;
 assert(g_admit(&g,0,0,store)==G_INVALID);
 for(i=0;i<10000;i++){
  assert(g_allocate(&g,0,store)==G_NEW);uint32_t id=g.active;
  assert(g_admit(&g,id,0,store)==G_NEW);starts++;
  /* Lost ACK: replay both while running and after completion. */
  assert(g_admit(&g,id,1,store)==G_REPLAY);assert(g_admit(&g,id,0,store)==G_REPLAY);
  /* Power loss immediately after journal commit, before or after HTTP. */
  g=reset();assert(g_admit(&g,id,0,store)==G_REPLAY);
  if(id>1)assert(g_admit(&g,id-1,0,store)==G_INVALID);
 }
 assert(starts==10000);
 assert(g_allocate(&g,1,store)==G_BUSY);
 uint32_t oldNext=g.next,oldLast=g.last;storageWorks=0;
 assert(g_allocate(&g,0,store)==G_STORE&&g.next==oldNext);
 storageWorks=1;assert(g_allocate(&g,0,store)==G_NEW);uint32_t id=g.active;
 storageWorks=0;assert(g_admit(&g,id,0,store)==G_STORE&&g.last==oldLast);
 storageWorks=1;g=reset();assert(g_admit(&g,id,0,store)==G_INVALID);
 g.next=UINT32_MAX;assert(g_allocate(&g,0,store)==G_EXHAUSTED);
 puts("PASS: 10,000 request cycles; lost ACK, busy replay, reset replay, stale ID, failed storage and ID exhaustion.");
 return 0;
}
