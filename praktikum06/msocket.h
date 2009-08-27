#ifndef __M_SOCKET_H__
#define __M_SOCKET_H__
/******************************************************************
*
* msocket.h
*
* Purpose: API for GET to web server
*
* Author J. Wuebbelmann
*
* History: 20070130 first implementation
*
********************************************************************/

/* specific data types: */
typedef enum 
{
  noError    =  0,
  fatalError = -1
} errorValue;



/* API functions: */

/********************************************************
*
* askServer()
*
* parameter:
*  address  : URL of server
*  page     : requested page on server
*  filename : Filename, where to save reply. If NULL: 
*             stdout is used.
*
* returns
*   noError on success, else fatalError
*
**********************************************************/
extern errorValue askServer(const char* address, 
                            const char* page, 
                            const char* filename);


#endif
