#include <assert.h>
#include <stdio.h>
#include "../casio/viewport.h"
int main(void){
 ViewProfile p=view_default();ViewMap map;ViewRect rect;
 assert(view_valid(p)&&!view_valid((ViewProfile){79,0,0})&&!view_valid((ViewProfile){121,0,0}));
 assert(!view_valid((ViewProfile){100,25,0})&&!view_valid((ViewProfile){100,0,-25}));
 view_build(&map,p);assert(map.x[0]==0&&map.x[384]==384&&map.y[0]==0&&map.y[216]==216);
 assert(view_rect(&map,0,0,384,216,&rect)&&rect.x==0&&rect.y==0&&rect.w==384&&rect.h==216);
 view_build(&map,(ViewProfile){80,0,0});assert(map.x[0]>0&&map.x[384]<384&&map.y[0]>0&&map.y[216]<216);
 for(int scale=80;scale<=120;scale++)for(int x=-24;x<=24;x+=8)for(int y=-24;y<=24;y+=8){
  view_build(&map,(ViewProfile){scale,x,y});
  for(int i=0;i<384;i++)assert(map.x[i+1]>=map.x[i]&&map.x[i+1]-map.x[i]<=2);
  for(int i=0;i<216;i++)assert(map.y[i+1]>=map.y[i]&&map.y[i+1]-map.y[i]<=2);
  for(int left=-10;left<=384;left+=37)for(int top=-10;top<=216;top+=29){
   if(view_rect(&map,left,top,24,24,&rect))assert(rect.x>=0&&rect.y>=0&&rect.w>0&&rect.h>0&&rect.x+rect.w<=384&&rect.y+rect.h<=216);
  }
 }
 view_build(&map,view_default());assert(!view_rect(&map,0,0,0,1,&rect)&&!view_rect(&map,384,0,8,14,&rect));
 assert(view_rect(&map,-5,-5,10,10,&rect)&&rect.x==0&&rect.y==0&&rect.w==5&&rect.h==5);
 puts("PASS: V4 viewport; 80-120% scales, +/-24 pixel offsets, identity mapping, monotonic pixel maps and bounds-safe clipping.");
}
