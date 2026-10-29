#ifndef CHATGPT_MODELS_H
#define CHATGPT_MODELS_H
#include <string.h>
/* V22 curated text-chat aliases. Official ChatModel plus deprecation notes:
 * openai-python f552fcffcf774520e946121704b5fbbf7ba986e8 (2026-10-29).
 * Excludes deprecated IDs, dated duplicates and specialist endpoints.
 * Advice is this app's selection guidance, not a benchmark or price quote.
 * Account GET metadata does not prove successful live completion support. */
typedef struct {const char *label,*id,*cost,*description,*advice;} ModelChoice;
static const ModelChoice modelChoices[]={
 {"GPT-6 Astra","gpt-6-astra","Niet bevestigd","Voor een lastige vraag met meerdere denkstappen.","Probeer dit bij uitgebreide uitleg en controle van een redenering."},
 {"GPT-6.1 Sol","gpt-6.1-sol","Niet bevestigd","Voor dagelijkse vragen, uitleg en programmeerhulp.","Een logische startkeuze als je niet weet welk model je wilt."},
 {"GPT-6.1 Terra","gpt-6.1-terra","Niet bevestigd","Voor vragen over code en het verbeteren van programma's.","Kies dit vooral als je programmeerhulp zoekt."},
 {"GPT-6 Sol","gpt-6-sol","Niet bevestigd","Voor algemene vragen en uitleg in een gesprek.","Een alternatief om met GPT-6.1 Sol te vergelijken."},
 {"GPT-6 Luna","gpt-6-luna","Niet bevestigd","Voor korte vragen en eenvoudige uitleg.","Probeer eerst dit model als je vraag weinig denkstappen nodig heeft."},
 {"GPT-5.6 Sol","gpt-5.6-sol","Niet bevestigd","Voor algemene tekstvragen uit de vorige generatie.","Een alternatief als je account geen toegang tot de GPT-6-keuzes heeft."},
 {"GPT-5.6 Terra","gpt-5.6-terra","Niet bevestigd","Voor programmeervragen uit de vorige generatie.","Een alternatief voor GPT-6.1 Terra als dat model niet beschikbaar is."},
 {"GPT-5.6 Luna","gpt-5.6-luna","Niet bevestigd","Voor eenvoudige tekstvragen uit de vorige generatie.","Een alternatief voor GPT-6 Luna als dat model niet beschikbaar is."},
 {"GPT-5.5","gpt-5.5","Niet bevestigd","Voor algemene uitleg en tekstvragen.","Vergelijk de antwoorden met een nieuwer model op jouw eigen vragen."},
 {"GPT-5.4","gpt-5.4","Niet bevestigd","Voor algemene tekstvragen uit de GPT-5.4-reeks.","Kies bij twijfel eerst een nieuwer model als jouw account dat aanbiedt."},
 {"GPT-5.4 Mini","gpt-5.4-mini","Niet bevestigd","Voor korte vragen en eenvoudige uitleg.","Een compacte keuze uit de GPT-5.4-reeks; vergelijk de actuele prijs zelf."}
};
#define MODEL_COUNT ((int)(sizeof(modelChoices)/sizeof(modelChoices[0])))
static int modelChoiceIndex(const char *id){int i;for(i=0;i<MODEL_COUNT;i++)if(!strcmp(id,modelChoices[i].id))return i;return -1;}
static const char *modelDisplayName(const char *id){int i=modelChoiceIndex(id);return i>=0?modelChoices[i].label:id;}
#endif
