#include "second_thread.h"

#define Clock_IRQ 0
#define EXC_TASK_CYCLE 1000

volatile LWORD  rtc_ct = 0;                                                         // Real Time Clock interrupt counter

int chid;
TMS_t MSR;                                //measure
STAT_t ST;                                //statistics
struct sigevent event;
int conid;

const struct sigevent* IntHandler ( void *arg, int id )
{
	struct sigevent *event = ( struct sigevent * )arg;
	rtc_ct++;
	if( rtc_ct >= EXC_TASK_CYCLE )
	{
	  rtc_ct = 0;
	  return( event );
	}
	else
	  return ( NULL );
 }
  
      
void* second_thread( void *arg )
{
    int int_id;                                                                             //interruption identificator
    int RS;
    int counter = 1;                                                                   
    double ex_time = 0;                                                           //execution time
    struct timespec td;                                                              // Time delay
    int counter_2;
    
    //--------------------------------------------Create Interruption-----------------------------------------------------------
    SIGEV_INTR_INIT( &event );
	
    RS = ThreadCtl_r( _NTO_TCTL_IO, NULL );                                                         // Set I/O rights
    if( RS != EOK ) { printf("ThreadCtl_r() error\n" );  exit(-1);  }

	  int_id = InterruptAttach(Clock_IRQ, IntHandler, &event, sizeof(event), 0);     //Adjust Interruption
	  if (int_id == -1)
	  { fprintf(stderr,"InterruptAttach error");
	    perror(NULL);		exit(-1);	}
	   
	  TimeCounterInit ( &MSR );
	  TimeStatisticsInit ( &ST );
	
	//------------------------------------------------------------Signal Processing-------------------------------------------------
  struct sigaction act;
  sigfillset( &(act.sa_mask) );                  //All existing signals
  sigdelset( &(act.sa_mask), SIGINT );           //All but SigInt - enable it
  act.sa_sigaction = catchint;
  act.sa_flags = SA_SIGINFO;
  sigaction( SIGINT, &act, NULL );
  sigprocmask( SIG_SETMASK, &(act.sa_mask), NULL );
  //------------------------------------------------------------------------------------------------------------------
	
	while( stop_cmd == 0 )
	{   
	  //--------------------------------------------------Interruption Processing--------------------------------------------
		InterruptWait(0, NULL);                                                                                                                   //wait for interruption in the endless cycle
    StartTimeCount ( &MSR );
    RS = nanospin( &td );
    if( RS != EOK ) { printf( "nanospin() error\n" );  stop_cmd = 1;  } // >>>>
 		ex_time = GetTimeCount ( &MSR );
		TimeStatisticsGet (&ST, ex_time);

	  TDB_print( &DTX, "Got intr signal %d\n" ,counter);
    TDB_print( &DTX, "Estimated time %0.4f\n",ex_time);
    TDB_print( &DTX, "average time %0.4f +/- %8.3e\n\n", ST.average_time, ST.standart_deviation);  
    counter++;                                                                                                                                  //print message about interruption signsl getting
    counter_2 = counter;
    counter_2 %= 5;
    TDB_Push( &DTX );
    if (counter_2 == 0)
    {
       printf("\n\nPulse Sent to %d\n\n", conid);
       MsgSendPulse_r( conid, -1, 11, 11);
    }
	}
	printf("Thread 2 has ended work succesfully\n");   

}