/*============================================================================*
 * FILE:                  Q N X _ I R Q _ S E T U P . C
 *============================================================================*
 *
 * COPYRIGHT (C) 1994 - 2010 BY
 *          GE INTELLIGENT PLATFORMS, INC., SANTA BARBARA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY GE
 *          INTELLIGENT PLATFORMS, INC
 *
 *===========================================================================*
 *
 * FUNCTION:   BusTools/1553-API Library:
 *             Hardware and software interrupt component.
 *             This file supports hardware interrupts and 
 *             software-only interrupts.
 *
 * DESCRIPTION:
 *
 * API ENTRY POINTS: (used by libbusapi.so.):
 *    vbtInterruptSetup       Initializes hardware and/or software interrupts
 *                            for a specified adapter offset.
 *    vbtInterruptClose       Shuts down hardware and/or software interrupts
 *                            for a specified adapter.  Closes all interrupt
 *                            functions if this is the final board being closed.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *    intTimerCallback        Interval timer callback procedure.
 *
 *===========================================================================*/
 
/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------     
  2/21/2001    Add hardware and software interrupt support

 
 *---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/                                                                                                   
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>  // @@@QQ
#include <pthread.h>
#include <sys/time.h> 
#include <sys/ioctl.h> 
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <hw/inout.h>
#include <syslog.h>

#include "qnx_irq_tst.h"  // @@@QQ

#define SA_RESTART 0
volatile int id0[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

typedef struct _hwevent {
   unsigned device;
   unsigned base_addr;
   unsigned int_channel;
   unsigned IRQnum;
   struct sigevent event;
}HWEVENT; 

HWEVENT hwevent[MAX_BTA];

#define ISA_GET_IRQ         0x13c6d
#define PCI_GET_INTERRUPT   0x13c64
// #define POLLING_TIMER_THREAD_PRIORITY 4
#define POLLING_TIMER_THREAD_PRIORITY   33    // Changed 12.05.2011

void intProc(int);
void timerFunc(int);
void * interruptThreadFunction(void * );
static BT_U32BIT bmRecordTime[MAX_BTA] = {0,};

static UINT  timerId = 0;            // Timer ID used to close the timer callback.   
static volatile int intEnabled[MAX_BTA] = {0,};
struct itimerval itimer;

struct sigaction act;
static int hIntEvent[MAX_BTA];          // Interrupt signal event, if it exists.

timer_t iTimer;
struct itimerspec tValue;
struct sigevent tevp;
pthread_t polling_timer_thread;
pthread_t iThread[MAX_BTA];
/*---------------------------------------------------------------------------*
 *          LOCAL ROUTINES, NOT REFERENCED OUTSIDE OF THIS MODULE
 *---------------------------------------------------------------------------*/

int delay_ms(int ms) 
{
   struct timespec req, rem;

   memset(&req, 0, sizeof(req));
   memset(&rem, 0, sizeof(rem));

   req.tv_nsec = (ms * 1000000);

   while(1)
   {
      if(nanosleep(&req, &rem) == 0)
         break;
      if(errno != EINTR)
      {
         //printf(" <DEBUG> error in delay_ms - status: %d\n", errno);
         return errno;
      }
      //printf(" <DEBUG> delay_ms interrupted: %ld, %ld\n", rem.tv_nsec, rem.tv_sec);
      req.tv_nsec = rem.tv_nsec;
      req.tv_sec = rem.tv_sec;
   }
   return 0;
}

// >>----->>>
void  BT_TTag_Conv( BT1553_TIME *ttG, Q_INT *ms );
Q_INT PTime_D[MAX_BTA], PTime_P[MAX_BTA], PTime_C[MAX_BTA];  // Diff / Prev / Curr
Q_INT PTime_Q;

void Load_PTime_D( Q_INT *PD )
{ int cx;  for( cx=0; cx< MAX_BTA; cx++ ) *( PD +cx ) = PTime_D[cx];  }

int get_polling_interval(void) { return api_polling_interval;  }
// >>--->>>

/*===========================================================================*
 * ENTRY POINT:           i n t T i m e r C a l l b a c k
 *===========================================================================*
 *
 * FUNCTION:    Windows multimedia timer callback procedure.
 *              See Multimedia function timeSetEvent() for details.
 *
 * DESCRIPTION: Count up the time to the BM Recording event.  Call vbt Notify
 *              for each board which is open, so it can process interrupts.
 *
 *      It will return:
 *              Nothing.
 *===========================================================================*/
 void* polling_timer_func(void* val) 
 {  BT1553_TIME tgC;
    struct sched_param cur_prm;
    int cardnum, cx, RS, cur_policy;
    short temp=0;
    pthread_t cur_thread;
    struct timespec td;   /* @@@TD for time delay imitation */

    // >>----->>>
    cur_thread = pthread_self();
    RS = pthread_getschedparam( cur_thread, &cur_policy, &cur_prm );

    printf( "Polling task started, timerId=%d   ", timerId );   
    printf( "prio=%d   policy=%d\n", cur_prm.sched_priority, cur_policy );
    td.tv_sec = 0;  td.tv_nsec = 10000000;  // 10_000_000 <- Time Delay
   // >>----->>>

    while(1)
    {
      delay_ms(api_polling_interval);
 //      delay_ms( 3 );  // @@@QQ
       for(cardnum=0; cardnum < MAX_BTA; cardnum++)
       {
          if(bt_inited[cardnum] <= 0) continue;
          
          // >>--->>> Time step measurement
          cx = cardnum;    RS = BusTools_TimeTagRead( cx, &tgC ); // @@@QQ
          PTime_P[cx] = PTime_C[cx];
          BT_TTag_Conv( &tgC, &(PTime_C[cx]) );
          PTime_D[cx] = PTime_C[cx] - PTime_P[cx];
          // >>--->>>

          if(_HW_FPGARev[cardnum] >= 0x388)
          {
             if((temp = vbtGetHWRegister(cardnum, HWREG_HEART_BEAT)) == heartbeat[cardnum])
                channel_status[cardnum].wcs_pulse = 1;
             heartbeat[cardnum] = temp;
          }

          // count the time to next BM recording event
          bmRecordTime[cardnum] += api_polling_interval;
           {
             // printf( " TM> %d  ", cardnum );       // @@@TD
             // nanosleep( &td, NULL );              // @@@TD
             RS = pthread_mutex_lock( &muIQ );    // @@@QQ1
             vbtNotify(cardnum, &bmRecordTime[cardnum]);
             RS = pthread_mutex_unlock( &muIQ );  // @@@QQ1
           }
       }
    }
 }

// >>----->>>
EV_CNT ECT[ECT_SIZE];
int    BT_IRQ_on=0, BT_IRQ_off=0;

void Event_CNT_Init( int ix )
{  memset( (void*)(  &(ECT[ix]) ), 0, sizeof(EV_CNT) );  }

struct sigevent *irq_handler(void *arg, int iid) 
{
   HWEVENT  *hwev = (HWEVENT *)arg;
   int device;
   int is_irq = 0;
   int indx;
   int chind[] = {1,2,4,8};

   int RS;    // @@QQQ
   int nch = 0;
   int host_interface;
   unsigned base_addr;
   unsigned short idata;
   EV_CNT *ECP;

   // if(hwev == NULL) return(NULL);          // @@AQQ - commented
   RS = InterruptMask( hwev->IRQnum, iid );   // @@@QQ !!!

   base_addr = hwev->base_addr;
   device = hwev->device;
   hwev->int_channel = 0;

   if(device >= MAX_BTA) return(NULL);
  
   host_interface = *(unsigned short *)base_addr;
   ECP = &(  ECT[device] );

   if((host_interface & 0x000f) != 0) nch = (host_interface & 0x7c0) >> 6;
   else                               nch = (host_interface & 0x3000)>>12;

   // Scan Hardware looking for source of interrupt
   for( indx = 0; indx < nch; indx++ ) 
    {
      idata = *(unsigned short *)(base_addr + (indx*0x200000) + 0x800);
      if(idata & 0x200)
      {
        *(unsigned short *)(base_addr + (indx*0x200000) + 0x816) = 0x0;
        if(idata & 0x4000) { is_irq=1;  hwev->int_channel += chind[indx];  }
      }
    }
	
 
  if(is_irq)
    { 
      atomic_add( &BT_IRQ_on,  1 );
      atomic_add( &( ECP->IRQ_on ),  1 ); 
      // RS = InterruptUnmask( hwev->IRQnum, iid );   // @@@QQ !!!
      // return(NULL);
      // RS = InterruptMask( hwev->IRQnum, iid );   // @@@QQ !!!
      return( &hwev->event );
     }
   else
    { atomic_add( &BT_IRQ_off, 1 ); 
      atomic_add( &( ECP->IRQ_off ),  1 ); 
      RS = InterruptUnmask( hwev->IRQnum, iid );   // @@@QQ !!!
      return(NULL);                                 }
}

/*===========================================================================*
 * ENTRY POINT:           q n x _ i r q _ s e t u p
 *===========================================================================*
 *
 * FUNCTION:    Setup specified adapter for interrupt processing.
 *
 * DESCRIPTION: Setup the hardware interrupt if hardware interrupts are
 *              enabled, and setup the polling interrupt if not setup already.
 *
 *      It will return:
 *              Nothing.
 *===========================================================================*/
pthread_mutex_t muIQ;  // @@@QQ1 --> mutex for Interrupt Queue control

int vbtInterruptSetup(unsigned int cardnum, int hw_int_enable, int api_dev)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   short *board_short_addr;  // Pointer to Host Interface Interrupt Registers
   unsigned IRQnum;

   hIntEvent[cardnum] = 0;
   /*******************************************************************
   *  Setup the hardware interrupt function if interrupts enabled,
   *   but only if HW interrupt is defined and if this is not the
   *   software version of the driver.
   *******************************************************************/

   if(hw_int_enable > 0 &&  hw_int_enable < 4)
   {
      if(hw_int_enable == API_SW_INTERRUPT || hw_int_enable ==  API_HW_INTERRUPT) //Polling mode no H/W interrupts
      {
// @@@>>> if(timerId==0) vbtSetPolling(api_polling_interval,TIMER_START);
// @@@>>>  else          timerId++;
//   >>--->>>
         // @@@>>> timerId++; is done in vbtSetPolling()
         pthread_mutex_init( &( muIQ ), NULL ); // @@@QQ1
         vbtSetPolling(api_polling_interval,TIMER_START);
//   >>--->>>

         if(hw_int_enable == API_SW_INTERRUPT) return 0;
      }
      vbtSetHWRegister(cardnum, HWREG_WRITE_INTERRUPT, 0);  // clear the interrupt write register
      
      if ( (CurrentCardType[cardnum] == PCI1553) ||
           (CurrentCardType[cardnum] == PMC1553) )
      {
         if ( _HW_PROMRev[cardnum] < 3 )
         {
            return API_OUTDATED_FIRMWARE; // Must have at least 2.61 firmware installed
         }
         if ( (IRQnum = vbtGetInterrupt(api_device[cardnum])) == 0 )
            return BTD_BAD_HW_INTERRUPT;
      }
      else if (CurrentCardType[cardnum] == QPCI1553)
      {
         BT_U16BIT * plx_base;
         if ( (IRQnum = vbtGetInterrupt(api_device[cardnum])) == 0 )
            return BTD_BAD_HW_INTERRUPT;
         plx_base = (BT_U16BIT *)bt_iobase[cardnum];
         plx_base[38] |= 0x48;
      }
      else if (CurrentCardType[cardnum]  == QPCX1553 || 
                CurrentCardType[cardnum] == QCP1553)
      {
         BT_U16BIT * plx_base;
         if ( (IRQnum = vbtGetInterrupt(api_device[cardnum])) == 0 )
            return BTD_BAD_HW_INTERRUPT;
         plx_base = (BT_U16BIT *)bt_iobase[cardnum];
         plx_base[52] |= 0x900;
      }
      else if ( CurrentCardType[cardnum] == Q1041553P || 
                CurrentCardType[cardnum] == QPMC1553)
      {
         if ( (IRQnum = vbtGetInterrupt(api_device[cardnum])) == 0 )
            return BTD_BAD_HW_INTERRUPT;
      }
      else if ( CurrentCardType[cardnum] == ISA1553 || 
                CurrentCardType[cardnum] == PCC1553 ||
                CurrentCardType[cardnum] == Q1041553)
      {
         if ( (IRQnum = vbtGetInterrupt(cardnum)) == 0 )
            return BTD_BAD_HW_INTERRUPT;
          // Clear interrupt by writing a zero to WRITE_INTERRUPT_BIT (Bit 9)
          //  of WRITE_INTERRUPT_REG(HWREG_WRITE_INTERRUPT)(reg 0xB/0x16-0x17).
          // Load the ISA-1553 interrupt control register with the interrupt number.
         board_short_addr = (short *)bt_PageAddr[cardnum][3];
         board_short_addr[HIR_IRQ_ENABLE] = (short)(1<<IRQnum); // Program the interrupt number
      }
      else
         return API_NO_BUILD_SUPPORT;   // Current API does not support card

      printf("vbtInterruptSetup: cN=%d, api_dev=%d, IRQn=%d, base_addr=0x%08x  ",
                     cardnum, api_dev, IRQnum, (unsigned)bt_PageAddr[cardnum][3] );          
      printf(" id0[api_dev]=%d\n", id0[api_dev] );

      if(id0[api_dev] == -1)
      {            
         Event_CNT_Init( api_dev ); // @@@ZZ
         hwevent[api_dev].base_addr = (unsigned)bt_PageAddr[cardnum][3]; 
         hwevent[api_dev].device = api_dev;
         hwevent[api_dev].int_channel = 0;
         hwevent[api_dev].IRQnum = IRQnum;    
         pthread_create(&iThread[api_dev],NULL,(void *)intProc,(void *)&api_dev);
      }
      else
      
      hIntEvent[cardnum] = 1;
      // Initialize the hardware so interrupts are enabled...
      //  (HWREG_CONTROL1) to enable interrupts.  CPU_INTERRUPT
      api_writehwreg_or(cardnum, HWREG_CONTROL1, CR1_INT_ENABLE);
      return API_SUCCESS;
   }
   return API_SUCCESS;
} // vbtInterruptSetup()

#define BT_DEV_CHANS 2 // Number of channels in one device

void intProc(int int_data)
{ 
  int device,i;
  int RS, cur_policy, iid, irqN, cN, iC; // @@@QQ
  int chid[4][4] = { {0,1,2,3}, {4,5,6,7}, {8,9,10,11}, {12,13,14,15} };
  device = *(int *)int_data;

  struct sched_param cur_prm; // @@@QQ
  pthread_t cur_thread;       // @@@QQ
  HWEVENT  *HEV;              // @@@QQ
  EV_CNT   *ECP;              // @@@QQ
  struct timespec td;   /* @@@TD for time delay imitation */

   // >>----->>>
   cur_thread = pthread_self();
   RS = pthread_getschedparam( cur_thread, &cur_policy, &cur_prm );
   printf( "intProc started, device = %d   ", device ); // @@@QQ
   printf( "prio=%d  policy=%d\n", cur_prm.sched_priority, cur_policy );
   ECP = &( ECT[device] );    HEV = &( hwevent[device] );
   td.tv_sec = 0;  td.tv_nsec = 10000000;  // @@@TD >>> 10_000_000 <- Time Delay
   // >>----->>>

   // Catch Hardware Interrupts. Turn into signals.
   ThreadCtl(_NTO_TCTL_IO,0); // Allow access to I/O operations
   SIGEV_INTR_INIT(&hwevent[device].event);
   if((id0[device] = InterruptAttach(hwevent[device].IRQnum, 
                     (void*) &irq_handler, &hwevent[device], 
                      sizeof(HWEVENT), _NTO_INTR_FLAGS_TRK_MSK)) == -1 )
    { id0[device] = -1;   }
   else
    { intEnabled[device]++;     }  

  iid = id0[device];   irqN = HEV->IRQnum;  // @@@QQ

  while(1)
   {
      InterruptWait(NULL,NULL);
      for( i=0; i < BT_DEV_CHANS; i++)
       {  
         RS = pthread_mutex_lock( &muIQ );    // @@@QQ1
         if( hwevent[device].int_channel & (1 << i) )
          { 
            // cN = chid[device][i];  
            cN = BT_DEV_CHANS * device + i;
            iC = hwevent[device].int_channel; // @@@QQ
            // printf( "IQ> ic=%x d=%d i=%d cN=%d\n",iC,device,i,cN );  // @@@TD
            // nanosleep( &td, NULL );                // @@@TD
            //RS = pthread_mutex_lock( &muIQ );    // @@@QQ1
            atomic_add( &( ECP->TaskCC[i] ), 1 ); 
            vbtNotify( cN, &bmRecordTime[cN] ); 
            // vbtNotify( cN, bmRecordTime ); 
            // RS = pthread_mutex_unlock( &muIQ );  // @@@QQ1
          }
         RS = pthread_mutex_unlock( &muIQ );  // @@@QQ1
       }
      RS = InterruptUnmask( irqN, iid );   // @@@QQ !!!
   }
}  // intProc()

/*===========================================================================*
 * ENTRY POINT:           v b t S e t P o l l i n g
 *===========================================================================*
 *
 * FUNCTION:    The function sets the polling interval for the application
 *
 * DESCRIPTION: The function stop the timer set the timing interval and
 *              restarts the timer
 *
 *      It will return: status
 *             
 *===========================================================================*/
BT_INT vbtSetPolling(BT_UINT polling, // (i) polling interval
                     BT_UINT tflag)   // (i) timer option 0 - start; 1 - restart
{
   int status;

   if(tflag)
   {   
     // if ( timerId )// if timer is running stop it @@@>>>
     // {                                            @@@>>>
         api_polling_interval = polling;
         if ( api_polling_interval > MAX_POLLING )
           api_polling_interval = MAX_POLLING;
         if ( api_polling_interval < MIN_POLLING )
            api_polling_interval = MIN_POLLING;
	    
         //timer_delete(iTimer);   @@@>>>
         //timerId = 0;            @@@>>>  // The timer has been destroyed.
     // }                          @@@>>>
   }

  // printf( "api_polling_interval=%d\n", api_polling_interval ); @@@>>>
  if(timerId==TIMER_START)
   {
      status = CEI_THREAD_CREATE(&polling_timer_thread, POLLING_TIMER_THREAD_PRIORITY, polling_timer_func, NULL);
      if(status) return status;
      timerId++;
   }

   return BTD_OK;
}

/*===========================================================================*
 * ENTRY POINT:           v b t I n t e r r u p t C l o s e
 *===========================================================================*
 *
 * FUNCTION:    Disables interrupts from the specified adapter.
 *
 * DESCRIPTION: The hardware interrupt is shutdown and the timer callback
 *              is distroyed if this is the last operational adapter.
 *
 *      It will return:
 *              Nothing.
 *===========================================================================*/
void vbtInterruptClose(BT_UINT cardnum)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int     i;

   // Disable the hardware interrupts from the board...
   if ( (CurrentCardType[cardnum] == PCI1553)  ||
        (CurrentCardType[cardnum] == PMC1553)  ||
        (CurrentCardType[cardnum] == QPMC1553) ||
        (CurrentCardType[cardnum] == QPCI1553) ||
        (CurrentCardType[cardnum] == QPCX1553) ||
        (CurrentCardType[cardnum] == QCP1553)  ||
        (CurrentCardType[cardnum] == Q1041553) ||
        (CurrentCardType[cardnum] == Q1041553P)||
        (CurrentCardType[cardnum] == R15EC)    ||
        (CurrentCardType[cardnum] == RXMC1553) ||
        (CurrentCardType[cardnum] == RPCIe1553)||
        (CurrentCardType[cardnum] == ISA1553)  ||
        (CurrentCardType[cardnum] == PCC1553) )
   {  // Set/clear CPU_INTERRUPT_ENABLE (bit 14) of 1553_CONTROL_REG
      //  (HWREG_CONTROL1) to enable/disable interrupts.  CPU_INTERRUPT
      //  (bit 9) will be set when a HW interrupt occurs.
      // Clear interrupt by writing a zero to WRITE_INTERRUPT_BIT (Bit 9)
      //  of WRITE_INTERRUPT_REG(reg 0x16-0x17), or set it by writing a one.
      api_writehwreg_and(cardnum, HWREG_CONTROL1, (BT_U16BIT)~CR1_INT_ENABLE);
      api_writehwreg(cardnum, HWREG_WRITE_INTERRUPT, 0);
      api_writehwreg_and(cardnum, HWREG_CONTROL1, (BT_U16BIT)~CR1_INT_ENABLE);
      api_writehwreg(cardnum, HWREG_WRITE_INTERRUPT, 0);
   }

   hIntEvent[cardnum] = 0;

   if((--intEnabled[api_device[cardnum]] ) == 0) //if no more channels running kill the thread
   {
      InterruptDetach(id0[api_device[cardnum]]);
      id0[api_device[cardnum]] = -1;
      pthread_kill(iThread[api_device[cardnum]],0);
   }
   /*******************************************************************
   *  If any boards are still running, return.  Else distroy the timer.
   *******************************************************************/
   bt_inuse[cardnum] = 0;               // This board is closed.
   for ( i = 0; i < MAX_BTA; i++ )
      if ( bt_inuse[i] )                // If any other boards are still
         return;                        //   open leave the timer running (V4.30).

   if ( timerId )
   {
      timer_delete(iTimer);
      timerId = 0;                      // The timer has been distroyed.
   } 
}
