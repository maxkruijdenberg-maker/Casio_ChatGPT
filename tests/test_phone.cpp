#include <cassert>
#include <string>
#include <map>
#include <functional>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <ArduinoJson.h>
class String:public std::string{
 public:using std::string::string;using std::string::operator=;
 String()=default;String(const std::string&s):std::string(s){}
 size_t length()const{return size();}bool startsWith(const char*s)const{return rfind(s,0)==0;}
 long toInt()const{try{return std::stol(*this);}catch(...){return 0;}}
};
namespace ArduinoJson {template<>struct Converter<String>{
 static void toJson(const String&s,JsonVariant v){v.set(static_cast<const std::string&>(s));}
 static String fromJson(JsonVariantConst v){return String(v.as<std::string>());}
 static bool checkJson(JsonVariantConst v){return v.is<const char*>();}
};}
static size_t serializeJson(const JsonDocument&doc,String&s){std::string out;size_t n=ArduinoJson::serializeJson(doc,out);s=out;return n;}
static auto deserializeJson(JsonDocument&doc,const String&s){return ArduinoJson::deserializeJson(doc,s.c_str());}
#include "../atom/ChatGPTAtom/config_validation.h"
#include "../atom/ChatGPTAtom/wire.h"
static uint32_t now=0;static uint32_t millis(){return now;}static uint32_t esp_random(){static uint32_t r=12345;return r=r*1664525+1013904223;}
struct FakePrefs{std::map<std::string,String> data;bool fail=false;size_t putString(const char*k,const String&v){if(fail)return 0;data[k]=v;return v.size();}String getString(const char*k,const char*d){return data.count(k)?data[k]:String(d);}}prefs;
enum{WIFI_STA,WIFI_AP,WL_CONNECTED=3};
struct FakeWiFi{int modeValue=WIFI_STA,statusValue=0,begins=0;bool apOK=true;String apName,apPass;void mode(int x){modeValue=x;}void disconnect(bool,bool){statusValue=0;}bool softAP(const char*n,const char*p,int,bool,int){apName=n;apPass=p;return apOK;}void softAPdisconnect(bool){}int softAPIP(){return 1;}int status(){return statusValue;}void begin(const char*,const char*){begins++;}}WiFi;
enum{HTTP_GET,HTTP_POST};
struct WebServer{
 std::map<std::string,String> args,headers;std::map<std::string,std::function<void()>> handlers;
 String body;int routesAdded=0,code=0;bool running=false;
 WebServer(int){}void stop(){running=false;}void begin(){running=true;}
 void collectHeaders(const char**,int){}void sendHeader(const char*,const char*){}
 void send(int c,const char*,const String&b){code=c;body=b;}
 String header(const char*k){return headers[k];}bool hasArg(const char*k){return args.count(k);}
 String arg(const char*k){return args[k];}
 template<class F>void on(const char*p,int,F f){handlers[p]=f;routesAdded++;}template<class F>void onNotFound(F){}
 void handleClient(){}
};
struct DNSServer{void stop(){}void start(int,const char*,int){}void processNextRequest(){}};
static String cfg[4]={"oldwifi","oldpass","sk-old","gpt-4.1-mini"};
struct Lock{};static bool isBusy=false,wifiDirty=false;static bool busy(){return isBusy;}
static int clears=0;static void clearHistory(){clears++;}
struct{bool restoring;String pending;}context;
struct{}upload;static void upload_clear(decltype(upload)*){}
struct{int allocated;}ledger;
#include "../atom/ChatGPTAtom/active_model.h"
static ActiveModel activeModel;
#include "../atom/ChatGPTAtom/phone_setup.h"
static void form(const String&token,const String&key="sk-new-key"){
 setupWeb.args={{"token",token},{"ssid","newwifi"},{"pass","newpassword"},{"key",key}};
 setupWeb.headers["Content-Length"]="180";
}
int main(){
 assert(startPortal());assert(portalActive&&portalStatus==1&&WiFi.modeValue==WIFI_AP);
 assert(portalPassword.length()==12&&portalToken.length()==32);
 String oldPass=portalPassword;setupPage();assert(setupWeb.code==200);
 assert(setupWeb.body.find("sk-old")==std::string::npos&&setupWeb.body.find("oldpass")==std::string::npos);
 form("wrong");setupSave();assert(setupWeb.code==403&&cfg[0]=="oldwifi");
 form(portalToken,"bad");setupSave();assert(setupWeb.code==400&&cfg[2]=="sk-old");
 form(portalToken);setupWeb.headers["Content-Length"]="2049";setupSave();assert(setupWeb.code==403);
 form(portalToken);prefs.fail=true;setupSave();assert(setupWeb.code==503&&cfg[0]=="oldwifi"&&portalStatus==1);
 prefs.fail=false;isBusy=true;setupSave();assert(setupWeb.code==503);isBusy=false;
 setupSave();assert(setupWeb.code==200&&portalSaved&&portalStatus==2&&cfg[2]=="sk-new-key"&&clears==1);
 assert(setupWeb.body.find("sk-new-key")==std::string::npos);
 setupSave();assert(setupWeb.code==403);now=1999;servicePortal();assert(portalActive);
 now=2000;servicePortal();assert(!portalActive&&portalPassword.empty()&&portalToken.empty()&&WiFi.begins==1);
 WiFi.statusValue=WL_CONNECTED;servicePortal();assert(portalStatus==3);
 cfg[0]="corrupt";cfg[2]="corrupt";loadConfiguration();assert(cfg[0]=="newwifi"&&cfg[2]=="sk-new-key");
 assert(startPortal());assert(portalPassword!=oldPass);form(portalToken);setupSave();
 stopPortal();assert(WiFi.begins==2); // EXIT immediately after saving must still connect.
 WiFi.statusValue=0;now+=20001;servicePortal();assert(portalStatus==4);
 assert(startPortal());now+=600001;servicePortal();assert(!portalActive&&portalStatus==0);
 assert(startPortal());stopPortal();assert(!portalActive&&portalStatus==0);
 WiFi.apOK=false;assert(!startPortal());assert(!portalActive&&WiFi.modeValue==WIFI_STA);assert(setupWeb.routesAdded==2);
 puts("PASS phone setup: validation, CSRF, atomic save failure, migration, timeout, close, reconnect; no API requests");
}
