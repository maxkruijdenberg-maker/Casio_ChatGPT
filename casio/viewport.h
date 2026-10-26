#ifndef CHATGPT_VIEWPORT_H
#define CHATGPT_VIEWPORT_H
#include <stdint.h>
#include <string.h>
#include "store.h"
/* The physical display remains 384x216. Only this app's rendered canvas is
 * scaled/moved; native Casio menus and stored chats are not modified. */
typedef struct {int percent,x,y;} ViewProfile;
typedef struct {int16_t x[385],y[217];} ViewMap;
typedef struct {int x,y,w,h;} ViewRect;
static ViewProfile view_default(void){ViewProfile p={100,0,0};return p;}
static int view_valid(ViewProfile p){return p.percent>=80&&p.percent<=120&&p.x>=-24&&p.x<=24&&p.y>=-24&&p.y<=24;}
static void view_build(ViewMap*m,ViewProfile p){
 int i,w,h,ox,oy;if(!view_valid(p))p=view_default();
 w=(384*p.percent+50)/100;h=(216*p.percent+50)/100;
 ox=(384-w)/2+p.x;oy=(216-h)/2+p.y;
 for(i=0;i<=384;i++)m->x[i]=(int16_t)(ox+(i*p.percent+50)/100);
 for(i=0;i<=216;i++)m->y[i]=(int16_t)(oy+(i*p.percent+50)/100);
}
static int view_rect(const ViewMap*m,int x,int y,int w,int h,ViewRect*out){
 int x1=x,y1=y,x2=x+w,y2=y+h;
 if(w<=0||h<=0||x2<=0||y2<=0||x1>=384||y1>=216)return 0;
 if(x1<0)x1=0;if(y1<0)y1=0;if(x2>384)x2=384;if(y2>216)y2=216;
 x1=m->x[x1];x2=m->x[x2];y1=m->y[y1];y2=m->y[y2];
 if(x1<0)x1=0;if(y1<0)y1=0;if(x2>384)x2=384;if(y2>216)y2=216;
 if(x2<=x1||y2<=y1)return 0;
 out->x=x1;out->y=y1;out->w=x2-x1;out->h=y2-y1;return 1;
}
static ViewProfile view_load(const StoreOps*ops){
 StoreRef ref=s_latest(ops,'D',0);uint8_t data[8];ViewProfile p=view_default();
 if(ref.valid&&ref.length==8&&s_read(ops,&ref,0,data,8)&&!memcmp(data,"VP4",3)&&!data[6]&&!data[7]){
  p.percent=data[3];p.x=(int)data[4]-24;p.y=(int)data[5]-24;if(!view_valid(p))p=view_default();
 }return p;
}
static int view_save(const StoreOps*ops,ViewProfile p){
 uint8_t data[8]={'V','P','4',0,0,0,0,0};StoreJoin join;
 if(!view_valid(p))return 0;
 data[3]=(uint8_t)p.percent;data[4]=(uint8_t)(p.x+24);data[5]=(uint8_t)(p.y+24);
 memset(&join,0,sizeof(join));join.ops=ops;join.count=1;join.part[0]=(StorePart){data,0,0,8};
 return s_save(ops,'D',0,8,s_join,&join,0);
}
#endif
