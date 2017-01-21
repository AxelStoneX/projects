 #include "thread_api.h"

 int thread_create ( void* (*start_routine)(void* ), int priority, pthread_t* thread_id )
 {
    int status = 0;
    pthread_attr_t attr;                                           //create thread's attribute
    struct sched_param SPR;
    
    pthread_attr_init( &attr );                                     //initialize thread's attribute
    
    status = pthread_attr_setschedpolicy( &attr, SCHED_FIFO );      //setting sheduling policy
    if( status != EOK )
    {   printf( "Error of pthread_attr_setschedpolicy " );
        printf( "in thread_create" );
        return RET_FAIL; /*.................................*/ }
        
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );  //set thread's synch
    if( status != EOK )
    {   printf( "Error of pthread_attr_setdetachstate() " );
        printf( "in thread_create\n" );
        return RET_FAIL; /*.....................................*/ }
        
    SPR.sched_priority = priority;
    
    pthread_attr_setschedparam (&attr, &SPR);
    if( status != RET_OK )
    {   printf( "Error of pthread_attr_setschedparam() " );
        printf( "in thread_create.c" );
        return RET_FAIL; /*...............................*/ }
  
    status = pthread_create (thread_id, &attr, start_routine, NULL);              //create thread itself
    if( status != EOK )
    {   printf( "Error of  pthread_create()  " );
        printf( "in thread_create.c\n" );
        return RET_FAIL; /*..................................*/ }
    
    return 0;
 }

int thread_flag_initialize ()
{
    pthread_mutex_init (&thread_flag_mutex, NULL);
    pthread_cond_init (&thread_flag_cv, NULL);
    thread_flag = 0;
    return 0;  
}

int set_thread_flag( int flag_value )
{
    pthread_mutex_lock( &thread_flag_mutex );
    thread_flag = flag_value;
    pthread_cond_signal( &thread_flag_cv );
    pthread_mutex_unlock( &thread_flag_mutex );
}
