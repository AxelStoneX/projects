/*============================================================================*
 * FILE:                     L L _ P O S I X . H
 *============================================================================*
 *
 * COPYRIGHT (C) 2006 - 2012 BY
 *          GE INTELLIGENT PLATFORMS, INC., GOLETA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY GE 
 *          INTELLIGENT PLATFORMS.
 *
 *===========================================================================*
 *
 * FUNCTION:    Header file for low-level POSIX pthread interface.
 *
 *              Warning: this file is shared by many Condor products, so 
 *              please exercise caution when making modifications.
 *
 *===========================================================================*/

/* $Revision:  1.07 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/09/2006    initial. bch
  05/25/2006    changed CEI_SIG_TIMER and CEI_SIG_HINT values, added
                 CEI_SIG_HINT_MAX, modified CEI_SIGNAL parameter list. bch
  09/14/2006    changed values for POSIX_ERROR and PTHREAD_COND_TIMEOUT, and 
                 added POSIX_SUCCESS. bch
  03/13/2007    changed CEI_MUTEX_LOCK, CEI_MUTEX_UNLOCK, and
                 CEI_MUTEX_TRYLOCK macros to functions. moved debugging macros
                 to posix.c. bch
  05/18/2007    changed iDelay in CEI_WAIT_FOR_EVENT from CEI_UINT to CEI_INT,
                 added define for CEI_UINT. bch
  02/27/2009    removed "lowlevel.h". bch
  03/26/2009    removed define CEI_UINT. bch
  07/19/2012    added CEI_THREAD_EXIT and CEI_HANDLE_DESTROY. bch
*/

#ifndef _LL_POSIX_H_
#define _LL_POSIX_H_

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>


// used by sigaction 
#ifndef SA_RESTART
 #define SA_RESTART 0
#endif
// real-time signals
#define CEI_SIG_TIMER     SIGRTMIN      // signal for timer
#define CEI_SIG_HINT      SIGRTMIN + 1  // signal for hardware interrupt
#define CEI_SIG_HINT_MAX  SIGRTMAX
// threads
#define CEI_CREATE   0
#define CEI_DESTROY  1
// timer
#define INFINITE  0
// errors
#define POSIX_ERROR          -1
#define POSIX_SUCCESS        0
#define PTHREAD_COND_TIMEOUT 1

#define CEI_MALLOC(a) malloc(a)
#define CEI_FREE(a) free(a)
#define CEI_MUTEX pthread_mutex_t
#define CEI_EVENT pthread_cond_t
#define CEI_THREAD pthread_t
#define CEI_HANDLE int

// prototypes
CEI_INT CEI_MUTEX_CREATE(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_LOCK(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_UNLOCK(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_TRYLOCK(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_DESTROY(CEI_MUTEX* pMutex);
CEI_INT CEI_EVENT_CREATE(CEI_EVENT* pEvent);
CEI_INT CEI_EVENT_DESTROY(CEI_EVENT* pEvent);
CEI_INT CEI_THREAD_CREATE(CEI_THREAD* pThread, CEI_INT iPriority, CEI_VOID* pFunc, CEI_VOID* pFuncArg);
CEI_INT CEI_THREAD_DESTROY(CEI_THREAD* pThread);
CEI_INT CEI_WAIT_FOR_EVENT(CEI_EVENT* pEvent, CEI_INT iDelay, CEI_MUTEX* pMutex);
CEI_INT CEI_EVENT_SIGNAL(CEI_EVENT* pEvent, CEI_MUTEX* pMutex);
CEI_INT CEI_TIMER(CEI_UINT cflag, CEI_UINT interval, CEI_VOID* pFunc);
CEI_INT CEI_SIGNAL(CEI_UINT cflag, CEI_INT signal, CEI_VOID* pFunc);
CEI_INT CEI_HANDLE_DESTROY(CEI_VOID *pHandle);
CEI_INT CEI_THREAD_EXIT(CEI_VOID* pStatus, CEI_VOID* pVal);
	

// debugging
#ifdef LL_DEBUG
 #define CHK_STATUS(status, funcName) \
  if(status != 0) {  \
    CEI_CHAR sFunc[40] = funcName; \
    printf(" <LL_DEBUG> error in %s - status: %d\n", sFunc, status);  \
  }
 #define PRNT_DIAG(id, val) \
  { \
  CEI_CHAR sId[100] = id; \
  printf(" <LL_DEBUG> %s: 0x%lx\n", sId, (CEI_ULONG)val); \
  } 
 #define PRNT_DIAG2(id, val1, val2) \
  { \
  CEI_CHAR sId[100] = id; \
  printf(" <LL_DEBUG> %s: 0x%lx, 0x%lx\n", sId, (CEI_ULONG)val1, (CEI_ULONG)val2); \
  } 
#else
 #define CHK_STATUS(status, funcName)
 #define PRNT_DIAG(id, val)
 #define PRNT_DIAG2(id, val1, val2)
#endif // LL_DEBUG


#endif  // _LL_POSIX_H_
