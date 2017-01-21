#ifndef _FUNC_SOFTWARE_H_
#define _FUNC_SOFTWARE_H_

#include <stdio.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <time.h>
#include <process.h>
#include "prog_dispatcher.h"
#include "1553_interface.h"
#include "lbm_bs.h"
#include "dbf_dv.h"

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

extern int conid;
extern int chid;

//--------------------------Function Prototypes---------------------------------

int func_software_init( void );

int func_software_send_data( void );

int func_software_get_data( DBF_TXT* DTX );

void* func_software_body( void *arg );


#endif
