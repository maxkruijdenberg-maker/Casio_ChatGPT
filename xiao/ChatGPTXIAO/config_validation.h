#ifndef CHATGPT_CONFIG_VALIDATION_H
#define CHATGPT_CONFIG_VALIDATION_H
#include "utf8.h"
static int setting_valid(int target,const char*s,size_t n){
 static const size_t caps[]={1024,32,63,256,255,1024,8192};size_t i;
 if(target<0||target>6||n>caps[target]||!u_valid(s,n))return 0;
 if(target==3){for(i=0;i<n;i++)if((unsigned char)s[i]<33||(unsigned char)s[i]>126)return 0;}
 if(target==4){if(!n)return 0;for(i=0;i<n;i++)if(!((s[i]>='a'&&s[i]<='z')||(s[i]>='A'&&s[i]<='Z')||(s[i]>='0'&&s[i]<='9')||s[i]=='-'||s[i]=='_'||s[i]=='.'||s[i]==':'))return 0;}
 return 1;
}
#endif
