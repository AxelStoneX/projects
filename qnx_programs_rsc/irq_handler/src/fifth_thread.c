#include "fifth_thread.h"

DBF_TXT DTX_5;

void setupPulseAndTimer (void)
{
    timer_t timerid;
    struct sigevent event;
    struct itimerspec timer;
    int coid;
    
    coid = ConnectAttach (0,0,chid,0,0);
    if(coid == -1)  
    {
       fprintf (stderr,"Error in ConnectAttcah(), Thread 4, setupPulseAndTimer() function\n");
       perror (NULL);
       exit (EXIT_FAILURE);
     }
    SIGEV_PULSE_INIT (&event, coid, SIGEV_PULSE_PRIO_INHERIT, CODE_TIMER, 0);
    if (timer_create(CLOCK_REALTIME, &event, &timerid) == -1) 
    {
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

 void* fifth_thread( void *arg )
 {
     int chid;
     int msg[15];
     int counter = 0;
     if((chid = ChannelCreate(0)) == -1) {
       fprintf ( stderr, "ChannelCrate() error in thread 5\n" );
       perror( NULL );
       exit( EXIT_FAILURE );
     }
     setupPulseAndTimer();
     while (stop_cmd == 0)
     {
        MsgReceive(chid, &msg, sizeof(msg), NULL );
        counter++;
        TDB_print (&DTX_5, "\n\nSignal %d from thread 5 delivered\n\n", counter );
        TDB_Push (&DTX_5);
      }
      return 0;
 }