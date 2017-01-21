#include "1553_interface.h"
#include "thread_api.h"

volatile ushort_t sei7_bt[BT_BTIME_SW];  // BTime buffer for Func. Task access
volatile ushort_t bt_BUF[BT_BTIME_SW];   // Primary Broadcast time buffer
volatile LWORD    bt_new = 0;
volatile LWORD    BTime_cnt[BR_BUS_N];

pthread_mutex_t mu_BT; // Mutex for Broadcast Time data access control

//============================================================================
void Init_Proc_BTime( void )
{   LWORD i;
    for( i=0; i<BR_BUS_N; i++ )
        BTime_cnt[i] = 0;
    for( i=0; i<BT_BTIME_SW; i++ )
    {   sei7_bt[i] = 0;
        bt_BUF[i] = 0;  /*.........*/ }

    pthread_mutex_init( &(mu_BT), NULL );  // Default: PRIO_INH. & REC._DISABLE
}

//============================================================================
void Load_BTime( void ) // load new value (if available) in bt_BUF[]
{   int status;
    if( bt_new == L_ON)
    {   //atomic_clr( &bt_new, L_ON );
        status = pthread_mutex_lock( &mu_BT );
        bt_new = L_OFF;
        memcpy((void*)(&sei7_bt[0]), (const void*)(&bt_BUF[0]), BT_BTIME_BT);
        status = pthread_mutex_unlock( &mu_BT ); /*........................*/ }
}

//============================================================================   
int Proc_BTime( LWORD cN, int CX, MESS_FIFO *FM )
{   API_RT_MBUF_READ MBR;
    LWORD RT, SA, tr, WC;
    const void *SRC;
    void *DST;
    int status;

    RT = FM->rtaddress;
    if( RT != BT_BROAD_ADDR )
        return BT_OFF;
    SA = FM->subaddress;
    if( SA != BR_TIME_SA )
        return BT_OFF;
    tr = FM->transrec;
    if( tr != BT_RECEIVE )
        return BT_OFF;    
    WC = FM->wordcount;
    if( WC != BT_BTIME_SW )
        return BT_OFF;

    status = pthread_mutex_lock( &mu_BT );
    if( status != EOK )
    {   printf("pthread_mutex_lock() error in Proc_BTime()\n " );
        return (BT_FAIL); /*.........................................*/ }

    BTime_cnt[CX]++;  //!!!

    if( CX == Proc_CX )
    {   status = BusTools_RT_MessageRead( cN, RT, SA, tr, FM->bufferID, &MBR );
        if( status != API_SUCCESS )
        {   printf("BusTools_RT_MessageRead() error in Proc_BTime()\n");
            return (BT_FAIL); /*........................................*/ }

        DST = (void*)( &(bt_BUF[0]) );
        SRC = (const void*)( &(MBR.mess_data[0]) );   //!!!

        memcpy( DST, SRC, BT_BTIME_BT );     // --- Data copy

        //atomic_set( &bt_new, L_ON );          // --- New data flag
        bt_new = L_ON;
    }

    status = pthread_mutex_unlock( &mu_BT );
    if( status != EOK )
    {   printf("BTime pthread_mutex_unlock() error in Proc_BTime()\n" );
        return (BT_FAIL); /*..........................................*/ }
    
    return ( BT_ON );  // Expected result
} // Proc_BTime()

//============================================================================ 
/*
void Show_BTime( DBF_TXT *dbT )
{   LWORD i;
    TDB_print( dbT, "BTime: ");
    for( i=0; i<BR_BUS_N; i++ )
         TDB_print( dbT, " %7d", BTime_cnt[i] );
    TDB_print( dbT, "\n" );
}
*/










