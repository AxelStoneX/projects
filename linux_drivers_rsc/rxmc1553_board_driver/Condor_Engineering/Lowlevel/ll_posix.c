/*============================================================================*
 * FILE:                       L L _ P O S I X . C
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
 * FUNCTION:   Low-level POSIX pthread interface functions
 *
 * EXTERNAL ENTRY POINTS:
 *
 *  Mutex functions:
 *   CEI_MUTEX_CREATE      -  create a mutex
 *   CEI_MUTEX_LOCK        -  lock a mutex
 *   CEI_MUTEX_TRYLOCK     -  try locking a mutex
 *   CEI_MUTEX_UNLOCK      -  unlock a mutex
 *   CEI_MUTEX_DESTROY     -  destroy a mutex
 *
 *  Event functions:
 *   CEI_EVENT_CREATE      -  creates an event
 *   CEI_EVENT_DESTROY     -  destroy an event
 *   CEI_WAIT_FOR_EVENT    -  wait (infinite or timed) for event
 *   CEI_EVENT_SIGNAL      -  signal an event
 *
 *  Thread functions:
 *   CEI_THREAD_CREATE     -  create a thread
 *   CEI_THREAD_DESTROY    -  destroys a thread
 *   CEI_TIMER             -  create and destroy timer (ms)
 *   CEI_SIGNAL            -  create and destroy a real-time signal
 *   CEI_THREAD_EXIT       -  exit thread
 *   CEI_HANDLE_DESTROY    -  close a handle
 *
 *===========================================================================*/

/* $Revision:  1.09 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/09/2006    Initial. bch
  05/15/2006    modified time delay for CEI_WAIT_FOR_EVENT, modified CEI_SIGNAL
                 argument list, modified CEI_SIGNAL signal configuration. bch
  09/14/2006    replaced BTD_OK with POSIX_SUCCESS. bch
  03/13/2007    added CEI_MUTEX_LOCK, CEI_MUTEX_TRYLOCK, and CEI_MUTEX_UNLOCK
                 functions. moved debugging macros from posix.h. bch
  05/18/2007    changed iDelay in CEI_WAIT_FOR_EVENT from CEI_UINT to CEI_INT. bch
  06/15/2007    modified CEI_WAIT_FOR_EVENT and CEI_MUTEX_CREATE. bch
  11/18/2008    modified CEI_MUTEX_CREATE, CEI_EVENT_SIGNAL, and 
                 CEI_THREAD_DESTROY. bch
  02/27/2009    added "lowlevel.h". bch
  03/26/2009    replaced "lowlevel.h" with "cei_types.h". replaced "CEI_U32"
                 with "CEI_UINT". bch
  07/19/2012    added CEI_THREAD_EXIT and CEI_HANDLE_DESTROY. bch
*/

#include "cei_types.h"
#include "ll_posix.h"


/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_CREATE
*
*  FUNCTION
*     Creates a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_CREATE(CEI_MUTEX* pMutex) {
   CEI_INT ret, status;
   pthread_mutexattr_t pt_mattr;

   ret=status=0;

   PRNT_DIAG("CEI_MUTEX_CREATE", pMutex)

   memset(pMutex, 0, sizeof(CEI_MUTEX));

   status = pthread_mutexattr_init(&pt_mattr);
   CHK_STATUS(status, "pthread_mutexattr_init")

//   status = pthread_mutexattr_setprotocol(&pt_mattr, PTHREAD_PRIO_NONE);
//   CHK_STATUS(status, "pthread_mutexattr_setprotocol");

  #ifdef _GNU_SOURCE
   status = pthread_mutexattr_setpshared(&pt_mattr, PTHREAD_PROCESS_SHARED); // default - PTHREAD_PROCESS_PRIVATE
   CHK_STATUS(status, "pthread_mutexattr_setpshared")
  #ifdef _REENTRANT
   status = pthread_mutexattr_settype(&pt_mattr, PTHREAD_MUTEX_RECURSIVE); // PTHREAD_MUTEX_ERRORCHECK
  #else
   status = pthread_mutexattr_settype(&pt_mattr, PTHREAD_MUTEX_DEFAULT); // PTHREAD_MUTEX_ERRORCHECK
  #endif
   CHK_STATUS(status, "pthread_mutexattr_settype")
  #endif

   ret = pthread_mutex_init(pMutex, &pt_mattr);
   CHK_STATUS(ret, "pthread_mutex_init")

   status = pthread_mutexattr_destroy(&pt_mattr);
   CHK_STATUS(status, "pthread_mutexattr_destroy")

   if(ret != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_LOCK
*
*  FUNCTION
*     Lock a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_LOCK(CEI_MUTEX* pMutex) {
   CEI_INT status=0;

   PRNT_DIAG("CEI_MUTEX_LOCK", pMutex)

   status = pthread_mutex_lock(pMutex);
   CHK_STATUS(status, "pthread_mutex_lock")
   if(status != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_TRYLOCK
*
*  FUNCTION
*     Try locking a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_TRYLOCK(CEI_MUTEX* pMutex) {
   CEI_INT status=0;

   PRNT_DIAG("CEI_MUTEX_TRYLOCK", pMutex)

   status = pthread_mutex_trylock(pMutex);
   CHK_STATUS(status, "pthread_mutex_trylock")
   if(status != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_UNLOCK
*
*  FUNCTION
*     Unlock a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_UNLOCK(CEI_MUTEX* pMutex) {
   CEI_INT status=0;

   PRNT_DIAG("CEI_MUTEX_UNLOCK", pMutex)

   status = pthread_mutex_unlock(pMutex);
   CHK_STATUS(status, "pthread_mutex_unlock")
   if(status != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_DESTROY
*
*  FUNCTION
*     Destroys a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_DESTROY(CEI_MUTEX* pMutex) {
   CEI_INT status=0;

   PRNT_DIAG("CEI_MUTEX_DESTROY", pMutex)

   status = pthread_mutex_destroy(pMutex);
   CHK_STATUS(status, "pthread_mutex_destroy")
   if(status != 0)
     return POSIX_ERROR;

   memset(pMutex, 0, sizeof(CEI_MUTEX));

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_CREATE
*
*  FUNCTION
*     Create a pthread event.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_EVENT_CREATE(CEI_EVENT* pEvent) {
   CEI_INT status=0;

   PRNT_DIAG("CEI_EVENT_CREATE", pEvent)

   memset(pEvent, 0, sizeof(CEI_EVENT));

   status = pthread_cond_init(pEvent, NULL);
   CHK_STATUS(status, "pthread_cond_init")
   if(status != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_DESTROY
*
*  FUNCTION
*     Destroys a pthread event.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_EVENT_DESTROY(CEI_EVENT* pEvent) {
   CEI_INT status=0;

   PRNT_DIAG("CEI_EVENT_DESTROY", pEvent)

   status = pthread_cond_destroy(pEvent);
   CHK_STATUS(status, "pthread_cond_destroy")
   if(status != 0)
     return POSIX_ERROR;

   memset(pEvent, 0, sizeof(CEI_EVENT));

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_WAIT_FOR_EVENT
*
*  FUNCTION
*     Waits for specified event.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
*     PTHREAD_COND_TIMEOUT
****************************************************************/
CEI_INT CEI_WAIT_FOR_EVENT(CEI_EVENT* pEvent, CEI_INT iDelay, CEI_MUTEX* pMutex) {
   CEI_INT ret,status;
   struct timespec time, cur_time;

   ret=status=POSIX_SUCCESS;

   PRNT_DIAG("CEI_WAIT_FOR_EVENT", pEvent)

   ret = CEI_MUTEX_LOCK(pMutex);
   CHK_STATUS(ret, "pthread_mutex_lock")

   if(iDelay == INFINITE) {
     // block the calling thread until signaled. Sequence:  lock mutex,
     // mutex is released while the thread waits, mutex locked again once
     // thread is signaled, and finally mutex is unlocked.
     ret = pthread_cond_wait(pEvent, pMutex);
     CHK_STATUS(ret, "pthread_cond_wait")
     if(ret != 0)
       status = POSIX_ERROR;
   }
   else {
     time.tv_sec = iDelay/1000;  
     if(iDelay >= 1000) 
       iDelay = iDelay%1000;
     time.tv_nsec = iDelay*1000000;  // ms resolution
     if(clock_gettime(CLOCK_REALTIME, &cur_time) == 0) {
       time.tv_sec += cur_time.tv_sec;
       time.tv_nsec += cur_time.tv_nsec;
       ret = pthread_cond_timedwait(pEvent, pMutex, &time);
       if(ret == ETIMEDOUT) {
        #ifdef LL_DEBUG
         printf(" <LL_DEBUG> timeout in pthread_cond_timedwait\n"); 
        #endif
         status = PTHREAD_COND_TIMEOUT;  
       }
       else {
         CHK_STATUS(ret, "pthread_cond_timedwait")
       }
     }
     else {
       printf("Error in clock_gettime - errno: %d\n",  errno);
       status = POSIX_ERROR;
     }
   }
   ret = CEI_MUTEX_UNLOCK(pMutex);
   CHK_STATUS(ret, "pthread_mutex_unlock")

   return status;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_SIGNAL
*
*  FUNCTION
*     Sets the specified event for all waiting threads.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_EVENT_SIGNAL(CEI_EVENT* pEvent, CEI_MUTEX* pMutex) {
   CEI_INT status=0;

   PRNT_DIAG("CEI_EVENT_SIGNAL", pEvent)

   status = pthread_cond_signal(pEvent);
   CHK_STATUS(status, "pthread_cond_signal")

   if(status != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_THREAD_CREATE
*
*  FUNCTION
*     Config and create a POSIX Thread.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_THREAD_CREATE(CEI_THREAD* pThread, CEI_INT iPriority, CEI_VOID* pFunc, CEI_VOID* pFuncArg) {
   CEI_INT ret,status;
   pthread_attr_t pt_attr;
   struct sched_param pt_param;

   ret=status=0;

   PRNT_DIAG("CEI_THREAD_CREATE", pThread)

   memset(pThread, 0, sizeof(CEI_THREAD));

   // configure pthread attribute
   status = pthread_attr_init(&pt_attr);
   CHK_STATUS(status, "pthread_attr_init")
   if(status != 0)
     return POSIX_ERROR;

   status = pthread_attr_setinheritsched(&pt_attr, PTHREAD_EXPLICIT_SCHED);  // PTHREAD_INHERIT_SCHED
   CHK_STATUS(status, "pthread_attr_setinheritsched")

   status = pthread_attr_setdetachstate(&pt_attr, PTHREAD_CREATE_DETACHED);  // PTHREAD_CREATE_JOINABLE
   CHK_STATUS(status, "pthread_attr_setdetachstate")

   status = pthread_attr_setschedpolicy(&pt_attr, SCHED_FIFO);  // SCHED_RR
   CHK_STATUS(status, "pthread_attr_setschedpolicy")

   // valid range for SCHED_FIFO and SCHED_RR is 1 to 99  
   if(iPriority < 1) iPriority = 1;
   pt_param.sched_priority = iPriority;
   status = pthread_attr_setschedparam(&pt_attr, &pt_param);
   CHK_STATUS(status, "pthread_attr_setschedparam")

   // status = pthread_attr_setscope(&pt_attr, PTHREAD_SCOPE_PROCESS);  //PTHREAD_SCOPE_SYSTEM
   // CHK_STATUS(status, "pthread_attr_setscope");

   // create pthread
   ret = pthread_create(pThread, &pt_attr, pFunc, pFuncArg);
   CHK_STATUS(ret, "pthread_create")
   // if caller does not have permissions then try with default attributes
   if(ret == EPERM) {
    #ifdef LL_DEBUG
     printf(" <LL_DEBUG> using default attributes for pthread_create\n");
    #endif
     ret = pthread_create(pThread, NULL, pFunc, pFuncArg);
     CHK_STATUS(ret, "pthread_create (default)")
   }
  #ifdef LL_DEBUG
   {
     int policy=0;
     if(ret == 0) {
       ret = pthread_getschedparam(*pThread, &policy, &pt_param);
       // policy:  refer to `sched_policies.h`
       printf(" <LL_DEBUG> CEI_THREAD schedule attributes: policy (0x%lx), priority (%d)\n", (CEI_ULONG)policy, pt_param.sched_priority);
     }
   }
  #endif

   status = pthread_attr_destroy(&pt_attr);
   CHK_STATUS(status, "pthread_attr_destroy")

   if(ret != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_THREAD_DESTROY
*
*  FUNCTION
*     Destroys a thread.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_THREAD_DESTROY(CEI_THREAD* pThread) {
   CEI_INT status=0;

   PRNT_DIAG("CEI_THREAD_DESTROY", pThread)

   // if thread exists, then cancel it  
   if(pthread_kill(*pThread, 0) == 0) { 
     status = pthread_cancel(*pThread);
     CHK_STATUS(status, "pthread_cancel")
   }

   if(status != 0)
     return POSIX_ERROR;

   memset(pThread, 0, sizeof(CEI_THREAD));

   return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_THREAD_EXIT
*
*  FUNCTION
*     Terminates the thread returning the value provided. This 
*     will not close the thread's handle.
*
*  RETURNS
*     BTD_OK
****************************************************************/
CEI_INT CEI_THREAD_EXIT(CEI_VOID* pStatus, CEI_VOID* pVal) {
  if(pStatus == NULL)
    return POSIX_ERROR;

  pthread_exit(pStatus);

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_TIMER
*
*  FUNCTION
*     Creates or destroys a POSIX timer which calls a user
*     supplied handler at the specified timing interval.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_TIMER(CEI_UINT cflag, CEI_UINT interval, CEI_VOID* pFunc) {
   static timer_t timer_id;
   struct sigaction act;
   static struct sigaction old_act;
   struct itimerspec value;
   struct sigevent evp;
   static CEI_UINT cnfg=0;
  #ifdef LL_DEBUG
   struct timespec res;
  #endif

   PRNT_DIAG("CEI_TIMER", (cnfg << 8) | cflag)

   switch(cflag) {
     case CEI_CREATE:
      if(cnfg != 0)
        break;
      // set function to call for signal CEI_SIG_TIMER
      sigemptyset(&act.sa_mask);
      act.sa_sigaction = pFunc;
      act.sa_flags = SA_RESTART | SA_SIGINFO;
//      sigemptyset(&act.sa_mask);
      if(sigaction(CEI_SIG_TIMER, &act, &old_act) == -1) {
       #ifdef LL_DEBUG
        printf(" <LL_DEBUG> CEI_TIMER - failed sigaction for signal %d, errno: %d\n", CEI_SIG_TIMER, errno);  // EINVAL, EFAULT, EINTR
       #endif
        return POSIX_ERROR;
      }
      cnfg = 0x1;

     #ifdef LL_DEBUG
      if(clock_getres(CLOCK_REALTIME, &res) == -1)
        printf("Failed clock_getres, errno: %d\n", errno);  // EPERM, EINVAL, EFAULT
      else
        printf(" <LL_DEBUG> CEI_TIMER - realtime clock resolution: %ld ns\n", res.tv_nsec);
     #endif

      // create a timer for polling the board by using the CLOCK_REALTIME clock id
      evp.sigev_signo = CEI_SIG_TIMER;
      evp.sigev_value.sival_int = getpid();
      evp.sigev_notify = SIGEV_SIGNAL;
      if(timer_create(CLOCK_REALTIME, &evp, &timer_id) == -1) {
       #ifdef LL_DEBUG
        printf(" <LL_DEBUG> CEI_TIMER - failed timer_create, errno: %d\n", errno);  // EAGAIN, EINVAL, ENOTSUP
       #endif
        CEI_TIMER(CEI_DESTROY, 0, NULL);
        return POSIX_ERROR;
      }
      cnfg |= 0x2;

      // set the timer time
      value.it_value.tv_sec = value.it_interval.tv_sec = 0;
      value.it_value.tv_nsec = value.it_interval.tv_nsec  = interval * 1000000;  // convert milli- to nano-seconds
      if(timer_settime(timer_id, 0, &value, NULL) == -1) {
       #ifdef LL_DEBUG     
        printf(" <LL_DEBUG> CEI_TIMER - failed timer_settime, errno: %d\n", errno);  // EINVAL
       #endif
        CEI_TIMER(CEI_DESTROY, 0, NULL);
        return POSIX_ERROR;
      }
      break;
     case CEI_DESTROY:
      if(cnfg & 0x2) {
        if(timer_delete(timer_id) == -1) {
         #ifdef LL_DEBUG
          printf(" <LL_DEBUG> CEI_TIMER - failed timer_delete, errno: %d\n", errno);  // EINVAL
         #endif
          return POSIX_ERROR;
        }
        cnfg &= ~0x2;
      }
      if(cnfg & 0x1) {
        if((sigaction(CEI_SIG_TIMER, &old_act, NULL)) == -1) {
         #ifdef LL_DEBUG
          printf(" <LL_DEBUG> CEI_TIMER - failed to restore previous sigaction for signal %d, errno: %d\n", CEI_SIG_TIMER, errno);  // EINVAL, EFAULT, EINTR
         #endif
        }
        cnfg &= ~0x1;
      }
      break;
     default:
      printf("CEI_TIMER - invalid cflag.\n");
      return POSIX_ERROR;
   };

   return 0;
}

/****************************************************************
*
*  PROCEDURE - CEI_SIGNAL
*
*  FUNCTION
*     Sets or unsets a user supplied handler to a specified
*     signal.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_SIGNAL(CEI_UINT cflag, CEI_INT signal, CEI_VOID* pFunc) {
  struct sigaction act;
  static struct sigaction old_act;
  static CEI_UINT cnfg_sig=0;
  CEI_UINT cur_sig=0;

  PRNT_DIAG2("CEI_SIGNAL", cflag, cnfg_sig)

  if(signal < ((CEI_UINT)CEI_SIG_HINT) || signal > ((CEI_UINT)CEI_SIG_HINT_MAX))
    return POSIX_ERROR;

  cur_sig = (0x1 << (signal - ((CEI_UINT)CEI_SIG_HINT)));
  switch(cflag) {
    case CEI_CREATE:
     if((cnfg_sig & cur_sig) == 1)
       return 0;
     // signal handling function for hardware interrupts (signal from the device driver)
     act.sa_sigaction = pFunc; // user's handler
     act.sa_flags = SA_RESTART | SA_SIGINFO;
     sigemptyset(&act.sa_mask);
//     sigaddset(&act.sa_mask, signal);  // block signal temporarily
     if(sigaction(signal, &act, &old_act) == -1) {
      #ifdef LL_DEBUG   
       printf(" <LL_DEBUG> CEI_SIGNAL - failed sigaction for signal %d, errno: %d\n", signal, errno);  // EINVAL, EFAULT, EINTR
      #endif
       return POSIX_ERROR;
     }
     cnfg_sig |= cur_sig;
     break;
    case CEI_DESTROY:
     if((cnfg_sig & cur_sig) == 0)
       return 0;
     if(sigaction(signal, &old_act, NULL) == -1) {
      #ifdef LL_DEBUG   
       printf(" <LL_DEBUG> CEI_SIGNAL - failed to restore previous sigaction for signal %d, errno: %d\n", signal, errno);  // EINVAL, EFAULT, EINTR
      #endif
     }
     cnfg_sig &= ~cur_sig;
     break;
    default:
     printf("CEI_SIGNAL - invalid cflag.\n");
     return POSIX_ERROR;
  };

  return 0;
}


/****************************************************************
*
*  PROCEDURE - CEI_HANDLE_DESTROY
*
*  FUNCTION
*
*  RETURNS
*     BTD_OK
****************************************************************/
CEI_INT CEI_HANDLE_DESTROY(CEI_VOID *pHandle) {
  PRNT_DIAG("CEI_HANDLE_DESTROY", pHandle)
  return POSIX_SUCCESS;
}
