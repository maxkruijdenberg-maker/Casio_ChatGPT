#ifndef CHATGPT_MODELS_H
#define CHATGPT_MODELS_H
#include <string.h>
/* Model IDs verified in openai-python ca1217ca59a5d0b4497e624771103853d39bd495.
 * SDK enumeration is not proof of account access or live completion support.
 * Relative costs are indicative, not live quotes or per-question prices.
 * Account access is checked by the Atom before the first paid POST. */
typedef struct {const char *label,*id,*cost,*description;} ModelChoice;
static const ModelChoice modelChoices[]={
 {"Voordelig","gpt-4.1-nano","Laag","Voor korte en eenvoudige vragen."},
 {"Dagelijks","gpt-4.1-mini","Middel","Goede startkeuze voor uitleg en oefenen."},
 {"Uitgebreid","gpt-4.1","Hoger","Groter model voor lastigere vragen."},
 {"GPT-6 Astra","gpt-6-astra","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-6.1 Sol","gpt-6.1-sol","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-6.1 Terra","gpt-6.1-terra","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-6 Sol","gpt-6-sol","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-6 Luna","gpt-6-luna","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.6 Sol","gpt-5.6-sol","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.6 Terra","gpt-5.6-terra","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.6 Luna","gpt-5.6-luna","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.5","gpt-5.5","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.4","gpt-5.4","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.4 Mini","gpt-5.4-mini","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.4 Nano","gpt-5.4-nano","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.3 Chat","gpt-5.3-chat-latest","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.2","gpt-5.2","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5.1","gpt-5.1","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5","gpt-5","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5 Mini","gpt-5-mini","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-5 Nano","gpt-5-nano","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-4o","gpt-4o","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"GPT-4o Mini","gpt-4o-mini","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"o4 Mini","o4-mini","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"o3","o3","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."},
 {"o3 Mini","o3-mini","Onbekend","Standaardkeuze. De beschikbaarheid hangt af van je API-account."}
};
#define MODEL_COUNT ((int)(sizeof(modelChoices)/sizeof(modelChoices[0])))
static int modelChoiceIndex(const char *id){int i;for(i=0;i<MODEL_COUNT;i++)if(!strcmp(id,modelChoices[i].id))return i;return -1;}
#endif
