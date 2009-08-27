#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "msocket.h"
#include "options.h"
#ifdef MEMDEBUG
#include "memcheck.h"
#endif
/******************************************************************
*
* socket.c
*
* Purpose: simple socket handling.
*          opens a socket and sends a GET request to socket.
*          The reply is written in a file.
*
* Author J. Wuebbelmann
*
* History: 20070130 first implementation
* based on source:
* http://www.troubleshooters.com/codecorn/sockets/
*
*
* Dependencies: This file was tested with the following platforms:
*   -Win32 using CYGWIN, gcc 3.4.4
*    No support of reentrant gethostbyname() function. A mutex is used
*    to protect function. Link with -lpthread flag
*   -Linux: gcc version 4.1.0 (SUSE Linux)
*    Thread save version of gethostbyname_r().
*    Link with -lpthread -lnsl flags
*   -Solaris SUNOS 5.11 on  i386 Architecture, gcc 3.4.3
*    Link with -lsocket -lnsl Flags
*    A different version of gethostbyname_r() is used.
*   -Solaris 5.8 on Sparc Architecture, gcc 2.95.1
*    Link with -lsocket -lnsl Flags
*
* List of limitations: 
*     - no timing issues, .... 
*       Don't use this as a base for a real life application!
*
*
********************************************************************/

/* some user defines: */
/* THREADSAVE may be undefined to show some bad surprises in */
/* a multithreading application                              */
#ifndef THREADSAVE
#define THREADSAVE
#endif
#define buflen 512

#define WAITSEC 20

/* Test... */
#define eq ==

#define isSuccess(x) (noError eq (x))

#if defined (__CYGWIN__) && defined (THREADSAVE)
#include <pthread.h>
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
#endif


/* fixed port number for HTTP protocol */
const unsigned int portno = 80;
const char         sendFormat[] = "GET %s HTTP/1.0\nUser-Agent: Mozilla/5.0 (X11; U; SunOS i86pc; en-US; rv:1.8.1.4) Gecko/20070622 Firefox/2.0.0.4\n\n"; 




/********************************************************
*
* printFromSocket()
*
* parameter:
*  sd  : socket 
*  buf : character buffer of size buflen
*  fd  : file descriptor
*
* returns
*  int: 0 on success, else -1
*
**********************************************************/
int printFromSocket(int sd, char *buf, int fd)
{
  int waitTime = time(NULL);
  int timeOut = 0;
  int len = buflen+1;
  int continueflag=1;
  while( ((len > 0) || continueflag) /* quit b4 U read an empty socket */
     &&  (timeOut eq 0) )
  {
     len = read(sd, buf, buflen);
     write(fd,buf,len);
     if (len) continueflag = 0;
     if (time(NULL) - waitTime > WAITSEC) timeOut = 1;
  }
  if (timeOut)
  {
    //DEBUGOUT("Timeout occured\n"); 
  }
  return(continueflag);
}


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
errorValue askServer(const char *address, const char* page, const char* filename) 
{
  int    sd            = 0;  /*  socket descriptor */
  struct sockaddr_in sin;
  struct hostent *host = NULL;
  char  *sendMessage   = NULL;
  char  *buf           = NULL;
  int   fd             = 0;   /* file descriptor  */
  int   exitStatus     = noError;
  mode_t mode  =  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ;
  int myflags  =   O_RDWR | O_CREAT | O_TRUNC;
#ifdef THREADSAVE
#ifdef linux
  int retValue = 0;
#endif
   struct hostent res;
#ifndef  __CYGWIN__
  char buffer[buflen];
#endif
  int h_errnop;
#endif

  if (NULL eq address)
  {
    fprintf(stderr,"askServer: called with NULL address");
    exitStatus = fatalError;
  }
  if (NULL eq page)
  {
    fprintf(stderr,"askServer: called with NULL page");
    exitStatus = fatalError;  
  }
  
  /* open socket  */
  if (isSuccess(exitStatus))
  {
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 eq sd)
    {
      perror("socket:");
      sd = 0;
      exitStatus = fatalError;
    }
  }
  
  if (isSuccess(exitStatus))
  {
/* resolve address */
#ifndef THREADSAVE
    host = gethostbyname(address);
#else
    //DEBUGOUT("Try to retrieve hostname %s\n", address);
#ifdef linux
    retValue = gethostbyname_r(address,
                               &res,
                               buffer,
                               buflen,
                               &host,
                               &h_errnop);
    if(retValue)
#elif defined (__SVR4) && defined (__sun)
    host = gethostbyname_r(address,
                           &res,
                           buffer,
                           buflen,
                           &h_errnop);
    if(NULL eq host)
#elif defined ( __CYGWIN__)
    pthread_mutex_lock(&mut);
    host = gethostbyname(address);
    if(host)
    {
      memcpy(&res, host, sizeof(struct hostent));
      host = &res;
    }
    pthread_mutex_unlock(&mut);
    if(!host)
#else
#error ... Wrong System... Only tested with cygwin linux and solaris
#endif
    {
      errno = h_errnop;
      perror("gethostbyname_r");
      exitStatus = fatalError;
    }
#endif           /*  !THREADSAVE */
  }

  if (isSuccess(exitStatus))
  {
    /**** get data buffer *******/
    if ( (buf = (char*) malloc(buflen * sizeof(char)) ) eq NULL)
    {
     perror("malloc");
     exitStatus = fatalError;
    }
  }

  /*** PLACE DATA IN sockaddr_in struct ***/
  if (isSuccess(exitStatus))
  {
    memcpy(&sin.sin_addr.s_addr, host->h_addr, host->h_length);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(portno);

    /*** CONNECT SOCKET TO THE SERVICE DESCRIBED BY sockaddr_in struct ***/
    //DEBUGOUT("try to connect to %s (%s)\n",host->h_name, address);
    if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
      perror("connecting");
      exitStatus = fatalError;
    }
    else
    {
      //DEBUGOUT("connected to %s\n", host->h_name);
    }
  }

  if (isSuccess(exitStatus))
  {
    if(filename)
    {
      if ( (fd = open(filename, myflags, mode)) < 0)
      {
        fd = STDOUT_FILENO;
        exitStatus =  fatalError;
      }
    }
    else
    {
      fd = STDOUT_FILENO;
      filename = "stdout";
    }
    //DEBUGOUT("opened %s\n",filename);
  }

  if (isSuccess(exitStatus))
  {
    /* allocate memory for message */
    sendMessage = (char *)malloc( (strlen(page)  + strlen(sendFormat)) * sizeof(char) );
    if (NULL eq sendMessage)
    {
      exitStatus =  fatalError;
    }
  }

  if (isSuccess(exitStatus))
  {
    sprintf(sendMessage,sendFormat,page);
    
    write(sd, sendMessage, strlen(sendMessage));
    printFromSocket(sd, buf, fd);
  }

  if (buf)         free(buf);
  if (sendMessage) free(sendMessage);

  if (sd)          close(sd);
  if (STDOUT_FILENO != fd)  
  { 
     close(fd);
     //DEBUGOUT("closed %s\n",filename);
  }
   

  return exitStatus;
}
