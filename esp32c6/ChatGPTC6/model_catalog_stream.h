#ifndef CHATGPT_MODEL_CATALOG_STREAM_H
#define CHATGPT_MODEL_CATALOG_STREAM_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "model_choices.h"
#include "wire.h"
#define MODEL_CATALOG_OUTPUT_BYTES 2048
#define MODEL_CATALOG_BODY_MAX (512u*1024u)
/* Incremental JSON recognizer for GET /v1/models. Memory is independent of
 * account size: only syntax state, one ID/key and maintained-alias flags.
 * HTTPClient must supply the decoded body (writeToStream), not raw chunks. */
class ModelCatalogParser {
 enum {OTHER=0,ROOT=1,DATA=2,ITEM=3};
 enum {KEY_OTHER=0,KEY_DATA=1,KEY_ID=2};
 struct Frame {uint8_t kind,state,role,key;bool idSeen;};
 Frame frames[24];size_t depth=0,bytes=0;
 bool started=false,done=false,dataSeen=false,found[MODEL_COUNT]={};
 uint8_t error=E_NONE,token=0,stringRole=0,escape=0,unicodeDigits=0;
 uint32_t unicode=0,high=0,utf8=0;uint8_t utf8Left=0;uint32_t utf8Min=0;
 char text[256];size_t textLen=0;bool textLong=false;
 const char *literal=nullptr;size_t literalAt=0;uint8_t number=0;
 bool fail(uint8_t e=E_JSON){error=e;return false;}
 static bool space(uint8_t c){return c==' '||c=='\t'||c=='\r'||c=='\n';}
 static bool delimiter(uint8_t c){return space(c)||c==','||c==']'||c=='}';}
 static bool digit(uint8_t c){return c>='0'&&c<='9';}
 static int hex(uint8_t c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;}
 bool value(uint8_t kind){
  if(!depth){if(started||kind!='{')return fail();started=true;return true;}
  Frame &f=frames[depth-1];
  if(f.kind=='{'){
   if(f.state!=2)return fail();
   if(f.role==ROOT&&f.key==KEY_DATA){if(kind!='['||dataSeen)return fail();dataSeen=true;}
   if(f.role==ITEM&&f.key==KEY_ID){if(kind!='"'||f.idSeen)return fail();f.idSeen=true;}
   f.state=3;
  }else {if(f.state!=0&&f.state!=2)return fail();if(f.role==DATA&&kind!='{')return fail();f.state=1;}
  return true;
 }
 bool open(uint8_t kind){
  uint8_t role=OTHER;
  if(!depth)role=ROOT;
  else {const Frame &p=frames[depth-1];if(p.role==ROOT&&p.key==KEY_DATA)role=DATA;else if(p.role==DATA)role=ITEM;}
  if(!value(kind))return false;if(depth==24)return fail(E_BIG);
  frames[depth++]={kind,0,role,KEY_OTHER,false};return true;
 }
 bool close(uint8_t kind){
  if(!depth)return fail();const Frame &f=frames[depth-1];
  if(f.kind!=kind)return fail();
  if(kind=='{'?(f.state!=0&&f.state!=3):(f.state!=0&&f.state!=1))return fail();
  if(f.role==ITEM&&!f.idSeen)return fail();
  depth--;if(!depth){if(!dataSeen)return fail();done=true;}return true;
 }
 bool decoded(uint32_t cp){
  if(stringRole==2&&(cp<33||cp>126))return fail();
  if(cp>127){textLong=true;return true;}
  if(textLen<255)text[textLen++]=(char)cp;else textLong=true;
  return stringRole!=2||!textLong||fail();
 }
 bool endString(){
  token=0;text[textLen]=0;
  if(stringRole==1){
   Frame &f=frames[depth-1];f.key=KEY_OTHER;
   if(!textLong&&f.role==ROOT&&textLen==4&&!memcmp(text,"data",4))f.key=KEY_DATA;
   if(!textLong&&f.role==ITEM&&textLen==2&&!memcmp(text,"id",2))f.key=KEY_ID;
   f.state=1;
  }else if(stringRole==2){
   if(!textLen||textLong)return fail();
   for(int i=0;i<MODEL_COUNT;i++)if(strlen(modelChoices[i].id)==textLen&&!memcmp(modelChoices[i].id,text,textLen))found[i]=true;
  }
  return true;
 }
 bool stringByte(uint8_t c){
  if(escape==3){if(c!='\\')return fail();escape=4;return true;}
  if(escape==4){if(c!='u')return fail();escape=2;unicode=0;unicodeDigits=0;return true;}
  if(escape==2){
   int h=hex(c);if(h<0)return fail();unicode=(unicode<<4)|(unsigned)h;
   if(++unicodeDigits<4)return true;
   if(high){if(unicode<0xdc00||unicode>0xdfff)return fail();unicode=0x10000+((high-0xd800)<<10)+(unicode-0xdc00);high=0;escape=0;return decoded(unicode);}
   if(unicode>=0xd800&&unicode<=0xdbff){high=unicode;escape=3;return true;}
   if(unicode>=0xdc00&&unicode<=0xdfff)return fail();escape=0;return decoded(unicode);
  }
  if(escape==1){
   escape=0;switch(c){case '"':case '\\':case '/':return decoded(c);case 'b':return decoded('\b');case 'f':return decoded('\f');case 'n':return decoded('\n');case 'r':return decoded('\r');case 't':return decoded('\t');case 'u':escape=2;unicode=0;unicodeDigits=0;return true;default:return fail();}
  }
  if(utf8Left){
   if((c&0xc0)!=0x80)return fail();utf8=(utf8<<6)|(c&0x3f);
   if(--utf8Left)return true;
   if(utf8<utf8Min||utf8>0x10ffff||(utf8>=0xd800&&utf8<=0xdfff))return fail();return decoded(utf8);
  }
  if(c=='"')return endString();if(c=='\\'){escape=1;return true;}if(c<32)return fail();
  if(c<128)return decoded(c);
  if(c>=0xc2&&c<=0xdf){utf8Left=1;utf8=c&0x1f;utf8Min=0x80;return true;}
  if(c>=0xe0&&c<=0xef){utf8Left=2;utf8=c&0x0f;utf8Min=0x800;return true;}
  if(c>=0xf0&&c<=0xf4){utf8Left=3;utf8=c&7;utf8Min=0x10000;return true;}return fail();
 }
 bool numberByte(uint8_t c){
  switch(number){
  case 1:if(c=='0'){number=2;return true;}if(c>='1'&&c<='9'){number=3;return true;}break;
  case 2:if(c=='.'){number=4;return true;}if(c=='e'||c=='E'){number=6;return true;}break;
  case 3:if(digit(c))return true;if(c=='.'){number=4;return true;}if(c=='e'||c=='E'){number=6;return true;}break;
  case 4:if(digit(c)){number=5;return true;}break;
  case 5:if(digit(c))return true;if(c=='e'||c=='E'){number=6;return true;}break;
  case 6:if(c=='+'||c=='-'){number=7;return true;}if(digit(c)){number=8;return true;}break;
  case 7:if(digit(c)){number=8;return true;}break;
  case 8:if(digit(c))return true;break;
  }
  if(!delimiter(c)||(number!=2&&number!=3&&number!=5&&number!=8))return fail();
  token=0;return syntax(c);
 }
 bool syntax(uint8_t c){
  if(space(c))return true;if(done)return fail();
  if(c=='{')return open('{');if(c=='[')return open('[');
  if(c=='}')return close('{');if(c==']')return close('[');
  if(c==','){
   if(!depth)return fail();Frame &f=frames[depth-1];if(f.kind=='{'?f.state!=3:f.state!=1)return fail();f.state=f.kind=='{'?4:2;return true;
  }
  if(c==':'){if(!depth||frames[depth-1].kind!='{'||frames[depth-1].state!=1)return fail();frames[depth-1].state=2;return true;}
  if(c=='"'){
   stringRole=0;
   if(depth&&frames[depth-1].kind=='{'&&(frames[depth-1].state==0||frames[depth-1].state==4))stringRole=1;
   else {if(depth&&frames[depth-1].role==ITEM&&frames[depth-1].key==KEY_ID)stringRole=2;if(!value('"'))return false;}
   textLen=0;textLong=false;escape=0;utf8Left=0;high=0;token=1;return true;
  }
  if(c=='t'||c=='f'||c=='n'){
   if(!value(c))return false;literal=c=='t'?"true":c=='f'?"false":"null";literalAt=1;token=2;return true;
  }
  if(c=='-'||digit(c)){if(!value(c))return false;number=c=='-'?1:c=='0'?2:3;token=3;return true;}
  return fail();
 }
 public:
 bool write(uint8_t c){
  if(error)return false;if(bytes++==MODEL_CATALOG_BODY_MAX)return fail(E_BIG);
  if(token==1)return stringByte(c);
  if(token==2){if(c!=(uint8_t)literal[literalAt])return fail();if(!literal[++literalAt])token=0;return true;}
  if(token==3)return numberByte(c);return syntax(c);
 }
 size_t write(const uint8_t *data,size_t n){size_t i=0;while(i<n&&write(data[i]))i++;return i;}
 uint8_t status()const{return error;}
 uint8_t finish(char *out,size_t cap){
  if(!cap)return E_BIG;out[0]=0;
  if(error)return error;if(token||depth||!done||!dataSeen)return E_JSON;
  size_t used=0;for(int i=0;i<MODEL_COUNT;i++)if(found[i]){
   size_t n=strlen(modelChoices[i].id);if(n+1>=cap-used){out[0]=0;return E_BIG;}
   memcpy(out+used,modelChoices[i].id,n);used+=n;out[used++]='\n';out[used]=0;
  }return E_NONE;
 }
};
#endif
