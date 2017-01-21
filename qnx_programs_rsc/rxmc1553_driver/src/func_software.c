#include "func_software.h"
#define Clock_IRQ 0
#define EXC_TASK_CYCLE 1000

int transmit_idx = -1;
int receive_idx = -1;
SWORD GET_DATA_BUFFER[RECEIVE_WC];
SWORD SEND_DATA_BUFFER[TRANSMIT_WC];
SWORD GLOBAL_COUNTER;

volatile LWORD  rtc_ct = 0;
struct sigevent event;
int int_id;
struct sigaction act;

extern void catchint( int signo, siginfo_t *info, void *context );

//=============================================================================
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

//=============================================================================
int func_software_init( void )
{   int status;

    transmit_idx = buffers_initialization( TRANSMIT_BUS, TRANSMIT_RT,
                                           TRANSMIT_SA, TRANSMIT_WC,
                                           BT_TRANSMIT );
    if( transmit_idx <= RET_FAIL )
    {   printf( ">>>>Error in transmit buffers initialization " );
        printf( "in func_software_init()\n" );
        printf( "Info: bus=%d : rt=%d : sa=%d\n", 
                       TRANSMIT_BUS, TRANSMIT_RT, TRANSMIT_SA );
        return RET_FAIL ; /*....................................*/  }
        
    receive_idx = buffers_initialization( RECEIVE_BUS, RECEIVE_RT, RECEIVE_SA,
                                          RECEIVE_WC, BT_RECEIVE );
    if( receive_idx <= RET_FAIL )
    {   printf( ">>>>Error in receive buffers initialization " );
        printf( "in func_software_init()\n" );
        printf( "Info: bus=%d : rt=%d : sa=%d\n", 
                       RECEIVE_BUS, RECEIVE_RT, RECEIVE_SA );
        return RET_FAIL ; /*..................................*/  }
        
    memset( (void*) &GET_DATA_BUFFER[0],  L_OFF, sizeof(SWORD) * RECEIVE_WC  );
    memset( (void*) &SEND_DATA_BUFFER[0], L_OFF, sizeof(SWORD) * TRANSMIT_WC );
    GLOBAL_COUNTER = 0;

    SIGEV_INTR_INIT( &event );
    
    int_id = InterruptAttach( Clock_IRQ, IntHandler, &event, sizeof(event), 0);
    if( int_id == RET_FAIL )
    {   printf( "Error of InterruptAttach() " );
        printf( "in func_software_init()\n" );
        return RET_FAIL; /*....................................................*/ }
        
    sigfillset( &(act.sa_mask) );                  //All existing signals
    sigdelset( &(act.sa_mask), SIGINT );           //All but SigInt - enable it
    act.sa_sigaction = catchint;
    act.sa_flags = SA_SIGINFO;
    sigaction( SIGINT, &act, NULL );
    sigprocmask( SIG_SETMASK, &(act.sa_mask), NULL );
        
    return RET_OK;
}

//=============================================================================
int func_software_send_data( void )
{   int status;

    SEND_DATA_BUFFER[0] = GLOBAL_COUNTER;
    status = buffer_transmit( receive_idx, (SWORD*) &SEND_DATA_BUFFER );
    if( status != RET_OK )
    {   printf( ">>>>Error while sending data in func_software_send_data()\n" );
        return RET_FAIL; /*.................................................*/ }
        
    GLOBAL_COUNTER++;
    
    return RET_OK;      
}

//=============================================================================
int func_software_get_data( DBF_TXT* DTX )
{   int status;

    status = buffer_receive( receive_idx, (SWORD*) &GET_DATA_BUFFER,
                                           sizeof(GET_DATA_BUFFER)      );
    if( status != RET_OK )
    {   printf( ">>>>Error while getting data in func_software_get_data\n" );
        return RET_FAIL; /*...............................................*/  }
        
    Show_Hex_DB( DTX, "Recv. data:", &GET_DATA_BUFFER[0], RECEIVE_WC );    
    return RET_OK;
}

//=============================================================================
void* func_software_body( void *arg )
{   int status;

    struct timespec TS;
    TS.tv_sec = 0;
    TS.tv_nsec = 200000000;
    void* pulse_buf[8];

    status = ThreadCtl_r( _NTO_TCTL_IO, NULL );
    if( status != EOK )
    {   printf( "ThreadCtl_r() error " );
        printf( "in func_coftware_body()>>>>\n" );
        stop_cmd = 1; /*.......................*/  }
        
    status = func_software_init();
    if( status != RET_OK )
    {   printf( "func_software_init() error " );
        printf( "in func_software_body()>>>>\n" );
        stop_cmd = 1; /*.......................*/  }
          
    while( stop_cmd == 0 )
    {   func_software_send_data();
        func_software_get_data( &DTX_1 );
        TDB_Push( &DTX_1 );
        InterruptWait( 0, NULL );
    }
}
    
#/** PhEDIT attribute block
#-11:13027014
#0:302:default:-3:-3:0
#302:368:monospace9:0:-1:0
#368:3973:default:-3:-3:0
#3973:3992:monospace10:0:-1:0
#3992:4625:default:-3:-3:0
#**  PhEDIT attribute block ends (-0000222)**/
