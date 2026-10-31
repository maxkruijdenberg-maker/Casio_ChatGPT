#ifndef CHATGPT_CATALOG_DEADLINE_H
#define CHATGPT_CATALOG_DEADLINE_H
#include <stdint.h>
#include <stddef.h>
/* HTTPClient can keep polling a connected peer after a zero-byte read. Enforce
 * the catalog's overall deadline at the client boundary, including when the
 * output sink is not called. Per-read socket timeouts remain necessary too. */
template<class Client,class Clock> class ModelCatalogDeadlineClient:public Client {
 Clock clock;uint32_t started;
 bool expired(){
  if(timedOut)return true;
  if((uint32_t)(clock()-started)<60000)return false;
  timedOut=true;Client::stop();return true;
 }
 public:
 bool timedOut=false;
 ModelCatalogDeadlineClient(Clock now,uint32_t at):clock(now),started(at){}
 int available() override{return expired()?0:Client::available();}
 uint8_t connected() override{return expired()?0:Client::connected();}
 int read() override{return expired()?-1:Client::read();}
 int read(uint8_t *out,size_t n) override{return expired()?-1:Client::read(out,n);}
};
#endif
