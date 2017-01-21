/*============================================================================*
 * FILE:                     CEI_POSIX_INTERRUPT_FUNCTIONS.C
 *============================================================================*
 *
 * COPYRIGHT (C) 1997 - 2010 BY
 *          GE INTELLIGENT PLATFORMS, INC., SANTA BARBARA, CALIFORNIA
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
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY GE INTELLIGENT  
 *          PLATFORMS.
 *
 *===========================================================================*
 *
 * FUNCTION:   BusTools/1553-API Library:
 *             Low-level POSIX specific interrupt, thread, and timer functons.
 *
 * EXTERNAL ENTRY POINTS:
 *
 * INTERRUPT AND THREAD FUNCTIONS:
 *
 *   CEI_THREAD_DESTROY    -  Destroys the thread
 *   CEI_EVENT_CREATE      -  Creates an event
 *   CEI_EVENT_DESTROY     -  Destroys the event
 *   CEI_THREAD_CREATE     -  Creates a thread from a pasted function
 *   CEI_WAIT_FOR_EVENT    -  Infinite or timed wait for event
 *   CEI_EVENT_SIGNAL      -  Singals the event
 *   CEI_MUTEX_CREATE      -  Create a MUTEX
 *   CEI_MUTEX_DESTROY     -  Destroys a Mutex
 *
 * SYSTEM SPECIFIC TIME FUNCTIONS:
 *   CEI_GET_TIME          -  Gets teh current system time in microseconds
 *   PerformanceCounter    -  Higly accurate time
 *
 *===========================================================================*/

/* $Revision:  6.32 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  1/31/2006    Creation date.

*/

#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "target_defines.h"
#include "lowlevel.h"

timer_t iTimer;
static struct timespec wdelay;
struct itimerspec tValue;   /* time to be set */

static pthread_attr_t pt_attr;
static struct sched_param pt_param;
static int pattr_cnt=0;

/***************************************************
*  PTHREAD INTERRUPT FUNCTIONS
***************************************************/

/****************************************************************
*
*  PROCEDURE - CEI_THREAD_DESTROY
*
*  FUNCTION
*     Destroys the thread.
*
*  RETURNS
*     BTD_OK
****************************************************************/
CEI_INT CEI_THREAD_DESTROY(CEI_THREAD * pThread)
{
   pthread_kill(*pThread, 9);
   if(--pattr_cnt == 0)
      pthread_attr_destroy(&pt_attr);

   return BTD_OK;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_CREATE
*
*  FUNCTION
*     Create pthread Event.
*
*  RETURNS
*     BTD_OK
*     BTD_ERR_BADOPEN
****************************************************************/
CEI_INT CEI_EVENT_CREATE(CEI_EVENT * pEvent)
{
   int status;

   status = pthread_cond_init(pEvent, NULL);
   if ( status )
   {
      return BTD_ERR_BADOPEN;
   }
   return BTD_OK;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_DESTROY
*
*  FUNCTION
*     Destroys pthread Event.
*
*  RETURNS
*     BTD_OK
****************************************************************/
CEI_INT CEI_EVENT_DESTROY(CEI_EVENT * pEvent)
{
   pthread_cond_destroy(pEvent);
   return BTD_OK;
}

/****************************************************************
*
*  PROCEDURE - p_attr_init
*
*  FUNCTION
*     initializes the pthread_attribute.
*
*  RETURNS
*     BTD_OK
****************************************************************/
CEI_INT p_attr_init(CEI_INT iPriority)
{
   int status;

   status = pthread_attr_init(&pt_attr);
   if(status)
      return BTD_ERR_BADOPEN;

   status = pthread_attr_setschedpolicy(&pt_attr, SCHED_FIFO);
   if(status)
      return BTD_ERR_BADOPEN;

   pt_param.sched_priority = iPriority;
   status = pthread_attr_setschedparam(&pt_attr,&pt_param);
   if(status)
      return BTD_ERR_BADOPEN;

   status = pthread_attr_setdetachstate(&pt_attr,PTHREAD_CREATE_DETACHED);
   if(status)
      return BTD_ERR_BADOPEN;

   return BTD_OK;
}

/****************************************************************
*
*  PROCEDURE - CEI_THREAD_CREATE
*
*  FUNCTION
*     Create POSIX Thread.
*
*  RETURNS
*     BTD_OK
*     BTD_ERR_BADOPEN
****************************************************************/
CEI_INT CEI_THREAD_CREATE(CEI_THREAD * pThread, CEI_INT iPriority,void * func, void * pFifo)
{
   int status;

   if(pattr_cnt == 0)
   {
      status = p_attr_init(iPriority);
      if(status)
         return status;
      pattr_cnt=1;
   }
   else
      pattr_cnt++;     

   status = pthread_create(pThread, &pt_attr, func, pFifo);

   if ( status )
   {
      return BTD_ERR_BADOPEN;
   }
   return BTD_OK;
}


/****************************************************************
*
*  PROCEDURE - CEI_WAIT_FOR_EVENT
*
*  FUNCTION
*     Waits for specified event.
*
*  RETURNS
*     Nothing
****************************************************************/
CEI_INT CEI_WAIT_FOR_EVENT(CEI_EVENT *pEvent, CEI_INT iDelay, CEI_MUTEX *pMutex)
{
   CEI_INT retval;

   if(iDelay==INFINITE)
   {
      pthread_mutex_lock(pMutex);
      retval = pthread_cond_wait(pEvent,pMutex);
      pthread_mutex_unlock(pMutex);
      return retval;
   }
   else
   {
      pthread_mutex_lock(pMutex);
      clock_gettime(CLOCK_REALTIME, &wdelay);
      wdelay.tv_nsec = (wdelay.tv_nsec)+(iDelay*1000000);
      retval = pthread_cond_timedwait(pEvent,pMutex,&wdelay);         
      pthread_mutex_unlock(pMutex);
      switch(retval)
      {
         case 0:
            return 0;
         case ETIMEDOUT:
            return BTD_EVENT_WAIT_TIMEOUT;
         default:
            return BTD_EVENT_WAIT_UNKNOWN;
      }
   }
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_SIGNAL
*
*  FUNCTION
*     Sets the specified event for all waiting threads.
*
*  RETURNS
*     Nothing
****************************************************************/
CEI_INT CEI_EVENT_SIGNAL(CEI_EVENT *pEvent, CEI_MUTEX *pMutex)
{
   int retval;
   pthread_mutex_lock(pMutex);
   retval =  pthread_cond_signal(pEvent);
   pthread_mutex_unlock(pMutex);
   return retval;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_CREATE
*
*  FUNCTION
*     Creates a pthread mutex.
*
*  RETURNS
*     BTD_OK
****************************************************************/
CEI_INT CEI_MUTEX_CREATE(CEI_MUTEX *pmutex)
{
   return pthread_mutex_init(pmutex,NULL);
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_DESTROY
*
*  FUNCTION
*     Destroys a pthread mutex.
*
*  RETURNS
*     BTD_OK
****************************************************************/
CEI_INT CEI_MUTEX_DESTROY(CEI_MUTEX *pmutex)
{
   pthread_mutex_destroy(pmutex);
   return BTD_OK;
}

/***************************************************************
*  CEI O/S Specific time functions
***************************************************************/

/****************************************************************
*
*  PROCEDURE - CEI_GET_TIME
*
*  FUNCTION
*     Destroys a Windows mutex.
*
*  RETURNS
*     Time in milliseconds
****************************************************************/
unsigned long CEI_GET_TIME(void)
{
   struct timespec ts;        /* Current time, use to get milliseconds */
   
   clock_gettime(CLOCK_REALTIME, &ts);
   return(unsigned long)ts.tv_nsec/1000000.;
}

/* ***************************************************************
*
*  PROCEDURE - PerformanceCounter
*
*  FUNCTION
*     Destroys a Windows mutex.
*
*  RETURNS
*     Time in milliseconds
*************************************************************** */
CEI_UINT64 PerformanceCounter(void)
{
   clockid_t clock;
   struct timespec ts;        /* Current time, use to get milliseconds */

   clock_gettime(clock, &ts);
   return (CEI_UINT64)((ts.tv_sec*1000000)+ts.tv_nsec/1000);
}
