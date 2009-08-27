#ifndef ALLOC_H
#define ALLOC_H

char *getenv_malloc (const char *name);
char *strtok_malloc(char * str, const char * delimiters);
char *strcpy_malloc (const char * source);
char* string_replace(const char* s, const char* find, const char* replace);

char *strtok_r_malloc(char * str, const char * delimiters, char **last);
#endif

