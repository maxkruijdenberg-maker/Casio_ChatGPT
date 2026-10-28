#include <cassert>
#include <cstdio>
#include <string>
#include "../atom/ChatGPTAtom/model_catalog.h"
#include "../atom/ChatGPTAtom/config_validation.h"
int main(){
 assert(setting_valid(4,"ft:gpt-4.1:project:custom",strlen("ft:gpt-4.1:project:custom")));
 assert(!setting_valid(4,"gpt-model/path",strlen("gpt-model/path")));
 const char *text=R"({"data":[{"id":"gpt-5-mini"},{"id":"gpt-4.1"},{"id":"gpt-5-mini"},{"id":"o3"},{"id":"ft:gpt-4.1:project:custom"},{"id":"gpt-4o-audio-preview"},{"id":"gpt-image-1"},{"id":"text-embedding-3-small"},{"id":"gpt-4o-transcribe"},{"id":"gpt-4o-realtime-preview"},{"id":"whisper-1"},{"id":"bad\nname"},{"id":32},{}]})";
 JsonDocument doc;assert(!deserializeJson(doc,text));char out[1024];
 assert(catalog_text(doc.as<JsonVariantConst>(),out,sizeof(out))==4);
 assert(!strcmp(out,"ft:gpt-4.1:project:custom\ngpt-4.1\ngpt-5-mini\no3\n"));
 assert(!catalog_text(doc.as<JsonVariantConst>(),out,8)&&!out[0]);
 doc.clear();assert(!catalog_text(doc.as<JsonVariantConst>(),out,sizeof(out)));
 assert(!chat_candidate("gpt-4o-mini-tts")&&!chat_candidate("gpt-3.5-turbo-instruct"));
 std::string huge="gpt-"+std::string(90,'a');assert(!chat_candidate(huge.c_str()));
 assert(chat_candidate("gpt-future-text")&&chat_candidate("o9-future"));
 doc.clear();auto data=doc["data"].to<JsonArray>();
 for(int i=0;i<CATALOG_COUNT+1;i++)data.add<JsonObject>()["id"]="gpt-test-"+std::to_string(i);
 char full[CATALOG_MAX];assert(!catalog_text(doc.as<JsonVariantConst>(),full,sizeof(full)));
 puts("PASS model catalog: account IDs, text candidates, non-chat filtering, sorting/deduplication, malformed fields, bounds and overflow without silently truncated list; no network requests.");
}
