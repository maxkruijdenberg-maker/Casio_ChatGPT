#ifndef CHATGPT_NETWORK_OPERATION_H
#define CHATGPT_NETWORK_OPERATION_H
#include <stdint.h>
#include <stddef.h>
/* One worker owns the socket. UART cancellation only sets an atomic flag;
 * this guard closes the socket from that worker, never from another task. */
template<class Clock,class Cancellation> class NetworkOperation {
 Clock clock;const Cancellation &cancellation;uint32_t started,limit;
 void (*pauseFn)();
 public:
 bool timedOut=false,wasCancelled=false;
 NetworkOperation(Clock now,const Cancellation &cancel,uint32_t duration,void (*pause)()=nullptr)
  :clock(now),cancellation(cancel),started((uint32_t)now()),limit(duration),pauseFn(pause){}
 bool stopped(){
  if(cancellation.load())wasCancelled=true;
  if((uint32_t)((uint32_t)clock()-started)>=limit)timedOut=true;
  return timedOut||wasCancelled;
 }
 uint32_t now()const{return (uint32_t)clock();}
 uint32_t remaining(){
  if(stopped())return 0;uint32_t used=(uint32_t)(now()-started);
  if(used>=limit){timedOut=true;return 0;}return limit-used;
 }
 void timeout(){timedOut=true;}
 void pause(){if(pauseFn)pauseFn();}
};
/* Stop finite blocking SDK calls before/after invocation and interrupt all
 * HTTPClient polling loops, including peers that stay connected with read()==0.
 * Header waiting may be longer than one socket operation; total lifetime is
 * still controlled by NetworkOperation. DNS is overridden separately. */
template<class Client,class Operation,class Address> class GuardedNetworkClient:public Client {
 protected:
 Operation &operation;bool closedByGuard=false;
 bool blocked(){
  if(!operation.stopped()){Client::setTimeout(5000);Client::setConnectionTimeout(5000);return false;}
  if(!closedByGuard){closedByGuard=true;Client::setTimeout(0);Client::setConnectionTimeout(0);Client::stop();}
  return true;
 }
 int finishConnect(int result){return blocked()?0:result;}
 int connectTimeout(int32_t requested){
  uint32_t left=operation.remaining(),cap=3000;
  if(requested>0&&(uint32_t)requested<cap)cap=(uint32_t)requested;
  if(left<cap)cap=left;return (int)cap;
 }
 public:
 explicit GuardedNetworkClient(Operation &work):operation(work){}
 int available()override{if(blocked())return 0;int n=Client::available();return blocked()?0:n;}
 uint8_t connected()override{if(blocked())return 0;uint8_t n=Client::connected();return blocked()?0:n;}
 int read()override{uint8_t b=0;return read(&b,1)==1?(int)b:-1;}
 int read(uint8_t *out,size_t n)override{if(blocked())return -1;int got=Client::read(out,n);return blocked()?-1:got;}
 int peek()override{if(blocked())return -1;int got=Client::peek();return blocked()?-1:got;}
 size_t readBytes(char *out,size_t n)override{return readBytes((uint8_t*)out,n);}
 size_t readBytes(uint8_t *out,size_t n)override{
  size_t used=0;uint32_t last=operation.now();
  while(used<n&&!blocked()){
   int got=read(out+used,n-used);
   if(got<=0&&!connected())break;
   if(got>0){used+=(size_t)got;last=operation.now();continue;}
   if((uint32_t)(operation.now()-last)>=5000){operation.timeout();blocked();break;}
   operation.pause();
  }
  return used;
 }
 size_t write(uint8_t b)override{return write(&b,1);}
 size_t write(const uint8_t *data,size_t n)override{
  size_t sent=0;
  while(sent<n&&!blocked()){
   size_t take=n-sent;if(take>256)take=256;
   size_t wrote=Client::write(data+sent,take);
   if(blocked())break;if(wrote>take)wrote=take;sent+=wrote;if(wrote<take)break;
  }return sent;
 }
 int connect(Address address,uint16_t port)override{
  if(blocked())return 0;int timeout=connectTimeout(3000);if(!timeout)return 0;
  Client::setTimeout((unsigned long)timeout);Client::setConnectionTimeout((uint32_t)timeout);return finishConnect(Client::connect(address,port));
 }
 int connect(Address address,uint16_t port,int32_t timeout)override{
  if(blocked())return 0;int bounded=connectTimeout(timeout);if(!bounded)return 0;
  Client::setTimeout((unsigned long)bounded);Client::setConnectionTimeout((uint32_t)bounded);return finishConnect(Client::connect(address,port));
 }
 int connect(const char *host,uint16_t port)override{return connect(host,port,3000);}
 int connect(const char *host,uint16_t port,int32_t timeout)override{
  if(blocked())return 0;int bounded=connectTimeout(timeout);if(!bounded)return 0;
  Client::setTimeout((unsigned long)bounded);Client::setConnectionTimeout((uint32_t)bounded);return finishConnect(Client::connect(host,port));
 }
};
#endif
