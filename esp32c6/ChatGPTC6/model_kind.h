#ifndef CHATGPT_MODEL_KIND_H
#define CHATGPT_MODEL_KIND_H
#include <string.h>
/* /models has no endpoint-capability metadata. Recognise known non-chat
 * families; leave unknown/new names visible rather than claiming certainty. */
static int model_kind(const char *id){
 const char *s=id;if(!strncmp(s,"ft:",3))s+=3;
 if(strstr(s,"embedding"))return 3;
 if(strstr(s,"moderation"))return 4;
 if(strstr(s,"sora")||strstr(s,"video"))return 5;
 if(strstr(s,"audio")||strstr(s,"realtime")||strstr(s,"transcrib")||strstr(s,"whisper")||strstr(s,"tts"))return 2;
 if(strstr(s,"image")||strstr(s,"dall-e"))return 1;
 if(strstr(s,"instruct")||strstr(s,"deep-research")||!strncmp(s,"davinci",7)||!strncmp(s,"babbage",7))return 6;
 if(!strncmp(s,"gpt-",4)||!strncmp(s,"chatgpt-",8)||(s[0]=='o'&&s[1]>='1'&&s[1]<='9'))return 0;
 return 7;
}
static const char *model_kind_label(int kind){
 static const char *names[]={"Chat","Beeld","Audio","Vector","Check","Video","Anders","Onbek."};
 return names[kind>=0&&kind<=7?kind:7];
}
#endif
