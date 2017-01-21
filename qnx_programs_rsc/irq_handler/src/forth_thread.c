#include "forth_thread.h"

DBF_TXT DTX_4;

void setupSignalAndTimer (void)
{
   timer_t timerid;
   struct sigevent event;
   struct itimerspec timer;
   int coid;
   
   coid = ConnectAttach (0,0,chid,0,0);
   if(coid == -1)  {
     fprintf (stderr,"Error in ConnectAttach(), Thread 4, setupPulseAndTimer() function\n");
     perror (NULL);
     exit (EXIT_FAILURE);
  }
  SIGEV_SIGNAL_INIT(&event, coid );
  if (timer_create(CLOCK_REALTIME, &event, &timerid) == -1) {
    fprintf ( stderr, "timer_create() error int Thread 4, setupPulseAndTimer function" );
    perror (NULL);
    exit (EXIT_FAILURE);
  }
  //Timer Setup: 1 sec lag, 1 sec reset
  timer.it_value.tv_sec = 1;
  timer.it_value.tv_nsec = 0;
  timer.it_interval.tv_nsec = 1;
  timer.it_interval.tv_nsec = 0;
  //Start the Timer
  timer_settime(timerid, 0, &timer, NULL);
}

void* forth_thread( void *arg )
{
    int counter = 0;
    setupSignalAndTimer();
    while (stop_cmd == 0)
    {
       sigwait();
       counter++;
       TDB_print (&DTX_4, "\n\nSignal %d from thread 4 delivered\n\n", counter);
       TDB_Push (&DTX_4);
    }
    return 0;
}