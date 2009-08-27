#ifndef _MEMCHECK_H_
#define _MEMCHECK_H_
/**************************************************************************
*
*  memcheck.h
*
* Author:  Prof. Dr.-Ing. Juergen Wuebbelmann
* Purpose: interface for memcheck
* created: 2007-12-14
* last modified:   
*      2007-12-17: JW
*         - Change in debug levels, added DL_None
*      2008-03-14: JW
*         - added defines for malloc / realloc / free
*           no change for names in SW to debug required.
*      2008-06-05: JW
*         - added calloc and strdup handling
*
* to do: add interface for strdup derivates
*
***************************************************************************/


#ifdef MEMDEBUG
#define malloc(x)       debMalloc((x), __FILE__, __LINE__)
#define calloc(x,y)     debCalloc((x),(y), __FILE__, __LINE__)
#define realloc(x, y)   debRealloc((x),(y), __FILE__, __LINE__)
#define free(x)         debFree((x))
#define strdup(x)       debStrdup((x), __FILE__, __LINE__)
#define M_SHOW()        debShowAlloc()
#define M_GETUSAGE()    debGetMem()
#define M_SETDEBUG(x)   setMemDebugLevel((x))
#else
#define M_SHOW()
#define M_GETUSAGE() 0
#define M_SETDEBUG(x)
#endif

/* available debug levels:  */
typedef enum debugLevel_e
{ 
  DL_Information = 0,
  DL_Warning,
  DL_Minor,
  DL_Major,
  DL_Fatal,
  DL_None
} DebugLevel_e;

#ifdef MEMDEBUG
/* prototypes: */


/******************************************************
*
*  debMalloc()
*  parameter:
*    size:     in: size of memory to allocate
*    filename: in: name of calling file
*    lineNo:   in: line number from where function is called
*  
*  return:
*    void*: Pointer to allocated memory or NULL if no memory
*          could be allocated
*
*  Purpose:
*   calls malloc and saves data of allocated memory in 
*   double linked list
*
*  Limitations:
*
******************************************************/
void*  debMalloc(size_t       size, 
                 char         *filename, 
                 unsigned int lineNo);


/******************************************************
*
*  debCalloc()
*  parameter:
*    n:      : n elements of an array
*    s:      : size of one array element  
*    filename: in: name of calling file
*    lineNo:   in: line number from where function is called
*  
*  return:
*    void*: Pointer to allocated memory or NULL if no memory
*          could be allocated
*
*  Purpose:
*   calls calloc and saves data of allocated memory in 
*   double linked list
*
*  Limitations:
*
******************************************************/
void*  debCalloc(size_t       n,
                 size_t       s, 
                 char         *filename, 
                 unsigned int lineNo);


/******************************************************
*
*  debRealloc()
*  parameter:
*    ptr:      in: pointer to already allocated memory
*    size:     in: size of memory to allocate
*    filename: in: name of calling file
*    lineNo:   in: line number from where function is called
*  
*  return:
*    void*: Pointer to allocated memory or NULL if no memory
*          could be allocated
*
*  Purpose:
*   calls realloc and saves data of allocated memory in 
*   double linked list (update of list)
*
*  Limitations:
*
******************************************************/
void*  debRealloc(void*        ptr, 
                  size_t       size, 
                  char         *filename, 
                  unsigned int lineNo);


/******************************************************
*
*  debStrdup()
*  parameter:
*    s1:       in: string to duplicate 
*    filename: in: name of calling file
*    lineNo:   in: line number from where function is called
*  
*  return:
*    char*: Pointer to allocated memory or NULL if no memory
*           could be allocated
*
*  Purpose:
*   calls strdup and saves data of allocated memory in 
*   double linked list (update of list)
*
*  Limitations:
*
******************************************************/
char*  debStrdup(char*        s1, 
                 char         *filename, 
                 unsigned int lineNo);




/******************************************************
*
*  debFree()
*  parameter:
*    mem:      in: memory pointer to free
*  
*  return:
*
*  Purpose:
*   calls free and removes data from double linked list
*
*  Limitations:
*
******************************************************/
void   debFree(void *mem);

/******************************************************
*
*  debGetMem()
*  parameter:
*  
*  return:
*    size_t: overall amount of allocated memory
*
*  Purpose:
*   returns overall amount of allocated memory
*
*  Limitations:
*
******************************************************/
size_t debGetMem(void);

/******************************************************
*
*  debShowAlloc()
*  parameter:
*  
*  return:
*
*  Purpose:
*    prints statistic of dynamic memory usage (contents
*    of double linked list)
*
*  Limitations:
*
******************************************************/
void   debShowAlloc(void);

/******************************************************
*
*  setMemDebugLevel()
*  parameter:
*    level: New debug level
*  
*  return:
*
*  Purpose:
*    sets debug level, amount debug output depends on this
*    value
*
*  Limitations:
*    level must be one of the following values:
*      DL_Information: everything is printed
*      DL_Warning:     warnings and errors are printed 
*      DL_Minor:       errors are printed
*      DL_Major:       Major and Fatal errors are printed
*      DL_Fatal:       only Fatal errors are printed
*      DL_None:        no output at all
*
******************************************************/
void   setMemDebugLevel(DebugLevel_e level);
#endif

#endif

