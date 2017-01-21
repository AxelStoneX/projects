#include "1553_interface.h"
#include "thread_api.h"

volatile int sei7_pf;  //Processing frame counter for access from func. task
volatile int prf_c=0;  // prf_c - primary processing frame counter

volatile LWORD BSYNC_age[BR_BUS_N], BSYNC_cnt[BR_BUS_N];
volatile LWORD BSYNC_pul[BR_BUS_N], BSYNC_PulT=0, BSYNC_sw = 0;
volatile LWORD BS_Age_Fail, BS_Age_Work;
volatile int   Proc_CX; // MessQ index for broadcast mess. processing

Q_INT BSyncTC, BSyncTP; // Selected BSync (CX=0) Curr/Prev time (quad)
Q_INT BSyncMP;          // Time delta message --> processing
LWORD BSYNC_tc = 0;     // Task cycle counter
pthread_mutex_t muBS;   // Mutex for Broadcast Sync data access control

//============================================================================

void Init_Proc_BSync( void )
{   int i;
    BS_Age_Fail = BSYNC_AGE_FAIL;
    BS_Age_Work = BSYNC_AGE_WORK;
    Proc_CX = BR_PROC_CX_DEF;
    FreeRun_Reset();  // BSYNC_tc = 10 --> free running pulse counter

    for( i=0; i<BR_BUS_N; i++ )
    {   BSYNC_age[i] = BSYNC_AGE_MAX;
        BSYNC_cnt[i] = 0;
        BSYNC_pul[i] = 0; /*.........*/ }

   pthread_mutex_init( &(muBS), NULL ); // Default: PRIO_INH.& REC._DISABLE
}

//============================================================================
int Proc_BSync( LWORD cN, int CX, MESS_FIFO *FM )
{   API_RT_MBUF_READ MBR;
    BT1553_TIME tgC;  // Current Time
    LWORD  RT, SA, tr, WC, PCN;
    int status, status1, status2, status3, common_status;

    RT = FM->rtaddress;
    if( RT != BT_BROAD_ADDR )
        return BT_OFF;

    SA = FM->subaddress;
    if( SA != MODE_CMD_SA1 && SA != MODE_CMD_SA2 )
        return BT_OFF;

    tr = FM->transrec;
    if( tr != BT_RECEIVE )
        return BT_OFF;

    WC = FM->wordcount;
    if( WC != CMD_SYNC_WORD )
        return BT_OFF;

   // Now it seems that it is a Broadcast Sync Message

   status = pthread_mutex_lock( &muBS );
   BSYNC_cnt[CX]++;                                          //!!!!!!!!!!!!!

   if( status != EOK )
   {   printf("pthread_mutex_lock() error in Proc_BSync()\n" );
       return BT_FAIL; /*......................................*/ }

   BSYNC_age[CX] = 0;         // Reset age counter
   common_status = BT_ON;     // Default return code is OK
   if( CX == Proc_CX )
   {  status1 = BusTools_RT_MessageRead( cN, RT, SA, tr, FM->bufferID, &MBR );
      prf_c = (int)( MBR.mess_data[0] );                    //!!!!!!!!!
      
      BSYNC_pul[CX]++;
 
      ft_send_pulse();

      BSyncTP = BSyncTC;
      BT_TTag_Conv( &(MBR.time), &BSyncTC );
 
      status3 = BusTools_TimeTagRead( cN, &tgC );
      BT_TTag_Diff( &tgC, &(MBR.time), &BSyncMP );
      
      if( status1 != API_SUCCESS )
      {   printf("BSYNC read error in Proc_BSync()\n");
          common_status = BT_FAIL; /*..................*/ }
      if( status2 != EOK )
      {   printf("BSYNC pulse error in Proc_BSync()\n");
          common_status = BT_FAIL; /*..................*/ }
      if( status3 != API_SUCCESS )
      {   printf("BSYNC TTag error in Proc_BSync()\n");
          common_status = BT_FAIL; /*..................*/ }
   }

   status = pthread_mutex_unlock( &muBS );
   if( status != EOK )
   {   printf("BSYNC unlock error in Proc_BSync()\n");
       common_status = BT_FAIL; /*.....................*/ }

   return( common_status ); // Expected result
}

//============================================================================
void Check_BSync( void ) // To be called in exchange task main cycle
{   
    int status;
    LWORD cx, FREE_RUN = L_OFF;  // Default free running flag is OFF
    status = pthread_mutex_lock( &muBS );
    if( status != EOK )
        //xt_halt( "Check_BSync lock error", status );
        rt_halt();

    for( cx=0; cx<BR_BUS_N; cx++ ) // Increment all BSYNC age counters
    {   BSYNC_age[cx]++;
        if( BSYNC_age[cx] > BSYNC_AGE_MAX )
            BSYNC_age[cx] = BSYNC_AGE_MAX;  }

    if( BSYNC_age[Proc_CX] > BS_Age_Fail )  // Selected BSYNC source is lost
    {   cx = 0;                             // --> try select some other
        while( 1 )                          // Scan all possible BSYNC sources
        {
            if( BSYNC_age[cx] < BS_Age_Work ) // Select first suitable source
            {   Proc_CX = cx;
                BSYNC_sw++;
                break; /*...................*/ } // --> to another source
            cx++;
            if( cx >= BR_BUS_N )
            {   FREE_RUN = L_ON;
                break; /*........*/ } // No BSYNC source
        }
    }
    if( FREE_RUN == L_ON )
        FreeRun_Step();  // System timer 100 ms pulse generation
    else
        FreeRun_Reset(); // -->pulse to be at once

    status = pthread_mutex_unlock( &muBS );
    if( status != EOK )
    {   printf( "pthread_mutex_unlock() error in Check_BSync()\n" );
        rt_halt(); /*............................................*/ }
}

//============================================================================
void FreeRun_Step( void )
{   int status;
    BSYNC_tc++;
    if( BSYNC_tc >= 8 )
    {   
        BSYNC_tc = 0;
        BSYNC_PulT++;
        prf_c++;
        if( prf_c > PRF_CNT_MAX )
            prf_c = 0;

        ft_send_pulse();

    }
}

//============================================================================
void FreeRun_Reset( void )
{    
    BSYNC_tc = 10;
}

//============================================================================
void Load_PRF( void )
{   
    int status;
    status = pthread_mutex_lock( &muBS );
    sei7_pf = prf_c;
    status = pthread_mutex_unlock( &muBS );
}

















 
