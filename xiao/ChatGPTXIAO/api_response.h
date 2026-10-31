#ifndef CHATGPT_API_RESPONSE_H
#define CHATGPT_API_RESPONSE_H
#include <ArduinoJson.h>
#include "wire.h"
#include "utf8.h"
#include "model_receipt.h"
#include "connection_check.h"
static const char *completion_model(JsonVariantConst root){
 if(!root["model"].is<const char*>())return "";
 JsonString value=root["model"].as<JsonString>();
 return receipt_model_valid(value.c_str(),value.size())?value.c_str():"";
}
static CheckHttpResult completion_check_response(int status,JsonVariantConst root,bool parsed){
 CheckHttpResult result;result.status=status;
 if(status!=200){const char *code=parsed?(root["error"]["code"]|""):"";result.error=check_http_error(status,code);return result;}
 if(!parsed){result.error=E_JSON;return result;}
 result.valid=!strcmp(root["object"]|"","chat.completion")&&root["choices"].is<JsonArrayConst>()&&root["choices"].size()>0;
 const char *reported=completion_model(root);memcpy(result.reported,reported,strlen(reported)+1);return result;
}
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
