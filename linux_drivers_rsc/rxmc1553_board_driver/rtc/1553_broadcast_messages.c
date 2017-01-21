#include "1553_interface.h"

int          bs_transactions[BR_BUS_N];
int          bt_transactions[BR_BUS_N];
API_INT_FIFO AIF_bm[BR_BUS_N];
MessPL       MPL_bm[BR_BUS_N];

//===================================================================================
int Init_BMP( void )
{
    LWORD bus_number;
    LWORD CX;
    int status;
    int i;
    printf( "\nBroadcast messages processing initiation:\n" );

    for( i=0; i<BR_BUS_N; i++ )
        bs_transactions[i] = -1;

    for( i=0; i<BR_BUS_N; i++ )
        bt_transactions[i] = -1;
    
    //--------------------------------------------------------------------------
    bus_number = 0;
    CX = 0;
    status = Init_BMPchan( bus_number, CX ); //Broadcast message source #1
    if( status != RET_OK )
    {   printf( "Init_BMPchan() error in Init_BMP()\n");
        printf( "Info: bus_number=%d\n", bus_number );
        return( RET_FAIL ); /*...........................*/ }

    bus_number = 1;
    CX = 1;
    status = Init_BMPchan(bus_number, CX); //Broadcast message source #1
    if( status != RET_OK )
    {   printf( "Init_BMPchan() error in Init_BMP()\n");
        printf( "Info: bus_number=%d\n", bus_number );
        return( RET_FAIL ); /*...........................*/ }

    //--------------------------------------------------------------------------
    Init_Proc_BSync();  // Broadcast Sync message processing init.
    Init_Proc_BTime();  // Broadcast Time message processing init.
    printf( "\nInit_BMP is correct\n" );
    return RET_OK;
}

//===================================================================================
int Init_BMPchan( LWORD bus_number, LWORD CX )
{
    BUS_PACKAGE *PKbS;
    BUS_PACKAGE *PKbT;
    LWORD card_number;
    LWORD RT;
    LWORD SA;
    LWORD WC;
    LWORD TR;
    LWORD mask;
    int status;
    int bs_transaction_index;
    int bt_transaction_index;
    API_INT_FIFO *PIF;
    MessPL       *MPL;

    PIF = &( AIF_bm[CX] );
    MPL = &( MPL_bm[CX] );


    // -------------- Broadcast Sync message buffer ----------------
    RT = BT_BROAD_ADDR;
    SA = MODE_CMD_SA2;
    WC = 1;
    TR = BT_RECEIVE;

    card_number = bus_to_card_number[bus_number];
    if( card_number <= -1 )
    {   printf( "Incorrect bus_number in Init_BMP_chan()\n" );
        return RET_FAIL; /*....................................*/ }

    if( CX >= BR_BUS_N )
    {   printf( "Bad channel index=%d in Init_BMPchan\n", CX );
        return RET_FAIL; /*....................................*/  }

    if( empty_tr > MAX_TRANSACTION_NUMBER )
    {   printf( ">>>>Too many initialized transactions " );
        printf( "in transaction_initialization()\n" );
        return RET_FAIL; /*...........................*/ }

    bs_transaction_index  =  Bus_PK_Init( bus_number, RT, SA, WC, TR);
    PKbS->ring_size = 1;
    bs_transactions[CX] = bs_transaction_index;

    status = tr_idx_to_BUS_PK_conv( bs_transaction_index, PKbS );
    if( status != RET_OK )
    {   printf( "tr_idx_to_BUS_PK_conv() error in transaction_initialization\n" );
        return RET_FAIL; /*.....................................................*/ }

    status = BT_ABUF_Init(PKbS->card_number, PKbS->remote_terminal);
    if( status != RET_OK )
    {   printf( "BT_ABUF_Init() error in transaction_initialization\n" );
        return RET_FAIL; /*.....................................................*/ }

    //Set_MessQ_Pair( cN, CX );        // One pair of CardN -> CX switches

    mask = (0x01 << CMD_SYNC_WORD );
    status = BT_CBUFBroad_InitOne( PKbS, mask );
    if( status != RET_OK )
    {   printf("BT_CBUFBroad_InitOne() error in Init_BMPchan()\n");
        return RET_FAIL; /*.........................................*/ }

    status = BT_MBUF_Init( PKbS );  // Interrupt Queue connection is enabled
    if( status != RET_OK )
    {   printf("BT_MBUF_Init() error in Init_BMPchan()\n");
        return RET_FAIL; /*.........................................*/ }

    printf( "BSync transaction had been ibitialiazed\n" );
    printf( "Info: tran_idx=%d CX=%d", bs_transaction_index, CX);

    // -------------- Broadcast Time message buffer ----------------
    printf( "Starting Broadcast Time message buffers init\n" );
    RT = BT_BROAD_ADDR;
    SA = BR_TIME_SA;
    WC = 8;
    TR = BT_RECEIVE;

    if( empty_tr > MAX_TRANSACTION_NUMBER )
    {   printf( ">>>>Too many initialized transactions " );
        printf( "in transaction_initialization()\n" );
        return RET_FAIL; /*...........................*/ }

    bt_transaction_index  =  Bus_PK_Init( bus_number, RT, SA, WC, TR);
    PKbS->ring_size = 1;
    bt_transactions[CX] = bt_transaction_index;

    status = tr_idx_to_BUS_PK_conv( bt_transaction_index, PKbT );
    if( status != RET_OK )
    {   printf( "tr_idx_to_BUS_PK_conv() error in transaction_initialization\n" );
        return RET_FAIL; /*.....................................................*/ }

    mask =  BT_WCMASK_ALL;
    //mask =  BT_WCMASK_NOT;

    status = BT_CBUFBroad_InitOne( PKbT, mask );
    if( status != RET_OK )
    {   printf("BT_CBUFBroad_InitOne() error in Init_BMPchan()\n");
        return RET_FAIL; /*.........................................*/ }

    status = BT_MBUF_Init( PKbT );  // Interrupt Queue connection is enabled
    if( status != RET_OK )
    {   printf("BT_MBUF_Init() error in Init_BMPchan()\n");
        return RET_FAIL; /*.........................................*/ }

    printf( "BSync transaction had been ibitialiazed\n" );
    printf( "Info: tran_idx=%d CX=%d", bt_transaction_index, CX);

    // ---------------API_INT_FIFO object initialization ---------------------------
    BT_AIF_Init( PIF, MPL, BMP_Task, BT_MPL_PRIO, card_number );  // PRIO = 29

    status = BT_AIF_SetFilterRTM( PIF, PKbS ); // Filter type --> EVENT_RT_MESSAGE
    if( status != RET_OK )
    {   printf( "BT_AIF_SetFilterRTM() error in Init_BMPchan()\n" );
        return RET_FAIL; /*.........................................*/ }

    status = BT_AIF_SetFilterRTM( PIF, PKbT ); // Filter type --> EVENT_RT_MESSAGE
    if( status != RET_OK )
    {   printf( "BT_AIF_SetFilterRTM() error in Init_BMPchan()\n" );
        return RET_FAIL; /*.........................................*/ }

    PIF->nUser[BSYNC_CHAN_X] = (int)CX;  //Broadcast message channel index

    printf( "Init_BMPchan final CX=%d is correct\n", CX );

    return RET_OK;
}

//===================================================================================
int BMP_Task( LWORD cN, struct api_int_fifo *PIF )
{
    int status, tail, head, mask, ix, CX;
    MessPL *MPL;  //Connected to PIF MessPL object

    status = BT_AIF_Check( PIF );
    if( status != RET_OK )
    {   printf( "brM PIF corrupted in BMP_Task()\n" );
        //BT_Halt();
        rt_halt();
        return(BT_BAD_PROC);/*........................*/ }
    else
    {   MPL = (MessPL*)( PIF->pUser[MESS_PL_IX] );       }

    if( PIF-> FilterType != EVENT_RT_MESSAGE )
    {   printf( "Bad FIFO->FilterType in BMP_Task()\n" );
        //BT_Halt();
        rt_halt();
        return(BT_BAD_PROC); /*........................*/ }

    CX = PIF->nUser[BSYNC_CHAN_X];  // PIF->nUser[0]
    if( CX >= BR_BUS_N )
    {   printf( "Bad CX=%d in BMP_Task()\n", CX );
        //BT_Halt();
        rt_halt();
        return(BT_BAD_PROC); /*.........................*/ }

    head = PIF->head_index;
    tail = PIF->tail_index;
    mask = PIF->mask_index;

    while( tail != head )  // Not processed events in the message queue
    {   if( (PIF->fifo[tail].event_type & EVENT_RT_MESSAGE) == 0 )
        {   printf( "Bad FIFO->event_type in BMP_Task()\n" );
            //BT_Halt();
            rt_halt();
            return(BT_BAD_PROC); /*...............................*/ }

        status = Proc_BSync( cN, CX, &(PIF->fifo[tail]) );
        if( status == BT_FAIL )
        {   printf( "Proc_BSync() error in BMP_Task()\n" );
            printf( "Info: cN = %d\n", cN );
            //BT_Halt();
            return(BT_BAD_PROC); /*..........................*/  }

        if( status == BT_OFF ) // Current event is not BSync message
        {   //Attempt to process it as BTime message
            status = Proc_BTime( cN, CX, (&PIF->fifo[tail]) );
            if( status == BT_FAIL )
            {   printf( "Proc_BTime() error in BMP_Task()\n" );
                printf( "Info: cN = %d\n", cN );
                //BT_Halt();
                rt_halt();
                return(BT_BAD_PROC); /*.......................*/ }
        }

        tail++;
        tail = tail & mask;
    }
    
    PIF->tail_index = tail;
    return ( API_SUCCESS ); //Non-zero return means request to stop task processing
}

//===================================================================================
int Start_BMP( void )
{
    int status;
    int CX;

    for( CX=0; CX < BR_BUS_N; CX++ )
    {   status = BT_AIF_Connect( &AIF_bm[CX] );
        if( status != RET_OK )
        {   printf( "BT_AIF_Connect() error in StartBMP()\n" );
            printf( "Info: CX=%d\n", CX );
            return RET_FAIL; /*...............................*/ }
    }
    return RET_OK;
}
           
//===================================================================================
void Stop_BMP( void )
{   
    int CX;
    for( CX=0; CX < BR_BUS_N; CX++ )
         BT_AIF_Close( &AIF_bm[CX] );   
}

//===================================================================================
API_INT_FIFO* BT_Get_AIF_BM( LWORD CardN )
{   API_INT_FIFO  *PIF;
    MessPL        *MPL;
    int ix, CX, cnt = 0;

    for( ix=0; ix < BR_BUS_N; ix++ )
    {   PIF = &( AIF_bm[ix] );
        MPL = (MessPL*)( PIF->pUser[MESS_PL_IX] );
        if( MPL->CardN == CardN )
        {   CX = ix;
            cnt++;   }
    }
    if( cnt == 1)
        return ( PIF );
    else
        return ( NULL );
}

    
        
