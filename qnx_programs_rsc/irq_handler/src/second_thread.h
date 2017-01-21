#ifndef __SECOND_THREAD_H__
#define __SECOND_THREAD_H__

#include <stdio.h>
#include <time.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sched.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <sys/netmgr.h>
#include "dbf_dv.h"
#include "lbm_bs.h"
#include "time_counter.h"
#include "time_statistics.h"
#include "thread_api.h"
#include "second_thread.h"
#include "third_thread.h"
#include "fifth_thread.h"

extern void catchint( int signo, siginfo_t *info, void *context );

void* second_thread( void *arg );                                                  //second launchable thread of main function

const struct sigevent* IntHandler  ( void *arg, int id );                  //interruption event srtucture for second thread

extern int chid;            //channel id for third_thread connection
extern int conid;
extern volatile LWORD stop_cmd;
#endif
#/** PhEDIT attribute block
#-11:16777215
#0:466:default:-3:-3:0
#466:533:monospace10:0:-1:0
#533:874:default:-3:-3:0
#874:905:monospace10:0:-1:0
#905:912:default:-3:-3:0
#**  PhEDIT attribute block ends (-0000218)**/
