#include "prog_dispatcher.h"


volatile LWORD stop_cmd = 0;
DBF_TXT DTX_1;

int main( void )
{
   int status;
   struct timespec TS;
   struct _clockperiod old;
   struct _clockperiod new;
   
   printf( "Starting program dispatcher initialization...\n" );
   
   TS.tv_sec  = 0;
   TS.tv_nsec = 300000000;
   new.nsec   = 1000000;
   new.fract  = 0;
   TDB_Init( &DTX_1 );
   
   status = nanospin_calibrate( 0 );
   if( status != EOK )
   {   printf( ">>>>nanospin_calibrate() error in main()\n" );
       return RET_FAIL; /*.................................*/   }
       
   ClockPeriod( CLOCK_REALTIME, NULL, &old, 0 );

   printf("Old clock %d nanoseconds has changed to ", old.nsec);
   ClockPeriod( CLOCK_REALTIME, &new, &old, 0 );
   printf("%d nanoseconds\n", new.nsec);
   
   printf( "Program dispatcher initialization completed\n" );
   printf( "Starting Bus Tools card initialization...\n" );
   
   status = init_all_cards();
   if( status != RET_OK )
   {   printf( ">>>>>Bus Tools cards initialization error in main()\n" );
       printf( "Program dispathcer shuts down.\n" );
       return RET_FAIL; /*............................................*/  }
       
   printf( "Bus Tools cards initialization completed.\n" );
   
   printf( "Starting functional software thread...\n" );
   thread_create( func_software_body, 10 );
   
   while( stop_cmd == 0 )
   {   nanosleep(&TS, NULL);
       TDB_Show( &DTX_1 );   }
   
   printf( "Program dispatcher has ended it's work succesfully.\n" );
   printf( "Program dispatcher shuts down." );
   return RET_OK;
}

//=============================================================================
void catchint( int signo, siginfo_t *info, void *context )
{
     ;
}