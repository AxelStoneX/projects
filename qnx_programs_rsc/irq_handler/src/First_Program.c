#include "first_program.h"

DBF_TXT DTX;
DBF_TXT DTX_3;                                                                                                  // Exchange task text output double buffer 3;
DBF_TXT DTX_4;                                                                                                  // Exchange task text output double buffer 4;
DBF_TXT DTX_5;                                                                                                  // Exchange task text output doble buffer 5;

int chid;                                                                                                                // channel id for third_thread connection


volatile LWORD stop_cmd=0;

void catchint( int signo, siginfo_t *info, void *context )
{
   printf("Signal is catched\n");
   stop_cmd = 1;
}

int main (int argc, char **argv)
{ 
  int RS,rt_cnt = 0;
  struct timespec TS;
  struct _clockperiod old;
  struct _clockperiod new;
  
  TS.tv_sec = 0;
  TS.tv_nsec = 200000000;

  TDB_Init( &DTX );
  TDB_Init( &DTX_3 );
  TDB_Init( &DTX_4 );
  TDB_Init( &DTX_5 );
  
  RS = nanospin_calibrate( 0 );
  if( RS != EOK ) { printf("nanospin_calibrate error\n");  exit( -1 );  }

  ClockPeriod(CLOCK_REALTIME, NULL, &old, 0);
  printf("Old Clock is %d nanoseconds\n", old.nsec);
  new.nsec = 1000000;
  new.fract = 0;
  ClockPeriod(CLOCK_REALTIME, &new, &old, 0);
  
  thread_create( second_thread, 10 );
  thread_create( third_thread, 15 );
  thread_create( forth_thread, 15 );
  thread_create(fifth_thread, 15 );
  
  //------------------------------------------------------------Signal Processing-------------------------------------------------
  struct sigaction act;
  sigfillset( &(act.sa_mask) );                        //All existing signals
  sigdelset( &(act.sa_mask), SIGINT );           //All but SigInt - enable it
  act.sa_sigaction = catchint;
  act.sa_flags = SA_SIGINFO;
  sigaction( SIGINT, &act, NULL );
  sigprocmask( SIG_SETMASK, &(act.sa_mask), NULL );
  //------------------------------------------------------------------------------------------------------------------------------------
  
  while( stop_cmd == 0 )
   {
      nanosleep(&TS, NULL);
   printf( "rt_cnt = %d\n", rt_cnt );  
      rt_cnt++;
      TDB_Show( &DTX );
      TDB_Show( &DTX_3 );
      TDB_Show( &DTX_4 );
      TDB_Show( &DTX_5 );
   }
  printf("Thread 1 has ended work succesfully\n");   
  return 0;
}