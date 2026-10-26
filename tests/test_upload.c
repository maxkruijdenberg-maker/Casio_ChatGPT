#include <assert.h>
#include <stdio.h>
#include "../atom/ChatGPTAtom/upload.h"
#include "../atom/ChatGPTAtom/config_validation.h"
int main(void){
 Upload u={0};unsigned i;uint8_t data[22];memset(data,'a',sizeof(data));
 assert(!upload_complete(&u));assert(upload_append(&u,0,data,1)==E_UPLOAD);
 assert(upload_begin(&u,0,1024)==0);
 for(i=0;i<1024;){unsigned n=1024-i;if(n>22)n=22;assert(upload_append(&u,i,data,n)==0);i+=n;}
 assert(upload_complete(&u)&&strlen(u.bytes)==1024);
 assert(upload_append(&u,1024,data,1)==E_UPLOAD);
 upload_clear(&u);assert(!upload_complete(&u)&&u.bytes[0]==0);
 assert(upload_begin(&u,3,257)==E_BIG&&!u.active);
 assert(upload_begin(&u,3,5)==0);assert(upload_append(&u,1,data,1)==E_UPLOAD&&u.used==0);
 assert(upload_append(&u,0,(const uint8_t*)"a\0b",3)==E_UPLOAD&&u.used==0);
 assert(upload_append(&u,0,data,3)==0);assert(!upload_complete(&u));
 assert(upload_append(&u,0,data,3)==E_UPLOAD&&u.used==3);
 assert(upload_append(&u,3,data,3)==E_UPLOAD&&u.used==3);
 assert(upload_append(&u,3,data,2)==0&&upload_complete(&u));
 assert(upload_begin(&u,1,0)==0&&upload_complete(&u));
 assert(upload_begin(&u,255,0)==E_BIG&&!upload_complete(&u));
 assert(setting_valid(1,"Mijn é-hotspot",strlen("Mijn é-hotspot")));
 assert(setting_valid(3,"",0));assert(!setting_valid(3,"fake key",8));
 assert(!setting_valid(3,"é",2));assert(!setting_valid(4,"",0));
 assert(setting_valid(4,"gpt-4.1-mini",12));assert(!setting_valid(4,"../../model",11));
 assert(!setting_valid(0,"\xed\xa0\x80",3));assert(!setting_valid(0,"a\0b",3));
 puts("PASS: actual upload state machine; bounds, invalidation, empty settings, replay offsets, NUL, UTF-8 and key/model validation.");
}
