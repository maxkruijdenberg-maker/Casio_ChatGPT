#ifndef CHATGPT_MODEL_RECEIPT_H
#define CHATGPT_MODEL_RECEIPT_H
#include "wire.h"
/* A receipt is tied to one admitted request, never to a selected history chat. */
struct ModelReceipt {uint32_t request=0;bool complete=false;char requested[256]={0},reported[256]={0};};
static int receipt_model_valid(const char *text,size_t len){
 if(!len||len>255)return 0;
 for(size_t i=0;i<len;i++){unsigned char c=text[i];if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c==':'||c=='/'))return 0;}
 return 1;
}
static void receipt_clear(ModelReceipt *receipt){*receipt=ModelReceipt{};}
static void receipt_publish(ModelReceipt *receipt,uint32_t request,const char *requested,const char *reported){
 receipt_clear(receipt);size_t n=strlen(requested);if(!request||!receipt_model_valid(requested,n))return;
 receipt->request=request;receipt->complete=true;memcpy(receipt->requested,requested,n+1);
 n=strlen(reported);if(receipt_model_valid(reported,n))memcpy(receipt->reported,reported,n+1);
}
/* Payload: field(0=requested,1=reported), byte offset, expected request ID (0=latest).
 * Response: error, complete, owner request ID, string length, chunk (<=17 bytes). */
static void receipt_read(const ModelReceipt *receipt,const WFrame *in,WFrame *out){
 if(in->len!=7||in->data[0]>1){out->data[0]=E_REQUEST;return;}
 uint32_t expected=w_get32(in->data+3);size_t at=w_get16(in->data+1);
 if(expected&&expected!=receipt->request){out->data[0]=E_REQUEST;return;}
 const char *value=in->data[0]?receipt->reported:receipt->requested;size_t len=strlen(value);
 if(at>len){out->data[0]=E_REQUEST;return;}
 size_t n=len-at;if(n>WIRE_MAX-7)n=WIRE_MAX-7;
 out->len=7+n;out->data[1]=receipt->complete;w_put32(out->data+2,receipt->request);out->data[6]=(uint8_t)len;memcpy(out->data+7,value+at,n);
}
#endif
