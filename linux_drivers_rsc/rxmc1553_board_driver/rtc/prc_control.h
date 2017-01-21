#ifndef _PRC_CONTROL_H_
#define _PRC_CONTROL_H_

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include "busapi.h"
#include "lbm_bs.h"
#include "dbf_dv.h"
#include "1553_interface.h"
#include "thread_api.h"

// Edit to control data transmit/receive destination
#define TRANSMIT_BUS  0
#define TRANSMIT_RT   15
#define TRANSMIT_SA   15
#define TRANSMIT_WC   64

#define RECEIVE_BUS 0
#define RECEIVE_RT  16
#define RECEIVE_SA  15
#define RECEIVE_WC  64

#define MAX_BUFFER_SIZE  56

// Threads identificators
extern int conid;
extern int chid;

// Process control variables
volatile LWORD stop_cmd;
extern   DBF_TXT DTX_1;

//--------------------------- Root Task -------------------------------------
void  rt_final( void );
void  rt_halt( void );

//--------------------------Functional Task-----------------------------------
int   func_task_init( void );
int   func_task_send_data( void );
int   func_task_get_data( DBF_TXT* DTX );
void* func_task_body( void *arg );
int   ft_send_pulse( void );
int   ft_wait_pulse( void );

//---------------------------Exchange Task --------------------------------------
int exchange_task_init( void );
void* exchange_task_body( void *arg );

#endif
