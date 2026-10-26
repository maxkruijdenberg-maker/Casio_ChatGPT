#ifndef CG3_STORAGE_DEVICE_H
#define CG3_STORAGE_DEVICE_H
#include <fxcg/file.h>
#include "store.h"
static int storageReadHandle=-1,storageWriteHandle=-1;
static char storageReadName[12];
static void storageClose(void){
 if(storageReadHandle>=0){Bfile_CloseFile_OS(storageReadHandle);storageReadHandle=-1;}
 if(storageWriteHandle>=0){Bfile_CloseFile_OS(storageWriteHandle);storageWriteHandle=-1;}
 storageReadName[0]=0;
}
static void storagePath(unsigned short*out,const char*name){
 char path[64];size_t i;sprintf(path,"\\\\fls0\\CGPT3\\%s",name);
 for(i=0;path[i]&&i<63;i++)out[i]=(unsigned char)path[i];out[i]=0;
}
static int deviceRead(void*ctx,const char*name,uint32_t at,void*out,size_t n){
 unsigned short path[64];(void)ctx;
 if(storageReadHandle<0||strcmp(storageReadName,name)){
  if(storageReadHandle>=0)Bfile_CloseFile_OS(storageReadHandle);
  storagePath(path,name);storageReadHandle=Bfile_OpenFile_OS(path,READ,0);strcpy(storageReadName,name);
 }
 if(storageReadHandle<0)return 0;
 return Bfile_ReadFile_OS(storageReadHandle,out,(int)n,(int)at)==(int)n;
}
static int deviceBegin(void*ctx,const char*name,uint32_t length){
 unsigned short path[64],dir[]={ '\\','\\','f','l','s','0','\\','C','G','P','T','3',0};size_t size=length;int h;(void)ctx;
 /* Close cached read handles before deleting/recreating an inactive bank. */
 storageClose();Bfile_CreateEntry_OS(dir,CREATEMODE_FOLDER,0);
 storagePath(path,name);h=Bfile_OpenFile_OS(path,READ,0);
 if(h>=0){Bfile_CloseFile_OS(h);if(Bfile_DeleteEntry(path)<0)return 0;}
 if(Bfile_CreateEntry_OS(path,CREATEMODE_FILE,&size)<0)return 0;
 storageWriteHandle=Bfile_OpenFile_OS(path,READWRITE,0);return storageWriteHandle>=0;
}
static int deviceWrite(void*ctx,const char*name,uint32_t at,const void*data,size_t n){
 (void)ctx;(void)name;if(storageWriteHandle<0)return 0;
 if(Bfile_SeekFile_OS(storageWriteHandle,(int)at)<0)return 0;
 /* OS variants return zero or a byte count. Full read-back verification in
  * s_save catches incomplete writes even when the syscall reports success. */
 return Bfile_WriteFile_OS(storageWriteHandle,data,(int)n)>=0;
}
static int deviceFinish(void*ctx,const char*name){int ok;(void)ctx;(void)name;
 if(storageWriteHandle<0)return 0;ok=Bfile_CloseFile_OS(storageWriteHandle)>=0;storageWriteHandle=-1;return ok;
}
static StoreOps disk={0,deviceRead,deviceBegin,deviceWrite,deviceFinish};
#endif
