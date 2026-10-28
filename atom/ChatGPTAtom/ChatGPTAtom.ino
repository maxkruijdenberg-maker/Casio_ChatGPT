/* ChatGPT V17.0 Atom Lite, ORIGINAL ESP32-PICO-D4. No sleep, no TX inversion.
   ESP32 Arduino core 3.3.0; ArduinoJson 7.4.2. Select M5Atom (ESP32).
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
#include "context.h"
#include "wire.h"
#include "ledger.h"
#include "trust.h"
#include "hardware.h"
#include "model_catalog.h"
static const size_t QUESTION_MAX=1024, ANSWER_MAX=8192, HISTORY_MAX=12288, BODY_MAX=32768;
HardwareSerial Calc(1);
Preferences prefs;
SemaphoreHandle_t gate;
static String cfg[4],question,answer;
static Upload upload={};
static ChatContext<String> context;
static uint8_t state=S_IDLE,errorCode=0;
static bool wifiDirty=true;
static std::atomic_bool cancelled{false};
static RequestLedger ledger={};
static int saveId(const char *name,uint32_t id){return prefs.putUInt(name,id)==4;}
static WParser parser={};
static WFrame cachedIn={},cachedOut={};
static bool cached=false;
static uint32_t lastByte=0,bootId=0;
struct Lock { Lock(){xSemaphoreTake(gate,portMAX_DELAY);} ~Lock(){xSemaphoreGive(gate);} };
static bool catalogRunning=false;
static uint8_t catalogState=0,catalogError=0;
static String catalog;
static bool busy(){return catalogRunning||(state>=S_WIFI&&state<=S_API);}
static void phase(uint8_t s,uint8_t e=0){Lock l;state=s;errorCode=e;}
static void fail(uint8_t e){phase(cancelled?S_CANCELLED:S_ERROR,e);}
static void clearHistory(){context.clear();}
class LimitedBody: public Stream {
 public: String body;bool overflow=false;
 LimitedBody(){body.reserve(BODY_MAX);}
 size_t write(uint8_t x) override {return write(&x,1);}
 size_t write(const uint8_t *p,size_t n) override {
  if(n>BODY_MAX-body.length()){overflow=true;return 0;}
  if(!body.concat((const char*)p,n)){overflow=true;return 0;}return n;
 }
 int available() override{return 0;}int read() override{return -1;}int peek() override{return -1;}
};
static uint8_t httpError(int code){
 if(code==401||code==403)return E_AUTH;if(code==429)return E_LIMIT;
 if(code==404)return E_MODEL;if(code==HTTPC_ERROR_READ_TIMEOUT)return E_TIMEOUT;
 if(code==HTTPC_ERROR_CONNECTION_LOST)return E_INTERNET;
 if(code<=0)return E_TLS;return E_HTTP;
}
static void performApi() {
 String ssid,pass,key,model,q;bool resetWiFi;
 {Lock l;ssid=cfg[0];pass=cfg[1];key=cfg[2];model=cfg[3];q=question;resetWiFi=wifiDirty;wifiDirty=false;}
 uint8_t resultError=0; bool success=false;
 do {
  if(!ssid.length()||!key.length()||!model.length()){resultError=E_CONFIG;break;}
  WiFi.mode(WIFI_STA);
  if(resetWiFi||WiFi.status()!=WL_CONNECTED||WiFi.SSID()!=ssid){WiFi.disconnect(false,false);WiFi.begin(ssid.c_str(),pass.c_str());}
  uint32_t started=millis();
  while(WiFi.status()!=WL_CONNECTED&&!cancelled&&millis()-started<20000)delay(30);
  if(cancelled)break;
  if(WiFi.status()!=WL_CONNECTED){wl_status_t ws=WiFi.status();resultError=ws==WL_NO_SSID_AVAIL?E_WIFI_NOT_FOUND:ws==WL_CONNECT_FAILED?E_WIFI_AUTH:E_WIFI;break;}
  phase(S_TIME);if(time(nullptr)<1767225600)configTime(0,0,"pool.ntp.org","time.google.com");started=millis();
  while(time(nullptr)<1767225600&&!cancelled&&millis()-started<15000)delay(30);
  if(cancelled)break;
  if(time(nullptr)<1767225600){resultError=E_TIME;break;}
  IPAddress ip;if(!WiFi.hostByName("api.openai.com",ip)){resultError=E_INTERNET;break;}
  WiFiClientSecure tls;tls.setCACert(ROOT_CA);tls.setHandshakeTimeout(12);tls.setTimeout(12000);
  HTTPClient http;http.setConnectTimeout(12000);http.setTimeout(20000);http.setReuse(false);
  /* Validate configured model against the authenticated API before paid work. */
  phase(S_MODEL);
  if(!http.begin(tls,"https://api.openai.com/v1/models/"+model)){resultError=E_TLS;break;}
  http.addHeader("Authorization","Bearer "+key);
  int code=http.GET();http.end();tls.stop();
  if(cancelled)break;
  if(code!=200){resultError=httpError(code);break;}
  JsonDocument req;req["model"]=model;req["max_completion_tokens"]=1200;req["stream"]=false;
  JsonArray msgs=req["messages"].to<JsonArray>();
  JsonObject system=msgs.add<JsonObject>();system["role"]="system";
  system["content"]="Antwoord standaard in het Nederlands. Gebruik platte tekst en lineaire formules, zonder LaTeX of tabellen. Wees beknopt: maximaal ongeveer 1800 tekens. Dit scherm is klein.";
  for(int i=0;i<context.count;i++){
   JsonObject u=msgs.add<JsonObject>();u["role"]="user";u["content"]=context.q[i];
   JsonObject a=msgs.add<JsonObject>();a["role"]="assistant";a["content"]=context.a[i];
  }
  JsonObject user=msgs.add<JsonObject>();user["role"]="user";user["content"]=q;
  String body;serializeJson(req,body);if(req.overflowed()||!body.length()){resultError=E_MEMORY;break;}
  req.clear();
  if(cancelled)break;phase(S_API);
  if(!http.begin(tls,"https://api.openai.com/v1/chat/completions")){resultError=E_TLS;break;}
  http.addHeader("Authorization","Bearer "+key);http.addHeader("Content-Type","application/json");
  /* Never automatically retry POST. Cancellation cannot undo API billing. */
  code=http.POST(body);body="";
  if(cancelled){http.end();break;}
  if(code!=200){http.end();resultError=httpError(code);break;}
  int total=http.getSize();if(total>(int)BODY_MAX){http.end();resultError=E_BIG;break;}
  LimitedBody incoming;int copied=http.writeToStream(&incoming);http.end();
  if(cancelled)break;
  if(incoming.overflow){resultError=E_BIG;break;}
  if(copied<0){resultError=E_TIMEOUT;break;}
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
  {Lock l;
   if(cancelled)break;
   if(!context.append(q,a)){clearHistory();context.dropped=true;}
   answer=std::move(a);success=true;state=S_READY;errorCode=0;
  }
 }while(false);
 if(!success){Lock l;
  if(cancelled){state=S_CANCELLED;errorCode=0;}
  else {state=S_ERROR;errorCode=resultError?resultError:E_REQUEST;}
 }
 /* Returning destroys all String/JSON/HTTP objects. vTaskDelete alone would
  * skip destructors of function-scope Strings and leak memory every question. */
}
static void runApi(void*){performApi();vTaskDelete(nullptr);}

/* Model discovery is free GET work on a separate task, never a completion. */
static void performCatalog(){
 String ssid,pass,key;bool dirty;uint8_t error=E_NONE;String result;
 {Lock l;ssid=cfg[0];pass=cfg[1];key=cfg[2];dirty=wifiDirty;wifiDirty=false;}
 do {
  if(!ssid.length()||!key.length()){error=E_CONFIG;break;}
  WiFi.mode(WIFI_STA);
  if(dirty||WiFi.status()!=WL_CONNECTED||WiFi.SSID()!=ssid){WiFi.disconnect(false,false);WiFi.begin(ssid.c_str(),pass.c_str());}
  uint32_t at=millis();while(WiFi.status()!=WL_CONNECTED&&millis()-at<20000)delay(30);
  if(WiFi.status()!=WL_CONNECTED){error=E_WIFI;break;}
  if(time(nullptr)<1767225600)configTime(0,0,"pool.ntp.org","time.google.com");
  at=millis();while(time(nullptr)<1767225600&&millis()-at<15000)delay(30);
  if(time(nullptr)<1767225600){error=E_TIME;break;}
  WiFiClientSecure tls;tls.setCACert(ROOT_CA);tls.setHandshakeTimeout(12);tls.setTimeout(20000);
  HTTPClient http;http.useHTTP10(true);http.setConnectTimeout(12000);http.setTimeout(20000);http.setReuse(false);
  if(!http.begin(tls,"https://api.openai.com/v1/models")){error=E_TLS;break;}
  http.addHeader("Authorization","Bearer "+key);
  int code=http.GET();if(code!=200){error=httpError(code);http.end();break;}
  if(http.getSize()>524288){error=E_BIG;http.end();break;}
  JsonDocument filter;filter["data"][0]["id"]=true;JsonDocument document;
  auto parsed=deserializeJson(document,http.getStream(),DeserializationOption::Filter(filter),DeserializationOption::NestingLimit(8));
  http.end();
  if(parsed||document.overflowed()){error=parsed==DeserializationError::NoMemory?E_MEMORY:E_JSON;break;}
  char *buffer=(char*)malloc(CATALOG_MAX);if(!buffer){error=E_MEMORY;break;}
  int count=catalog_text(document.as<JsonVariantConst>(),buffer,CATALOG_MAX);
  if(count>0){result=buffer;if(result.length()!=strlen(buffer))error=E_MEMORY;}
  else error=E_MODEL;
  free(buffer);
 }while(false);
 {Lock l;catalogError=error;catalogState=error?3:2;if(!error)catalog=std::move(result);catalogRunning=false;}
}
static void runCatalog(void*){performCatalog();vTaskDelete(nullptr);}

#include "phone_setup.h"

static bool sameFrame(const WFrame&a,const WFrame&b){return a.type==b.type&&a.seq==b.seq&&a.len==b.len&&!memcmp(a.data,b.data,a.len);}
static void sendFrame(const WFrame& f){uint8_t out[WIRE_ENCODED];size_t n=w_encode(&f,out);Calc.write(out,n);}
static void handle(const WFrame& in) {
 if(in.type&0x80)return;
 if(cached&&sameFrame(in,cachedIn)){sendFrame(cachedOut);return;}
 WFrame out={};out.type=in.type|0x80;out.seq=in.seq;out.len=1;out.data[0]=E_NONE;
 bool start=false,startCatalog=false;
 {Lock l;
 if(portalActive && in.type!=W_SETUP && in.type!=W_HELLO && in.type!=W_INFO && in.type!=W_DEVICE && in.type!=W_STATUS && in.type!=W_GET_CONFIG){out.data[0]=E_BUSY;}else switch(in.type){
 case W_MODELS:
  /* action 0 status, 1 refresh, 2 read(offset). Reply status includes byte length. */
  if(!in.len||in.data[0]>2||(in.data[0]==2?in.len!=3:in.len!=1)){out.data[0]=E_REQUEST;break;}
  if(in.data[0]==1){
   if(busy()){out.data[0]=E_BUSY;break;}
   catalogState=1;catalogError=0;catalogRunning=true;startCatalog=true;
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
  answer="";cancelled=false;state=S_WIFI;errorCode=0;start=true;break;
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
  upload_clear(&upload);if(busy())cancelled=true;else {ledger.allocated=0;if(context.restoring)clearHistory();context.restoring=false;context.pending="";}break;
 case W_NEW:
  if(in.len){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  upload_clear(&upload);context.restoring=false;context.pending="";ledger.allocated=0;clearHistory();question="";answer="";state=S_IDLE;errorCode=0;break;
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
  out.len=8;out.data[1]=17;out.data[2]=0;out.data[3]=31;w_put32(out.data+4,bootId);break;
 case W_CONTEXT:
  if(in.len!=1||in.data[0]>1){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  if(in.data[0]==0){
   upload_clear(&upload);context.beginRestore();ledger.allocated=0;question="";answer="";state=S_IDLE;errorCode=0;
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
  catalog="";catalogState=0;upload_clear(&upload);context.restoring=false;context.pending="";wifiDirty=true;ledger.allocated=0;clearHistory();question="";answer="";state=S_IDLE;errorCode=0;break;
 default:out.data[0]=E_REQUEST;
 }
 }
 cachedIn=in;cachedOut=out;cached=true;sendFrame(out);
 if(startCatalog&&xTaskCreate(runCatalog,"models",16384,nullptr,1,nullptr)!=pdPASS){Lock l;catalogRunning=false;catalogState=3;catalogError=E_MEMORY;}
 if(start&&xTaskCreate(runApi,"openai",16384,nullptr,1,nullptr)!=pdPASS)fail(E_MEMORY);
}
void setup(){
 /* USB logging deliberately disabled. GPIO26 stays UART idle-high while active. */
 bootId=esp_random();pinMode(TX_PIN,INPUT);gate=xSemaphoreCreateMutex();if(!gate)return;
 prefs.begin("casiochat",false);cfg[0]=prefs.getString("ssid","");cfg[1]=prefs.getString("pass","");
 cfg[2]=prefs.getString("key","");cfg[3]=prefs.getString("model","gpt-4.1-mini");
 loadConfiguration();
 ledger.next=prefs.getUInt("next",0);ledger.last=prefs.getUInt("last",0);
 if(ledger.last){state=S_ERROR;errorCode=E_RESET;}
 Calc.setRxBufferSize(512);Calc.begin(9600,SERIAL_8N1,RX_PIN,TX_PIN,false);
}
void loop(){
 if(!gate){delay(1000);return;}
 servicePortal();
 if(parser.n&&millis()-lastByte>500)memset(&parser,0,sizeof(parser));
 while(Calc.available()){
  WFrame f;uint8_t c=Calc.read();lastByte=millis();if(w_feed(&parser,c,&f)==1)handle(f);
 }
 delay(1);
}
