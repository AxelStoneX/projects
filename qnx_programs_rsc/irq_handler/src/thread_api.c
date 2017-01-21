 #include "thread_api.h"
 
 int thread_create ( void* (*start_routine)(void* ), int priority )
 {
    pthread_attr_t attr;                                                                                           //create thread's attribute
    struct sched_param SPR;
    
    pthread_attr_init( &attr );                                                                                  //initialize thread's attribute
    pthread_attr_setschedpolicy( &attr, SCHED_FIFO );                                         //setting sheduling policy
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );            //set thread's synch
    SPR.sched_priority = priority;
    pthread_attr_setschedparam (&attr, &SPR);
  
    pthread_create (NULL, &attr, start_routine, NULL);                                            //create thread itself
    return 0;
 }