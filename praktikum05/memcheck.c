/**************************************************************************
*
*  memcheck.c
*
* Author:  Prof. Dr.-Ing. Juergen Wuebbelmann
* Purpose: log usage of dynamic memory
* created: 2007-12-14
* last modified:  2007-12-17: JW
*         - bugfix in struct memList_t
*         - Change in debug levels
*
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memcheck.h"


const int  DL_Information = 0;
const int  DL_Warning     = 1;
const int  DL_Minor       = 2;
const int  DL_Major       = 3;
const int  DL_Fatal       = 4;
const int  DL_None        = 5;


/* definitions: */
#define DEBUGOUT_INFORMATION if(debugLevel <= DL_Information) printf
#define DEBUGOUT_WARNING     if(debugLevel <= DL_Warning)     printf
#define DEBUGOUT_MINOR       if(debugLevel <= DL_Minor)       printf
#define DEBUGOUT_MAJOR       if(debugLevel <= DL_Major)       printf
#define DEBUGOUT_FATAL       if(debugLevel <= DL_Fatal)       printf

/* types:  */
struct listData_t
{
  size_t       size;
  void *       memLoc;
  char *       fileName;
  unsigned int lineNo;
};


struct memList_t
{
  struct listData_t listData;
  struct memList_t* next;
  struct memList_t* prev;
};


/* file globals: */
static struct memList_t  *root = NULL;
static size_t            allocatedMemory = 0;
static unsigned int      allocatedBlocks = 0;
static int               debugLevel      = 0;


/* functions:  */


void* debMalloc(size_t size, char *filename, unsigned int lineNo)
{
   void*      mem         = NULL;
   
	 struct memList_t *link;
	 struct memList_t *element;
   
   DEBUGOUT_INFORMATION("debMalloc(%i, \"%s\", %u)\n", size, filename, lineNo);
   
   // Speicher fuer Listenelement anfordern
   element = (struct memList_t *) malloc(sizeof(struct memList_t));
   
   
   if (element != NULL) {
     // Speicher anfordern
     mem = malloc(size);
     
     if (NULL != mem)
     {
       DEBUGOUT_INFORMATION("  malloc() erfolgreich\n");
       // listData initialisieren
       element->listData.size = size;
       element->listData.memLoc = mem;
       element->listData.fileName = filename;
       element->listData.lineNo = lineNo;
  
       // Zeiger initialisieren
       element->prev = NULL;
       element->next = NULL;
  
       // Wenn root noch nicht existiert
       if (root == NULL) {
         // root element setzen
         root = element;
  
         DEBUGOUT_INFORMATION("  root Element erstellt\n");
       }
       // Sonst an Kette anhaengen
       else {
         // Link initialisieren (zeigt auf erstes Element)
         link = root;
  
         // Durch Kette iterieren
         while(link->next != NULL)
           link = link->next;
  
         // -> link ist nun letztes Element
  
         // elemente verlinken
         element->prev = link;
         link->next = element;
  
         DEBUGOUT_INFORMATION("  Element angehaengt\n");
       }
  
       // Speichergroesse anpassen
       DEBUGOUT_INFORMATION("  allocatedMemory: %i+%i = ", allocatedMemory, size);
       allocatedMemory += size;
       DEBUGOUT_INFORMATION("%i\n", allocatedMemory);
  
       // Speicherbloecke anpassen
       allocatedBlocks++;
  
     }
     // Kein Speicher bekommen
     else
       DEBUGOUT_FATAL("FATAL: %s:%u debMalloc() malloc konnte keinen Speicher bereitstellen\n", filename, lineNo);
   }
   // Kein Speicher fuer Listenelement bekommen
   else
     DEBUGOUT_FATAL("FATAL: %s:%u debMalloc() malloc konnte keinen Speicher fuer Listenelement bereitstellen\n", filename, lineNo);

   return mem;
}


void* debRealloc(void         *ptr,
                 size_t       size,
                 char         *fileName,
                 unsigned int lineNo)
{
  void       *mem  = NULL;

  struct memList_t *link;

  DEBUGOUT_INFORMATION("debRealloc(%p, %i, \"%s\", %u)\n", ptr, size, fileName, lineNo);
	
	// realloc() ausfuehren
	mem = realloc(ptr, size);
	
	// Wenn root element besteht
	if (root != NULL)  {
		// Link initialisieren
		link = root;

		// passenden link suchen
		while(link != NULL) {
			// Wenn Speicherstelle uebereinstimmt
			if (link->listData.memLoc != ptr)
				break;
			// Durch Liste iterieren
			link = link->next;
		}

		// Wenn link vorhanden ist
		if (link != NULL) {
      // Wenn realloc() erfolgreich
      if (mem != NULL) {
        DEBUGOUT_INFORMATION("  realloc() erfolgreich\n");

        DEBUGOUT_INFORMATION("  allocatedMemory: %i%+-i = ", allocatedMemory, size - link->listData.size);
        // allocatedMemory anpassen
        allocatedMemory += size - link->listData.size;
        DEBUGOUT_INFORMATION("%i\n", allocatedMemory);
        
        // Groesse neu setzen
        link->listData.size = size;
        // Neue Speicheradresse setzen
        link->listData.memLoc = mem;
      }
      // realloc() nicht erfolgreich
      else
        DEBUGOUT_FATAL("FATAL: %s:%u debRealloc() konnte keinen Speicher bekommen\n", fileName, lineNo);
		}
		// Wert nicht gefunden
		else
			DEBUGOUT_WARNING("WARNING: %s:%u debRealloc() Speicher nicht in Liste gefunden\n", fileName, lineNo);
	}
	// Root Element existiert nicht
	else
		DEBUGOUT_WARNING("WARNING: %s:%u debRealloc() Root Element existiert nicht", fileName, lineNo);

  return mem;
}


void debFree(void *mem, char *filename, unsigned int lineNo)
{
  struct memList_t *link, *prev, *next;

  DEBUGOUT_INFORMATION("debFree(%p,  \"%s\", %u)\n", mem, filename, lineNo);

	// Wenn root element besteht
	if (root != NULL)  {
		// Link initialisieren
		link = root;

		// passenden link suchen
		while(link != NULL) {
			// Wenn Speicherstelle uebereinstimmt
			if (link->listData.memLoc == mem)
				break;
			// Durch Liste iterieren
			link = link->next;
		}

		// Wenn link vorhanden ist
		if (link != NULL) {

			// vorheriges und naechstes Element ermitteln
			prev = link->prev;
			next = link->next;

			// Wenn kein Element mehr existiert
			if (prev == NULL && next == NULL) {
				// root element loeschen
				root = NULL;
        DEBUGOUT_INFORMATION("  root geloescht\n");
      }
			// Wenn noch Elemente existieren
			else {
				// Wenn vorheriges Element existiert
				if (prev != NULL)
					// Vorheriges Element neu verlinken
					prev->next = next;
        // Sonst wurde root geloescht
				else
					// root neu verlinken
					root = next;

				// Wenn naechstes Element existiert
				if (next != NULL)
					// Naechstes Element neu verlinken
					next->prev = prev;

        DEBUGOUT_INFORMATION("  Element geloescht\n");
			}

  		// Speichergroesse anpassen
      DEBUGOUT_INFORMATION("  allocatedMemory: %i-%i = ", allocatedMemory, link->listData.size);
			allocatedMemory -= link->listData.size;
      DEBUGOUT_INFORMATION("%i\n", allocatedMemory);
			// Speicherbloecke anpassen
			allocatedBlocks--;

			// Speicher des Elementes freigeben
			free(link);
		} 
		// Wert nicht in Liste
		else
			DEBUGOUT_WARNING("WARNING: %s:%i debFree(%p) Wert nicht in Liste\n", filename, lineNo, mem);
	}
	// Root Element existiert nicht
	else
		DEBUGOUT_WARNING("WARNING: %s:%i debFree(%p) Liste leer, kein Speicher vergeben\n", filename, lineNo, mem);
	
	free(mem);
  DEBUGOUT_INFORMATION("  free() ausgefuehrt\n");
}


void debShowAlloc(void)
{
	struct memList_t *link;
	struct listData_t *data;
	int n;

	link = root;

	n = 1;
	while(link != NULL) {
		data = &link->listData;
		printf("Eintrag %i\n", n);
    printf("  address: $%p\n", data->memLoc);
		printf("  size   : %i (0x%X)\n", data->size, data->size);
		printf("  file   : %s:%i\n", data->fileName, data->lineNo);
		putchar('\n');

		link = link->next;
		n++;
	}
}




size_t debGetMem(void)
{
	 return allocatedMemory;
}


void setMemDebugLevel(int level)
{
  if (level >= DL_Information && level <= DL_None)
  {
    debugLevel = level;
  }
  else
  {
     DEBUGOUT_MINOR("MINOR: try to set illegal debug level.\n");
  }
}
