#ifndef __FIFTH_THREAD_H__
#define __FIFTH_THREAD_H__

#include <stdio.h>
#include <time.h>
#include <sys/neutrino.h>
#include <stdlib.h>
#include <process.h>
#include <errno.h>
#include <signal.h>
#include <sys/siginfo.h>
#include "dbf_dv.h"
#include "lbm_bs.h"
#include "first_program.h"

#endif

#define CODE_TIMER 1                                  //Pulse from timer

 void setupPulseAndTimer (void);
 void* fifth_thread( void *arg );
 extern volatile LWORD stop_cmd;