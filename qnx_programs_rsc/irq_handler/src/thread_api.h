#ifndef __THREAD_API_H__
#define __THREAD_API_H__

#include <sys/neutrino.h>
#include <pthread.h>

int thread_create ( void* (*start_routine)(void* ), int priority );    //create new thread by function name

#endif