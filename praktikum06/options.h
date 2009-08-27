#ifndef _OPTIONS_H_
#define _OPTIONS_H_
/**************************************************************************
*
*  options.h
*
* Author:  Prof. Dr.-Ing. Juergen Wuebbelmann
* Purpose: interface for debug output. 
*          if compile flag OPTIONCHECK is used, debug output can be controled
*          by global variable Options_t options.verbose at run time.
*          Else, compile flag DEBUG controls debug output at compile time.
* created: 2007-12-14
* last modified:   
*
*
***************************************************************************/
#ifndef TRUE
#define TRUE (1==1)
#define FALSE !TRUE
#endif

typedef unsigned char boolean;

#ifdef OPTIONCHECK
 #define DEBUGOUT if (TRUE == options.verbose) printf
 #define DEBUGFFLUSH  if (TRUE == options.verbose) fflush(stdout)

 typedef struct options_t
 {
   boolean verbose;
   unsigned int noOfClients;
 } Options_t;
 extern Options_t options;

#else
 #ifdef DEBUG
  #define DEBUGOUT printf
  #define DEBUGFFLUSH  fflush(stdout)
 #else
  #define DEBUGOUT
  #define DEBUGFFLUSH
 #endif
#endif



#endif
