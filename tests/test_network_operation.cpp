#include <cassert>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <vector>
#include "../atom/ChatGPTAtom/network_operation.h"
#include "../atom/ChatGPTAtom/async_lookup.h"
static uint32_t tick;
static uint32_t fakeMillis(){return tick;}
static void pauseClock(){tick+=10;}
using Work=NetworkOperation<decltype(&fakeMillis),std::atomic_bool>;
struct Address{unsigned value=0;};
static std::atomic_bool *cancelDuringOperation;
struct Client {
 bool open=true;int stops=0,reads=0,writes=0,connects=0,timeout=0,socketTimeout=30000,lastConnectTimeout=0,peekValue=42;
 int mode=0,position=0;std::vector<uint8_t> output;
 virtual ~Client()=default;
 virtual void stop(){stops++;open=false;}
 void setTimeout(unsigned long ms){timeout=(int)ms;}
 void setConnectionTimeout(uint32_t ms){socketTimeout=(int)ms;}
 virtual int available(){return mode==1?3:0;}
 virtual uint8_t connected(){return open;}
 virtual int peek(){if(mode==4)cancelDuringOperation->store(true);return peekValue;}
 virtual int read(){assert(!"guard must use initialized single-byte wrapper");return 0;}
 virtual int read(uint8_t *out,size_t n){
  reads++;
  if(mode==2&&position<3&&n){out[0]=(uint8_t)('a'+position++);return 1;}
  if(mode==3&&n){out[0]='x';cancelDuringOperation->store(true);return 1;}
  return mode==7?-1:0;
 }
 virtual size_t readBytes(char*,size_t){assert(!"base readBytes must not bypass guard");return 0;}
 virtual size_t readBytes(uint8_t*,size_t){assert(!"base readBytes must not bypass guard");return 0;}
 virtual size_t write(uint8_t){assert(!"single writes must use guarded chunk path");return 0;}
 virtual size_t write(const uint8_t *p,size_t n){
  writes++;output.insert(output.end(),p,p+n);if(mode==5)cancelDuringOperation->store(true);return n;
 }
 virtual int connect(Address,uint16_t){connects++;lastConnectTimeout=socketTimeout;if(mode==6)cancelDuringOperation->store(true);return 1;}
 virtual int connect(Address ip,uint16_t port,int32_t ms){timeout=ms;return connect(ip,port);}
 virtual int connect(const char*,uint16_t){connects++;lastConnectTimeout=socketTimeout;if(mode==6)cancelDuringOperation->store(true);return 1;}
 virtual int connect(const char *host,uint16_t port,int32_t ms){timeout=ms;return connect(host,port);}
};
using Guard=GuardedNetworkClient<Client,Work,Address>;
static void testDeadlines(){
 std::atomic_bool cancel{false};cancelDuringOperation=&cancel;
 tick=100;Work work(fakeMillis,cancel,120000,pauseClock);Guard client(work);int polls=0;
 // Actual HTTPClient loop condition for unknown-length bodies, no sink calls.
 while(client.connected()){assert(client.available()==0);tick+=1000;assert(++polls<=120);}
 assert(work.timedOut&&!work.wasCancelled&&polls==120&&client.stops==1&&client.timeout==0&&client.socketTimeout==0);
 assert(client.read()==-1&&client.peek()==-1&&client.stops==1&&client.reads==0);
 tick=0xfffffff0;Work wrap(fakeMillis,cancel,120000,pauseClock);tick+=119999;assert(!wrap.stopped());tick++;assert(wrap.stopped()&&wrap.timedOut);
 // Known length, repeated zero-byte read: operation exits without a sink write.
 tick=0;Work known(fakeMillis,cancel,120000,pauseClock);Guard stalled(known);uint8_t bytes[100];
 assert(stalled.readBytes(bytes,sizeof(bytes))==0&&known.timedOut&&tick==5000&&stalled.stops==1);
 // Actual secure client reports -1 for no data while still connected.
 tick=0;Work negative(fakeMillis,cancel,120000,pauseClock);Guard noData(negative);noData.mode=7;
 assert(noData.readBytes(bytes,sizeof(bytes))==0&&negative.timedOut&&tick==5000&&noData.stops==1);
 // Positive partial data followed by a connected but stalled peer is not success.
 tick=0;Work partial(fakeMillis,cancel,120000,pauseClock);Guard prefix(partial);prefix.mode=2;
 assert(prefix.readBytes((char*)bytes,sizeof(bytes))==3&&!memcmp(bytes,"abc",3)&&partial.timedOut&&prefix.stops==1);
}
static void testCancellation(){
 std::atomic_bool cancel{false};cancelDuringOperation=&cancel;uint8_t data[1024]={};
 // Cancellation raised while the native read returns data discards that result.
 tick=0;Work readWork(fakeMillis,cancel,120000,pauseClock);Guard reader(readWork);reader.mode=3;
 assert(reader.readBytes(data,1)==0&&readWork.wasCancelled&&reader.stops==1&&reader.timeout==0&&reader.socketTimeout==0);
 // A native write may already have sent a prefix; do not send remaining chunks.
 cancel=false;Work writeWork(fakeMillis,cancel,120000,pauseClock);Guard writer(writeWork);writer.mode=5;
 assert(writer.write(data,sizeof(data))==0&&writer.output.size()==256&&writer.writes==1&&writeWork.wasCancelled);
 assert(writer.write(data,sizeof(data))==0&&writer.writes==1&&writer.stops==1);
 cancel=false;Work peekWork(fakeMillis,cancel,120000,pauseClock);Guard peeker(peekWork);peeker.mode=4;
 assert(peeker.peek()==-1&&peekWork.wasCancelled&&peeker.stops==1);
 cancel=false;Work connectWork(fakeMillis,cancel,120000,pauseClock);Guard connector(connectWork);connector.mode=6;
 assert(!connector.connect("api.openai.com",443,60000)&&connectWork.wasCancelled&&connector.connects==1&&connector.stops==1);
 cancel=false;Work remaining(fakeMillis,cancel,100,pauseClock);Guard bounded(remaining);tick=90;
 assert(bounded.connect(Address{1},443,60000)&&bounded.connects==1); // <= remaining10ms before guarded post-check
 assert(remaining.remaining()==10&&bounded.lastConnectTimeout==10);
 // A cancelled operation can never reconnect or publish a later successful read.
 cancel=true;assert(!bounded.connect("api.openai.com",443)&&bounded.connects==1&&remaining.stopped());
 cancel=false;assert(remaining.stopped()&&remaining.wasCancelled);
 tick=0;Work zero(fakeMillis,cancel,120000,pauseClock);Guard empty(zero);assert(empty.read()==-1);
 // Fresh independent operation after cleanup is usable without resetting the board.
 Work fresh(fakeMillis,cancel,120000,pauseClock);Guard retry(fresh);assert(retry.connect("api.openai.com",443,3000)&&!fresh.stopped());
 uint8_t one=7;assert(retry.write(one)==1&&retry.output.size()==1);
}
static void testPersistentLookup(){
 AsyncLookup<Address> lookup;Address result{99},out;
 assert(lookup.begin()&&!lookup.ready()&&!lookup.result(out));
 // A cancelled worker cannot start an unbounded number of pending callbacks.
 for(int worker=0;worker<1000;worker++)assert(!lookup.begin());
 // A late callback writes persistent storage, not the destroyed worker stack.
 lookup.complete(&result);assert(lookup.ready()&&lookup.result(out)&&out.value==99);
 assert(lookup.begin()&&!lookup.ready());lookup.complete(nullptr);assert(lookup.ready()&&!lookup.result(out));
 assert(lookup.begin());result.value=123;lookup.complete(&result);assert(lookup.result(out)&&out.value==123);
}
int main(){
 testDeadlines();testCancellation();testPersistentLookup();
 puts("PASS network operation guards: actual client read/write/connect/peek overrides, stalled known/unknown bodies, partial data, per-read timeout, 120s total deadline, wraparound, cancellation during native calls, no late resurrection, bounded persistent DNS callback storage and fresh operation recovery.");
}
