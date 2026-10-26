#ifndef CHATGPT_LEDGER_H
#define CHATGPT_LEDGER_H
#include <stdint.h>
/* NVS-backed at-most-once admission, not a retry promise. A reset between the
 * committed journal entry and HTTPS can lose a request; it cannot retry it.
 * Persist callbacks must report success only after durable commit. */
typedef struct {uint32_t next,last,active;uint8_t allocated;} RequestLedger;
enum {G_NEW=1,G_REPLAY=2,G_INVALID=-1,G_BUSY=-2,G_STORE=-3,G_EXHAUSTED=-4};
typedef int (*LedgerSave)(const char *name,uint32_t value);
static int g_allocate(RequestLedger *g,int busy,LedgerSave save){
 uint32_t id;if(busy)return G_BUSY;if(g->next==UINT32_MAX)return G_EXHAUSTED;
 id=g->next+1;if(!save("next",id))return G_STORE;
 g->next=id;g->active=id;g->allocated=1;return G_NEW;
}
static int g_admit(RequestLedger *g,uint32_t id,int busy,LedgerSave save){
 if(id&&id==g->last)return G_REPLAY;
 if(busy)return G_BUSY;
 if(!g->allocated||!id||id!=g->active)return G_INVALID;
 if(!save("last",id))return G_STORE;
 g->last=id;g->allocated=0;return G_NEW;
}
#endif
