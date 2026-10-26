#ifndef CHATGPT_TEXT_H
#define CHATGPT_TEXT_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* UTF-8 only on the wire and in the editor. Never split a code point. */
#include "../atom/ChatGPTAtom/utf8.h"
static int u_insert(char*s,size_t cap,size_t *cursor,uint32_t cp){char b[5];size_t n=u_encode(cp,b),len=strlen(s);
 if(!n||cp==0||len+n>=cap||*cursor>len)return 0;
 memmove(s+*cursor+n,s+*cursor,len-*cursor+1);memcpy(s+*cursor,b,n);*cursor+=n;return 1;
}
static void u_delete(char*s,size_t *cursor){size_t prev;if(!*cursor)return;prev=u_prev(s,*cursor);memmove(s+prev,s+*cursor,strlen(s+*cursor)+1);*cursor=prev;}
/* Atomic multi-character insertion: a function token never lands half-written. */
static int u_insert_text(char *s,size_t cap,size_t *cursor,const char *text){
 size_t len=strlen(s),n=strlen(text);
 if(!n||*cursor>len||len>=cap||n>=cap-len||!u_valid(text,n))return 0;
 if(*cursor<len&&((unsigned char)s[*cursor]&0xc0)==0x80)return 0;
 memmove(s+*cursor+n,s+*cursor,len-*cursor+1);memcpy(s+*cursor,text,n);*cursor+=n;return 1;
}
/* Text equivalents of the Casio primary/yellow-label mathematical functions.
 * These are prompt text, not local evaluations or native 2D Casio tokens. */
static const char *math_key(int key,int shifted){
 if(shifted)switch(key){
  case 66:return "10^(";case 56:return "e^(";
  case 46:return "asin(";case 36:return "acos(";case 26:return "atan(";
  case 67:return "sqrt(";case 57:return "root(";
  case 51:return "π";case 61:return "=";case 41:return "Ans";
  case 42:return "[";case 32:return "]";case 43:return "{";case 33:return "}";
  case 72:return "List ";case 62:return "Mat ";
  default:break;
 }
 switch(key){
  case 76:return "x";case 66:return "log(";case 56:return "ln(";
  case 46:return "sin(";case 36:return "cos(";case 26:return "tan(";
  case 67:return "^2";case 57:return "^(";
  case 75:return "/";case 51:return "*10^(";case 25:return "->";
  default:return 0;
 }
}
/* Matrix positions are libfxcg PRGM_GetKey values. Letters follow key labels. */
static int alpha_key(int key){
 const int map[26]={76,66,56,46,36,26,75,65,55,45,35,25,74,64,54,73,63,53,43,33,72,62,52,42,32,71};
 int i;for(i=0;i<26;i++)if(key==map[i])return 'a'+i;if(key==61)return ' ';if(key==51)return '"';return 0;
}
static int number_key(int key){
 switch(key){case 71:return '0';case 72:return '1';case 62:return '2';case 52:return '3';case 73:return '4';case 63:return '5';case 53:return '6';case 74:return '7';case 64:return '8';case 54:return '9';case 61:return '.';case 51:return 'e';case 41:return '-';case 42:return '+';case 32:return '-';case 43:return '*';case 33:return '/';case 55:return '(';case 45:return ')';case 35:return ',';default:return 0;}
}
#endif
