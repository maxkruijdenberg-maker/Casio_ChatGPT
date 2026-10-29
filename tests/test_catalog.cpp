#include <cassert>
#include <cstdio>
#include <string>
#include "../atom/ChatGPTAtom/model_catalog.h"
#include "../atom/ChatGPTAtom/model_kind.h"
#include "../atom/ChatGPTAtom/config_validation.h"
int main(){
 const char *text=R"({"data":[{"id":"gpt-6-astra"},{"id":"gpt-6.1-sol"},{"id":"gpt-4.1"},{"id":"gpt-image-1"},{"id":"whisper-1"},{"id":"tts-1"},{"id":"text-embedding-3-small"},{"id":"omni-moderation-latest"},{"id":"sora-2"},{"id":"dall-e-3"},{"id":"ft:gpt-4.1:project:custom"},{"id":"gpt-6-astra"},{"id":"new-future-family"}]})";
 JsonDocument doc;assert(!deserializeJson(doc,text));char out[2048];
 assert(catalog_text(doc.as<JsonVariantConst>(),out,sizeof(out))==12);
 for(const char *id:{"gpt-6-astra","gpt-image-1","whisper-1","tts-1","text-embedding-3-small","omni-moderation-latest","sora-2","dall-e-3","new-future-family"})assert(strstr(out,id));
 assert(!strncmp(out,"dall-e-3\n",9));assert(!catalog_text(doc.as<JsonVariantConst>(),out,8)&&!out[0]);
 assert(model_kind("gpt-6-astra")==0&&model_kind("gpt-image-1")==1&&model_kind("whisper-1")==2&&model_kind("text-embedding-3-small")==3&&model_kind("sora-2")==5&&model_kind("new-future-family")==7);
 doc.clear();assert(!catalog_text(doc.as<JsonVariantConst>(),out,sizeof(out)));
 assert(!deserializeJson(doc,R"({"data":[{"id":"good"},{"id":"bad\nname"}]})"));assert(!catalog_text(doc.as<JsonVariantConst>(),out,sizeof(out)));
 assert(!deserializeJson(doc,R"({"data":[{"id":32}]})"));assert(!catalog_text(doc.as<JsonVariantConst>(),out,sizeof(out)));
 std::string longest(255,'a'),tooLong(256,'a');assert(catalog_id(longest.c_str())&&!catalog_id(tooLong.c_str()));
 assert(setting_valid(4,longest.c_str(),255)&&!setting_valid(4,tooLong.c_str(),256));
 assert(setting_valid(4,"ft:gpt-4.1:project:custom",strlen("ft:gpt-4.1:project:custom")));
 assert(catalog_id("future/model")&&!setting_valid(4,"future/model",12));
 doc.clear();auto data=doc["data"].to<JsonArray>();
 for(int i=0;i<CATALOG_COUNT+1;i++)data.add<JsonObject>()["id"]="future-"+std::to_string(i);
 char full[CATALOG_MAX];assert(!catalog_text(doc.as<JsonVariantConst>(),full,sizeof(full)));
 puts("PASS full OpenAI account catalog: no family filtering, GPT-6/Astra, image/audio/video/vector/moderation/unknown IDs retained, sorting/deduplication, long IDs, malformed metadata and bounds; no network requests.");
}
