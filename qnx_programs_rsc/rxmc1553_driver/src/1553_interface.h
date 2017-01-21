#ifndef _1553_INTERFACE_H_
#define _1553_INTERFACE_H_

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
//#include "func_software.h"
#include "busapi.h"
#include "lbm_bs.h"

#define BT_SLOT_SIZE    32      // Slot size (16 bit words in 1 subaddress)
#define BT_SLOT_N       32      // Number of slots (SA) for 1 bus address
#define BT_ADDR_N       32      // Number of bus addresses
#define BT_TRANS_MAX    12      // Max possible slots in one transaction

#define BT_BROAD_ADDR   31      // Broadcast address

#define MODE_CMD_SA1    0       // First Command code subaddress
#define MODE_CMD_SA2    31      // Second Command code subaddress
#define BR_TIME_SA      29      // Broadcast time message subaddress

#define CMD_SYNC_WORD   17      // Sync with data word command code
#define CMD_TRAN_VECT   16      // Transmit vector word command code

#define MAX_BUS_NUMBER     30
#define MAX_DEVICE_NUMBER  3
#define MAX_CHANNEL_NUMBER 2

#define MAX_CARD_NUMBER   ( MAX_CHANNEL_NUMBER * MAX_DEVICE_NUMBER ) 

#define MAX_TRANSACTION_NUMBER 50

typedef struct
{   LWORD bus_number;
    LWORD card_number;
    LWORD remote_terminal;
    LWORD sub_address;
    LWORD transaction_direction;
    LWORD slots_number;
    LWORD word_count;
    LWORD last_sub_address;      //eSA = SA + SL - 1
    LWORD init;
    SWORD *DATA_BUFFER;
} BUS_PACKAGE;

typedef struct
{   int    dev;
    LWORD  bus_number;
    LWORD  channel;
    LWORD  init;   
} BUS_ENTRY;

#define BT_TRANSMIT  1
#define BT_RECEIVE   0

#define BT_MAX_BUS   30          // Max possible bus number

#define MAX_RT_NUMBER 32
#define MAX_SA_NUMBER 32

#define L_ON  1                  // Logical on
#define L_OFF 0                  // Logical off

//---------------------------Global Variables----------------------------------
static SWORD       bus_to_card_number[BT_MAX_BUS];
static BUS_ENTRY   bus_ent[MAX_CARD_NUMBER];
static BUS_PACKAGE busPK_heap[MAX_TRANSACTION_NUMBER];
int    empty_tr;  // Number of next empty transaction in heap

//------------------------Functions Prototypes---------------------------------

int init_all_cards( void );

int init_single_card( LWORD bus_number, LWORD mode, int dev, LWORD channel );

int buffers_initialization( int bus_number,  int remote_terminal,
                            int sub_address, int word_count,
                            int transaction_direction             );

int buffer_transmit( int transaction_index, ushort_t* DATA_BUFFER );

int buffer_receive( int transaction_index, void* DATA_BUFFER, 
                                           LWORD buffer_size );

int check_tr_entry( int transaction_index );

int check_buf_repeating_entry( LWORD bus_number, 
                               LWORD remote_terminal, LWORD sub_address );
                               
int check_bus_repeating_entry( LWORD bus_number, int dev, LWORD channel );

int check_bus_PK( int transaction_index, int transaction_direction );

#endif
