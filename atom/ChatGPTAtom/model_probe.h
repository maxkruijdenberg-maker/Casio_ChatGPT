#ifndef CHATGPT_MODEL_PROBE_H
#define CHATGPT_MODEL_PROBE_H
#include "model_choices.h"
#include "wire.h"
#define MODEL_PROBE_BYTES 2048
struct ModelProbeResult {bool available;uint8_t error;};
static ModelProbeResult model_probe_status(int code){
 if(code==200)return {true,E_NONE};
 if(code==404)return {false,E_NONE};
 if(code==401||code==403)return {false,E_AUTH};
 if(code==429)return {false,E_LIMIT};
 return {false,E_HTTP};
}
/* GET metadata for exactly the maintained aliases. No response body, JSON
 * document, full account list or 64-KiB sorting buffer is retained. A failed
 * check clears the entire output, so partial access is never reported as full. */
template<class Probe> static uint8_t probe_model_choices(Probe probe,char *out,size_t cap){
 if(!cap)return E_BIG;out[0]=0;size_t used=0;
 for(int i=0;i<MODEL_COUNT;i++){
  ModelProbeResult result=probe(modelChoices[i].id);
  if(result.error){out[0]=0;return result.error;}
  if(!result.available)continue;
  size_t n=strlen(modelChoices[i].id);
  if(n+1>=cap-used){out[0]=0;return E_BIG;}
  memcpy(out+used,modelChoices[i].id,n);used+=n;out[used++]='\n';out[used]=0;
 }
 return E_NONE;
}
#endif
