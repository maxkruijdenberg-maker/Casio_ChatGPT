#ifndef CHATGPT_CONNECTION_CHECK_H
#define CHATGPT_CONNECTION_CHECK_H
#include "active_model.h"
enum {CHECK_WIFI=1,CHECK_INTERNET=2,CHECK_ACCOUNT=4,CHECK_MODEL=8};
struct CheckHttpResult {int status=0;uint8_t error=0;bool valid=false;char reported[256]={0};};
struct ConnectionResult {uint8_t attempted=0,error=0;bool internet=false,account=false,model=false;char reported[256]={0};};
static uint8_t check_http_error(int code,const char *apiCode){
 if(code==429&&(!strcmp(apiCode,"insufficient_quota")||!strcmp(apiCode,"billing_hard_limit_reached")))return E_CREDIT;
 if(code==401||code==403)return E_AUTH;
 if(code==429)return E_LIMIT;
 if(code==404)return E_MODEL;
 return code<=0?E_INTERNET:E_HTTP;
}
/* A guarded GET followed by at most ONE tiny completion. No retry, account
 * balance claim, chat-context mutation or synthetic successful model proof. */
template<class Work,class Get,class Post>
static ConnectionResult check_connection(Work &work,const char *selected,Get get,Post post){
 ConnectionResult r;
 if(work.stopped())return r;
 r.attempted|=CHECK_INTERNET;CheckHttpResult meta=get();
 if(work.stopped())return ConnectionResult{};
 r.internet=meta.status>0;
 if(meta.status!=200){
  if(meta.status==401||meta.status==403||meta.status==429)r.attempted|=CHECK_ACCOUNT;
  r.error=meta.error?meta.error:check_http_error(meta.status,"");return r;
 }
 if(work.stopped())return ConnectionResult{};
 if(!receipt_model_valid(selected,strlen(selected))){r.attempted|=CHECK_ACCOUNT;r.error=E_CONFIG;return r;}
 r.attempted|=CHECK_ACCOUNT;CheckHttpResult test=post();
 if(work.stopped())return ConnectionResult{};
 if(test.status!=200){r.error=test.error?test.error:check_http_error(test.status,"");return r;}
 if(!test.valid){r.error=E_JSON;return r;}
 r.account=true;r.attempted|=CHECK_MODEL;
 size_t n=strlen(test.reported);if(receipt_model_valid(test.reported,n))memcpy(r.reported,test.reported,n+1);
 r.model=active_model_matches(selected,r.reported);
 return r;
}
#endif
