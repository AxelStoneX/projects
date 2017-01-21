#include "prc_control.h"

struct timespec TS;

//============================================================================
int exchange_task_init( void )
{
   TS.tv_nsec = 0;
   TS.tv_nsec = 100000000;
   return RET_OK;
}

//============================================================================
void* exchange_task_body( void *arg )
{
   int status;

   status = exchange_task_init();
   if( status != RET_OK )
   {   printf( "exchange_task_init() error in exchange_task_body()\n" );
       rt_halt(); /*.....................................................*/ }

   while( stop_cmd == 0)
   {
       nanosleep(&TS, NULL );
       Check_BSync();
   }
}
