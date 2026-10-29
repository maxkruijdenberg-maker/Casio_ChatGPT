#ifndef CG3_CHATS_H
#define CG3_CHATS_H
static void loadLibrary(void){
 int i;if(libraryReady)return;
 clear("Lokale opslag");printAt(12,80,"Gesprekken laden...",C_MUTED);show();
 for(i=0;i<STORE_SLOTS;i++){savedChats[i]=lib_entry(&disk,'C',i);}
 storageClose();libraryReady=1;
}
static int freeSlot(const LibraryEntry*entries){int i;for(i=0;i<STORE_SLOTS;i++)if(!entries[i].used&&!entries[i].damaged)return i;return -1;}
static int loadChatPair(int slot,int turn){
 if(slot<0||slot>=STORE_SLOTS||!savedChats[slot].used||!lib_pair(&disk,&savedChats[slot].ref,turn,lastQuestion,sizeof(lastQuestion),answer,sizeof(answer))){
  message("Leesfout","Dit gesprek kon niet worden gelezen. De opgeslagen bestanden zijn niet gewist.");return 0;
 }storageClose();viewTurn=turn;return 1;
}
static int saveReply(void){
 if(!unsavedReply)return 1;
 if(activeChatSlot<0)activeChatSlot=freeSlot(savedChats);
 if(activeChatSlot<0||!lib_append(&disk,activeChatSlot,&savedChats[activeChatSlot],lastQuestion,answer,&savedChats[activeChatSlot])){
  storageClose();message("Niet opgeslagen","Het antwoord staat nog op het scherm, maar opslaan is mislukt. Controleer vrije opslagruimte. F3 probeert opnieuw. Verstuur de vraag niet opnieuw om dit te herstellen.");return 0;
 }
 storageClose();viewTurn=savedChats[activeChatSlot].turns-1;unsavedReply=0;return 1;
}
static int canLeaveReply(void){
 if(!unsavedReply)return 1;
 if(!confirm("Dit antwoord is nog niet opgeslagen. Toch doorgaan en het antwoord verwerpen?"))return 0;
 unsavedReply=0;contextDirty=1;
 if(activeChatSlot>=0&&savedChats[activeChatSlot].used)loadChatPair(activeChatSlot,savedChats[activeChatSlot].turns-1);else newLocalChat();return 1;
}
static void newLocalChat(void){
 activeChatSlot=-1;contextDirty=1;viewTurn=0;unsavedReply=0;prompt[0]=answer[0]=lastQuestion[0]=0;
 chatInputLetters=chatInputUpper=0;chatFollowTail=1;invalidateConversation();
}
static uint32_t lastAtomBoot=0;
static int atomBootKnown=0;
static int restoreChat(void){
 WFrame r;uint8_t action=0;int i,start;
 if(!checkedRpc(W_DEVICE,0,0,&r))return 0;
 if(r.len<4||r.data[1]<3||!(r.data[3]&2)){message("Update nodig","Werk de Atom bij met de meegeleverde firmware om opgeslagen gesprekken te vervolgen.");return 0;}
 if(r.len>=8&&(r.data[3]&4)){
  uint32_t boot=w_get32(r.data+4);if(atomBootKnown&&boot!=lastAtomBoot)contextDirty=1;
  lastAtomBoot=boot;atomBootKnown=1;
 }else contextDirty=1;
 if(!contextDirty)return 1;
 if(activeChatSlot<0||!savedChats[activeChatSlot].used){
  if(!checkedRpc(W_NEW,0,0,&r))return 0;contextDirty=0;return 1;
 }
 if(!checkedRpc(W_CONTEXT,&action,1,&r))return 0;
 start=savedChats[activeChatSlot].turns>2?savedChats[activeChatSlot].turns-2:0;
 for(i=start;i<savedChats[activeChatSlot].turns&&!quitting;i++){
  if(!lib_pair(&disk,&savedChats[activeChatSlot].ref,i,field,sizeof(field),answer,sizeof(answer))||!uploadText(5,field)||!uploadText(6,answer)){
   erase(field,sizeof(field));storageClose();rpc(W_CANCEL,0,0,&r,0);
   if(!quitting)message("Herstellen afgebroken","Er is nog geen nieuwe API-vraag gestart. Probeer later opnieuw.");return 0;
  }erase(field,sizeof(field));
 }
 storageClose();action=1;if(!checkedRpc(W_CONTEXT,&action,1,&r))return 0;
 contextDirty=0;return 1;
}
/* A real left sidebar; selecting a thread reads local files without wifi. */
static int chatSidebar(void){
 int map[STORE_SLOTS],count=0,selected=0,k,i;loadLibrary();
 if(!canLeaveReply())return 0;
 while(!quitting){
  count=0;for(i=0;i<STORE_SLOTS;i++)if(savedChats[i].used||savedChats[i].damaged)map[count++]=i;
  if(selected>count)selected=count;
  clear("Chats");rect(0,32,168,160,C_PANEL);rect(167,32,1,160,C_LINE);
  for(i=0;i<=count;i++){
   int y=39+i*16;if(i==selected){rounded(4,y-1,158,16,C_PURPLE);rounded(5,y,156,14,C_PURPLE_SOFT);}
   clipped(9,y,i==0?"+ Nieuw gesprek":savedChats[map[i-1]].damaged?"Beschadigd":savedChats[map[i-1]].title,18,i==selected?C_MINT:C_TEXT);
  }
  rounded(236,49,64,58,C_PURPLE_SOFT);drawLogo(248,57,40);printAt(228,115,"Jouw chats",C_TEXT);printAt(208,137,"Lokaal bewaard",C_MUTED);
  printAt(184,157,"Ook zonder wifi",C_MUTED);footer("Open","Wis","Nieuw","","","Terug");show();k=waitKey();
  if(k==KEY_PRGM_EXIT||k==KEY_PRGM_F6)return 0;
  if(k==KEY_PRGM_UP&&selected>0)selected--;if(k==KEY_PRGM_DOWN&&selected<count)selected++;
  if(k==KEY_PRGM_F2&&selected>0&&confirm("Dit opgeslagen gesprek verwijderen?")){
   int slot=map[selected-1];if(!lib_delete(&disk,'C',slot)){storageClose();message("Wisfout","Het gesprek is niet gewist.");continue;}
   savedChats[slot]=lib_entry(&disk,'C',slot);storageClose();if(activeChatSlot==slot)newLocalChat();continue;
  }
  if(k==KEY_PRGM_F3||((k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)&&selected==0)){if(prompt[0]&&!confirm("De niet-verstuurde vraag verwerpen en een nieuw gesprek beginnen?"))continue;newLocalChat();return 2;}
  if((k==KEY_PRGM_F1||k==KEY_PRGM_RETURN)&&selected>0){
   int slot=map[selected-1],switching=slot!=activeChatSlot;
   if(switching&&prompt[0]&&!confirm("De niet-verstuurde vraag verwerpen en een ander gesprek openen?"))continue;
   if(loadChatPair(slot,savedChats[slot].turns-1)){
    activeChatSlot=slot;contextDirty=1;unsavedReply=0;if(switching)prompt[0]=0;return 1;
   }
  }
 }return 0;
}
static void resumeAfterMenu(void){
 quitting=0;waitForKeyRelease();
}
#endif
