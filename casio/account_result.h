#ifndef CHATGPT_ACCOUNT_RESULT_H
#define CHATGPT_ACCOUNT_RESULT_H
/* Brief, non-interactive result on the same page as the account loader. */
static void accountResultStroke(int x,int y,int x2,int y2,unsigned short color){
 int dx=x2>x?x2-x:x-x2,sx=x<x2?1:-1;
 int dy=y2>y?y-y2:y2-y,sy=y<y2?1:-1,err=dx+dy;
 for(;;){
  rect(x-1,y-1,4,4,color);if(x==x2&&y==y2)break;
  int twice=2*err;if(twice>=dy){err+=dy;x+=sx;}if(twice<=dx){err+=dx;y+=sy;}
 }
}
static void paintAccountResult(int success){
 clear("ChatGPT");ellipse(192,106,32,32,C_PANEL);
 if(success){accountResultStroke(174,106,187,119,C_GREEN);accountResultStroke(187,119,211,91,C_GREEN);}
 else {accountResultStroke(179,93,205,119,C_RED);accountResultStroke(205,93,179,119,C_RED);}
 centered(157,success?"Account gecontroleerd":"Controle mislukt",C_TEXT);
 footer("","","","","","");show();
}
static void showAccountResult(int success){
 paintAccountResult(success);OS_InnerWait_ms(900);
}
#endif
