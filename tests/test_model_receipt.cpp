#include <cassert>
#include <cstdio>
#include <string>
#include "../atom/ChatGPTAtom/model_receipt.h"
static std::string read(const ModelReceipt &receipt,int field,uint32_t expected=0){
 std::string result;WFrame in={},out={};in.len=7;in.data[0]=field;w_put32(in.data+3,expected);
 do{
  out={};out.len=1;w_put16(in.data+1,result.size());receipt_read(&receipt,&in,&out);
  assert(!out.data[0]&&out.len>=7&&out.len<=WIRE_MAX&&out.data[1]==receipt.complete&&w_get32(out.data+2)==receipt.request);
  result.append((const char*)out.data+7,out.len-7);
 }while(result.size()<out.data[6]);return result;
}
int main(){
 ModelReceipt r;assert(!r.complete&&read(r,0).empty()&&read(r,1).empty());
 receipt_publish(&r,123,"gpt-selected","gpt-reported-2026-10-30");assert(r.complete&&r.request==123&&read(r,0,123)=="gpt-selected"&&read(r,1,123)=="gpt-reported-2026-10-30");
 std::string selected="gpt-new-setting";assert(read(r,0)=="gpt-selected");
 WFrame in={},out={};in.len=7;w_put32(in.data+3,124);receipt_read(&r,&in,&out);assert(out.data[0]==E_REQUEST);
 in={};in.len=7;in.data[0]=2;out={};receipt_read(&r,&in,&out);assert(out.data[0]==E_REQUEST);
 in={};in.len=6;out={};receipt_read(&r,&in,&out);assert(out.data[0]==E_REQUEST);
 in={};in.len=7;w_put16(in.data+1,255);out={};receipt_read(&r,&in,&out);assert(out.data[0]==E_REQUEST);
 receipt_publish(&r,124,std::string(255,'x').c_str(),std::string(255,'y').c_str());assert(read(r,0,124)==std::string(255,'x')&&read(r,1,124)==std::string(255,'y'));
 receipt_publish(&r,125,"gpt-requested","");assert(r.complete&&read(r,0)=="gpt-requested"&&read(r,1).empty());
 receipt_publish(&r,126,"gpt-requested","bad\nmodel");assert(r.complete&&read(r,1).empty());
 receipt_publish(&r,0,"gpt-requested","gpt-returned");assert(!r.complete&&!r.request);
 receipt_publish(&r,127,"bad model","gpt-returned");assert(!r.complete);
 assert(!receipt_model_valid("abc\0def",7)&&!receipt_model_valid(std::string(256,'x').c_str(),256));
 receipt_clear(&r);assert(!r.complete&&!r.request&&read(r,0).empty());
 puts("PASS model receipts: requested/reported IDs remain separate, immutable request ownership, 17-byte chunks, 255-byte limits, absent metadata, invalid/NUL metadata, clear on lifecycle and stale/malformed reads; no model-name guessing or fallback.");
}
