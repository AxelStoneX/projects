#include "prc_control.h"


volatile LWORD stop_cmd = 0;
DBF_TXT DTX_1;


int main( void )
{
   int status;
   struct timespec TS;
   pthread_t func_task_id;
   pthread_t exch_task_id;
   
   printf( "Starting program dispatcher initialization...\n" );
   
   TS.tv_sec  = 0;
   TS.tv_nsec = 300000000;
   memset( (void*) &func_task_id, L_OFF, sizeof(pthread_t) );
   memset( (void*) &exch_task_id, L_OFF, sizeof(pthread_t) );
   TDB_Init( &DTX_1 );
   
   printf( "Program dispatcher initialization completed\n" );
   printf( "Starting Bus Tools card initialization...\n" );
   
   status = init_all_cards();
   if( status != RET_OK )
   {   printf( ">>>>>Bus Tools cards initialization error in main()\n" );
       printf( "Program dispathcer shuts down.\n" );
       return RET_FAIL; /*............................................*/  }

   status = Init_BMP();
   if( status != RET_OK )
   {   printf( ">>>>>BMP_Init() error in main()\n" );
       return RET_FAIL; /*............................*/ }
       
   printf( "Bus Tools cards initialization completed.\n" );
   
   printf( "Starting functional task thread...\n" );
   thread_create( func_task_body, FUNC_TASK_PRIO, &func_task_id );

   printf( "Starting exchange task thread...\n" );
   thread_create( exchange_task_body, EXCH_TASK_PRIO, &exch_task_id );
   
   while( stop_cmd == 0 )
   {   
       nanosleep( &TS, NULL );
       TDB_Show( &DTX_1 );
       
   }  
   printf( "Program dispatcher has ended it's work.\n" );
   printf( "Program dispatcher shuts down.\n" );
   return RET_OK;
}

//============================================================================
void rt_final( void )
{
    stop_cmd = 1;
}

//============================================================================
void rt_halt( void )
{
   printf( "Root Task emergency exit" );
   stop_cmd = 1;
}


