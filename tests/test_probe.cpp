#include <cassert>
#include <cstdio>
#include <string>
#include "../atom/ChatGPTAtom/model_probe.h"
int main(){
 char result[MODEL_PROBE_BYTES];int calls=0;
 auto all=[&](const char *id){assert(!strcmp(id,modelChoices[calls++].id));return model_probe_status(200);};
 assert(probe_model_choices(all,result,sizeof(result))==E_NONE&&calls==MODEL_COUNT);
 std::string expected;for(int i=0;i<MODEL_COUNT;i++)expected+=std::string(modelChoices[i].id)+"\n";
 assert(expected==result&&expected.size()<MODEL_PROBE_BYTES);
 calls=0;auto mixed=[&](const char*){return model_probe_status(calls++%2?404:200);};
 assert(probe_model_choices(mixed,result,sizeof(result))==E_NONE&&calls==MODEL_COUNT);
 assert(strstr(result,modelChoices[0].id)&&!strstr(result,modelChoices[1].id));
 calls=0;auto none=[&](const char*){calls++;return model_probe_status(404);};
 assert(probe_model_choices(none,result,sizeof(result))==E_NONE&&calls==MODEL_COUNT&&!result[0]);
 for(int code:{401,403,429,500}){
  calls=0;auto failed=[&](const char*){return model_probe_status(++calls==3?code:200);};
  assert(probe_model_choices(failed,result,sizeof(result))!=E_NONE&&calls==3&&!result[0]);
 }
 calls=0;auto timed=[&](const char*){calls++;return ModelProbeResult{false,E_TIMEOUT};};
 assert(probe_model_choices(timed,result,sizeof(result))==E_TIMEOUT&&calls==1&&!result[0]);
 assert(probe_model_choices(none,result,0)==E_BIG);
 calls=0;assert(probe_model_choices(all,result,4)==E_BIG&&!result[0]&&calls==1);
 assert(model_probe_status(401).error==E_AUTH&&model_probe_status(429).error==E_LIMIT);
 puts("PASS fixed-memory account probes: exact shared aliases, all/mixed/zero access, 404 skips, auth/rate/server/timeout failures, no partial publication, 256-byte result and bounded output; no response bodies or paid requests.");
}
