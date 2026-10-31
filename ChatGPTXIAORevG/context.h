#ifndef CG3_CONTEXT_H
#define CG3_CONTEXT_H
#include <stdint.h>
#include <utility>
#include "wire.h"
/* Shared with host tests. No network requests or persistent request IDs here. */
template<class Text> struct ChatContext {
 static const size_t MAX_BYTES=12288;
 Text q[2],a[2],pending;
 uint8_t count=0;
 bool dropped=false,restoring=false;
 void clear(){for(int i=0;i<2;i++){q[i]="";a[i]="";}pending="";count=0;dropped=false;restoring=false;}
 bool append(const Text&question,const Text&answer){
  if(!question.length()||question.length()>1024||!answer.length()||answer.length()>8192)return false;
  Text qc=question,ac=answer;if(qc.length()!=question.length()||ac.length()!=answer.length())return false;
  size_t retained=qc.length()+ac.length();for(int i=0;i<count;i++)retained+=q[i].length()+a[i].length();
  while(count&&(count>=2||retained>MAX_BYTES)){
   retained-=q[0].length()+a[0].length();q[0]=std::move(q[1]);a[0]=std::move(a[1]);count--;dropped=true;
  }
  q[count]=std::move(qc);a[count]=std::move(ac);count++;return true;
 }
 void beginRestore(){clear();restoring=true;}
 int restoreQuestion(const Text&question){
  if(!restoring||pending.length()||!question.length()||question.length()>1024)return E_UPLOAD;
  pending=question;return pending.length()==question.length()?E_NONE:E_MEMORY;
 }
 int restoreAnswer(const Text&answer){
  if(!restoring||!pending.length()||!answer.length()||answer.length()>8192)return E_UPLOAD;
  if(!append(pending,answer))return E_MEMORY;pending="";return E_NONE;
 }
 int finishRestore(){if(!restoring||pending.length())return E_UPLOAD;restoring=false;return E_NONE;}
};
#endif
