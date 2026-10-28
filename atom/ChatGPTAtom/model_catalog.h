#ifndef CHATGPT_MODEL_CATALOG_H
#define CHATGPT_MODEL_CATALOG_H
#include <ArduinoJson.h>
#include <string.h>
#include <stdlib.h>
#define CATALOG_MAX 32768
#define CATALOG_COUNT 384
/* /models has no endpoint-capability metadata. These are text-chat candidates,
 * not an assertion that every account/model accepts Chat Completions. */
static int chat_candidate(const char *id){
 if(!id||!*id||strlen(id)>80)return 0;
 const char *base=id;if(!strncmp(base,"ft:",3))base+=3;
 if(strncmp(base,"gpt-",4)&&strncmp(base,"chatgpt-",8)&&!(base[0]=='o'&&base[1]>='1'&&base[1]<='9'))return 0;
 const char *excluded[]={"audio","realtime","transcrib","tts","image","instruct","embedding","moderation","deep-research",0};
 for(int i=0;excluded[i];i++)if(strstr(base,excluded[i]))return 0;
 for(const char *p=id;*p;p++)if(!((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||*p=='-'||*p=='_'||*p=='.'||*p==':'))return 0;
 return 1;
}
static int catalog_compare(const void*a,const void*b){return strcmp(*(const char*const*)a,*(const char*const*)b);}
/* Fail rather than silently present an incomplete account list. */
static int catalog_text(JsonVariantConst response,char *out,size_t cap){
 const char *ids[CATALOG_COUNT];int count=0;size_t used=0;
 if(!cap)return 0;out[0]=0;
 if(!response["data"].is<JsonArrayConst>())return 0;
 for(JsonVariantConst item:response["data"].as<JsonArrayConst>()){
  const char *id=item["id"].as<const char*>();if(!chat_candidate(id))continue;
  int duplicate=0;for(int i=0;i<count;i++)if(!strcmp(ids[i],id)){duplicate=1;break;}
  if(duplicate)continue;if(count==CATALOG_COUNT)return 0;ids[count++]=id;
 }
 qsort(ids,count,sizeof(ids[0]),catalog_compare);
 for(int i=0;i<count;i++){
  size_t n=strlen(ids[i]);if(used+n+1>=cap){out[0]=0;return 0;}
  memcpy(out+used,ids[i],n);used+=n;out[used++]='\n';
 }
 out[used]=0;return count;
}
#endif
