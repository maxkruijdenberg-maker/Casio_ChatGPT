#ifndef CHATGPT_MODELS_H
#define CHATGPT_MODELS_H
#include <string.h>
/* V30 familiar GPT-4/5/6 text-chat aliases. Official ChatModel and shutdown dates:
 * openai-python f552fcffcf774520e946121704b5fbbf7ba986e8 (2026-10-29).
 * Includes announced future shutdowns with an Info warning; excludes IDs whose
 * shutdown date has passed, dated duplicates and specialist endpoints.
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
 {"GPT-5.4 Mini","gpt-5.4-mini","Niet bevestigd","Voor korte vragen en eenvoudige uitleg.","Een compacte keuze uit de GPT-5.4-reeks; vergelijk de actuele prijs zelf."},
 {"GPT-5.4 Nano","gpt-5.4-nano","Niet bevestigd","Voor korte, eenvoudige tekstvragen.","Oudere keuze. Aangekondigd einde: 1 april 2027; kies voor langere inzet liever een nieuwer model."},
 {"GPT-5.2","gpt-5.2","Niet bevestigd","Voor algemene tekstvragen en uitleg uit de GPT-5.2-reeks.","Een bekende oudere keuze. Vergelijk de actuele beschikbaarheid en prijs met nieuwere modellen."},
 {"GPT-5.1","gpt-5.1","Niet bevestigd","Voor algemene vragen en uitgebreide tekstuitleg.","Oudere keuze. Aangekondigd einde: 1 april 2027; een nieuwer model is een betere start voor langdurig gebruik."},
 {"GPT-5","gpt-5","Niet bevestigd","Voor algemene vragen en uitleg uit de oorspronkelijke GPT-5-reeks.","Oudere keuze. Aangekondigd einde: 11 december 2026; kies bij voorkeur een nieuwer model."},
 {"GPT-5 Mini","gpt-5-mini","Niet bevestigd","Voor korte vragen en eenvoudige uitleg uit de GPT-5-reeks.","Oudere keuze. Aangekondigd einde: 11 december 2026; controleer beschikbaarheid en vergelijk de actuele prijs."},
 {"GPT-5 Nano","gpt-5-nano","Niet bevestigd","Voor eenvoudige, beperkte tekstvragen uit de GPT-5-reeks.","Oudere keuze. Aangekondigd einde: 11 december 2026; kies voor nieuwe gesprekken liever een nieuwer model."},
 {"GPT-4.1","gpt-4.1","Niet bevestigd","Voor gewone tekstvragen en programmeeruitleg uit de GPT-4.1-reeks.","Bekende oudere keuze. Aangekondigd einde: 14 januari 2027; kies voor langdurig gebruik liever een nieuwer model."},
 {"GPT-4.1 Mini","gpt-4.1-mini","Niet bevestigd","Voor korte tekstvragen en eenvoudige uitleg uit de GPT-4.1-reeks.","Bekende oudere keuze. Aangekondigd einde: 14 januari 2027; controleer ook de actuele API-prijs."},
 {"GPT-4o","gpt-4o","Niet bevestigd","Voor algemene tekstvragen met het bekende GPT-4o-model.","Deze app gebruikt alleen tekst. Aangekondigd einde: 14 januari 2027; kies bij voorkeur een nieuwer model."},
 {"GPT-4o Mini","gpt-4o-mini","Niet bevestigd","Voor korte tekstvragen met het bekende GPT-4o Mini-model.","Deze app gebruikt alleen tekst. Aangekondigd einde: 14 januari 2027; vergelijk de actuele prijs en beschikbaarheid."}
};
#define MODEL_COUNT ((int)(sizeof(modelChoices)/sizeof(modelChoices[0])))
static int modelChoiceIndex(const char *id){int i;for(i=0;i<MODEL_COUNT;i++)if(!strcmp(id,modelChoices[i].id))return i;return -1;}
static const char *modelDisplayName(const char *id){int i=modelChoiceIndex(id);return i>=0?modelChoices[i].label:id;}
#endif
