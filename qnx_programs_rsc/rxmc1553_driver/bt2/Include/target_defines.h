/*============================================================================*
 * FILE:                   T A R G E T _ D E F I N E S . H
 *============================================================================*
 *
 * COPYRIGHT (C) 1994 - 2010 BY
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
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY GE 
 *          INTELLIGENT PLATFORMS.
 *
 *===========================================================================*
 *
 * FUNCTION:    Targer specific header file for BusTools API structures.
 *              Busapi.h includes this file.  Use this file to customize the
 *              installation for new targets.
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  09/12/2001    Target_def.h added to the Busapi.h file
  01/31/2002    Update for modular API
  06/05/2002    Update for v4.52 release
  12/02/2004    Update to support both CEI-x20 and BusTool\1553 APIs
  01/02/2006    Support for CEI Thread and interrupts
  12/15/2006    API version 5.62 Compatible

******************************************************************************/
#ifndef TARGET_DEFINES_H
#define TARGET_DEFINES_H

#ifndef MAX_BTA
#define MAX_BTA   16    /* 16 1553 Channel on up to 16 boards are supported */
#endif //MAX_BTA
/************************************************************************************
 * Target defines for 32-Bit Windows for all supported 1553 boards                  *
 * Set common definition for Windows 32 bit environment                             *
 * Microsoft uses WIN32, Borland uses __WIN32__                                     *
 ************************************************************************************/
#if defined(_WIN32) || defined(WIN32) /* MSVC for 32-bit WINDOWS PM                 */
 #ifndef __WIN32__  /*                                                              */
 #define __WIN32__  /* This label is used in the API source code                    */
 #endif             /*                                                              */ 
 #ifndef _Windows   /*                                                              */ 
 #define _Windows   /*                                                              */ 
 #endif             /*                                                              */ 
#endif              /*                                                              */ 
/************************************************************************************/
#if defined(__WIN32__)     /*                                                       */
  #include <windows.h>     /*                                                       */
 #if !defined NO_HOST_TIME /*                                                       */
  #include <sys/types.h>   /*                                                       */
  #include <sys/timeb.h>   /*                                                       */
 #endif                    /*                                                       */
  #undef USE_BM_DMA        /*                                                       */
#ifdef _MS_VPX_            /* Use AR15-VPX build settings                           */
  #define BM_EXPRESS       /* Define for AR15-VPX                                   */
  #define SYSTEM_MONITOR   /* AR15-VPX System Monitor code                          */
#else                      /*                                                       */
  #undef BM_EXPRESS        /*                                                       */
  #undef SYSTEM_MONITOR    /* AR15-VPX System Monitor code                          */
#endif //_VPX_             /*                                                       */
  #undef WIN32_LEAN_AND_MEAN /*                                                     */
/************************** Use the following for custom Build **********************/
  #define FILE_SYSTEM      /* Include Dump and memory test output functions         */
  #undef  ADD_TRACE        /* Include the function call trace code                  */
  #define DO_BUS_LOADING   /* Include the bus loading code                          */
  #define _USER_INIT_      /*                                                       */
  #define _LABVIEW_        /*                                                       */
  #undef  _BM_USER_DATA_   /* Define for BM User Data (not for use with playback    */
  #undef DEMO_CODE         /* Enable demo mode operation                            */
  #define _USER_DLL_       /*                                                       */
  #define _PLAYBACK_       /*                                                       */
  #define ERROR_INJECTION  /* Include Error Injection into the Build                */
  #define INCLUDE_VME_VXI_1553 /*                                                   */
  #define INCLUDE_PCCD     /* Include the dual channel PCCard-D1553                 */
  #define INCLUDE_LEGACY_PCI /* Support for PCI-1553 and ISA-1553                   */
/************************************************************************************/
  #define CCONV     _stdcall /*                                                     */
  #define NOMANGLE         /*                                                       */
  #define PACKED           /*                                                       */
  #define MSDELAY(a) Sleep(a)    /* Define Sleep marcro                             */
  #define PRAGMA_PACK      /*  local pack directive                                 */
  /* the following are the O/S specific definitions for malloc, free and threads    */
  /* mutexes and events                                                             */  
  #define CEI_MALLOC(a) GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,a)/*                   */
  #define CEI_FREE(a)   GlobalFree(a) /*                                            */
  #define CEI_MUTEX CRITICAL_SECTION /*  define mutex type here                     */
  #define CEI_EVENT HANDLE /*  define event type here                               */
  #define CEI_THREAD HANDLE/*  define thread type here                              */
  #define CEI_MUTEX_LOCK(a) EnterCriticalSection(a) /* Mutex lock MACRO             */
  #define CEI_MUTEX_UNLOCK(a) LeaveCriticalSection(a) /* Mutex unlock MACRO         */
  #define CEI_THREAD_EXIT(a) ExitThread(0) /* Define Thread exit MACRO here         */
#endif  /* (__WIN32__)                                                              */
/************************************************************************************/

/************************************************************************************
 * Target defines for Linux running for PCI Boards.                                 *
 ************************************************************************************/
#if defined(_LINUX_X86_)     /*                                                     */
  #define _UNIX_             /*                                                     */
  #define _GCC_              /* Using GCC compiler                                  */
  #undef  _Windows           /*                                                     */
  #undef  __WIN32__          /*                                                     */
  #define _LINUXBOARDSETUP   /*                                                     */
  #undef  INCLUDE_VME_VXI_1553 /*                                                   */
  #define INCLUDE_LEGACY_PCI /* Support for PCI-1553 and ISA-1553                   */
  #define INCLUDE_PCCD       /*                                                     */
  #undef  INCLUDE_VMIC       /* 32-bit Windows code supports the VMIC VME           */
  #undef  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                               */
  #undef  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)              */
  #define FILE_SYSTEM        /* Include Dump and memory test output functions       */
  #define _PLAYBACK_         /*                                                     */
  #undef  ADD_TRACE          /* Exclude the function call trace code                */
  #undef  DO_BUS_LOADING     /* Exclude the bus loading code                        */
  #undef  DEMO_CODE          /* Enable demo mode operation                          */
  #undef  _USER_INIT_        /*                                                     */
  #undef  _LABVIEW_          /*                                                     */
  #undef  _USER_DLL_         /*                                                     */
  #undef  _BM_USER_DATA_     /* Define for BM User Data (not for use with playback  */
  #include <unistd.h>        /*                                                     */
  #include <string.h>        /*                                                     */
  #include <sys/timeb.h>     /*                                                     */
  #define MSDELAY(p1) usleep(p1*1000) /* Define Sleep marcro                        */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */
  #include <pthread.h>    /* pthread for H/W Interrupts                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
#endif /* end _LINUX_X86_                                                           */
/************************************************************************************/

/************************************************************************************
 * Target defines for Linux on PowerPC                                              *
 ************************************************************************************/
#if defined(_LINUX_PPC_)     /*                                                     */
  #define _UNIX_             /*                                                     */ 
  #define _GCC_              /* Using GCC compiler                                  */
  #undef _Windows            /*                                                     */
  #undef __WIN32__           /*                                                     */
  #define _LINUXBOARDSETUP   /*                                                     */
  #undef INCLUDE_VME_VXI_1553 /*                                                    */
  #undef INCLUDE_PCCD        /*                                                     */
  #define INCLUDE_LEGACY_PCI /* Support for PCI-1553 and ISA-1553                   */
  #undef INCLUDE_VMIC        /*                                                     */
  #undef PPC_SYNC            /*                                                     */
  #define WORD_SWAP          /*                                                     */
  #define NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                               */
  #define NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)              */
  #undef ADD_TRACE           /* Exclude the function call trace code                */
  #undef DO_BUS_LOADING      /* Exclude the bus loading code                        */
  #undef _PLAYBACK_          /*                                                     */
  #define FILE_SYSTEM        /* Include Dump and memory test output functions       */
  #undef _USER_INIT_         /*                                                     */
  #undef _LABVIEW_           /*                                                     */
  #undef _USER_DLL_          /*                                                     */
  #undef  _BM_USER_DATA_     /*                                                     */
  #define _PLAYBACK_         /*                                                     */
  #define ERROR_INJECTION    /* Include Error Injection into the Build              */
  #include <unistd.h>        /*                                                     */
  #include <string.h>        /*                                                     */
  #include <sys/timeb.h>     /*                                                     */
  #define MSDELAY(p1) usleep(p1*1000) /* Define Sleep marcro                        */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */
  #include <pthread.h>    /* pthread for H/W Interrupts                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
#endif   /* end _LINUX_PPC                                                          */
/************************************************************************************/

/************************************************************************************
 * Target defines for Linux running for the PCI-1553, PMC-1553 and the cPCI-1553.   *
 * Also use this for the PCI-1553-IP.                                               */
/************************************************************************************/
#if defined(_LINUX_X86_VME_) /*                                                     */
  #define  _UNIX_            /*                                                     */
  #define  _GCC_             /* Using GCC compiler                                  */
  #undef   _Windows          /*                                                     */
  #undef   __WIN32__         /*                                                     */
  #define  _LINUXVMEBOARDSETUP /*                                                   */
  #define  INCLUDE_VME_VXI_1553 /*                                                  */
  #undef   INCLUDE_PCCD      /*                                                     */
  #undef   INCLUDE_LEGACY_PCI /* Support for PCI-1553 and ISA-1553                  */
  #undef   INCLUDE_VMIC      /* 32-bit Windows code supports the VMIC VME           */
  #undef   PPC_SYNC          /*                                                     */
  #undef   WORD_SWAP         /*                                                     */
  #undef   NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #undef   NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #define  FILE_SYSTEM       /* Include Dump and memory test output functions       */
  #undef   ADD_TRACE         /* Exclude the function call trace code                */
  #undef   DO_BUS_LOADING    /* Exclude the bus loading code                        */
  #undef   _PLAYBACK_        /*                                                     */
  #undef   _USER_INIT_       /*                                                     */
  #undef   _LABVIEW_         /*                                                     */
  #undef   _USER_DLL_        /*                                                     */
  #undef   _BM_USER_DATA_    /*                                                     */
  #undef  ERROR_INJECTION    /* Include Error Injection into the Build              */
  #include <pthread.h>       /* pthread for H/W Interrupts                          */
  #include <unistd.h>        /*                                                     */
  #include <string.h>        /*                                                     */
  #include <sys/timeb.h>     /*                                                     */
  #define MSDELAY(p1) usleep(p1*1000) /* Define Sleep marcro                        */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
#endif /* end _LINUX_X86_                                                           */
/************************************************************************************/

/************************************************************************************
 * Target defines for Solaris SPARC running for the PCI-1553, PMC-1553 and the      *
 *  cPCI-1553.                                                                      */
/************************************************************************************/
#if defined(_SOLARIS_SPARC_)/*                                                      */
  #include <pthread.h>    /* pthread for H/W Interrupts                             */
  #include <unistd.h>     /*                                                        */
  #include <string.h>     /*                                                        */
  #include <sys/timeb.h>  /*                                                        */
  #define _UNIX_          /*                                                        */
  #define _GCC_           /* Using GCC compiler                                     */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #define _SOLARISBOARDSETUP/*                                                      */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #undef  _BM_USER_DATA_  /* Define for BM User Data (not for use with playback     */
  #undef _PLAYBACK_       /*                                                        */
  #define FILE_SYSTEM     /* Include Dump and memory test output functions          */
  #define  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #define  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #undef  WORD_SWAP       /* define the flipw macro                                 */
  #undef  ADD_TRACE       /* Exclude the function call trace code                   */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  #define MSDELAY(p1) usleep(p1*1000) /* Define Sleep marcro                        */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a) /*                                */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a) /*                            */ 
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
#endif /* end _SOLARIS_SPARC_                                                       */
/************************************************************************************/

/************************************************************************************
 * Target defines for Solaris SPARC running for the PCI-1553, PMC-1553 and the      *
 *  cPCI-1553. FORTE COMPILER (NATIVE SUN)                                          */
/************************************************************************************/
#if defined(_SOLARIS_FORTE_)/*                                                      */
  #pragma pack(2)         /* Two byte alignment                                     */
  #include <pthread.h>    /* pthread for H/W Interrupts                             */
  #include <unistd.h>     /*                                                        */
  #include <string.h>     /*                                                        */
  #include <sys/timeb.h>  /*                                                        */
  #define _UNIX_          /*                                                        */
  #define _FORTE_         /* Using FORTE compiler                                   */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #define _SOLARISBOARDSETUP/*                                                      */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #define FILE_SYSTEM     /* Include Dump and memory test output functions          */
  #define  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #define  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #undef  WORD_SWAP       /* define the flipw macro                                 */
  #undef  ADD_TRACE       /* Exclude the function call trace code                   */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a) /*                                */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a) /*                            */ 
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
#endif /* end _SOLARIS_SPARC_                                                       */
/************************************************************************************/

/************************************************************************************
 * Target defines for Solaris x86 running for the PCI-1553, PMC-1553 and the        *
 *  cPCI-1553.                                                                      */
/************************************************************************************/
#if defined(_SOLARIS_X86_)/*                                                        */
  #define _UNIX_          /*                                                        */
  #define _GCC_           /* Using GCC compiler                                     */
  #include <pthread.h>    /*                                                        */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #define _SOLARISBOARDSETUP/*                                                      */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #define FILE_SYSTEM     /* Include Dump and memory test output functions          */
  #undef  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                               */
  #undef  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)              */
  #undef  ADD_TRACE       /* Exclude the function call trace code                   */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  #define MSDELAY(p1) usleep(p1*1000) /* Define Sleep marcro                        */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a) /*                                */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a) /*                            */ 
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
#endif /* end _SOLARIS_X86_                                                         */
/************************************************************************************/

/************************************************************************************
 * Target Defines for VxWorks x86 BSP for the PCI Boards                            * 
 ************************************************************************************/
#if defined (VXW_PCI_X86) /*                                                        */
  #include <sys/types.h>  /*                                                        */
  #include <string.h>     /*                                                        */
  #define _UNIX_          /*                                                        */
  #define _GCC_           /* Using GCC compiler                                     */
  #define VXWORKS         /*                                                        */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #define _X86BOARDSETUP  /*                                                        */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #undef FILE_SYSTEM      /* Include Dump and memory test output functions          */
  #undef  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                               */
  #undef  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)              */
  #undef  ADD_TRACE       /* Exclude the function call trace code                   */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  #undef PLX_DEBUG /*                                                               */
  #define INCLUDE_LEGACY_PCI /* Support for PCI-1553 and ISA-1553                   */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #undef _POSIX_          /* if POSIX dwfined use posis threads else use VxWorks    */
  #ifdef _POSIX_          /*                                                        */
   #include <pthread.h>  /* Use this for VxWorks 5.5 Delete for VxWorks 5.4         */
   #define CEI_MUTEX pthread_mutex_t /* define mutex type here                      */
   #define CEI_THREAD pthread_t      /* define thread type here                     */
   #define CEI_EVENT pthread_cond_t  /* define event variable here                  */
   #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                    */
   #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a) /*                               */
   #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a) /*                           */ 
  /* Use this for VxWorks 5.4 and earlier or if you don't want pthreads             */
  #else /* POSIX theads VxWorks 5.5 or greater                                      */
   #include <msgQLib.h> /* Using Message Queue                                      */
   #include <semLib.h>  /* Use mutual exclusion semaphore                           */
   #include <timers.h>  /* Timer data type include                                  */
   #include <taskLib.h> /* task Lib for taskDelete                                  */
   #define CEI_MUTEX  SEM_ID /* define mutex type here                              */
   #define CEI_THREAD int /* define thread type here                                */
   #define CEI_EVENT  MSG_Q_ID  /* define event variable here                       */
   #define CEI_THREAD_EXIT(a) taskDelete(*a) /*                                     */
   #define CEI_MUTEX_LOCK(a) semTake(*a,WAIT_FOREVER) /*                            */
   #define CEI_MUTEX_UNLOCK(a) semGive(*a) /*                                       */ 
  #endif //_POSIX_             /*                                                   */
  void MSDELAY(int msec); /*                                                        */
#endif   /* end VXW-PCI-X86                                                         */
/************************************************************************************/

/************************************************************************************
 * Target Defines for VxWorks x86 BSP for the VME-1553 and VME-1553-IP.             *
 ************************************************************************************/
#if defined (VXW_VME_X86) /*                                                        */
  #include <sys/types.h>
  #define _UNIX_          /* UNIX TYPE System.                                      */
  #define _GCC_           /* Using GCC compiler                                     */
  #define VXWORKS         /*                                                        */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #define _VMEBOARDSETUP  /*                                                        */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #undef FILE_SYSTEM      /* Include Dump and memory test output functions          */
  #define INCLUDE_VME_VXI_1553 /*                                                   */
  #undef  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                               */
  #undef  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)              */
  #undef  ADD_TRACE       /* Exclude the function call trace code                   */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #undef _POSIX_          /* if POSIX dwfined use posis threads else use VxWorks    */
  #ifdef _POSIX_          /*                                                        */
   #include <pthread.h>  /* Use this for VxWorks 5.5 Delete for VxWorks 5.4         */
   #define CEI_MUTEX pthread_mutex_t /* define mutex type here                      */
   #define CEI_THREAD pthread_t      /* define thread type here                     */
   #define CEI_EVENT pthread_cond_t  /* define event variable here                  */
   #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                    */
   #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                            */
   #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                            */ 
  #else /* POSIX theads VxWorks 5.5 or greater                                      */
   #include <msgQLib.h> /* Using Message Queue                                      */
   #include <semLib.h>  /* Use mutual exclusion semaphore                           */
   #include <timers.h>  /* Timer data type include                                  */
   #define CEI_MUTEX  SEM_ID /* define mutex type here                              */
   #define CEI_THREAD int /* define thread type here                                */
   #define CEI_EVENT  MSG_Q_ID  /* define event variable here                       */
   #define CEI_THREAD_EXIT(a) taskDelete(a) /*                                      */
   #define CEI_MUTEX_LOCK(a) semTake(*a,WAIT_FOREVER) /*                            */
   #define CEI_MUTEX_UNLOCK(a) semGive(*a) /*                                       */
  #endif //_POSIX_             /*                                                   */
  void MSDELAY(int msec); /*                                                        */
#endif   /* end VXW-PCI-X86                                                         */
/************************************************************************************/

/************************************************************************************
 * Target Defines for VxWorks PowerPC BSP for the VME-1553 and                      *
 * VME-1553-IP on either the ACROMAG or GreenSpring carrier.                        */
/************************************************************************************/
#if defined (VXW_VME_PPC) /* For use with VME PowerPC SBCs                          */
  #include <sys/types.h>  /*                                                        */
  #include <taskLib.h>
  #include <string.h>
  #define _UNIX_          /* UNIX TYPE System.                                      */
  #define _GCC_           /* Using GCC compiler                                     */
  #define PPC_SYNC        /* enforce sync                                           */
  #define VXWORKS         /*                                                        */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #define _VMEBOARDSETUP  /*                                                        */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #undef FILE_SYSTEM      /* Include Dump and memory test output functions          */
  #define INCLUDE_VME_VXI_1553 /* Include the support for the VME-1553              */
  #define  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #define  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #undef ADD_TRACE       /* Exclude the function call trace code                    */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #undef _POSIX_          /* if POSIX dwfined use posis threads else use VxWorks    */
  #ifdef _POSIX_          /*                                                        */
   #include <pthread.h>   /*                                                        */
   #define CEI_MUTEX pthread_mutex_t /* define mutex type here                      */
   #define CEI_THREAD pthread_t      /* define thread type here                     */
   #define CEI_EVENT pthread_cond_t  /* define event variable here                  */
   #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                    */
   #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                            */
   #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                            */ 
  /* Use the follwoing for VxWorks 5.4 and earlier or if you don't want pthreads    */
  #else /* POSIX theads VxWorks 5.5 or greater                                      */
   #include <msgQLib.h> /* Using Message Queue                                      */
   #include <semLib.h>  /* Use mutual exclusion semaphore                           */
   #include <timers.h>  /* Timer data type include                                  */
   #define CEI_MUTEX  SEM_ID /* define mutex type here                              */
   #define CEI_THREAD int /* define thread type here                                */
   #define CEI_EVENT  MSG_Q_ID  /* define event variable here                       */
   #define CEI_THREAD_EXIT(a) taskDelete(*a) /*                                      */
   #define CEI_MUTEX_LOCK(a) semTake(*a,WAIT_FOREVER) /*                            */
   #define CEI_MUTEX_UNLOCK(a) semGive(*a) /*                                       */ 
  #endif //_POSIX_             /*                                                   */
  void MSDELAY(int msec); /*                                                        */
#endif   /* end VXW-VME-PPC                                                         */
/************************************************************************************/

/************************************************************************************
 * Target Defines for VxWorks PowerPC or PCI boards                                 *
 ************************************************************************************/
#if defined(VXW_PCI_PPC) || defined (VXW_PCI_THALES) || defined(VXW_MCP750_PPC) || defined(VXW_MV5500_PPC) || defined(VXW_DY4_PCI_PPC) || defined(VXW_PCI_MVPPC)
  #include <string.h>     /* for mset                                               */
  #define _UNIX_          /* For use with the PMC or PCI bus                        */
  #define _GCC_           /* Using GCC compiler                                     */
  #define PPC_SYNC        /* enforce sync                                           */
  #define VXWORKS         /*                                                        */
  #undef USE_BM_DMA       /*                                                        */   
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #undef INCLUDE_VME_VXI_1553 /*                                                    */
  #define ERROR_INJECTION  /* Error injection Disabled                              */
  #undef FILE_SYSTEM      /* Include Dump and memory test output functions          */
  #define  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #define  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #define  WORD_SWAP      /* define the flipw macro                                 */
  #undef  ADD_TRACE       /* Exclude the function call trace code                   */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  #undef PLX_DEBUG        /*                                                        */
  #undef BM_EXPRESS /* Add special SBC330 for AR15-VPX BM-Only code                 */ 
  #undef SYSTEM_MONITOR   /* AR15-VPX System Monitor                                */
  #define INCLUDE_LEGACY_PCI
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #undef _POSIX_          /* if POSIX defined use posis threads else use VxWorks    */
  #ifdef _POSIX_          /*                                                        */
   #include <pthread.h>  /* Use this for VxWorks 5.5 Delete for VxWorks 5.4         */
   #define CEI_MUTEX pthread_mutex_t /* define mutex type here                      */
   #define CEI_THREAD pthread_t      /* define thread type here                     */
   #define CEI_EVENT pthread_cond_t  /* define event variable here                  */
   #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                            */
   #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                            */ 
   #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                    */
  #else /* Native VxWorks Threads                                                   */
   #include <msgQLib.h> /* Using Message Queue                                      */
   #include <semLib.h>  /* Use mutual exclusion semaphore                           */
   #include <timers.h>  /* Timer data type include                                  */
   #include <taskLib.h> /*                                                          */
   #define CEI_MUTEX  SEM_ID /* define mutex type here                              */
   #define CEI_THREAD int /* define thread type here                                */
   #define CEI_EVENT  MSG_Q_ID  /* define event variable here                       */
   #define CEI_THREAD_EXIT(a) taskDelete(*a) /*                                     */
   #define CEI_MUTEX_LOCK(a) semTake(*a,WAIT_FOREVER) /*                            */
   #define CEI_MUTEX_UNLOCK(a) semGive(*a) /*                                       */ 
  #endif /* POSIX_                                                                  */
  void MSDELAY(int msec); /*                                                        */
#endif   /* end VXW-PCI-PPC or VXW_PCI_THALES                                       */
/************************************************************************************/

/************************************************************************************
 * Target Defines for Integrity PowerPC BSP for the PMC Boards                      *
 ************************************************************************************/
#if defined (INTEGRITY_PCI_PPC) /* For use with PPC and related processor with the  */
#include <INTEGRITY.h>    /* Integrity definitions                                  */
  #define _UNIX_          /*                                                        */
  #undef _GCC_            /* Using GCC compiler defines                             */
  #undef VXWORKS          /*                                                        */
  #define _INTEGRITY_     /*                                                        */
  #define INTEGRITY_PPC_SYNC /* enforce sync                                        */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #include <pthread.h>    /* Use this for POSIX Thread                              */
  #undef FILE_SYSTEM      /* Include Dump and memory test output functions          */
  #undef INCLUDE_VME_VXI_1553 /* Include the support for the VME-1553               */
  #define  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #define  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #define WORD_SWAP       /*                                                        */
  #undef ADD_TRACE        /* Exclude the function call trace code                   */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  #define MSDELAY(p1) usleep(p1*1000)  /* Define the Sleep function                 */
  #define CEI_MUTEX pthread_mutex_t /*                                              */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */ 
  #define CEI_MALLOC(a) malloc(a)         /*                                        */
  #define CEI_FREE(a)   free(a)           /*                                        */
#endif   /* end INTEGRITY_PCI_PPC                                                   */
/************************************************************************************/

/************************************************************************************
 * Target Defines for Integrity PowerPC BSP for the VME Boards                      *
 ************************************************************************************/
#if defined (INTEGRITY_VME_PPC) /* For use with PPC and related processor with the  */
#include <INTEGRITY.h>    /* Integrity definitions                                  */
  #define _UNIX_          /*                                                        */
  #undef _GCC_            /* Using GCC compiler defines                             */
  #undef VXWORKS          /*                                                        */
  #define _INTEGRITY_     /*                                                        */
  #define INTEGRITY_PPC_SYNC /* enforce sync                                        */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #include <pthread.h>    /* Use this for POSIX Thread                              */
  #undef FILE_SYSTEM      /* Include Dump and memory test output functions          */
  #define INCLUDE_VME_VXI_1553 /* Include the support for the VME-1553              */
  #define  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #define  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #undef WORD_SWAP        /*                                                        */
  #undef ADD_TRACE        /* Exclude the function call trace code                   */
  #undef  DO_BUS_LOADING  /* Exclude the bus loading code                           */
  #define MSDELAY(p1) usleep(p1*1000)  /* Define the Sleep function                 */
  #define CEI_MUTEX pthread_mutex_t /*                                              */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */ 
  #define CEI_MALLOC(a) malloc(a)         /*                                        */
  #define CEI_FREE(a)   free(a)           /*                                        */
#endif   /* end INTEGRITY_VME_PPC                                                   */
/************************************************************************************/

/************************************************************************************
 * Target defines for QNX Neutrino for the PCI                                      *
 *                                                                                  */
/************************************************************************************/
#if defined(_QNXNTO_PCI_X86_)     /*                                                */
  #define _QNXNTO_        /*                                                        */
  #include <pthread.h>    /* pthread for H/W Interrupts                             */
  #include <sys/timeb.h>  /*                                                        */
  #define _UNIX_          /*                                                        */
  #define _GCC_           /* Using GCC compiler                                     */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #define FILE_SYSTEM     /* Include Dump and memory test output functions          */
  #undef  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                               */
  #undef  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)              */
  #undef  WORD_SWAP       /* define the flipw macro                                 */
  #undef  ADD_TRACE       /* Exclude the function call trace code                   */
  #undef DO_BUS_LOADING   /* Exclude the bus loading code                           */
  #undef _USER_INIT_      /*                                                        */
  #undef _LABVIEW_        /*                                                        */
  #undef _USER_DLL_       /*                                                        */
  #undef _PLAYBACK_       /*                                                        */
  #define INCLUDE_LEGACY_PCI /* Include PCI cPCI and PMC board support              */
  #undef INCLUDE_VME_VXI_1553 /*                                                    */
  #include <unistd.h>     /*                                                        */
  #include <string.h>     /*                                                        */
  #define MSDELAY(p1) usleep(p1*1000) /* Define Sleep marcro                        */
  int linux_irq_setup(unsigned int,int,int);/*                                      */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */ 
#endif /* end _QNXNTO_                                                              */
/************************************************************************************/

/************************************************************************************
 * Target defines for QNX Neutrino for the QPMC-1553 on PowerPC                     *
 *                                                                                  */
/************************************************************************************/
#if defined(_QNXNTO_PCI_PPC_)     /*                                                */
  #include <pthread.h>    /* pthread for H/W Interrupts                             */
  #include <sys/timeb.h>  /*                                                        */
  #define _QNXNTO_        /*                                                        */
  #define _UNIX_          /*                                                        */
  #define _GCC_           /* Using GCC compiler                                     */
  #undef _Windows         /*                                                        */
  #undef __WIN32__        /*                                                        */
  #undef INCLUDE_VMIC     /* 32-bit Windows code supports the VMIC VME              */
  #define FILE_SYSTEM     /* Include Dump and memory test output functions          */
  #define NON_INTEL_BIT_FIELDS  /* Non Intel Bit Ordering                           */
  #define NON_INTEL_WORD_ORDER  /* Non Intel Word Ordering (Little Endian)          */
  #define  WORD_SWAP      /* define the flipw macro                                 */
  #undef  ADD_TRACE       /* Exclude the function call trace code                   */
  #undef DO_BUS_LOADING   /* Exclude the bus loading code                           */
  #undef _USER_INIT_      /*                                                        */
  #undef _LABVIEW_        /*                                                        */
  #undef _USER_DLL_       /*                                                        */
  #undef _PLAYBACK_       /*                                                        */
  #undef INCLUDE_VME_VXI_1553 /*                                                    */
  #include <unistd.h>     /*                                                        */
  #include <string.h>     /*                                                        */
  #define MSDELAY(p1) usleep(p1*1000) /* Define Sleep marcro                        */
  int linux_irq_setup(unsigned int,int,int);/*                                      */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */ 
#endif /* end _QNXNTO_PCI_PPC_                                                      */
/************************************************************************************/

/************************************************************************************
 * Target Defines on LynxOS PowerPC for the QVME-1553 and VME-1553                  */ 
/************************************************************************************/
#if defined(LYNXOS_VME_PPC)  /*                                                     */
  #define  LYNXOS	     /* 			                            */
  #define  _UNIX_            /*                                                     */
  #define  _GCC_             /* Using GCC compiler                                  */
  #define  INCLUDE_VME_VXI_1553  /*                                                 */
  #define  PPC_SYNC          /* enforce sync                                        */
  #define  _LYNXOSBOARDSETUP /*                                                     */
  #define  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #define  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #define  FILE_SYSTEM       /* Include Dump and memory test output functions       */
  #undef   VXWORKS           /*                                                     */
  #undef   _Windows          /*                                                     */
  #undef   __WIN32__         /*  	                                            */
  #undef   INCLUDE_VMIC      /*                                                     */
  #undef   WORD_SWAP         /* undefine the flipw macro                            */
  #undef   ADD_TRACE         /* Exclude the function call trace code                */
  #undef   DO_BUS_LOADING    /* Exclude the bus loading code                        */
  #undef   _USER_INIT_       /*                                                     */
  #undef   _BM_USER_DATA     /* 	                                            */
  #undef   DEMO_CODE         /* 	                                            */
  #undef   _PLAYBACK_        /* 	                                            */
  #undef   ERROR_INJECTION   /* 	                                            */
  #define  MSDELAY(p1) usleep(p1*1000) /* Define the Sleep function                 */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #include <pthread.h>       /* Using POSIX Threads                                 */
  #include <timeb.h>         /*                                                     */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */ 
#endif   /* end (LYNXOS_VME_PPC)                                                    */
/************************************************************************************/

/************************************************************************************
 * Target Defines on LynxOS PowerPC for the QPMC-1553 and PMC-1553                  */ 
/************************************************************************************/
#if defined(LYNXOS_PMC_PPC)  /*                                                     */
  #define  LYNXOS            /*	                                                    */
  #define  _UNIX_            /*                                                     */
  #define  _GCC_             /* Using GCC compiler                                  */
  #define  PPC_SYNC          /* enforce sync                                        */
  #define  _LYNXOSBOARDSETUP /*                                                     */
  #define  NON_INTEL_BIT_FIELDS  /* Intel Bit Ordering                              */
  #define  NON_INTEL_WORD_ORDER  /* Intel Word Ordering (Little Endian)             */
  #define  FILE_SYSTEM       /* Include Dump and memory test output functions       */
  #undef   INCLUDE_VME_VXI_1553  /*                                                 */
  #undef   VXWORKS           /*                                                     */
  #undef   _Windows          /*                                                     */
  #undef   __WIN32__         /* 	                                            */
  #undef   INCLUDE_VMIC      /*                                                     */
  #define  WORD_SWAP         /* undefine the flipw macro                            */
  #undef   ADD_TRACE         /* Exclude the function call trace code                */
  #undef   DO_BUS_LOADING    /* Exclude the bus loading code                        */
  #undef   _USER_INIT_       /*                                                     */
  #undef   _BM_USER_DATA     /* 	                                            */
  #undef   DEMO_CODE         /* 	                                            */
  #undef   _PLAYBACK_        /* 	                                            */
  #undef   ERROR_INJECTION   /* 	                                            */
  #define  MSDELAY(p1) usleep(p1*1000) /* Define the Sleep function                 */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #include <pthread.h>       /* Using POSIX Threads                                 */
  #include <timeb.h>         /*                                                     */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */ 
#endif   /* end (LYNXOS_PMC_PPC)                                                    */
/************************************************************************************/
  
/************************************************************************************
 * Target Defines on LynxOS x86 for the Q104-1553P, QPCI-1553, and PCI-1553         */
/************************************************************************************/
#if defined(LYNXOS_X86)      /*                                                     */
  #define  LYNXOS            /*	                                                    */
  #define  _UNIX_            /*                                                     */
  #define  _GCC_             /* Using GCC compiler                                  */
  #define  _LYNXOSBOARDSETUP /*                                                     */
  #define  FILE_SYSTEM       /* Include Dump and memory test output functions       */
  #undef   INCLUDE_VME_VXI_1553  /*                                                 */
  #undef   NON_INTEL_BIT_FIELDS /* Intel Bit Ordering                               */
  #undef   NON_INTEL_WORD_ORDER /* Intel Word Ordering (Little Endian)              */
  #undef   PPC_SYNC          /*                                                     */
  #undef   VXWORKS           /*                                                     */
  #undef   _Windows          /*                                                     */
  #undef   __WIN32__         /*                                                     */
  #undef   INCLUDE_VMIC      /*                                                     */
  #undef   WORD_SWAP         /* undefine the flipw macro                            */
  #undef   ADD_TRACE         /* Exclude the function call trace code                */
  #undef   DO_BUS_LOADING    /* Exclude the bus loading code                        */
  #undef   _USER_INIT_       /*                                                     */
  #undef   _BM_USER_DATA     /* 	                                            */
  #undef   DEMO_CODE         /* 	                                            */
  #undef   _PLAYBACK_        /* 	                                            */
  #undef   ERROR_INJECTION   /*                                                     */  
  #define  MSDELAY(p1) usleep(p1*1000) /* Define the Sleep function                 */
  /* the following are the O/S specific definitions for malloc, free threads,       */
  /* mutexes and events                                                             */ 
  #define CEI_MALLOC(a) malloc(a) /* define memory alloc function here              */
  #define CEI_FREE(a)   free(a)   /* define memory free function here               */
  #include <pthread.h>       /* Using POSIX Threads                                 */
  #include <timeb.h>         /*                                                     */
  #define CEI_MUTEX pthread_mutex_t /* define mutex type here                       */
  #define CEI_THREAD pthread_t      /* define thread type here                      */
  #define CEI_EVENT pthread_cond_t  /* define event variable here                   */
  #define CEI_THREAD_EXIT(a) pthread_exit(0) /*                                     */
  #define CEI_MUTEX_LOCK(a) pthread_mutex_lock(a)    /*                             */
  #define CEI_MUTEX_UNLOCK(a) pthread_mutex_unlock(a)/*                             */ 
#endif   /* end (LYNXOS_X86)                                                        */
/************************************************************************************/

/************************************************************************************
 * Target Defines on National Instruments LabView-RT                                */
/************************************************************************************/
#if defined(LVRT)
 #define  _LVRT_
 #define  _LABVIEW_
 #define  _LABVIEWRTSETUP
 #undef   _UNIX_
 #undef   VXWORKS
 #undef   _Windows
 #undef   _GCC_
 #undef   FILE_SYSTEM
 #undef   NON_INTEL_BIT_FIELDS
 #undef   NON_INTEL_WORD_ORDER
 #undef   INCLUDE_PCCD
 #undef   INCLUDE_VME_VXI_1553
 #undef   INCLUDE_VMIC
 #undef   DEMO_CODE
 #undef   WORD_SWAP
 #undef   DO_BUS_LOADING
 #define  NO_ASSEMBLY
 #undef   _PLAYBACK_
 #undef   ADD_TRACE
 #undef   _USER_DLL_
 #undef   _USER_INIT_
 #undef   _BM_USER_DATA
 #undef   ERROR_INJECTION
 #define  PACKED
 #define  CCONV _stdcall
 #define  NOMANGLE
 #undef   LocalMalloc
 #undef   LocalFree
 #undef   PRAGMA_PACK
 #pragma  pack(2)
 #ifndef CEI_UINT64
   #define CEI_UINT64  unsigned __int64
 #endif
#endif  // end LVRT
/***********************************************************************************/ 

/************************************************************************************
 * CVI uses the __WIN32__ Block above.  These are modifications to that block to    *
 * allow CVI to compile.                                                            *
*************************************************************************************/
#if defined(_CVI_) && !defined(_LVRT_) 
#define NO_ASSEMBLY
#undef  ADD_TRACE
#undef  _USER_DLL_
#define  _PLAYBACK_
#undef INCLUDE_VMIC
#undef  _USER_INIT_
#define __int64 double long 
#define TimeGetTime() 
#endif
/***********************************************************************************/

#if defined(__BORLANDC__) || defined(_UNIX_)
#define _ftime ftime
#define _timeb timeb
#endif

#if defined(__BORLANDC__)
#undef PRAGMA_PACK
#pragma pack(2)
#endif

/**********************************************************************
 * The following defines are for UNIX systems.  This includes Linux,
 * Solaris, QNX, and VxWorks.
 *********************************************************************/
#ifdef _UNIX_

#ifdef _GCC_
  #define PACKED __attribute__ ((aligned(2),packed))  /* GCC Compiler Only */
#else
#define PACKED
#endif

#ifdef PPC_SYNC        /* enforce sync */
  #define IO_SYNC  __asm__ volatile ("	eieio;	sync")
#endif /* PPC_SYNC */ 

#define INFINITE 0
#define NO_ASSEMBLY
#define THREAD_PRIORITY_NORMAL 32 
#define THREAD_PRIORITY_BELOW_NORMAL 64
#define THREAD_PRIORITY_ABOVE_NORMAL 16
#define THREAD_PRIORITY_CRITICAL 2

#ifndef UTILDEFS_H
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef int             HWND;
typedef char  * LPSTR;
typedef int   * LPINT;
typedef WORD  * LPWORD;
typedef long  * LPLONG;
typedef DWORD * LPDWORD;
typedef void  * LPVOID;
#define HANDLE int
#define VOID            void
#define CALLBACK
#define NOMANGLE
#define CCONV
#define _stdcall
#ifndef VXWORKS
typedef unsigned int    UINT;
#endif
#define BOOL int
#endif /* UTILDEFS_H */

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1        
#endif

extern DWORD timeGetTime();

#undef PRAGMA_PACK

#endif /*_UNIX_ */

/**********************************************************************
*  Record the build options for the resource file:
**********************************************************************/
#define BUILD_OPTIONS_INT "Hardware Interrupts"
#if defined(ADD_TRACE) && defined(FILE_SYSTEM)
#define BUILD_OPTIONS "TRACE and DUMP enabled"
#elif defined(FILE_SYSTEM)
#define BUILD_OPTIONS "DUMP enabled, TRACE disabled"
#elif defined(ADD_TRACE)
#define BUILD_OPTIONS "Diagnostic TRACE enabled, DUMP disabled"
#else
#define BUILD_OPTIONS "No Trace or Dump enabled"
#endif

#endif /* TARGET_DEFINES_H */
