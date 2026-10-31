#ifndef CHATGPT_MODEL_CATALOG_H
#define CHATGPT_MODEL_CATALOG_H
#include <ArduinoJson.h>
#include <string.h>
#include <stdlib.h>
#define CATALOG_MAX 65536
#define CATALOG_COUNT 1024
/* Full account catalog: no family, prefix or modality filtering. Invalid or
 * oversized metadata fails the whole refresh, never silently drops an ID. */
static int catalog_id(const char *id){
 if(!id||!*id||strlen(id)>255)return 0;
 for(const unsigned char *p=(const unsigned char*)id;*p;p++)if(*p<33||*p>126)return 0;
 return 1;
}
static int catalog_compare(const void*a,const void*b){return strcmp(*(const char*const*)a,*(const char*const*)b);}
/* Fail rather than silently present an incomplete account list. */
static int catalog_text(JsonVariantConst response,char *out,size_t cap){
 const char *ids[CATALOG_COUNT];int count=0;size_t used=0;
 if(!cap)return 0;out[0]=0;
 if(!response["data"].is<JsonArrayConst>())return 0;
 for(JsonVariantConst item:response["data"].as<JsonArrayConst>()){
  const char *id=item["id"].as<const char*>();if(!catalog_id(id))return 0;
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
