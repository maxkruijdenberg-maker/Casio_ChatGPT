#ifndef CHATGPT_ACTIVE_MODEL_H
#define CHATGPT_ACTIVE_MODEL_H
#include "model_receipt.h"
/* Accept an exact model ID, or its dated snapshot. Never use fuzzy/prefix-only
 * matching: gpt-6, gpt-6.1-sol and gpt-6.1-terra are different selections. */
static bool model_date(const char *s){
 if(strlen(s)!=10||s[4]!='-'||s[7]!='-')return false;
 for(int i=0;i<10;i++)if(i!=4&&i!=7&&(s[i]<'0'||s[i]>'9'))return false;
 int y=(s[0]-'0')*1000+(s[1]-'0')*100+(s[2]-'0')*10+s[3]-'0';
 int m=(s[5]-'0')*10+s[6]-'0',d=(s[8]-'0')*10+s[9]-'0';
 static const int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
 if(y<2000||m<1||m>12)return false;
 return d>=1&&d<=days[m-1]+(m==2&&y%4==0&&(y%100!=0||y%400==0));
}
static bool active_model_matches(const char *selected,const char *reported){
 size_t n=strlen(selected),m=strlen(reported);
 if(!receipt_model_valid(selected,n)||!receipt_model_valid(reported,m))return false;
 if(!strcmp(selected,reported))return true;
 return m==n+11&&!strncmp(selected,reported,n)&&reported[n]=='-'&&model_date(reported+n+1);
}
struct ActiveModel {ModelReceipt receipt;uint32_t epoch=0;};
static void active_reset(ActiveModel *model,const char *selected){
 if(!++model->epoch)++model->epoch;
 receipt_clear(&model->receipt);model->receipt.request=model->epoch;
 size_t n=strlen(selected);if(receipt_model_valid(selected,n))memcpy(model->receipt.requested,selected,n+1);
}
static bool active_publish(ActiveModel *model,uint32_t epoch,const char *selected,const char *reported){
 if(epoch!=model->epoch||strcmp(selected,model->receipt.requested))return false;
 model->receipt.complete=active_model_matches(selected,reported);
 model->receipt.reported[0]=0;
 size_t n=strlen(reported);if(receipt_model_valid(reported,n))memcpy(model->receipt.reported,reported,n+1);
 return model->receipt.complete;
}
#endif
