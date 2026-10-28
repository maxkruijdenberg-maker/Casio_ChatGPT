#ifndef CHATGPT_MODELS_H
#define CHATGPT_MODELS_H
#include <string.h>
/* Chat Completions family supported by the existing Atom firmware.
 * Relative costs are indicative, not live quotes or per-question prices.
 * Account access is checked by the Atom before the first paid POST. */
typedef struct {const char *label,*id,*cost,*description;} ModelChoice;
static const ModelChoice modelChoices[]={
 {"Voordelig","gpt-4.1-nano","Laag","Voor korte en eenvoudige vragen."},
 {"Dagelijks","gpt-4.1-mini","Middel","Goede startkeuze voor uitleg en oefenen."},
 {"Uitgebreid","gpt-4.1","Hoger","Groter model voor lastigere vragen."}
};
#define MODEL_COUNT ((int)(sizeof(modelChoices)/sizeof(modelChoices[0])))
static int modelChoiceIndex(const char *id){int i;for(i=0;i<MODEL_COUNT;i++)if(!strcmp(id,modelChoices[i].id))return i;return -1;}
#endif
