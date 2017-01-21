#ifndef __FORTH_THREAD_H__
#define __FORTH_THREAD_H__

#include <stdio.h>
#include <time.h>
#include <sys/neutrino.h>
#include <stdlib.h>
#include <process.h>
#include <signal.h>
#include <sys/siginfo.h>
#include "dbf_dv.h"
#include "lbm_bs.h"
#include "first_program.h"
#include "fifth_thread.h"

#endif

void setupPulseAndTimer (void);

void* forth_thread( void *arg );

extern volatile LWORD stop_cmd;