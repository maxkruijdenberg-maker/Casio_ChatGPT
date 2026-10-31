#include <cassert>
#include <atomic>
#include <cstdio>
#include <string>
#include <ArduinoJson.h>
#include "../atom/ChatGPTAtom/api_response.h"
#include "../atom/ChatGPTAtom/network_operation.h"
#include "../atom/ChatGPTAtom/ledger.h"
static uint32_t savedNext=0,savedLast=0;static bool failSave=false;
static int save(const char *name,uint32_t id){if(failSave)return 0;(!strcmp(name,"next")?savedNext:savedLast)=id;return 1;}
static CheckHttpResult parse(int status,const char *json){JsonDocument doc;auto error=deserializeJson(doc,json);return completion_check_response(status,doc.as<JsonVariantConst>(),!error);}
int main(){
 const char *id="gpt-6.1-sol";const char *good="{\"object\":\"chat.completion\",\"model\":\"gpt-6.1-sol-2026-10-29\",\"choices\":[{\"finish_reason\":\"length\",\"message\":{\"content\":\"\"}}]}";
 auto ok=parse(200,good);assert(ok.valid&&active_model_matches(id,ok.reported));
 assert(active_model_matches(id,id));
 for(const char *wrong:{"gpt-6","gpt-6.1-terra","gpt-6.1-sol-extra","gpt-6.1-sol-2026-02-30","gpt-6.1-sol-2026-10-29-extra","","gpt-6.1-sol\n"})assert(!active_model_matches(id,wrong));
 assert(active_model_matches(id,"gpt-6.1-sol-2024-02-29"));assert(!active_model_matches(id,"gpt-6.1-sol-2025-02-29"));
 ActiveModel active;active_reset(&active,id);uint32_t epoch=active.epoch;assert(!active.receipt.complete&&active.receipt.request);
 assert(active_publish(&active,epoch,id,ok.reported));active_reset(&active,"gpt-6.1-terra");assert(!active.receipt.complete&&!active_publish(&active,epoch,id,ok.reported));
 assert(!active_publish(&active,active.epoch,"gpt-6.1-terra",id));
 active_reset(&active,id);assert(!active_publish(&active,active.epoch,id,""));
 uint32_t ticks=0;std::atomic_bool cancelled{false};int gets=0,posts=0;uint32_t limit=90000;
 auto run=[&](int cut,int status,const char *errorJson){
  cancelled=false;ticks=0;gets=posts=0;
  NetworkOperation work([&](){return ticks;},cancelled,limit);
  if(cut==1)cancelled=true;
  return check_connection(work,id,[&](){gets++;if(cut==2)cancelled=true;if(cut==3)ticks=limit;CheckHttpResult m;m.status=status;if(status!=200)m=parse(status,errorJson);return m;},[&](){posts++;if(cut==4)cancelled=true;if(cut==5)ticks=limit;return ok;});
 };
 auto ready=run(0,200,"");assert(ready.internet&&ready.account&&ready.model&&ready.attempted==(CHECK_INTERNET|CHECK_ACCOUNT|CHECK_MODEL)&&gets==1&&posts==1);
 for(int cut:{1,2,3}){auto r=run(cut,200,"");assert(posts==0&&!r.account&&!r.model&&gets==(cut==1?0:1));}
 for(int cut:{4,5}){auto r=run(cut,200,"");assert(posts==1&&!r.account&&!r.model);}
 auto auth=run(0,401,"{\"error\":{\"code\":\"invalid_api_key\"}}");assert(auth.internet&&!auth.account&&auth.error==E_AUTH&&posts==0);
 auto credit=run(0,429,"{\"error\":{\"code\":\"insufficient_quota\"}}");assert(credit.error==E_CREDIT&&!posts);
 auto rate=run(0,429,"{\"error\":{\"code\":\"rate_limit_exceeded\"}}");assert(rate.error==E_LIMIT&&!posts);
 auto missing=run(0,404,"{}");assert(missing.error==E_MODEL&&missing.internet&&posts==0);
 auto network=run(0,-1,"{}");assert(!network.internet&&!network.account&&!posts);
 assert(!parse(200,"broken").valid&&!parse(200,"{\"object\":\"error\",\"choices\":[{}]}").valid);
 cancelled=false;ticks=0;NetworkOperation work([&](){return ticks;},cancelled,90000);
 auto wrong=check_connection(work,id,[](){CheckHttpResult r;r.status=200;return r;},[](){return parse(200,"{\"object\":\"chat.completion\",\"model\":\"gpt-6.1-terra\",\"choices\":[{}]}");});assert(wrong.account&&!wrong.model);
 for(const char *body:{"{\"object\":\"chat.completion\",\"choices\":[{}]}","{\"object\":\"chat.completion\",\"model\":null,\"choices\":[{}]}","{\"object\":\"chat.completion\",\"model\":\"gpt-6.1-sol\\u0000bad\",\"choices\":[{}]}"}){
  auto response=parse(200,body);assert(response.valid&&!response.reported[0]);
  auto unconfirmed=check_connection(work,id,[](){CheckHttpResult r;r.status=200;return r;},[&](){return response;});assert(unconfirmed.account&&!unconfirmed.model);
 }
 /* Separate NVS-backed ledger: lost start ACK, later replay and reboot all
  * preserve at-most-once diagnostic admission without touching chat IDs. */
 RequestLedger ledger={};assert(g_allocate(&ledger,0,save)==G_NEW);auto allocated=ledger.active;
 assert(g_admit(&ledger,allocated,0,save)==G_NEW);assert(g_admit(&ledger,allocated,0,save)==G_REPLAY);
 RequestLedger reboot={savedNext,savedLast,0,0};assert(g_admit(&reboot,allocated,0,save)==G_REPLAY);
 failSave=true;assert(g_allocate(&reboot,0,save)==G_STORE);failSave=false;
 puts("PASS V40 connection/model verification: real JSON parser, empty one-token completions, exact/date aliases, mismatches, missing/invalid metadata, stale settings epochs, pre/during-call cancellation/deadlines, no late POST, quota versus rate limit, no retries, durable diagnostic replay ledger.");
}
