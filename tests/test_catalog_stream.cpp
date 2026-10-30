#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include "../atom/ChatGPTAtom/model_catalog_stream.h"
#include "../atom/ChatGPTAtom/catalog_deadline.h"
static uint8_t parse(const std::string &body,std::string &result,size_t chunk=1,size_t cap=MODEL_CATALOG_OUTPUT_BYTES){
 ModelCatalogParser p;char out[MODEL_CATALOG_OUTPUT_BYTES];assert(cap<=sizeof(out));
 for(size_t at=0;at<body.size();at+=chunk){size_t n=body.size()-at;if(n>chunk)n=chunk;p.write((const uint8_t*)body.data()+at,n);}
 uint8_t error=p.finish(out,cap);result=cap?out:"";return error;
}
struct IdleClient {
 int stops=0,reads=0;bool open=true;
 virtual int available(){return 0;}
 virtual uint8_t connected(){return open;}
 virtual int read(){reads++;return -1;}
 virtual int read(uint8_t*,size_t){reads++;return 0;}
 virtual void stop(){stops++;open=false;}
};
static uint32_t now;
static uint32_t fakeMillis(){return now;}
static void testDeadline(){
 now=100;ModelCatalogDeadlineClient<IdleClient,decltype(&fakeMillis)> client(fakeMillis,now);
 assert(client.connected()&&!client.timedOut&&!client.available());
 now=60099;assert(client.connected());now=60100;assert(!client.available()&&client.timedOut&&client.stops==1&&!client.connected());
 uint8_t byte=0;assert(client.read()==-1&&client.read(&byte,1)==-1&&client.stops==1&&client.reads==0);
 now=0xfffffff0;ModelCatalogDeadlineClient<IdleClient,decltype(&fakeMillis)> wrapped(fakeMillis,now);
 now+=59999;assert(wrapped.connected());now++;assert(!wrapped.connected()&&wrapped.timedOut&&wrapped.stops==1);
 /* Simulate upstream unknown-length identity-body polling: no sink writes. */
 now=0;ModelCatalogDeadlineClient<IdleClient,decltype(&fakeMillis)> idle(fakeMillis,now);int polls=0;
 while(idle.available()||idle.connected()){now+=1000;polls++;assert(polls<=60);}
 assert(idle.timedOut&&polls==60&&idle.stops==1);
 /* Known-length zero-byte reads also terminate at the overall deadline. */
 now=0;ModelCatalogDeadlineClient<IdleClient,decltype(&fakeMillis)> stalled(fakeMillis,now);polls=0;
 while(stalled.connected()){assert(stalled.read(&byte,1)==0);now+=5000;polls++;assert(polls<=12);}
 assert(stalled.timedOut&&polls==12&&stalled.stops==1);
}
int main(){
 testDeadline();
 static_assert(sizeof(ModelCatalogParser)<1024,"Catalog parser must use bounded small working memory");
 std::string all="{\"object\":\"list\",\"data\":[",expected;
 for(int i=MODEL_COUNT-1;i>=0;i--){if(i!=MODEL_COUNT-1)all+=",";all+="{\"id\":\""+std::string(modelChoices[i].id)+"\",\"created\":123,\"owned_by\":\"openai\"}";}
 all+="]}";for(int i=0;i<MODEL_COUNT;i++)expected+=std::string(modelChoices[i].id)+"\n";
 for(size_t chunk:{1u,2u,3u,7u,22u,63u,512u,4096u}){std::string out;assert(parse(all,out,chunk)==E_NONE&&out==expected);}
 std::string out;
 assert(parse("{\"data\":[]}",out)==E_NONE&&out.empty());
 std::string id=modelChoices[0].id,escaped=id;escaped.replace(0,1,"\\u0067");
 std::string mixed="{\"data\":[{\"id\":\"unknown-model\"},{\"id\":\""+escaped+"\"},{\"id\":\""+id+"\"}]}";
 assert(parse(mixed,out)==E_NONE&&out==id+"\n");
 assert(parse("{\"da\\u0074a\":[{\"id\":\""+id+"\"}],\"metadata\":{\"data\":[{\"id\":\"ignored\"}],\"n\":[0,-1,1.25,2e+2,1E-2,true,false,null],\"utf8\":\"é😀\",\"escaped\":\"\\uD83D\\uDE00\\n\\t\\b\\r\\f\\\\\\\"\\/\"}}",out,2)==E_NONE&&out==id+"\n");
 std::vector<std::string> bad={"", "[]", "{}", "{\"data\":{}}", "{\"data\":null}", "{\"data\":[1]}","{\"data\":[[]]}","{\"data\":[{}]}","{\"data\":[{\"id\":1}]}","{\"data\":[{\"id\":\"\"}]}","{\"data\":[{\"id\":\"bad id\"}]}","{\"data\":[{\"id\":\"\\u0000\"}]}","{\"data\":[{\"id\":\"é\"}]}","{\"data\":[],\"data\":[]}","{\"data\":[{\"id\":\"x\",\"id\":\"y\"}]}","{\"data\":[],}","{\"data\":[,]}","{\"data\":[{\"id\":\"x\"},]}","{\"data\":[]}{}", "{\"data\":[],\"x\":01}","{\"data\":[],\"x\":1.}","{\"data\":[],\"x\":1e}","{\"data\":[],\"x\":-}","{\"data\":[],\"x\":truefalse}","{\"data\":[],\"x\":\"\\x\"}","{\"data\":[],\"x\":\"\\uD800\"}","{\"data\":[],\"x\":\"\\uDC00\"}","{\"data\":[],\"x\":\"\\uD800\\u0041\"}","{\"data\":[],\"x\":\"\\uD800a\"}","{\"data\":[],\"x\":\"\n\"}","{\"data\":[],\"x\":\"\xC0\x80\"}","{\"data\":[],\"x\":\"\xED\xA0\x80\"}","{\"data\":[],\"x\":\"\xF4\x90\x80\x80\"}","{\"data\":[],\"x\":\"\xE2\"}","{\"data\\u0000x\":[]}", "2f\r\n{\"data\":[]}\r\n0\r\n\r\n"};
 for(const auto &body:bad)for(size_t chunk:{1u,7u,512u}){assert(parse(body,out,chunk)!=E_NONE&&out.empty());}
 for(size_t n=0;n<all.size();n++)assert(parse(all.substr(0,n),out,19)!=E_NONE&&out.empty());
 assert(parse(all,out,5,4)==E_BIG&&out.empty());assert(parse(all,out,5,0)==E_BIG);
 assert(parse("{\"data\":[{\"id\":\""+std::string(256,'x')+"\"}]}",out)==E_JSON&&out.empty());
 std::string large="{\"data\":[";
 for(int i=0;i<3500;i++){if(i)large+=",";large+="{\"id\":\"unknown-"+std::to_string(i)+"\",\"metadata\":{\"description\":\""+std::string(40,'a')+"\"}}";}
 large+=",{\"id\":\""+id+"\"}]}";assert(large.size()<MODEL_CATALOG_BODY_MAX);
 assert(parse(large,out,1024)==E_NONE&&out==id+"\n");
 std::string longText="{\"data\":[],\"metadata\":\""+std::string(20000,'x')+"\"}";assert(parse(longText,out,103)==E_NONE&&out.empty());
 std::string tooBig="{\"data\":[],\"metadata\":\""+std::string(MODEL_CATALOG_BODY_MAX,'x')+"\"}";assert(parse(tooBig,out,1024)==E_BIG&&out.empty());
 std::string deep="{\"data\":[],\"x\":"+std::string(24,'[')+"0"+std::string(24,']')+"}";assert(parse(deep,out)==E_BIG&&out.empty());
 printf("PASS streaming account catalog: %d maintained aliases, fixed %zu-byte parser, single-byte/arbitrary chunks, all/mixed/zero matches, ordering/deduplication, JSON/UTF-8/escapes/truncation validation, 3500 unknown models, bounded depth/body/output, stalled-peer overall deadline/wrap and no partial publication.\n",MODEL_COUNT,sizeof(ModelCatalogParser));
}
