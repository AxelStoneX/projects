#include "prc_control.h"

int   transmit_idx = -1;
int   receive_idx = -1;
sem_t ft_semaphore;
SWORD GET_DATA_BUFFER[RECEIVE_WC];
SWORD SEND_DATA_BUFFER[TRANSMIT_WC];
SWORD GLOBAL_COUNTER;

//=============================================================================
int func_task_init( void )
{   int status;

    transmit_idx = standart_transaction_initialization( TRANSMIT_BUS, TRANSMIT_RT,
                                                        TRANSMIT_SA, TRANSMIT_WC,
                                                        BT_TRANSMIT );
    if( transmit_idx <= RET_FAIL )
    {   printf( ">>>>Error in transmit buffers initialization " );
        printf( "in func_task_init()\n" );
        printf( "Info: bus=%d : rt=%d : sa=%d\n", 
                       TRANSMIT_BUS, TRANSMIT_RT, TRANSMIT_SA );
        return RET_FAIL ; /*....................................*/  }

    //START OF DEBUG
    printf( "-----------------------------------------\n" );
    printf( "------transmit index had been obtained\n" );
    printf( "------transmit_idx = %d\n", transmit_idx);
    printf( "-----------------------------------------\n" );
    //END OF DEBUG
        
    receive_idx = standart_transaction_initialization( RECEIVE_BUS, RECEIVE_RT, RECEIVE_SA,
                                              RECEIVE_WC, BT_RECEIVE );
    if( receive_idx <= RET_FAIL )
    {   printf( ">>>>Error in receive buffers initialization " );
        printf( "in func_task_init()\n" );
        printf( "Info: bus=%d : rt=%d : sa=%d\n", 
                       RECEIVE_BUS, RECEIVE_RT, RECEIVE_SA );
        return RET_FAIL ; /*..................................*/  }

    //START OF DEBUG
    printf( "-----------------------------------------\n" );
    printf( "------receive index had been obtained\n" );
    printf( "------receive_idx = %d\n", receive_idx);
    printf( "-----------------------------------------\n" );
    //END OF DEBUG
        
    memset( (void*) &(GET_DATA_BUFFER[0]),  0x0000, 2 * RECEIVE_WC  );
    memset( (void*) &(SEND_DATA_BUFFER[0]), 0x0000, 2 * TRANSMIT_WC );
    GLOBAL_COUNTER = 1;

    sem_init( &ft_semaphore, 0, 0 );
    
    return RET_OK;
}

//=============================================================================
int func_task_send_data( void )
{   int status;
    int i;

    for( i=0; i<TRANSMIT_WC; i++ )
        SEND_DATA_BUFFER[i] = GLOBAL_COUNTER + i;
    status = buffer_transmit( transmit_idx, (SWORD*) &(SEND_DATA_BUFFER[0]), 
                                             sizeof(SEND_DATA_BUFFER[0]) * TRANSMIT_WC );
    if( status != RET_OK )
    {   printf( ">>>>Error while sending data in func_task_send_data()\n" );
        return RET_FAIL; /*.................................................*/ }
        
    //GLOBAL_COUNTER++;
    
    return RET_OK;      
}

//=============================================================================
int func_task_get_data( DBF_TXT* DTX )
{   int status;
    int i = 0;

    //START OF DEBUG
    printf( "-----------------------------------------\n" );
    printf( "------buffer_receive() start\n");
    printf( "-----------------------------------------\n" );
    //END OF DEBUG

    status = buffer_receive( receive_idx, (SWORD*) &(GET_DATA_BUFFER[0]),
                                           sizeof(GET_DATA_BUFFER[0]) * RECEIVE_WC );

    //START OF DEBUG
    printf( "-----------------------------------------\n" );
    printf( "------buffer_receive() end\n");
    printf( "-----------------------------------------\n" );
    //END OF DEBUG
    
    if( status <= RET_FAIL )
    {   printf( ">>>>Error while getting data in func_task_get_data\n" );
        return RET_FAIL; /*...............................................*/  }

    if( status > RET_FAIL && status <= 1 )
    {   printf( ">>>>There were no data to receive in func_task_get_data\n" );
        return RET_FAIL; /*..............................................*/   }
        
    Show_Hex_DB( DTX, "Recv. data:", &(GET_DATA_BUFFER[0]), RECEIVE_WC );    
    return RET_OK;
}

//=============================================================================
void* func_task_body( void *arg )
{   int status;

    struct timespec TS;
    TS.tv_sec = 0;
    TS.tv_nsec = 20000000;
        
    status = func_task_init();
    if( status != RET_OK )
    {   printf( "func_task_init() error " );
        printf( "in func_task_body()>>>>\n" );
        rt_halt(); /*.......................*/  }

    Start_BMP();

    start_all_rt();
          
    while( stop_cmd == 0 )
    {   
        ft_wait_pulse();

        if(func_task_send_data() != RET_OK)
        {   printf( ">>>Error while calling func_task_send_data() " );
            printf( "in func_task_body\n" );
            rt_halt(); /*..............................................*/ }

        nanosleep(&TS, NULL);

        if( func_task_get_data( &DTX_1 )!= RET_OK)
        {   printf( ">>>Error while calling func_task_get_data() " );
            printf( "in func_task_body\n" );
            rt_halt(); /*..............................................*/ }

        TDB_Push( &DTX_1 );

    }
}

//=============================================================================
int ft_send_pulse( void )
{
    sem_post( &ft_semaphore );
    return RET_OK;
}

//=============================================================================
int ft_wait_pulse( void )
{
   sem_wait( &ft_semaphore );
   return RET_OK;
}

