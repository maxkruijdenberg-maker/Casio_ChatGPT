/* ChatGPT Atom Lite, ORIGINAL ESP32-PICO-D4. No sleep, no TX inversion.
   ESP32 Arduino core 3.3.0; ArduinoJson 7.4.2. Select M5Atom (ESP32).
   Secrets are entered on the Casio, not in this sketch or USB logs. */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>
#include "wire.h"
#include "ledger.h"
#include "trust.h"
#include "hardware.h"
static const size_t QUESTION_MAX=1024, ANSWER_MAX=8192, HISTORY_MAX=12288, BODY_MAX=32768;
HardwareSerial Calc(1);
Preferences prefs;
SemaphoreHandle_t gate;
static String cfg[5],upload,question,answer;
static String oldQ[2],oldA[2];
static uint8_t historyCount=0,uploadTarget=0,state=S_IDLE,errorCode=0;
static bool contextDropped=false;
static volatile bool cancelled=false;
static RequestLedger ledger={};
static int saveId(const char *name,uint32_t id){return prefs.putUInt(name,id)==4;}
static uint16_t expectedUpload=0;
static WParser parser={};
static WFrame cachedIn={},cachedOut={};
static bool cached=false;
static uint32_t lastByte=0;
struct Lock { Lock(){xSemaphoreTake(gate,portMAX_DELAY);} ~Lock(){xSemaphoreGive(gate);} };
static bool busy(){return state>=S_WIFI&&state<=S_API;}
static void phase(uint8_t s,uint8_t e=0){Lock l;state=s;errorCode=e;}
static void fail(uint8_t e){phase(cancelled?S_CANCELLED:S_ERROR,e);}
static void clearHistory(){for(int i=0;i<2;i++){oldQ[i]="";oldA[i]="";}historyCount=0;contextDropped=false;}
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
static void runApi(void*) {
 String ssid,pass,key,model,q;
 {Lock l;ssid=cfg[0];pass=cfg[1];key=cfg[2];model=cfg[3];q=question;}
 uint8_t resultError=0; bool success=false;
 do {
  if(!ssid.length()||!key.length()||!model.length()){resultError=E_CONFIG;break;}
  WiFi.mode(WIFI_STA);WiFi.begin(ssid.c_str(),pass.c_str());
  uint32_t started=millis();
  while(WiFi.status()!=WL_CONNECTED&&!cancelled&&millis()-started<20000)delay(30);
  if(cancelled)break;
  if(WiFi.status()!=WL_CONNECTED){resultError=E_WIFI;break;}
  phase(S_TIME);configTime(0,0,"pool.ntp.org","time.google.com");started=millis();
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
  for(int i=0;i<historyCount;i++){
   JsonObject u=msgs.add<JsonObject>();u["role"]="user";u["content"]=oldQ[i];
   JsonObject a=msgs.add<JsonObject>();a["role"]="assistant";a["content"]=oldA[i];
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
  const char *text=response["choices"][0]["message"]["content"];
  const char *finish=response["choices"][0]["finish_reason"]|"";
  if(!text||!*text){resultError=E_JSON;break;}
  if(strlen(text)>ANSWER_MAX){resultError=E_BIG;break;}
  String a=text;if(a.length()!=strlen(text)){resultError=E_MEMORY;break;}
  if(!strcmp(finish,"length")){
   const char *note="\n[Antwoord afgebroken door tokenlimiet.]";
   if(a.length()+strlen(note)>ANSWER_MAX){resultError=E_BIG;break;}a+=note;
  }
  {Lock l;
   if(cancelled)break;
   size_t retained=q.length()+a.length();
   for(int i=0;i<historyCount;i++)retained+=oldQ[i].length()+oldA[i].length();
   while(historyCount&&(historyCount>=2||retained>HISTORY_MAX)){
    retained-=oldQ[0].length()+oldA[0].length();oldQ[0]=oldQ[1];oldA[0]=oldA[1];
    oldQ[1]="";oldA[1]="";historyCount--;contextDropped=true;
   }
   if(retained<=HISTORY_MAX){oldQ[historyCount]=q;oldA[historyCount]=a;historyCount++;}
   answer=a;success=true;state=S_READY;errorCode=0;
  }
 }while(false);
 if(!success){Lock l;
  if(cancelled){state=S_CANCELLED;errorCode=0;}
  else {state=S_ERROR;errorCode=resultError?resultError:E_REQUEST;}
 }
 key="";pass="";vTaskDelete(nullptr);
}
static bool sameFrame(const WFrame&a,const WFrame&b){return a.type==b.type&&a.seq==b.seq&&a.len==b.len&&!memcmp(a.data,b.data,a.len);}
static void sendFrame(const WFrame& f){uint8_t out[WIRE_ENCODED];size_t n=w_encode(&f,out);Calc.write(out,n);}
static void handle(const WFrame& in) {
 if(in.type&0x80)return;
 if(cached&&sameFrame(in,cachedIn)){sendFrame(cachedOut);return;}
 WFrame out={};out.type=in.type|0x80;out.seq=in.seq;out.len=1;out.data[0]=E_NONE;
 bool start=false;
 {Lock l;
 switch(in.type){
 case W_HELLO:
  if(in.len){out.data[0]=E_REQUEST;break;}
  out.len=5;out.data[1]=1;out.data[2]=state;w_put16(out.data+3,ANSWER_MAX);break;
 case W_ALLOC:
  if(in.len){out.data[0]=E_REQUEST;break;}
  {int g=g_allocate(&ledger,busy(),saveId);
   if(g!=G_NEW){out.data[0]=g==G_BUSY?E_BUSY:E_MEMORY;break;}}
  out.len=5;w_put32(out.data+1,ledger.active);break;
 case W_BEGIN:
  if(in.len!=3||in.data[0]>4){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  uploadTarget=in.data[0];expectedUpload=w_get16(in.data+1);upload="";
  {size_t cap=uploadTarget==0?QUESTION_MAX:uploadTarget==1?32:uploadTarget==2?63:uploadTarget==3?256:80;
   if(expectedUpload>cap||!upload.reserve(expectedUpload)){out.data[0]=E_BIG;expectedUpload=0;}}
  break;
 case W_APPEND:
  if(busy()){out.data[0]=E_BUSY;break;}
  if(in.len<3||w_get16(in.data)!=upload.length()||in.len-2>expectedUpload-upload.length()){
   out.data[0]=E_UPLOAD;break;
  }
  if(memchr(in.data+2,0,in.len-2)||!upload.concat((const char*)in.data+2,in.len-2))out.data[0]=E_UPLOAD;
  break;
 case W_APPLY:
  if(in.len||busy()){out.data[0]=busy()?E_BUSY:E_REQUEST;break;}
  if(upload.length()!=expectedUpload){out.data[0]=E_UPLOAD;break;}
  if(uploadTarget==0){question=upload;}
  else {
   int idx=uploadTarget-1;
   if(idx==3){bool good=upload.length()>0;for(size_t i=0;i<upload.length();i++)if(!isalnum((unsigned char)upload[i])&&upload[i]!='-'&&upload[i]!='_'&&upload[i]!='.')good=false;
    if(!good){out.data[0]=E_MODEL;break;}}
   const char *names[]={"ssid","pass","key","model"};
   if(prefs.putString(names[idx],upload)!=upload.length()){out.data[0]=E_MEMORY;break;}
   cfg[idx]=upload;clearHistory();
  }
  upload="";expectedUpload=0;break;
 case W_ASK:
  if(in.len!=4){out.data[0]=E_REQUEST;break;}
  if(!question.length()&&w_get32(in.data)!=ledger.last){out.data[0]=E_REQUEST;break;}
  {int g=g_admit(&ledger,w_get32(in.data),busy(),saveId);
   if(g==G_REPLAY){out.len=2;out.data[1]=state;break;}
   if(g!=G_NEW){out.data[0]=g==G_BUSY?E_BUSY:g==G_STORE?E_MEMORY:E_REQUEST;break;}}
  answer="";cancelled=false;state=S_WIFI;errorCode=0;start=true;break;
 case W_STATUS:
  if(in.len){out.data[0]=E_REQUEST;break;}
  out.len=10;out.data[1]=state;out.data[2]=errorCode;w_put16(out.data+3,answer.length());
  w_put32(out.data+6,ledger.last);
  out.data[5]=historyCount|(contextDropped?0x80:0);break;
 case W_READ:
  if(in.len!=2){out.data[0]=E_REQUEST;break;}
  if(state!=S_READY){out.data[0]=E_BUSY;break;}
  {size_t at=w_get16(in.data);if(at>answer.length()){out.data[0]=E_REQUEST;break;}
   size_t n=min((size_t)WIRE_MAX-1,answer.length()-at);out.len=n+1;memcpy(out.data+1,answer.c_str()+at,n);}
  break;
 case W_CANCEL:
  if(in.len){out.data[0]=E_REQUEST;break;}
  if(busy())cancelled=true;break;
 case W_NEW:
  if(in.len){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  clearHistory();question="";answer="";state=S_IDLE;errorCode=0;break;
 case W_INFO:
  if(in.len){out.data[0]=E_REQUEST;break;}
  out.len=5;for(int i=0;i<4;i++)out.data[i+1]=cfg[i].length()>0;break;
 case W_WIPE:
  if(in.len!=4||memcmp(in.data,"WIPE",4)){out.data[0]=E_REQUEST;break;}
  if(busy()){out.data[0]=E_BUSY;break;}
  /* Keep request ledger so erasing settings cannot make old POSTs replayable. */
  {const char *names[]={"ssid","pass","key","model"};for(int i=0;i<4;i++){prefs.remove(names[i]);cfg[i]="";}}
  cfg[3]="gpt-4.1-mini";clearHistory();question="";answer="";state=S_IDLE;break;
 default:out.data[0]=E_REQUEST;
 }
 }
 cachedIn=in;cachedOut=out;cached=true;sendFrame(out);
 if(start&&xTaskCreate(runApi,"openai",16384,nullptr,1,nullptr)!=pdPASS)fail(E_MEMORY);
}
void setup(){
 /* USB logging deliberately disabled. GPIO26 stays UART idle-high while active. */
 pinMode(TX_PIN,INPUT);gate=xSemaphoreCreateMutex();if(!gate)return;
 prefs.begin("casiochat",false);cfg[0]=prefs.getString("ssid","");cfg[1]=prefs.getString("pass","");
 cfg[2]=prefs.getString("key","");cfg[3]=prefs.getString("model","gpt-4.1-mini");
 ledger.next=prefs.getUInt("next",0);ledger.last=prefs.getUInt("last",0);
 if(ledger.last){state=S_ERROR;errorCode=E_RESET;}
 Calc.setRxBufferSize(512);Calc.begin(9600,SERIAL_8N1,RX_PIN,TX_PIN,false);
}
void loop(){
 if(!gate){delay(1000);return;}
 if(parser.n&&millis()-lastByte>500)memset(&parser,0,sizeof(parser));
 while(Calc.available()){
  WFrame f;uint8_t c=Calc.read();lastByte=millis();if(w_feed(&parser,c,&f)==1)handle(f);
 }
 delay(1);
}
