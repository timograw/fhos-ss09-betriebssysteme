#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "memcheck.h"

/*
Function strtok_malloc()
Purpose: strtok mit malloc.

Parameters:
	char *str: Zu zerlegender String
	const char *delimiters: Zerlegerchars
	
Returns
	char*: Pointer auf Teilstring
*/

char *strtok_malloc(char * str, const char * delimiters) {
	char *ret_ptr;
	char *tok_ptr;
	
	if (!delimiters)
		return NULL;
	
	tok_ptr = strtok(str, delimiters);
	
	/* Kein weiteres Token */
	if (!tok_ptr) {
		return NULL;
	}

	ret_ptr = (char*)M_MALLOC((strlen(tok_ptr)+1) * sizeof(char));
	
	if (ret_ptr) {
		/* Alles gut */
		strcpy(ret_ptr, tok_ptr);
		return ret_ptr;
	}
	else {
		/* Kein Speicher bekommen */
		fprintf(stderr, "strtok_malloc(): Malloc fehlgeschlagen\n");
		return (char *)-1;
	}
	
}

/*
Function: getenv_malloc()
Purpose:
	gentenv mit malloc. Rueckgabe von Leerstrings wenn Variable nicht gefunden.
	
Paramters:
	const char *name: Name der Variable

Returns:
	char*: Zeiger auf Variableninhalt oder Fehlercode
*/
char *getenv_malloc (const char *name) {
	char *tmp_ptr;
	char *ret_ptr;
	
	if (!name)
		return NULL;
	
	tmp_ptr = getenv(name);
	
	if (!tmp_ptr) {
		ret_ptr = (char*)M_MALLOC(sizeof(char));
		if (!ret_ptr) {
			fprintf(stderr, "getenv_malloc(): Malloc fehlgeschlagen\n");
			return NULL;
		}
		ret_ptr[0] = '\0';
		return ret_ptr;
	}
		
	ret_ptr = (char*)M_MALLOC((strlen(tmp_ptr)+1) * sizeof(char));
	
	if (ret_ptr) {
		strcpy(ret_ptr, tmp_ptr);
		return ret_ptr;
	}
	else {
		/* Kein Speicher bekommen */
		fprintf(stderr, "getenv_malloc(): Malloc fehlgeschlagen\n");
		return NULL;
	}
}

/*
Function: strcpy_malloc()

Purpose:
	strcpy und malloc.
	
Parameters:
	const char* source: Ausgangsstring
	
Returns:
	char*: Zeiger auf neuen String
*/
char *strcpy_malloc (const char * source) {
	char *ret_ptr;
	
	if (!source)
		return NULL;
		
	ret_ptr = (char*)M_MALLOC((strlen(source)+1) * sizeof(char));
	
	strcpy(ret_ptr, source);
	
	return ret_ptr;
}

/* replace will replace the first 'find' in 's' with 'replace'
 * Returns: NULL if 'find' is not found
 *          the result string allocated with 'malloc' on success
 */
char* string_replace(const char* s, const char* find, const char* replace)
{
	const char* p;
	char* ret;

	p = strstr(s, find);

	if (p == NULL)
		return NULL;

	ret = M_MALLOC(strlen(s) + strlen(replace) + 1 - strlen(find));

	if (ret == NULL) {
		fprintf(stderr, "string_replace(): Malloc fehlgeschlagen\n");
		return NULL;
	}

	if (p != s)
		strncpy(ret, s, p-s);

	strcpy(ret + (p-s), replace);
	strcat(ret, p + strlen(find));

	return ret;
}

/*
Function strtok_r_malloc()
Purpose: strtok mit malloc (.

Parameters:
	char *str: Zu zerlegender String
	const char *delimiters: Zerlegerchars
	
Returns
	char*: Pointer auf Teilstring
*/

char *strtok_r_malloc(char * str, const char * delimiters, char **last) {
	char *ret_ptr;
	char *tok_ptr;
	
	if (!delimiters)
		return NULL;
	
	tok_ptr = strtok_r(str, delimiters, last);
	
	/* Kein weiteres Token */
	if (!tok_ptr) {
		return NULL;
	}

	ret_ptr = (char*)M_MALLOC((strlen(tok_ptr)+1) * sizeof(char));
	
	if (ret_ptr) {
		/* Alles gut */
		strcpy(ret_ptr, tok_ptr);
		return ret_ptr;
	}
	else {
		/* Kein Speicher bekommen */
		fprintf(stderr, "strtok_r_malloc(): Malloc fehlgeschlagen\n");
		return (char *)-1;
	}
}

