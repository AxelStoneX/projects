#include "third_thread.h"

DBF_TXT DTX_3;                                                                              // Exchange task text output double buffer 2
int conid;

int channel_create (void)
{
    chid = ChannelCreate_r( NULL );
    if(chid<=0)
    { printf("ChannelCreate_r( NULL ) error in thread 3");
       return -1; }
    if( ( conid = ConnectAttach_r ( 0, 0, chid, NULL, NULL )) < 0 )
    { printf( "Connection Attach Error in thread 3\n" );
       return -1; }
    return 0;
}

void *third_thread (void *arg )
{
    void* pulse_buf[64];
    int pls_cnt = 0;     //pulse counter
    //---------------------------------------------------------------Create Channel---------------------------------------------------------------
    if ( channel_create() < 0 )
    { printf( "channel_create() error\n" );  exit( -1 ); };
    //------------------------------------------------------------Signal Processing-------------------------------------------------
    struct sigaction act;
    sigfillset( &(act.sa_mask) );                  //All existing signals
    sigdelset( &(act.sa_mask), SIGINT );           //All but SigInt - enable it
    act.sa_sigaction = catchint;
    act.sa_flags = SA_SIGINFO;
    sigaction( SIGINT, &act, NULL );
    sigprocmask( SIG_SETMASK, &(act.sa_mask), NULL );
    //------------------------------------------------------------Create Interruption------------------------------------------------------------
    while(stop_cmd == 0)
    { 
       //printf("\n\nThread 3 is waiting for pulse from %d\n\n", chid);
       MsgReceivePulse_r( chid, pulse_buf, sizeof(pulse_buf), NULL );
       //printf("\n\nThread 3 received pulse\n\n");
       pls_cnt++;
       TDB_print( &DTX_3, "Thread 3 get pulse number %d\n\n", pls_cnt );
       TDB_Push( &DTX_3 );
     }
     printf("Thread 3 has ended work succesfully\n");   
   
 }