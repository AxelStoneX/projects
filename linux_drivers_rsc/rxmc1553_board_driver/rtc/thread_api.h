#ifndef __THREAD_API_H__
#define __THREAD_API_H__

//#include <sys/neutrino.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
//#include <asm/atomic.h>  // atomic_add()
#include "lbm_bs.h"

pthread_cond_t thread_flag_cv;        // Flag for thread synchronising
pthread_mutex_t thread_flag_mutex;    // Mutex for thread synchronising
int thread_flag;                      // Actual flag

//create new thread by function name
int thread_create ( void* (*start_routine)(void* ), int priority, pthread_t* thread_id );    

#endif
