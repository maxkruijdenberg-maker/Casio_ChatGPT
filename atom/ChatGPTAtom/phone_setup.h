#ifndef CHATGPT_PHONE_SETUP_H
#define CHATGPT_PHONE_SETUP_H
/* Called only from the UART/web loop. API worker is excluded while AP is active.
 * A single NVS blob commits all settings together; legacy values are migration only. */
static WebServer setupWeb(80);
static DNSServer setupDNS;
static bool portalActive=false,portalSaved=false,portalRoutesReady=false;
static uint8_t portalStatus=0; /* 0 closed, 1 open, 2 saved/connecting, 3 connected, 4 failed */
static uint16_t portalId;
static uint32_t portalOpened,portalCloseAt,portalConnectAt;
static String portalPassword,portalToken;
static bool saveConfiguration(const String values[4]){
 JsonDocument doc;for(int i=0;i<4;i++)doc["v"][i]=values[i];
 String blob;if(!serializeJson(doc,blob)||doc.overflowed())return false;
 if(prefs.putString("config11",blob)!=blob.length())return false;
 for(int i=0;i<4;i++)cfg[i]=values[i];active_reset(&activeModel,cfg[3].c_str());return true;
}
static void loadConfiguration(){
 String blob=prefs.getString("config11","");if(!blob.length())return;
 JsonDocument doc;if(deserializeJson(doc,blob))return;
 String values[4];for(int i=0;i<4;i++){
  if(!doc["v"][i].is<const char*>())return;values[i]=doc["v"][i].as<String>();
  if(!setting_valid(i+1,values[i].c_str(),values[i].length()))return;
 }
 for(int i=0;i<4;i++)cfg[i]=values[i];
}
static String randomSetupText(size_t n){
 const char chars[]="ABCDEFGHJKLMNPQRSTUVWXYZ23456789";String s;s.reserve(n);
 for(size_t i=0;i<n;i++)s+=chars[esp_random()%32];return s;
}
static void stopPortal(){
 if(portalActive){setupWeb.stop();setupDNS.stop();WiFi.softAPdisconnect(true);WiFi.mode(WIFI_STA);}
 if(portalActive&&portalSaved){WiFi.begin(cfg[0].c_str(),cfg[1].c_str());portalConnectAt=millis();}
 portalActive=false;portalPassword="";portalToken="";
 if(!portalSaved)portalStatus=0;
}
static void setupHeaders(){setupWeb.sendHeader("Cache-Control","no-store");setupWeb.sendHeader("X-Content-Type-Options","nosniff");setupWeb.sendHeader("Content-Security-Policy","default-src 'none'; style-src 'unsafe-inline'; form-action 'self'; frame-ancestors 'none'");}
static void setupPage(){
 setupHeaders();
 String html=R"HTML(<!doctype html><html lang="nl"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>ChatGPT instellen</title><style>body{background:#171717;color:#eee;font:16px system-ui;margin:0;padding:24px}main{max-width:440px;margin:auto}h1{font-size:28px}p{color:#bbb;line-height:1.5}label{display:block;margin:20px 0 8px}input,button{box-sizing:border-box;width:100%;padding:14px;border-radius:12px;border:1px solid #555;background:#262626;color:#fff;font:inherit}button{background:#a3ecd5;color:#152820;border:0;margin-top:28px;font-weight:700}small{color:#bbb}</style><main><h1>Welkom bij ChatGPT</h1><p>Verbind je Casio met je eigen wifi. Plak daarna je OpenAI API-sleutel. Je hoeft niets op je rekenmachine over te typen.</p><form method="post" action="/save"><input type="hidden" name="token" value=")HTML";
 html+=portalToken;
 html+=R"HTML("><label for="ssid">Wifi-netwerk (2,4 GHz)</label><input id="ssid" name="ssid" required maxlength="32" autocomplete="off" autocapitalize="none" spellcheck="false"><label for="pass">Wifi-wachtwoord</label><input id="pass" name="pass" type="password" maxlength="63" autocomplete="new-password"><label for="key">OpenAI API-sleutel</label><input id="key" name="key" type="password" required maxlength="256" autocomplete="new-password" spellcheck="false" autocapitalize="none"><button>Opslaan en verbinden</button></form><p><small>Een eigen OpenAI API-account met tegoed is nodig. ChatGPT Plus bevat geen API-tegoed. De gegevens worden lokaal op de wifi-module opgeslagen. Dit tijdelijke netwerk sluit na opslaan of na 10 minuten. Opslaan verstuurt geen betaalde vraag.</small></p></main></html>)HTML";
 setupWeb.send(200,"text/html; charset=utf-8",html);
}
static void setupSave(){
 setupHeaders();
 if(!portalActive||portalSaved||setupWeb.header("Content-Length").toInt()>2048||!setupWeb.hasArg("token")||setupWeb.arg("token")!=portalToken){setupWeb.send(403,"text/plain","Sessie verlopen. Start installatie opnieuw op de Casio.");return;}
 String values[4]={setupWeb.arg("ssid"),setupWeb.arg("pass"),setupWeb.arg("key"),cfg[3]};
 if(!values[0].length()||!values[2].startsWith("sk-")){setupWeb.send(400,"text/plain","Vul je wifi-naam en volledige OpenAI API-sleutel (sk-...) in. Ga terug om te corrigeren.");return;}
 for(int i=0;i<4;i++)if(!setting_valid(i+1,values[i].c_str(),values[i].length())){setupWeb.send(400,"text/plain","Ongeldige invoer of te lange waarde. Ga terug om te corrigeren.");return;}
 {Lock l;if(busy()||!saveConfiguration(values)){setupWeb.send(503,"text/plain","Opslaan mislukt. Probeer opnieuw.");return;}
  wifiDirty=true;clearHistory();context.restoring=false;context.pending="";upload_clear(&upload);ledger.allocated=0;}
 portalSaved=true;portalStatus=2;portalCloseAt=millis();
 setupWeb.send(200,"text/html; charset=utf-8","<!doctype html><html lang=nl><meta name=viewport content='width=device-width,initial-scale=1'><body style='background:#171717;color:white;font:18px system-ui;padding:24px'><h1>Gegevens opgeslagen</h1><p>Het installatienetwerk sluit nu. Verbind je telefoon weer met je eigen wifi en kijk op de Casio of de verbinding lukt.</p><p>Er is nog geen API-vraag verstuurd. Je sleutel wordt bij je eerste vraag gecontroleerd.</p></body></html>");
}
static bool startPortal(){
 WiFi.disconnect(false,false);WiFi.mode(WIFI_AP);
 portalSaved=false;portalStatus=0;portalId=(uint16_t)esp_random();portalPassword=randomSetupText(12);portalToken=randomSetupText(32);
 char name[24];snprintf(name,sizeof(name),"ChatGPT-%04X",portalId);
 if(!WiFi.softAP(name,portalPassword.c_str(),1,false,1)){stopPortal();WiFi.mode(WIFI_STA);return false;}
 portalActive=true;portalStatus=1;portalOpened=millis();
 setupDNS.start(53,"*",WiFi.softAPIP());
 if(!portalRoutesReady){
 const char* headers[]={"Content-Length"};setupWeb.collectHeaders(headers,1);
 setupWeb.on("/",HTTP_GET,setupPage);setupWeb.on("/save",HTTP_POST,setupSave);
 setupWeb.onNotFound([](){setupWeb.sendHeader("Location","http://192.168.4.1/");setupWeb.send(302,"text/plain","");});
 portalRoutesReady=true;}
 setupWeb.begin();return true;
}
static void servicePortal(){
 if(portalActive){
  setupDNS.processNextRequest();setupWeb.handleClient();
  if(portalSaved&&millis()-portalCloseAt>=2000){stopPortal();}
  else if(millis()-portalOpened>=600000)stopPortal();
 }else if(portalStatus==2){
  if(WiFi.status()==WL_CONNECTED)portalStatus=3;
  else if(millis()-portalConnectAt>=20000)portalStatus=4;
 }
}
#endif
