#ifndef CHATGPT_CATALOG_RESPONSE_H
#define CHATGPT_CATALOG_RESPONSE_H
#include <ArduinoJson.h>
#include "wire.h"
#define CATALOG_HTTP_MAX 524288
/* Receives decoded HTTP body bytes, never raw transfer/chunk framing.
 * Allocate only as bytes arrive, with separate size and allocation failures. */
template<class Text> struct CatalogBodyBuffer {
 Text text;bool overflow=false,noMemory=false;size_t limit;
 explicit CatalogBodyBuffer(size_t cap=CATALOG_HTTP_MAX):limit(cap){}
 size_t write(const uint8_t *p,size_t n){
  if(overflow||noMemory)return 0;
  if(n>limit-text.length()){overflow=true;return 0;}
  if(n&&!text.concat((const char*)p,n)){noMemory=true;return 0;}
  return n;
 }
};
/* The caller must feed HTTPClient::writeToStream output, not getStream().
 * JSON filtering keeps only IDs in the document; the input buffer is released
 * before allocating the sorted output list. Missing data is a format error. */
template<class Text> static int catalog_response(Text &body,JsonDocument &document){
 JsonDocument filter;filter["data"][0]["id"]=true;
 auto parsed=deserializeJson(document,body,DeserializationOption::Filter(filter),DeserializationOption::NestingLimit(8));
 if(parsed||document.overflowed())return parsed==DeserializationError::NoMemory?E_MEMORY:E_JSON;
 if(!document["data"].is<JsonArrayConst>())return E_JSON;
 return E_NONE;
}
#endif
