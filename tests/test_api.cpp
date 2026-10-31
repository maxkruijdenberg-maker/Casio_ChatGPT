#include <cassert>
#include <cstdio>
#include "../atom/ChatGPTAtom/api_response.h"
static int parse(const char *json,size_t limit=8192){
 JsonDocument doc;assert(!deserializeJson(doc,json));CompletionView v={};return completion_view(doc.as<JsonVariantConst>(),limit,&v);
}
int main(){
 assert(parse("{\"choices\":[{\"message\":{\"content\":\"65\"},\"finish_reason\":\"stop\"}]}")==E_NONE);
 assert(parse("{\"choices\":[{\"message\":{\"content\":\"éπ😀\"}}]}")==E_NONE);
 assert(parse("{\"choices\":[{\"message\":{\"content\":\"abcdef\"}}]}",5)==E_BIG);
 assert(parse("{\"choices\":[]}")==E_JSON);
 assert(parse("{\"choices\":[{\"message\":{\"content\":null}}]}")==E_JSON);
 assert(parse("{\"choices\":[{\"message\":{\"content\":[]}}]}")==E_JSON);
 assert(parse("{\"choices\":[{\"message\":{\"content\":\"\"}}]}")==E_JSON);
 assert(parse("{\"choices\":[{\"message\":{\"content\":\"a\\u0000b\"}}]}")==E_JSON);
 assert(parse("{\"choices\":[{\"message\":{\"refusal\":\"No\",\"content\":null}}]}")==E_REFUSED);
 assert(parse("{\"choices\":[{\"message\":{\"content\":\"no\"},\"finish_reason\":\"content_filter\"}]}")==E_REFUSED);
 JsonDocument doc;deserializeJson(doc,"{\"choices\":[{\"message\":{\"content\":\"ok\"},\"finish_reason\":\"length\"}]}");
 CompletionView v={};assert(completion_view(doc.as<JsonVariantConst>(),8192,&v)==0&&v.truncated&&v.size==2);
 JsonDocument metadata;assert(!deserializeJson(metadata,"{\"model\":\"gpt-selected-2026-10-30\"}"));assert(!strcmp(completion_model(metadata.as<JsonVariantConst>()),"gpt-selected-2026-10-30"));
 assert(!deserializeJson(metadata,"{\"model\":\"bad\\nmodel\"}"));assert(!*completion_model(metadata.as<JsonVariantConst>()));
 assert(!deserializeJson(metadata,"{\"model\":\"abc\\u0000def\"}"));assert(!*completion_model(metadata.as<JsonVariantConst>()));
 metadata.clear();assert(!*completion_model(metadata.as<JsonVariantConst>()));metadata["model"]=42;assert(!*completion_model(metadata.as<JsonVariantConst>()));
 puts("PASS: actual OpenAI response parser; success, accents, size limits, null/array/empty text, embedded NUL, refusal, filtering and truncation.");
}
