#include <cassert>
#include <cstdio>
#include <string>
#include "../atom/ChatGPTAtom/model_catalog.h"
#include "../atom/ChatGPTAtom/catalog_response.h"
#include "../atom/ChatGPTAtom/model_kind.h"
#include "../atom/ChatGPTAtom/config_validation.h"
struct TestBodyText {
 std::string value;bool fail=false;
 size_t length()const{return value.size();}
 bool concat(const char *p,size_t n){if(fail)return false;value.append(p,n);return true;}
};
int main(){
 /* Network blocks may split any JSON token. The sink receives decoded body
  * bytes from HTTPClient::writeToStream, not chunk-size lines from getStream. */
 std::string json=R"({"object":"list","data":[{"id":"gpt-6-astra","owned_by":"openai","created":123},{"id":"gpt-6.1-sol"}]})";
 for(size_t block:{size_t(1),size_t(3),size_t(17),json.size()}){
  CatalogBodyBuffer<TestBodyText> body;
  for(size_t at=0;at<json.size();at+=block){size_t n=std::min(block,json.size()-at);assert(body.write((const uint8_t*)json.data()+at,n)==n);}
  JsonDocument parsed;assert(catalog_response(body.text.value,parsed)==E_NONE);
  assert(parsed["data"].size()==2&&!parsed["data"][0]["owned_by"].is<const char*>());
  char names[100];assert(catalog_text(parsed.as<JsonVariantConst>(),names,sizeof(names))==2);
 }
 JsonDocument parsed;std::string rawChunked="3\r\n{\"d\r\n0\r\n\r\n";
 assert(catalog_response(rawChunked,parsed)==E_JSON);
 for(std::string bad:{std::string("{\"data\":["),std::string("<html>bad gateway</html>"),std::string("{\"error\":{\"message\":\"not a list\"}}")})assert(catalog_response(bad,parsed)==E_JSON);
 CatalogBodyBuffer<TestBodyText> bound(3);const uint8_t bytes[]={'a','b','c','d'};
 assert(bound.write(bytes,3)==3&&!bound.overflow);assert(!bound.write(bytes,1)&&bound.overflow&&!bound.noMemory&&bound.text.value=="abc");
 CatalogBodyBuffer<TestBodyText> memory;memory.text.fail=true;
 assert(!memory.write(bytes,1)&&memory.noMemory&&!memory.overflow);

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
 puts("PASS bounded decoded HTTP body with split JSON tokens, malformed/truncated/raw chunk framing rejection, separate memory/size limits; full OpenAI account catalog: no family filtering, GPT-6/Astra, image/audio/video/vector/moderation/unknown IDs retained, sorting/deduplication, long IDs, malformed metadata and bounds; no network requests.");
}
