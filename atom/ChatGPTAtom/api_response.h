#ifndef CHATGPT_API_RESPONSE_H
#define CHATGPT_API_RESPONSE_H
#include <ArduinoJson.h>
#include "wire.h"
#include "utf8.h"
struct CompletionView {const char *text;size_t size;bool truncated;};
static int completion_view(JsonVariantConst root,size_t max,CompletionView *out){
 JsonVariantConst choice=root["choices"][0],msg=choice["message"];
 const char *finish=choice["finish_reason"]|"";
 const char *refusal=msg["refusal"]|"";
 if(*refusal||!strcmp(finish,"content_filter"))return E_REFUSED;
 if(!msg["content"].is<const char*>())return E_JSON;
 JsonString text=msg["content"].as<JsonString>();
 if(!text.size())return E_JSON;
 if(text.size()>max)return E_BIG;
 if(!u_valid(text.c_str(),text.size()))return E_JSON;
 out->text=text.c_str();out->size=text.size();out->truncated=!strcmp(finish,"length");return E_NONE;
}
#endif
