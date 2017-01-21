#ifndef __THIRD_THREAD_H__
#define __THIRD_THREAD_H__

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

extern int chid;            //channel id for third_thread connection

extern int conid;

void *third_thread (void *arg );

int channel_create (void);

extern volatile LWORD stop_cmd;


#endif

#/** PhEDIT attribute block
#-11:16777215
#0:464:default:-3:-3:0
#464:531:monospace10:0:-1:0
#531:683:default:-3:-3:0
#683:714:monospace10:0:-1:0
#714:724:default:-3:-3:0
#**  PhEDIT attribute block ends (-0000218)**/
