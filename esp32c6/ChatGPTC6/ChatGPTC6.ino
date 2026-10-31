/* ChatGPT V40.1 Rev E ESP32-C6-MINI-1-H4. No sleep. TX inversion for Q2/Q3.
   ESP32 Arduino core 3.3.0; ArduinoJson 7.4.2. Select ESP32C6 Dev Module, 4MB, Huge APP, USB CDC off.
   Secrets are entered on the Casio or temporary phone setup page; never USB logs. */
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>
#include <atomic>
#include <esp_random.h>
#include "upload.h"
#include "config_validation.h"
#include "api_response.h"
#include "wifi_check.h"
#include "connection_check.h"
#include "idle_power.h"
#include "context.h"
#include "wire.h"
#include "ledger.h"
#include "trust.h"
#include "hardware.h"
#include "model_catalog_stream.h"
#include "network_operation.h"
#include "async_lookup.h"
#include <lwip/dns.h>
#include <lwip/tcpip.h>
static const size_t QUESTION_MAX=1024, ANSWER_MAX=8192, HISTORY_MAX=12288, BODY_MAX=32768;
HardwareSerial Calc(1);
Preferences prefs;
SemaphoreHandle_t gate;
static String cfg[4],question,answer;
static Upload upload={};
static ChatContext<String> context;
static uint8_t state=S_IDLE,errorCode=0;
static bool wifiDirty=true;
static std::atomic_bool cancelled{false},catalogCancelled{false};
static bool apiRunning=false;
static RequestLedger ledger={};
static int saveId(const char *name,uint32_t id){return prefs.putUInt(name,id)==4;}
static WParser parser={};
static WFrame cachedIn={},cachedOut={};
static bool cached=false;
static uint32_t lastByte=0,bootId=0;
static IdlePower idlePower;
static uint32_t activeCpuMHz=0;
struct Lock { Lock(){xSemaphoreTake(gate,portMAX_DELAY);} ~Lock(){xSemaphoreGive(gate);} };
static ModelReceipt modelReceipt;
static ActiveModel activeModel;
static RequestLedger diagnosticLedger={};
static int saveDiagnosticId(const char *name,uint32_t id){return prefs.putUInt(!strcmp(name,"next")?"checknext":"checklast",id)==4;}
static std::atomic_bool diagnosticCancelled{false};
static bool diagnosticRunning=false,diagnosticFull=false;
static uint8_t diagnosticAttempted=0,diagnosticInternet=0,diagnosticAccount=0,diagnosticModel=0;
static uint8_t diagnosticState=0,diagnosticQuality=0,diagnosticError=0;
static bool catalogRunning=false;
static uint8_t catalogState=0,catalogError=0;
static String catalog;
static bool busy(){return diagnosticRunning||apiRunning||catalogRunning||(state>=S_WIFI&&state<=S_API);}
static void phase(uint8_t s,uint8_t e=0){Lock l;state=s;errorCode=e;}
static void fail(uint8_t e){phase(cancelled?S_CANCELLED:S_ERROR,e);}
static void clearHistory(){context.clear();}
static void networkPause(){delay(1);}
using NetworkWork=NetworkOperation<decltype(&millis),std::atomic_bool>;
static CheckHttpResult diagnosticHttp(NetworkWork &work,const String &key,const String &model,bool post);
static bool resolveApi(NetworkWork &work,IPAddress &out);
/* Only api.openai.com is looked up. This storage outlives any cancelled task,
 * so an eventual lwIP callback can never access a dead worker's stack. */
static AsyncLookup<ip_addr_t> apiLookup;
static void dnsFinished(const char*,const ip_addr_t *address,void*){apiLookup.complete(address);}
static void startDns(void*){
 ip_addr_t address;err_t status=dns_gethostbyname("api.openai.com",&address,dnsFinished,nullptr);
 if(status==ERR_OK)apiLookup.complete(&address);else if(status!=ERR_INPROGRESS)apiLookup.complete(nullptr);
}
static bool resolveApi(NetworkWork &work,IPAddress &out){
 if(apiLookup.begin()&&tcpip_try_callback(startDns,nullptr)!=ERR_OK)apiLookup.complete(nullptr);
 uint32_t started=millis();
 while(!apiLookup.ready()&&!work.stopped()&&(uint32_t)(millis()-started)<5000)delay(10);
 if(work.stopped())return false;
 ip_addr_t address;if(!apiLookup.result(address)||!IP_IS_V4(&address))return false;
 out=IPAddress(ip4_addr_get_u32(ip_2_ip4(&address)));return true;
}
class ApiNetworkClient:public GuardedNetworkClient<WiFiClientSecure,NetworkWork,IPAddress>{
 using Guard=GuardedNetworkClient<WiFiClientSecure,NetworkWork,IPAddress>;
 public:
 bool dnsFailed=false;
 explicit ApiNetworkClient(NetworkWork &work):Guard(work){}
 using Guard::connect;
 int connect(const char *host,uint16_t port)override{return connect(host,port,3000);}
 int connect(const char *host,uint16_t port,int32_t requested)override{
  if(blocked())return 0;
  IPAddress address;
  if(strcmp(host,"api.openai.com")||!resolveApi(operation,address)){dnsFailed=!operation.stopped();blocked();return 0;}
  if(blocked())return 0;
  int timeout=connectTimeout(requested);if(!timeout)return 0;
  WiFiClientSecure::setTimeout((unsigned long)timeout);
  WiFiClientSecure::setConnectionTimeout((uint32_t)timeout);
  WiFiClientSecure::setHandshakeTimeout((timeout+999)/1000);
  // Connect by resolved IP while retaining hostname SNI and CA validation.
  int result=WiFiClientSecure::connect(address,port,host,ROOT_CA,nullptr,nullptr);
  return finishConnect(result);
 }
};
class LimitedBody:public Stream{
 NetworkWork &work;
 public:String body;bool overflow=false;
 explicit LimitedBody(NetworkWork &operation):work(operation){body.reserve(BODY_MAX);}
 size_t write(uint8_t x)override{return write(&x,1);}
 size_t write(const uint8_t *p,size_t n)override{
  if(work.stopped())return 0;
  if(n>BODY_MAX-body.length()){overflow=true;return 0;}
  if(!body.concat((const char*)p,n)){overflow=true;return 0;}
  return work.stopped()?0:n;
 }
 int available()override{return 0;}int read()override{return -1;}int peek()override{return -1;}
};
/* HTTPClient decodes chunked transfer encoding before calling this sink. */
class ModelCatalogBody:public Stream{
 NetworkWork &work;
 public:ModelCatalogParser parser;
 explicit ModelCatalogBody(NetworkWork &operation):work(operation){}
 size_t write(uint8_t x)override{return write(&x,1);}
 size_t write(const uint8_t *p,size_t n)override{return work.stopped()?0:parser.write(p,n);}
 int available()override{return 0;}int read()override{return -1;}int peek()override{return -1;}
};
static uint8_t httpError(int code){
 if(code==401||code==403)return E_AUTH;if(code==429)return E_LIMIT;
 if(code==404)return E_MODEL;if(code==HTTPC_ERROR_READ_TIMEOUT)return E_TIMEOUT;
 if(code==HTTPC_ERROR_CONNECTION_LOST)return E_INTERNET;
 if(code<=0)return E_TLS;return E_HTTP;
}
static void performApi(){
 NetworkWork work(millis,cancelled,120000,networkPause);
 uint32_t proofEpoch;{Lock l;proofEpoch=activeModel.epoch;}
 String resultQuestion,resultAnswer,resultRequested,resultReported;uint8_t resultError=E_NONE;bool success=false;
 { // Destroy HTTP/TLS/JSON and credentials before publishing a terminal state.
 String ssid,pass,key,model,q;bool resetWiFi;
 {Lock l;ssid=cfg[0];pass=cfg[1];key=cfg[2];model=cfg[3];q=question;resetWiFi=wifiDirty;wifiDirty=false;}
 do{
  if(work.stopped())break;
  if(!ssid.length()||!key.length()||!model.length()){resultError=E_CONFIG;break;}
  WiFi.mode(WIFI_STA);
  if(resetWiFi||WiFi.status()!=WL_CONNECTED||WiFi.SSID()!=ssid){WiFi.disconnect(false,false);WiFi.begin(ssid.c_str(),pass.c_str());}
  uint32_t started=millis();
  while(WiFi.status()!=WL_CONNECTED&&!work.stopped()&&(uint32_t)(millis()-started)<20000)delay(30);
  if(work.stopped())break;
  if(WiFi.status()!=WL_CONNECTED){wl_status_t ws=WiFi.status();resultError=ws==WL_NO_SSID_AVAIL?E_WIFI_NOT_FOUND:ws==WL_CONNECT_FAILED?E_WIFI_AUTH:E_WIFI;break;}
  phase(S_TIME);if(time(nullptr)<1767225600)configTime(0,0,"pool.ntp.org","time.google.com");started=millis();
  while(time(nullptr)<1767225600&&!work.stopped()&&(uint32_t)(millis()-started)<15000)delay(30);
  if(work.stopped())break;
  if(time(nullptr)<1767225600){resultError=E_TIME;break;}
  ApiNetworkClient tls(work);tls.setCACert(ROOT_CA);tls.setHandshakeTimeout(3);tls.setTimeout(5000);
  HTTPClient http;http.setConnectTimeout(3000);http.setTimeout(60000);http.setReuse(false);
  /* Validate configured model before paid work. Neither GET nor POST is retried. */
  phase(S_MODEL);
  if(!http.begin(tls,"https://api.openai.com/v1/models/"+model)){resultError=E_TLS;break;}
  http.addHeader("Authorization","Bearer "+key);
  int code=http.GET();http.end();tls.stop();
  if(work.stopped())break;
  if(code!=200){resultError=tls.dnsFailed?E_INTERNET:httpError(code);break;}
  JsonDocument req;req["model"]=model;req["max_completion_tokens"]=1200;req["stream"]=false;
  JsonArray msgs=req["messages"].to<JsonArray>();
  JsonObject system=msgs.add<JsonObject>();system["role"]="system";
  system["content"]=String("De applicatie vraagt in dit verzoek model ")+model+" aan. Als de gebruiker naar het gebruikte model vraagt, noem dit als de aangevraagde model-ID; de API-modelbevestiging is apart te controleren via Instellingen, Hulp, Actief model. "+"Antwoord standaard in het Nederlands. Gebruik platte tekst en lineaire formules, zonder LaTeX, tabellen, emoji of afsluitende decoraties. Wees beknopt: maximaal ongeveer 1800 tekens. Dit scherm is klein.";
  for(int i=0;i<context.count;i++){
   JsonObject u=msgs.add<JsonObject>();u["role"]="user";u["content"]=context.q[i];
   JsonObject a=msgs.add<JsonObject>();a["role"]="assistant";a["content"]=context.a[i];
  }
  JsonObject user=msgs.add<JsonObject>();user["role"]="user";user["content"]=q;
  String body;serializeJson(req,body);if(req.overflowed()||!body.length()){resultError=E_MEMORY;break;}
  req.clear();
  if(work.stopped())break;phase(S_API);
  if(!http.begin(tls,"https://api.openai.com/v1/chat/completions")){resultError=E_TLS;break;}
  http.addHeader("Authorization","Bearer "+key);http.addHeader("Content-Type","application/json");
  /* Exactly one POST per admitted ledger ID. A cancelled POST can still bill. */
  code=http.POST(body);body="";
  if(work.stopped()){http.end();break;}
  if(code!=200){http.end();resultError=tls.dnsFailed?E_INTERNET:httpError(code);break;}
  int total=http.getSize();if(total>(int)BODY_MAX){http.end();resultError=E_BIG;break;}
  LimitedBody incoming(work);int copied=http.writeToStream(&incoming);http.end();
  if(work.stopped())break;
  if(incoming.overflow){resultError=E_BIG;break;}
  if(copied<0||(total>=0&&copied!=total)){resultError=E_TIMEOUT;break;}
  JsonDocument response;
  DeserializationError parsed=deserializeJson(response,incoming.body);incoming.body="";
  if(parsed||response.overflowed()){resultError=parsed==DeserializationError::NoMemory?E_MEMORY:E_JSON;break;}
  CompletionView view={};resultError=completion_view(response.as<JsonVariantConst>(),ANSWER_MAX,&view);
  if(resultError)break;
  String a;if(!a.concat(view.text,view.size)){resultError=E_MEMORY;break;}
  if(view.truncated){
   const char *note="\n[Antwoord afgebroken door tokenlimiet.]";
   if(a.length()+strlen(note)>ANSWER_MAX){resultError=E_BIG;break;}
   if(!a.concat(note)){resultError=E_MEMORY;break;}
  }
  resultRequested=model;resultReported=completion_model(response.as<JsonVariantConst>());
  resultQuestion=std::move(q);resultAnswer=std::move(a);success=true;
 }while(false);
 }
 {Lock l;
  // This check shares the lock with W_CANCEL: late data cannot undo cancellation.
  if(work.stopped()){
   if(work.wasCancelled){state=S_CANCELLED;errorCode=0;}
   else{state=S_ERROR;errorCode=E_TIMEOUT;}
  }else if(success){
   if(!context.append(resultQuestion,resultAnswer)){clearHistory();context.dropped=true;}
   receipt_publish(&modelReceipt,ledger.last,resultRequested.c_str(),resultReported.c_str());
   active_publish(&activeModel,proofEpoch,resultRequested.c_str(),resultReported.c_str());
   answer=std::move(resultAnswer);state=S_READY;errorCode=0;
  }else{state=S_ERROR;errorCode=resultError?resultError:E_REQUEST;}
  apiRunning=false;
 }
}
static void runApi(void*){performApi();vTaskDelete(nullptr);}

/* Full diagnosis owns a separate durable request ledger. Tiny completions
 * never enter chat history or replace the latest chat answer/receipt. */
static CheckHttpResult diagnosticHttp(NetworkWork &work,const String &key,const String &model,bool post){
 CheckHttpResult result;
 if(work.stopped())return result;
 ApiNetworkClient tls(work);tls.setCACert(ROOT_CA);tls.setHandshakeTimeout(3);tls.setTimeout(5000);
 HTTPClient http;http.setConnectTimeout(3000);http.setTimeout(15000);http.setReuse(false);
 String body;
 if(post){JsonDocument req;req["model"]=model;req["max_completion_tokens"]=16;req["stream"]=false;req["store"]=false;
  JsonObject msg=req["messages"].to<JsonArray>().add<JsonObject>();msg["role"]="user";msg["content"]="Reply OK.";
  if(!serializeJson(req,body)||req.overflowed()){result.error=E_MEMORY;return result;}}
 if(work.stopped())return result;
 if(!http.begin(tls,post?String("https://api.openai.com/v1/chat/completions"):String("https://api.openai.com/v1/models/")+model)){result.error=E_TLS;return result;}
 http.addHeader("Authorization","Bearer "+key);if(post)http.addHeader("Content-Type","application/json");
 if(work.stopped()){http.end();return result;}
 result.status=post?http.POST(body):http.GET();body="";
 if(work.stopped()){http.end();return result;}
 if(result.status==200&&!post){http.end();result.valid=true;return result;}
 int total=http.getSize();if(total>(int)BODY_MAX){http.end();result.error=E_BIG;return result;}
 LimitedBody incoming(work);int copied=http.writeToStream(&incoming);http.end();
 if(work.stopped())return result;
 if(incoming.overflow){result.error=E_BIG;return result;}
 if(copied<0||(total>=0&&copied!=total)){result.error=E_TIMEOUT;return result;}
 JsonDocument response;auto parsed=deserializeJson(response,incoming.body);incoming.body="";
 return completion_check_response(result.status,response.as<JsonVariantConst>(),!parsed&&!response.overflowed());
}
static void performWifiDiagnostic(){
 String ssid,pass,key,model;bool dirty,full;uint32_t epoch;
 {Lock l;ssid=cfg[0];pass=cfg[1];key=cfg[2];model=cfg[3];dirty=wifiDirty;wifiDirty=false;full=diagnosticFull;epoch=activeModel.epoch;}
 struct Hotspot {
  String &ssid,&pass;
  bool ready(){return WiFi.status()==WL_CONNECTED&&WiFi.SSID()==ssid;}
  void begin(){WiFi.mode(WIFI_STA);WiFi.disconnect(false,false);WiFi.begin(ssid.c_str(),pass.c_str());}
  int rssi(){return WiFi.RSSI();}
  uint8_t error(){wl_status_t s=WiFi.status();return s==WL_NO_SSID_AVAIL?E_WIFI_NOT_FOUND:s==WL_CONNECT_FAILED?E_WIFI_AUTH:E_WIFI;}
 } hotspot{ssid,pass};
 NetworkWork work(millis,diagnosticCancelled,90000,networkPause);
 WifiCheckResult result=check_wifi(hotspot,ssid.length()>0,dirty,millis,diagnosticCancelled,[](){delay(30);});
 ConnectionResult check;
 if(full&&!result.error&&!result.cancelled&&!work.stopped()){
  if(time(nullptr)<1767225600)configTime(0,0,"pool.ntp.org","time.google.com");uint32_t at=millis();
  while(time(nullptr)<1767225600&&!work.stopped()&&(uint32_t)(millis()-at)<15000)delay(30);
  if(time(nullptr)<1767225600)check.error=E_TIME;
  else {check=check_connection(work,model.c_str(),[&](){return diagnosticHttp(work,key,model,false);},[&](){if(!key.length()){CheckHttpResult missing;missing.error=E_CONFIG;return missing;}return diagnosticHttp(work,key,model,true);});if(!key.length())check.error=E_CONFIG;}
 }
 {Lock l;
  diagnosticAttempted=CHECK_WIFI;diagnosticInternet=diagnosticAccount=diagnosticModel=0;
  if(diagnosticCancelled||result.cancelled||work.wasCancelled){diagnosticState=4;diagnosticQuality=0;diagnosticError=0;diagnosticAttempted=0;}
  else if(work.timedOut){diagnosticState=3;diagnosticQuality=result.quality;diagnosticError=E_TIMEOUT;}
  else{diagnosticError=result.error?result.error:check.error;diagnosticState=diagnosticError?3:2;diagnosticQuality=result.quality;
   diagnosticAttempted|=check.attempted;diagnosticInternet=check.internet;diagnosticAccount=check.account;diagnosticModel=check.model;
   if(full&&(check.attempted&CHECK_MODEL))active_publish(&activeModel,epoch,model.c_str(),check.reported);
  }
  diagnosticRunning=false;
 }
}
static void runWifiDiagnostic(void*){performWifiDiagnostic();vTaskDelete(nullptr);}

/* Model discovery is free GET work on a separate task, never a completion. */
static void performCatalog(){
 NetworkWork work(millis,catalogCancelled,60000,networkPause);
 uint8_t error=E_NONE;String result;
 {
 String ssid,pass,key;bool dirty;
 {Lock l;ssid=cfg[0];pass=cfg[1];key=cfg[2];dirty=wifiDirty;wifiDirty=false;}
 do{
  if(work.stopped())break;
  if(!ssid.length()||!key.length()){error=E_CONFIG;break;}
  WiFi.mode(WIFI_STA);
  if(dirty||WiFi.status()!=WL_CONNECTED||WiFi.SSID()!=ssid){WiFi.disconnect(false,false);WiFi.begin(ssid.c_str(),pass.c_str());}
  uint32_t at=millis();while(WiFi.status()!=WL_CONNECTED&&!work.stopped()&&(uint32_t)(millis()-at)<20000)delay(30);
  if(work.stopped())break;
  if(WiFi.status()!=WL_CONNECTED){error=E_WIFI;break;}
  if(time(nullptr)<1767225600)configTime(0,0,"pool.ntp.org","time.google.com");
  at=millis();while(time(nullptr)<1767225600&&!work.stopped()&&(uint32_t)(millis()-at)<15000)delay(30);
  if(work.stopped())break;
  if(time(nullptr)<1767225600){error=E_TIME;break;}
  ApiNetworkClient tls(work);tls.setCACert(ROOT_CA);tls.setHandshakeTimeout(3);tls.setTimeout(5000);
  HTTPClient http;http.setConnectTimeout(3000);http.setTimeout(5000);http.setReuse(false);
  if(work.stopped())break;
  if(!http.begin(tls,"https://api.openai.com/v1/models")){error=E_TLS;break;}
  http.addHeader("Authorization","Bearer "+key);
  int code=http.GET();
  if(work.stopped()){http.end();break;}
  if(code!=200){http.end();error=tls.dnsFailed?E_INTERNET:httpError(code);break;}
  int total=http.getSize();if(total>(int)MODEL_CATALOG_BODY_MAX){http.end();error=E_BIG;break;}
  ModelCatalogBody incoming(work);int copied=http.writeToStream(&incoming);http.end();
  if(work.stopped())break;
  if(incoming.parser.status()){error=incoming.parser.status();break;}
  if(copied<0||(total>=0&&copied!=total)){error=E_TIMEOUT;break;}
  char found[MODEL_CATALOG_OUTPUT_BYTES];error=incoming.parser.finish(found,sizeof(found));
  if(error)break;
  result=found;if(result.length()!=strlen(found))error=E_MEMORY;
 }while(false);
 } // HTTP/TLS/JSON and credential cleanup completes before busy is released.
 {Lock l;
  if(work.stopped()){
   catalog="";catalogError=work.wasCancelled?E_NONE:E_TIMEOUT;catalogState=work.wasCancelled?4:3;
  }else{
   catalogError=error;catalogState=error?3:2;
   if(error)catalog="";else catalog=std::move(result);
  }
  catalogRunning=false;
 }
}
static void runCatalog(void*){performCatalog();vTaskDelete(nullptr);}

#include "phone_setup.h"

/* Radio off after peer loss; UART remains running for automatic reopening.
 * None of these transitions allocates or sends an API request. */
static void powerActivity(const WFrame &frame){
 if(frame.type<W_HELLO||frame.type>W_ACTIVE)return;
 if(!idlePower.contact(millis()))return;
 if(activeCpuMHz)setCpuFrequencyMhz(activeCpuMHz);
 WiFi.mode(WIFI_STA);
 String ssid,pass;{Lock l;ssid=cfg[0];pass=cfg[1];wifiDirty=true;}
 if(ssid.length()){WiFi.begin(ssid.c_str(),pass.c_str());Lock l;wifiDirty=false;}
}
static void servicePower(){
 {Lock l;if(!idlePower.park(millis(),busy()))return;wifiDirty=true;}
 if(portalActive)stopPortal();
 WiFi.disconnect(false,false);WiFi.mode(WIFI_OFF);
 setCpuFrequencyMhz(80);
}

static bool sameFrame(const WFrame&a,const WFrame&b){return a.type==b.type&&a.seq==b.seq&&a.len==b.len&&!memcmp(a.data,b.data,a.len);}
static void sendFrame(const WFrame& f){uint8_t out[WIRE_ENCODED];size_t n=w_encode(&f,out);Calc.write(out,n);}
static void handle(const WFrame& in) {
 if(in.type&0x80)return;
 if(cached&&sameFrame(in,cachedIn)){sendFrame(cachedOut);return;}
 WFrame out={};out.type=in.type|0x80;out.seq=in.seq;out.len=1;out.data[0]=E_NONE;
 bool start=false,startCatalog=false,startDiagnostic=false;
 {Lock l;
 if(portalActive && in.type!=W_SETUP && in.type!=W_HELLO && in.type!=W_INFO && in.type!=W_DEVICE && in.type!=W_STATUS && in.type!=W_GET_CONFIG){out.data[0]=E_BUSY;}else switch(in.type){
 case W_RECEIPT:receipt_read(&modelReceipt,&in,&out);break;
 case W_ACTIVE:receipt_read(&activeModel.receipt,&in,&out);break;
 case W_CHECK:
  if(!in.len||in.data[0]>3||(in.data[0]==1?in.len!=5:in.len!=1)){out.data[0]=E_REQUEST;break;}
  if(in.data[0]==2){
   int g=g_allocate(&diagnosticLedger,busy(),saveDiagnosticId);
   if(g!=G_NEW){out.data[0]=g==G_BUSY?E_BUSY:E_MEMORY;break;}
   out.len=5;w_put32(out.data+1,diagnosticLedger.active);break;
  }
  if(in.data[0]==1){
   int g=g_admit(&diagnosticLedger,w_get32(in.data+1),busy(),saveDiagnosticId);
   if(g!=G_NEW&&g!=G_REPLAY){out.data[0]=g==G_BUSY?E_BUSY:g==G_STORE?E_MEMORY:E_REQUEST;break;}
   if(g==G_NEW){diagnosticFull=true;diagnosticCancelled=false;diagnosticRunning=true;diagnosticState=1;
    diagnosticQuality=diagnosticError=diagnosticAttempted=diagnosticInternet=diagnosticAccount=diagnosticModel=0;
    active_reset(&activeModel,cfg[3].c_str());startDiagnostic=true;}
  }
  if(in.data[0]==3){diagnosticCancelled=true;diagnosticLedger.allocated=0;if(!diagnosticRunning){diagnosticState=4;diagnosticAttempted=0;}}
  out.len=8;out.data[1]=diagnosticState;out.data[2]=diagnosticQuality;out.data[3]=diagnosticError;out.data[4]=diagnosticAttempted;
  out.data[5]=diagnosticInternet;out.data[6]=diagnosticAccount;out.data[7]=diagnosticModel;break;
 case W_DIAG:
  if(in.len!=1||(in.data[0]!=0&&in.data[0]!=1&&in.data[0]!=3)){out.data[0]=E_REQUEST;break;}
  if(in.data[0]==1){if(busy()){out.data[0]=E_BUSY;break;}diagnosticFull=false;diagnosticCancelled=false;diagnosticRunning=true;diagnosticState=1;diagnosticQuality=diagnosticError=0;startDiagnostic=true;}
  if(in.data[0]==3){diagnosticCancelled=true;if(!diagnosticRunning){diagnosticState=4;diagnosticQuality=diagnosticError=0;}}
  out.len=5;out.data[1]=diagnosticState;out.data[2]=diagnosticQuality;out.data[3]=diagnosticError;out.data[4]=cfg[0].length()>0;break;
 case W_MODELS:
  /* 0 status, 1 refresh, 2 read(offset), 3 cancel; cancellation is idempotent. */
  if(!in.len||in.data[0]>3||(in.data[0]==2?in.len!=3:in.len!=1)){out.data[0]=E_REQUEST;break;}
  if(in.data[0]==1){
   if(busy()){out.data[0]=E_BUSY;break;}
   catalog=String();catalogState=1;catalogError=0;catalogCancelled=false;catalogRunning=true;startCatalog=true;
  }
  if(in.data[0]==3){
   catalogCancelled=true;
   if(!catalogRunning){catalog="";catalogState=4;catalogError=0;}
  }
  if(in.data[0]==2){
   size_t at=w_get16(in.data+1);if(catalogState!=2||at>catalog.length()){out.data[0]=E_REQUEST;break;}
   size_t n=min((size_t)WIRE_MAX-1,catalog.length()-at);out.len=n+1;memcpy(out.data+1,catalog.c_str()+at,n);
  }else {out.len=5;out.data[1]=catalogState;out.data[2]=catalogError;w_put16(out.data+3,catalog.length());}
  break;
 case W_SETUP:
  if(in.len!=1||in.data[0]>2){out.data[0]=E_REQUEST;break;}
  if(in.data[0]==1){if(busy()){out.data[0]=E_BUSY;break;}if(!portalActive&&!startPortal()){out.data[0]=E_WIFI;break;}}
  if(in.data[0]==2)stopPortal();
  out.len=17;out.data[1]=portalStatus;out.data[2]=WiFi.status()==WL_CONNECTED;
  w_put16(out.data+3,portalId);if(portalActive)memcpy(out.data+5,portalPassword.c_str(),12);break;
 case W_HELLO:
  if(in.len){out.data[0]=E_REQUEST;break;}
  out.len=5;out.data[1]=1;out.data[2]=state;w_put16(out.data+3,ANSWER_MAX);break;
 case W_ALLOC:
  if(in.len){out.data[0]=E_REQUEST;break;}
  {int g=g_allocate(&ledger,busy(),saveId);
   if(g!=G_NEW){out.data[0]=g==G_BUSY?E_BUSY:E_MEMORY;break;}}
  out.len=5;w_put32(out.data+1,ledger.active);break;
 case W_BEGIN:
  if(in.len!=3){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  if(in.data[0]>=5&&!context.restoring){out.data[0]=E_REQUEST;break;}
  if(in.data[0]==0&&context.restoring){out.data[0]=E_UPLOAD;break;}
  out.data[0]=upload_begin(&upload,in.data[0],w_get16(in.data+1));break;
 case W_APPEND:
  if(busy()){out.data[0]=E_BUSY;break;}
  if(in.len<3){out.data[0]=E_UPLOAD;break;}
  out.data[0]=upload_append(&upload,w_get16(in.data),in.data+2,in.len-2);break;
 case W_APPLY:
  if(in.len||busy()){out.data[0]=busy()?E_BUSY:E_REQUEST;break;}
  if(!upload_complete(&upload)){out.data[0]=E_UPLOAD;break;}
  if(!setting_valid(upload.target,upload.bytes,upload.used)){out.data[0]=E_CONFIG;upload_clear(&upload);break;}
  if(upload.target==0){
   question=upload.bytes;if(question.length()!=upload.used)out.data[0]=E_MEMORY;
  }else if(upload.target>=5){
   if(!context.restoring||!upload.used){out.data[0]=E_UPLOAD;upload_clear(&upload);break;}
   String restored=upload.bytes;
   if(restored.length()!=upload.used){out.data[0]=E_MEMORY;upload_clear(&upload);break;}
   out.data[0]=upload.target==5?context.restoreQuestion(restored):context.restoreAnswer(restored);
  }else {
   int idx=upload.target-1;
   String next=upload.bytes;if(next.length()!=upload.used){out.data[0]=E_MEMORY;upload_clear(&upload);break;}
   String values[4];for(int i=0;i<4;i++)values[i]=cfg[i];values[idx]=next;
   if(!saveConfiguration(values)){out.data[0]=E_MEMORY;upload_clear(&upload);break;}
   catalog="";catalogState=0;if(idx<2)wifiDirty=true;clearHistory();context.restoring=false;context.pending="";
  }
  upload_clear(&upload);break;
 case W_ASK:
  if(in.len!=4){out.data[0]=E_REQUEST;break;}
  if(context.restoring){out.data[0]=E_UPLOAD;break;}
  if(w_get32(in.data)!=ledger.last&&(!cfg[0].length()||!cfg[2].length()||!cfg[3].length())){out.data[0]=E_CONFIG;break;}
  if(!question.length()&&w_get32(in.data)!=ledger.last){out.data[0]=E_REQUEST;break;}
  {int g=g_admit(&ledger,w_get32(in.data),busy(),saveId);
   if(g==G_REPLAY){out.len=2;out.data[1]=state;break;}
   if(g!=G_NEW){out.data[0]=g==G_BUSY?E_BUSY:g==G_STORE?E_MEMORY:E_REQUEST;break;}}
  receipt_clear(&modelReceipt);active_reset(&activeModel,cfg[3].c_str());answer="";cancelled=false;apiRunning=true;state=S_WIFI;errorCode=0;start=true;break;
 case W_STATUS:
  if(in.len){out.data[0]=E_REQUEST;break;}
  out.len=10;out.data[1]=state;out.data[2]=errorCode;w_put16(out.data+3,answer.length());
  w_put32(out.data+6,ledger.last);
  out.data[5]=context.count|(context.dropped?0x80:0);break;
 case W_READ:
  if(in.len!=2){out.data[0]=E_REQUEST;break;}
  if(state!=S_READY){out.data[0]=E_BUSY;break;}
  {size_t at=w_get16(in.data);if(at>answer.length()){out.data[0]=E_REQUEST;break;}
   size_t n=min((size_t)WIRE_MAX-1,answer.length()-at);out.len=n+1;memcpy(out.data+1,answer.c_str()+at,n);}
  break;
 case W_CANCEL:
  if(in.len){out.data[0]=E_REQUEST;break;}
  upload_clear(&upload);
  if(apiRunning||(state>=S_WIFI&&state<=S_API))cancelled=true;
  if(catalogRunning)catalogCancelled=true;
  if(diagnosticRunning)diagnosticCancelled=true;
  if(!busy()){ledger.allocated=0;if(context.restoring)clearHistory();context.restoring=false;context.pending="";}break;
 case W_NEW:
  if(in.len){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  upload_clear(&upload);context.restoring=false;context.pending="";ledger.allocated=0;clearHistory();receipt_clear(&modelReceipt);question="";answer="";state=S_IDLE;errorCode=0;break;
 case W_INFO:
  if(in.len){out.data[0]=E_REQUEST;break;}
  out.len=5;for(int i=0;i<4;i++)out.data[i+1]=cfg[i].length()>0;break;
 case W_GET_CONFIG:
  if(in.len!=3||(in.data[0]!=1&&in.data[0]!=4)){out.data[0]=E_REQUEST;break;}
  {const String &value=cfg[in.data[0]-1];size_t at=w_get16(in.data+1);
   if(at>value.length()){out.data[0]=E_REQUEST;break;}
   size_t n=min((size_t)WIRE_MAX-2,value.length()-at);out.len=n+2;
   out.data[1]=(uint8_t)value.length();memcpy(out.data+2,value.c_str()+at,n);}
  break;
 case W_DEVICE:
  if(in.len){out.data[0]=E_REQUEST;break;}
  out.len=8;out.data[1]=40;out.data[2]=1;out.data[3]=255;w_put32(out.data+4,bootId);break;
 case W_CONTEXT:
  if(in.len!=1||in.data[0]>1){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  if(in.data[0]==0){
   upload_clear(&upload);context.beginRestore();receipt_clear(&modelReceipt);ledger.allocated=0;question="";answer="";state=S_IDLE;errorCode=0;
  }else {
   out.data[0]=context.finishRestore();if(out.data[0])break;upload_clear(&upload);
  }break;
 case W_WIPE:
  if(in.len!=4||memcmp(in.data,"WIPE",4)){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  /* Keep request ledger so erasing settings cannot make old POSTs replayable. */
  {String empty[4];empty[3]="gpt-4.1-mini";
   if(!saveConfiguration(empty)){out.data[0]=E_MEMORY;break;}
   const char *names[]={"ssid","pass","key","model"};for(int i=0;i<4;i++)prefs.remove(names[i]);}
  catalog="";catalogState=0;upload_clear(&upload);context.restoring=false;context.pending="";wifiDirty=true;ledger.allocated=0;clearHistory();receipt_clear(&modelReceipt);question="";answer="";state=S_IDLE;errorCode=0;break;
 default:out.data[0]=E_REQUEST;
 }
 }
 cachedIn=in;cachedOut=out;cached=true;sendFrame(out);
 if(startDiagnostic&&xTaskCreate(runWifiDiagnostic,"connection-check",16384,nullptr,1,nullptr)!=pdPASS){Lock l;diagnosticRunning=false;diagnosticState=3;diagnosticError=E_MEMORY;}
 if(startCatalog&&xTaskCreate(runCatalog,"models",16384,nullptr,1,nullptr)!=pdPASS){Lock l;catalogRunning=false;catalogState=3;catalogError=E_MEMORY;}
 if(start&&xTaskCreate(runApi,"openai",16384,nullptr,1,nullptr)!=pdPASS){Lock l;apiRunning=false;state=cancelled?S_CANCELLED:S_ERROR;errorCode=cancelled?0:E_MEMORY;}
}
void setup(){
 /* USB logging deliberately disabled. GPIO21 uses TX-only inversion and the gated open-drain output. */
 activeCpuMHz=getCpuFrequencyMhz();idlePower.begin(millis());
 boardInitPins();bootId=esp_random();pinMode(TX_PIN,INPUT);gate=xSemaphoreCreateMutex();if(!gate)return;
 prefs.begin("casiochat",false);cfg[0]=prefs.getString("ssid","");cfg[1]=prefs.getString("pass","");
 cfg[2]=prefs.getString("key","");cfg[3]=prefs.getString("model","gpt-4.1-mini");
 loadConfiguration();active_reset(&activeModel,cfg[3].c_str());
 diagnosticLedger.next=prefs.getUInt("checknext",0);diagnosticLedger.last=prefs.getUInt("checklast",0);
 ledger.next=prefs.getUInt("next",0);ledger.last=prefs.getUInt("last",0);
 if(ledger.last){state=S_ERROR;errorCode=E_RESET;}
 Calc.setRxBufferSize(512);Calc.begin(9600,SERIAL_8N1,RX_PIN,TX_PIN,false);
 boardUartReady=uart_set_line_inverse(UART_NUM_1,UART_SIGNAL_TXD_INV)==ESP_OK;
}
void loop(){
 if(!gate){delay(1000);return;}
 boardServiceLease();servicePortal();
 if(parser.n&&millis()-lastByte>500)memset(&parser,0,sizeof(parser));
 while(Calc.available()){
  WFrame f;uint8_t c=Calc.read();lastByte=millis();if(w_feed(&parser,c,&f)==1){powerActivity(f);if(boardAcceptFrame(f))handle(f);}
 }
 boardServiceLease();servicePower();delay(idlePower.parked?20:1);
}
