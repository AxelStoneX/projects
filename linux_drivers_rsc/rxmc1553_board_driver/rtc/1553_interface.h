#ifndef _1553_INTERFACE_H_
#define _1553_INTERFACE_H_

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include "busapi.h"
#include "prc_control.h"
#include "lbm_bs.h"

#define BT_SLOT_SIZE    32      // Slot size (16 bit words in 1 subaddress)
#define BT_SLOT_N       32      // Number of slots (SA) for 1 bus address
#define BT_ADDR_N       32      // Number of bus addresses
#define BT_TRANS_MAX    12      // Max possible slots in one transaction

#define BT_BROAD_ADDR   31      // Broadcast address

#define BT_DEFAULT_RSZ  1       // Default ring size for cyclic buffer

#define MODE_CMD_SA1    0       // First Command code subaddress
#define MODE_CMD_SA2    31      // Second Command code subaddress
#define BR_TIME_SA      29      // Broadcast time message subaddress

#define CMD_SYNC_WORD   17      // Sync with data word command code
#define CMD_TRAN_VECT   16      // Transmit vector word command code

#define MAX_BUS_NUMBER     30
#define MAX_DEVICE_NUMBER  2
#define MAX_CHANNEL_NUMBER 2

#define MAX_CARD_NUMBER   ( MAX_CHANNEL_NUMBER * MAX_DEVICE_NUMBER ) 

#define MAX_TRANSACTION_NUMBER 50

#define BT_TRANSMIT      1
#define BT_RECEIVE       0

#define BT_MAX_BUS       30       // Max possible bus number

#define MAX_RT_NUMBER    32
#define MAX_SA_NUMBER    32

#define L_ON             1        // Logical on
#define L_OFF            0        // Logical off

#define BT_WCMASK_ALL  0xFFFFFFFF

// Return codes, specific for Bus Tools Control module
#define BT_ON         1  // Expected operation result
#define BT_OFF        0  // "Empty" operation result
#define BT_FAIL      -1  // Error condition detected
#define BT_BAD_PROC  11     // Failure return code for FIFO proc. functions

//-------------------------- FIFO Processing ----------------------------------
#define BT_RSZ_DEF     1   // Default ring size
#define BT_RSZ_TRAN    1   // Transmit ring size
#define BT_RSZ_RECV    2   // Receive ring size

#define BSYNC_CHAN_X   0   // Index in API_INT_FIFO.nUser[8] -> BSYNC chan index

#define MESS_PL_IX     0  // Index in API_INT_FIFO.pUser[8] -> MessPL object
#define BT_CARD_MAX    2  // Max. possible number of cards
#define BT_MAX_cN      (BT_CARD_MAX - 1)
#define BT_MPL_PRIO    25  // Message processing waiting task priority

//------------------------- Broadcast messages ---------------------------------
#define CBUFBROAD_MASK_N  31  // API_RT_CBUFBROAD.legal_wordcount[] number
#define BR_BUS_N   2          // Number of buses - broadcast message sources
#define BSYNC_AGE_MAX  100000 // 100_000
#define BSYNC_AGE_FAIL 12     // 12 * 12.5 ms = 150 ms
#define BSYNC_AGE_WORK  9     //  9 * 12.5 ms = 112.5 ms
#define PRF_CNT_MAX    99     // Processing frame counter - max value
// Broadcast messages processing - manual configuration !!!
#define BR_PROC_CX_DEF 0  // Default broadcast sync/time processing AIF index
// Broadcast Time message size in 16-bit words / bytes
#define BT_BTIME_SW    8   
#define BT_BTIME_BT  ( 2 * BT_BTIME_SW )

//------------------------- Task Priorities ------------------------------------
#define FUNC_TASK_PRIO  25
#define EXCH_TASK_PRIO  31
#define AIF_TASK_PRIO   29

//------------------------- Data Structures ------------------------------------

typedef struct
{   LWORD bus_number;
    LWORD card_number;
    LWORD remote_terminal;
    LWORD sub_address;
    LWORD transaction_direction;
    LWORD slots_number;
    LWORD word_count;
    LWORD last_sub_address;      //eSA = SA + SL - 1
    LWORD ring_size;
    LWORD init;
} BUS_PACKAGE;

typedef struct
{   int    dev;
    LWORD  bus_number;
    LWORD  channel;
    LWORD  init;   
} BUS_ENTRY;

typedef struct   // Message Processing Pipeline object
{ LWORD   init;
  LWORD   CardN;
  API_INT_FIFO *PIF;
} MessPL;

typedef struct BT_FIFO MESS_FIFO;
typedef void ( *p_void_func) ();  // Pointer to function with void fname(void) sign
typedef int  ( *P_AIF_FUNC )( LWORD cN, API_INT_FIFO *PIF );

// --- 1553_broadcast_messages.c -----
extern BUS_PACKAGE* BSyncPK[BR_BUS_N];
extern API_INT_FIFO AIF_bm[BR_BUS_N];  // Broadcast mess. proc. object
extern MessPL       MPL_bm[BR_BUS_N];
extern volatile int      sei7_pf;               // Processing frame counter


// --- 1553_bsync_processing.c -------
extern Q_INT BSyncTC, BSyncTP; // Selected BSync (CX=0) Curr/Prev time (quad)
extern volatile int   Proc_CX; // MessQ index for broadcast mess. processing
extern pthread_mutex_t muBS;
extern volatile ushort_t sei7_bt[BT_BTIME_SW];  // Broadcast time buffer

//---------------------------Global Variables-------------------------------------
extern LWORD        bus_to_card_number[BT_MAX_BUS];
extern BUS_ENTRY    bus_ent[MAX_CARD_NUMBER];
extern BUS_PACKAGE  busPK_heap[MAX_TRANSACTION_NUMBER];
extern int          empty_tr;  // Number of next empty transaction in heap
extern int          bm_transactions[BR_BUS_N];
extern int          bt_transactions[BR_BUS_N];
extern API_INT_FIFO AIF_bm[BR_BUS_N];
extern MessPL       MPL_bm[BR_BUS_N];

//------------------------Functions Prototypes------------------------------------

//------------------------1553_card_operations.c----------------------------------
int  init_all_cards( void );
int  init_single_card( LWORD bus_number, LWORD mode, int dev, LWORD channel );
int  register_single_card (LWORD bus_number, LWORD card_number, int dev, LWORD channel);
int  start_all_rt( void );
int  stop_all_rt( void );
void close_all_cards( void );

//--------------------1553_transaction_operations.c-------------------------------
int standart_transaction_initialization( int bus_number,  int remote_terminal,
                                         int sub_address, int word_count,
                                         int transaction_direction             );
int buffer_transmit( int transaction_index, SWORD* DATA_BUFFER, SWORD buffer_size );
int buffer_receive( int transaction_index, SWORD* DATA_BUFFER, SWORD buffer_size );
int Bus_PK_Init ( int bus_number, int remote_terminal, 
                  int sub_address, int word_count, 
                  int transaction_direction );
int BUS_PK_Conv( BUS_PACKAGE *PK, LWORD *cN, LWORD *RT, LWORD *SA, LWORD *tr, LWORD *SN );
int transaction_index_convert( int transaction_index, LWORD *cN, 
                               LWORD *RT, LWORD *SA, LWORD *tr, LWORD *SN );
int tr_idx_to_BUS_PK_conv( int transaction_index, BUS_PACKAGE *PK );

//-----------------------1553_buffers_operations.c--------------------------------------
int BT_ABUF_Init( LWORD cN, LWORD RT );
int BT_CBUF_Init( BUS_PACKAGE *PK );
int BT_CBUFBroad_InitOne( BUS_PACKAGE *PK, LWORD mask );
int BT_MBUF_Init_NoQ( BUS_PACKAGE *PK );
int BT_MBUF_Init( BUS_PACKAGE *PK);


//-----------------------1553_error_protection.c----------------------------------------
int check_tr_entry( int transaction_index );
int check_buf_repeating_entry( LWORD bus_number, 
                               LWORD remote_terminal, LWORD sub_address );                              
int check_bus_repeating_entry( LWORD bus_number, int dev, LWORD channel );
int check_bus_PK( int transaction_index, int transaction_direction );

//---------------------- 1553_broadcast_messages.c -------------------------------------
int Init_BMP( void );
int Init_BMPchan( LWORD bus_number, LWORD CX );
int BMP_Task( LWORD cN, struct api_int_fifo *PIF );
int Start_BMP( void );
void Stop_BMP( void );
API_INT_FIFO* BT_Get_AIF_BM( LWORD CardN );

//---------------------- 1553_bsync_processing.c ----------------------------------------
void Init_Proc_BSync( void );
int Proc_BSync( LWORD cN, int CX, MESS_FIFO *FM );
void Check_BSync( void );
void FreeRun_Step( void );
void FreeRun_Reset( void );
void Load_PRF( void );

//---------------------------- 1553_time_tags.c ------------------------------------------
void BT_TTag_Diff( BT1553_TIME *tG1, BT1553_TIME *tG2, Q_INT *df );
void BT_TTag_Conv( BT1553_TIME *ttG, Q_INT *ms );

//------------------------ 1553_btime_processing.c ---------------------------------------
void Init_Proc_BTime( void );
void Load_BTime( void );
int Proc_BTime( LWORD cN, int CX, MESS_FIFO *FM );
//void Show_BTime( DBF_TXT *dbT );

#endif
