#include <cassert>
#include <string>
#include <cstdio>
#include "../atom/ChatGPTAtom/context.h"
int main(){
 ChatContext<std::string> c;
 assert(c.restoreQuestion("q")==E_UPLOAD&&c.finishRestore()==E_UPLOAD);
 c.beginRestore();assert(c.restoring&&c.count==0);
 assert(c.restoreAnswer("orphan")==E_UPLOAD);
 assert(c.restoreQuestion("eerste vraag")==0);
 assert(c.restoreQuestion("double")==E_UPLOAD&&c.finishRestore()==E_UPLOAD);
 assert(c.restoreAnswer("antwoord")==0);
 assert(c.restoreQuestion("tweede vraag")==0&&c.restoreAnswer("antwoord twee")==0);
 assert(c.finishRestore()==0&&!c.restoring&&c.count==2);
 assert(c.q[0]=="eerste vraag"&&c.a[1]=="antwoord twee");
 assert(c.append("nieuwe vraag","nieuw antwoord")&&c.count==2&&c.dropped);
 assert(c.q[0]=="tweede vraag"&&c.q[1]=="nieuwe vraag");
 c.beginRestore();std::string q(1024,'q'),a(8192,'a');
 assert(c.restoreQuestion(q)==0&&c.restoreAnswer(a)==0);
 assert(c.restoreQuestion(q)==0&&c.restoreAnswer(a)==0);
 assert(c.count==1&&c.dropped&&c.q[0].size()+c.a[0].size()<=c.MAX_BYTES);
 assert(c.finishRestore()==0);
 assert(!c.append("", "a")&&!c.append(q+"q",a)&&!c.append(q,a+"a"));
 c.beginRestore();assert(c.restoreQuestion("pending")==0);c.clear();
 assert(!c.restoring&&c.count==0&&c.pending.empty());
 puts("PASS: actual Atom context state machine; restore ordering, complete pairs, abort, turn/byte limits, invalid lengths and continuation history.");
}
