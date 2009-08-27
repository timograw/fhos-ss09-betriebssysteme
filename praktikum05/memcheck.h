#ifndef _MEMCHECK_H_
#define _MEMCHECK_H_
/**************************************************************************
*
*  memcheck.h
*
* Author:  Prof. Dr.-Ing. Juergen Wuebbelmann
* Purpose: interface for memcheck
* created: 2007-12-14
* last modified:   2007-12-17: JW
*         - Change in debug levels, added DL_None
*
* to do: no known issues
*
***************************************************************************/


#ifdef MEMDEBUG
  #define M_MALLOC(x)     debMalloc((x), __FILE__, __LINE__)
  #define M_REALLOC(x, y) debRealloc((x),(y), __FILE__, __LINE__)
  #define M_FREE(x)       debFree((x), __FILE__, __LINE__)
  #define M_SHOW()        debShowAlloc()
  #define M_GETUSAGE()    debGetMem()
  #define M_SETDEBUG(x)   setMemDebugLevel((x))
#else
  #define M_MALLOC  malloc
  #define M_REALLOC realloc
  #define M_FREE    free
  #define M_SHOW()
  #define M_GETUSAGE() 0
  #define M_SETDEBUG(x)
#endif

/* available debug levels:  */
extern const int  DL_Information;
extern const int  DL_Warning;
extern const int  DL_Minor;
extern const int  DL_Major;
extern const int  DL_Fatal;
extern const int  DL_None;

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
void   debFree(void *mem, char *filename, unsigned int lineNo);

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
void   setMemDebugLevel(int level);
#endif

#endif

