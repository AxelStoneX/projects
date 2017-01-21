/*============================================================================*
 * FILE:                          B C . C
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
 *          INTELLIGENT PLATFORMS.
 *
 *===========================================================================*
 *
 * FUNCTION:   BusTools/1553-API Library:
 *             This file contains the API routines for BC operation of
 *             the BusTools board, and routines which are specific to the
 *             One Shot Bus Controller Mode.
 *             These functions assume that the BusTools_API_Init*() function
 *             has been successfully called.
 *
 * USER ENTRY POINTS: 
 *    BusTools_BC_AperiodicRun   - Executes a list of aperiodic messages
 *    BusTools_BC_AperiodicTest  - Tests if aperiodic message list is complete
 *    BusTools_BC_AutoIncrMessageData - Increment message data
 *    BusTools_BC_ControlWordUpdate - Updates certain control word parameters
 *    BusTools_BC_Checksum1760   - Calculates 1760 checksum on a BC message buffer
 *    BusTools_BC_Init           - Set the BC init data in the RAM registers
 *    BusTools_BC_ReadLastMessage- Read the las message in interrupt queue
 *    BusTools_BC_ReadLastMessageBlock - Reads the last group of messages in the interrupt queue
 *    BusTools_BC_ReadNextMessage- Reads the next message in the interrupt queue.
 *    BusTools_BC_RetryInit      - Set up multiple hardware retries
 *    BusTools_BC_IsRunning      - Check if BC is running.
 *    BusTools_BC_MessageAlloc   - Allocate memory on board for BC msg buffers.
 *    BusTools_BC_MessageGetaddr - Convert BC message id to an address
 *    BusTools_BC_MessageGetid   - Convert BT1553 address to a BC message id
 *    BusTools_BC_MessageNoop    - Toggles a BC message between message & NOOP
 *    BusTools_BC_MessageRead    - Read the specified BC Message Block
 *    BusTools_BC_MessageReadData- Read the data only from a specified BC message
 *    BusTools_BC_MessageReadDataBuffer - Read the data only from a specified BC message buffer
 *    BusTools_BC_MessageWrite   - Write the specified BC Message Block
 *    BusTools_BC_MessageUpdate  - Update specified BC Message Block data area
 *    BusTools_BC_MessageUpdateBuffer  - Update specified BC Message Block data area buffer
 *    BusTools_BC_SetFrameRate   - Dynamically updates the frame rate.
 *    BusTools_BC_Start          - Start BC at specified message number.
 *    BusTools_BC_StartStop      - Turn the BC on or off.
 *    BusTools_BC_Trigger        - Setup the BC external trigger mode.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    DumpBC                     - Helper function for BusTools_DumpMemory()
 * 
 * INTERNAL ROUTINES:
 *    BC_MBLOCK_ADDR()           - Compute BC message address.
 *
 *===========================================================================*/

/* $Revision:  6.30 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  04/22/1999   Changed StartStop() to stop if BC_RUN bit is clear, or if BC_BUSY
               bit is clear.V3.05.ajh
  04/26/1999   Fixed error in BC_MessageWrite() and friends which would cause
               BC message blocks to become corrupted when switched between msg
               and branchs.V3.05.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  01/27/2000   Added offset IM_GAP_ADJ to the PCI-/ISA-/IP-1553 gap time.V4.01.1jh
  04/13/2000   Changed MessageWrite to correctly limit the gap time.V4.02.ajh
  06/19/2000   Changed BC_Start to return API_SINGLE_FUNCTION_ERR if either the
               RT or the BM is running, and _HW_1Function[cardnum] is set.
               Swapped order of RT-RT broadcast words from MessageRead.V4.05.ajh
  06/22/2000   Added support for the User Interface DLL.V4.06.ajh
  10/04/2000   Changed BC_MessageWrite to support short gap times.V4.16.ajh
  10/23/2000   Removed change BC_MessageWrite to support short gap times.V4.18.ajh
  11/08/2000   Changed BusTools_BC_Init() to support longer time-out and late
               responses.V4.21.ajh
  12/04/2000   Changed BC_IsRunning() to set/clear bc_running[cardnum] based on
               the current value of the hardware BC_RUN bit.  Changed BC_Start
               to report single function warning after starting BC.V4.26.ajh
  04/25/2001   Modified message read and write to support counted conditional
               messages, also the LabView wrappers.V4.39.ajh
  05/18/2001   Added code to allow multiple hardware retries 4.40 rhc
  02/15/2002   Add interrupt queue processing functions. v4.48
  01/27/2003   Fix LabView Word swap problem in BusTools_BC_OneShotExecuteLV.
  01/27/2003   Fix Signed/Unsigned mismatch
  09/18/2003   Move Labview functions to LabView.c
  10/01/2003   32-Bit API
  08/02/2004   Add 3 new functions, BusTools_BC_MessageUpdateBuffer, 
               BusTools_BC_MessageReadDataBuffer, and BusTools_BC_ControlWordUpdate
  12/07/2006   Improve timing in BusTools_BC_OneShotExecute.
  12/12/2006   Improve BusTools_BC_MessageRead.
  11/19/2007   Change BusTools_BC_Init to add the fixed time flag in place of bPrimeA.
  03/05/2009   Add message scheduling and timed noop features.
  06/10/2009   Add BC_CONTROL_HALT to BusTools_BC_MessageWrite
  10/22/2009   Extended timing, Frame Start timing channel sharing
               
*/

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#define MAX_ONESHOT_MESSAGES 100     /* Max number of 1 shot BC messages */

BT_INT _stdcall bc_auto_incr(BT_UINT,struct api_int_fifo *);
static API_INT_FIFO *pIntFIFO[MAX_BTA][512];     // Thread FIFO structure pointer
static BT_INT bit_running[MAX_BTA];

/****************************************************************************
*  The following defines the interrupt status bits which are valid for the BC
****************************************************************************/
#define BCMASK  (0x8E3FFFFF)        /* V4.16 */

/****************************************************************************
*  BC control information, local to this module.
****************************************************************************/
static BT_U16BIT bc_num_dbufs[MAX_BTA];    /* BC data buffers per msg      */
static BT_U16BIT bc_size_dbuf[MAX_BTA];    /* Bytes in 1 BC Data Buffer    */
static BT_U32BIT iqptr_bc_last[MAX_BTA];

/*==========================================================================*
 * LOCAL FUNCTION:          B C _ M B L O C K _ S I Z E
 *==========================================================================*
 *
 * FUNCTION:    Calculates and returns the size of a BC message block.
 *
 * DESCRIPTION: This macro uses the board type to compute the size of a
 *              BC Message Block.  The PCI-1553 pads the message block
 *              to 16 words, other boards use 12 words.
 *
 *      It will return:
 *              Size of the specified message block in bytes.
 *=========================================================================*/

static BT_U32BIT BC_MBLOCK_SIZE(
   BT_UINT   cardnum)       // (i) card number (0 based)
{
   /*******************************************************************
   *  Return the size of the BC Message Block.
   *  This does NOT include the size of the data buffer(s).
   ********************************************************************/
   return sizeof(BC_MESSAGE);  // PCI-1553 pads the message block
}

/*==========================================================================*
 * LOCAL FUNCTION:          B C _ M B L O C K _ A D D R
 *==========================================================================*
 *
 * FUNCTION:    Calculates and returns the address of a BC message block,
 *              given the zero-based index of the message block.
 *
 * DESCRIPTION: This macro uses the base address of the BC message blocks,
 *              the size of a message block, the size and number of data
 *              blocks, the size of a message block and the message index
 *              to calculate the address requested.
 *
 *      It will return:
 *              Byte address of the specified message block.
 *=========================================================================*/

static BT_U32BIT BC_MBLOCK_ADDR(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT mblock_id)
{
   BT_U32BIT  addr;          // Base address of requested BC message
   BT_U32BIT  msg_blk_size;  // Size of a BC message block, including data bufs

   msg_blk_size = BC_MBLOCK_SIZE(cardnum) +
                               (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);
   addr = btmem_bc[cardnum] + ( mblock_id * msg_blk_size );
   return addr;
}


/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_AperiodicRun
*
* FUNCTION
*     If an aperiodic message list is currently being processed return
*     an error, then
*     Loads the address of the specified message(s) into either the high-
*     or the low-priority aperiodic message register, then optionally waits
*     for the message(s) to be executed.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTRUNNING       -> BC not running and wait specified
*     API_BC_NOTINITED        -> BC has not been initialized
*     API_HARDWARE_NOSUPPORT  -> Non-IP does not support priority messages
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*     API_BC_APERIODIC_RUNNING-> BC Aperiodics still running, cannot start new msg list
*     API_BC_APERIODIC_TIMEOUT-> Aperiodic messages did not complete in time
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_AperiodicRun(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) Number of BC message which begins list
   BT_UINT   Hipriority,    // (i)  1 -> Hi Priority msgs, 0 -> Low Priority msgs
   BT_UINT   WaitFlag,      // (i)  1 -> Wait for BC to complete executing msgs
   BT_UINT   WaitTime)      // (i)  Timeout in seconds(16-bit) or milliseconds(32-bit)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT addr;         /* Address of the first BC msg */
   BT_UINT   high_low_reg; /* Aperiodic register address */

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if ( bc_running[cardnum] == 0 )
      return API_BC_NOTRUNNING;

   /*******************************************************************
   *  Check parameter ranges
   *******************************************************************/
   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /* Setup which register is to be used. */
   if ( Hipriority )
      high_low_reg = RAMREG_HIGHAPTR;
   else
      high_low_reg = RAMREG_LOWAPTR;

   /* If currently active, return error. */
   if ( vbtGetFileRegister(cardnum, high_low_reg) )
      return API_BC_APERIODIC_RUNNING;

   /* Address of BC message is base plus length of preceeding messages. */
   /* Convert it from bytes to words. */
   addr = (BT_U16BIT)(BC_MBLOCK_ADDR(cardnum, messageid)>>hw_addr_shift[cardnum]);

   /* Start the aperiodic message list processing. */
   vbtSetFileRegister(cardnum, high_low_reg, addr);

   if ( WaitFlag == 0 )
      return API_SUCCESS;

   while ( vbtGetFileRegister(cardnum, high_low_reg) )
   {
      MSDELAY(1);
      if ( WaitTime )
         WaitTime--;
      else
         return API_BC_APERIODIC_TIMEOUT;
   }

   return API_SUCCESS;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_AperiodicTest
*
* FUNCTION
*     Tests to see if an aperiodic message list is currently being
*     processed.
*
*   Returns
*     API_SUCCESS             -> success, aperiodics not running
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC has not been initialized
*     API_HARDWARE_NOSUPPORT  -> Non-IP does not support priority messages
*     API_BC_APERIODIC_RUNNING-> BC Aperiodics still running
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_AperiodicTest(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   Hipriority)    // (i) 1 -> Hi Priority msgs, 0 -> Low Priority msgs
{
   /***********************************************************************
   *  Check initial conditions
   ***********************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /*******************************************************************
   *  See if message list still running.
   *******************************************************************/
   if ( Hipriority )
   {
      if ( vbtGetFileRegister(cardnum, RAMREG_HIGHAPTR) )
         return API_BC_APERIODIC_RUNNING;
   }
   else
   {
      if ( vbtGetFileRegister(cardnum, RAMREG_LOWAPTR) )
         return API_BC_APERIODIC_RUNNING;
   }
   return API_SUCCESS;
}


/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_Init
*
* FUNCTION
*     Set the BC initialization data in the RAM registers
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_RUNNING          -> BC currently running
*     API_BC_INITED           -> Error condition not used
*     API_BC_BADTIMEOUT1      -> No Response Time < 4 or >= 32/62
*     API_BC_BADTIMEOUT2      -> Late Response Time < 4 or >= 32/62
*     API_BC_BADFREQUENCY     -> frame > 1638375 us or frame < 1000 us
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bc_options,    // (i) Gap Time from start = 0xf; Msg Schd = 0x10; 
                            //     Frame Timing 0x20, no prediction logic 0x40
   BT_U32BIT Enable,        // (i) interrupt enables
   BT_UINT   wRetry,        // (i) retry enables
   BT_UINT   wTimeout1,     // (i) no response time out in microseconds
   BT_UINT   wTimeout2,     // (i) late response time out in microseconds
   BT_U32BIT frame,         // (i) minor frame period, in microseconds
   BT_UINT   num_buffers)   // (i) number of BC message buffers ( 1 or 2 )
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_U16BIT data;
   BT_U16BIT wValue;
   int i;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(channel_is_shared[cardnum])
   {
      vbtReadRAM8(cardnum,(LPSTR)&bc_inited[cardnum],BTMEM_CH_SHARE + SHARE_BC_INITED,1);
      if(bc_inited[cardnum])
         return API_BC_INITED;
   } 

   /*******************************************************************
   *  Check parameter ranges
   *******************************************************************/
   if ((wTimeout1 < 4) || (wTimeout1 > 61))
      return API_BC_BADTIMEOUT1;
   if ((wTimeout2 < 4) || (wTimeout2 > 61))
      return API_BC_BADTIMEOUT2;

   if((bc_options & FIXED_GAP) && (bc_options & FRAME_START_TIMING))
      return API_PARAM_CONFLICT;

   /*******************************************************************
   *  BC is either single or double buffered.  Record
   *  the size of a buffer, and the number of buffers.
   *******************************************************************/
   if ( bc_running[cardnum] == 0 )
   {
      bc_size_dbuf[cardnum] = 2 * PCI1553_BUFCOUNT;  /* 40 words per buffer */

      if ( num_buffers != 1 ) /* Number of BC message buffers ( 1 or 2 ) */
         bc_num_dbufs[cardnum] = 2;  /* Double buffered. */
      else
         bc_num_dbufs[cardnum] = 1;  /* Single buffered. */
   }

   /*******************************************************************
   *  BC interrupt enables
   *******************************************************************/
   vbtSetFileRegister(cardnum,RAMREG_BC_INT_ENB1,(BT_U16BIT)(Enable      ));
   vbtSetFileRegister(cardnum,RAMREG_BC_INT_ENB2,(BT_U16BIT)(Enable >> 16));

   vbtSetFileRegister(cardnum,RAMREG_BC_RETRY, 0);
   vbtSetFileRegister(cardnum,RAMREG_DQ_RETRY, 0);
   // check for the retry methosd to use based on card type

   /******************************************************************
	* use the retry buffer to allow up to 8 retries
	******************************************************************/
   vbtSetFileRegister(cardnum,RAMREG_BC_RETRYPTR,(BT_U16BIT)(BTMEM_BC_RETRY_BUF>>hw_addr_shift[cardnum]));
   data = 0; 
   for(i=0;i<9;i++) // clear the retry buffer
   {
      vbtWrite(cardnum,(LPSTR)(&data),BTMEM_BC_RETRY_BUF+(i*2),2);
   }
   if(wRetry) // If there are any retry options selected set up a single retry
   {
      if ( wRetry & BC_RETRY_ALTB )
      {
         data = RETRY_ALTERNATE_BUS;
         vbtWrite(cardnum,(LPSTR)(&data),BTMEM_BC_RETRY_BUF,2); //Retry Alernate Bus
      }
      else
      {
         data = RETRY_SAME_BUS;
         vbtWrite(cardnum,(LPSTR)(&data),BTMEM_BC_RETRY_BUF,2);      // Retry Same Bus
      }
      data = RETRY_END;
      vbtWrite(cardnum,(LPSTR)(&data),(BTMEM_BC_RETRY_BUF+2),2);            // Single Retry
   }

   /*******************************************************************
   *  Store retry bits into RAM registers 45 & 78,
   *  using the write under mask low-level function.
   *******************************************************************/
   if ( wRetry & BC_RETRY_NRSP )  /* Register 78 Bit 10,No Response    */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_FLAGS*2, BIT10, BIT10);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_FLAGS*2, 0,     BIT10);

   if ( wRetry & BC_RETRY_ME   )  /* Register 45 Bit 10,Message Error  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT10, BIT10);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT10);

   if ( wRetry & BC_RETRY_BUSY )  /* Register 45 Bit  3,Busy Bit Set   */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT03, BIT03);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT03);

   if ( wRetry & BC_RETRY_TF   )  /* Register 45 Bit  0,Terminal Flag  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT00, BIT00);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT00);

   if ( wRetry & BC_RETRY_SSF  )  /* Register 45 Bit  2,SubSystem Flag */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT02, BIT02);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT02);

   if ( wRetry & BC_RETRY_INSTR)  /* Register 45 Bit  9,Instrumentation*/
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT09, BIT09);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT09);

   if ( wRetry & BC_RETRY_SRQ  )  /* Register 45 Bit  8,Service Request*/
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT08, BIT08);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT08);

   if ( wRetry & BC_RETRY_INV_WRD   )  /* Register 7 Bit 2,Invalid Word  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT01, BIT01);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT01);

   if ( wRetry & BC_RETRY_INV_SYNC )  /* Register 7 Bit  3,Inverted Sync   */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT03, BIT03);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT03);

   if ( wRetry & BC_RETRY_MID_BIT   )  /* Register 7 Bit  4,Mid Bit  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT04, BIT04);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT04);

   if ( wRetry & BC_RETRY_TWO_BUS  )  /* Register 7 Bit  5,Two Bus */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT05, BIT05);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT05);

   if ( wRetry & BC_RETRY_PARITY)  /* Register 7 Bit  6, Parity */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT06, BIT06);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT06);

   if ( wRetry & BC_RETRY_CONT_DATA  )  /* Register 7 Bit  7,  Non Contiguous Data */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT07, BIT07);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT07);

   if ( wRetry & BC_RETRY_EARLY_RSP   )  /* Register 7 Bit 8, Early Response  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT08, BIT08);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT08);

   if ( wRetry & BC_RETRY_LATE_RSP )  /* Register 7 Bit  9, Late Response   */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT09, BIT09);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT09);

   if ( wRetry & BC_RETRY_BAD_ADDR   )  /* Register 7 Bit  10 , Bad RT Address  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT10, BIT10);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT10);

   if ( wRetry & BC_RETRY_WRONG_BUS  )  /* Register 7 Bit  13, Wrong Bus */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT13, BIT13);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT13);

   if ( wRetry & BC_RETRY_BIT_COUNT)  /* Register 7 Bit  14, Bit Count */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT14, BIT14);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT14);

   if ( wRetry & BC_RETRY_NO_GAP  )  /* Register 7 Bit  15,  No Inter-message gap */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT15, BIT15);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT15);

   /*******************************************************************
   *  Store response register.  Round it up by 1/2 microsecond.V4.01.ajh
   *******************************************************************/
   wTimeout1 = (wTimeout1 << 1) | 1;
   wTimeout2 = (wTimeout2 << 1) | 1;
   wValue = (BT_U16BIT)(((wTimeout1 & 0x3f) << 8) + (wTimeout2 & 0x3f));

   /* The HWREG_RESPONSE register must be programmed after one of the three  */
   /* run bits has been set.                                                 */

   wResponseReg4[cardnum] = wValue;  /* Save value for later.*/
   api_writehwreg(cardnum,HWREG_RESPONSE,wValue);

   if((_HW_FPGARev[cardnum] & 0xfff) >= 0x499)
   {
      board_using_extended_timing[cardnum]=1;
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue |= CR2_EXTD_TIME;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
   }
   else
      board_using_extended_timing[cardnum]=0;

   /*******************************************************************
   *  Store minor frame register -- frame time is in
   *   units of 1 or 25 micro-seconds
   *******************************************************************/
   if(board_using_extended_timing[cardnum])// 1 microsecond timing
   {
      BT_U16BIT frame_lsb,frame_msb;

      if (frame < 250)
         return API_BC_BADFREQUENCY;
      frametime32[cardnum] = (BT_U32BIT)frame;   /* Must be less than 65535. */
      frame_lsb = (BT_U16BIT)frametime32[cardnum] & 0xffff;
      frame_msb = (BT_U16BIT)((frametime32[cardnum] & 0xffff000)>>16);
      vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME_LSB, frame_lsb );
      vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME_MSB, frame_msb );
   }
   else //25 microsecond timing
   {
      if ((frame > 1638375L) || (frame < 250))
         return API_BC_BADFREQUENCY;
      frametime[cardnum] = (BT_U16BIT)(frame/25);   /* Must be less than 65535. */
      vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME, frametime[cardnum]);
   }

   //iqptr_bc_last[cardnum] = BTMEM_IQ;
   iqptr_bc_last[cardnum] = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;

   /*******************************************************************
   *  gap timing options fix or relative
   *******************************************************************/
   if(bc_options < 0xf)
      bc_options = 0;

   if((_HW_FPGARev[cardnum] & 0xfff) >= 0x415)
   {
      if(bc_options & FIXED_GAP)// use fixed timing from start of message
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_FIXED_GAP;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      }
      else // use relative timing from end of message.
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue &= ~CR2_FIXED_GAP;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      }
   }

   if(bc_options & MSG_SCHD)   
   {
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x439)
      {
         if(num_buffers == 2)
            return API_PARAM_CONFLICT;

         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_MSG_SCHD;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
         board_using_msg_schd[cardnum]=1;
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }
   else // no message scheduling.
   {
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue &= ~CR2_MSG_SCHD;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      board_using_msg_schd[cardnum]=0;
   }

   if(bc_options & FRAME_START_TIMING)   
   {
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x499)
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_FR_STRT_TIME;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
         board_using_frame_start_timing[cardnum]=1;
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }
   else // No frame start timing.
   {
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue &= ~CR2_FR_STRT_TIME;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      board_using_frame_start_timing[cardnum]=0;
   }

   if(bc_options & NO_PRED_LOGIC)   
   {
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x440)
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_NO_PREDICT;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }
   else // no message scheduling.
   {
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue &= ~CR2_NO_PREDICT;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
   }
   
      
   /*******************************************************************
   *  Show BC being initialized.
   *******************************************************************/
   bc_inited[cardnum] = 1;

   if(channel_is_shared[cardnum])
      vbtWriteRAM8(cardnum,(LPSTR)&bc_inited[cardnum],BTMEM_CH_SHARE + SHARE_BC_INITED,1); 

   return API_SUCCESS;
}

/****************************************************************
*                               
*  PROCEDURE NAME - BusTools_BC_RetryInit
*   
*  FUNCTION
*     This routine setup retry options. These include the number
*     of retries and the bus on which the retry occurs. 
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_ERR_PARAM           -> Bad retry parameter.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_RetryInit(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT *bc_retry)     //
{
   int i;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   for(i=0;i<9;i++)
   {
      if(bc_retry[i] < 3)
         vbtWrite(cardnum,(char *)&bc_retry[i],BTMEM_BC_RETRY_BUF+(i*2),2);
      else
         return API_BAD_PARAM;

      if(bc_retry[i] == 0)  // 0 = end of retry buffer
         break;
   }
   return API_SUCCESS;
}

/****************************************************************
*                               
*  PROCEDURE NAME - BusTools_BC_IsRunning
*   
*  FUNCTION
*     This routine returns the running/not-running state
*     of the BC.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_NOTRUNNING       -> BC not currently running
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_IsRunning(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT * flag)      // (o) Returns 1 if running, 0 if not
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_UINT value;

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /************************************************
   *  Read the hardware register and check BC bit
   ************************************************/
   value = api_readhwreg(cardnum,HWREG_CONTROL1);

   if ( value & CR1_BCRUN )
   {
      // Let the API know the status.V4.26.ajh
      bc_running[cardnum] = 1;
      *flag = 1;
   }
   else
   {
      // Let the API know the status.V4.26.ajh
      bc_running[cardnum] = 0;
      *flag = 0;
   }
   return API_SUCCESS;
}

/****************************************************************
*                               
*  PROCEDURE NAME - BusTools_BC_IsRunning2
*   
*  FUNCTION
*     This routine returns the running/not-running state
*     of the BC.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_NOTRUNNING       -> BC not currently running
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_IsRunning2(
   BT_UINT   cardnum)       // (i) card number (0 based)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_UINT value;

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /************************************************
   *  Read the hardware register and check BC bit
   ************************************************/
   value = api_readhwreg(cardnum,HWREG_CONTROL1);

   if ( value & CR1_BCRUN )
   {
      bc_running[cardnum] = 1;
      return API_BC_IS_RUNNING;
   }
   else
   {
      bc_running[cardnum] = 0;
      return API_BC_IS_STOPPED;
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageAlloc
*
*  FUNCTION
*     This routine allocates memory on the BT1553 board for the specified
*     number of BC message buffers.  The buffers are cleared, with the
*     message and data buffer link addresses inserted.
*
*     The message buffers are allocated from btmem_bc[] up, with message zero
*     located at btmem_BC[].  The data buffers are allocated from btmem_bc_next[]
*     down, with data buffer zero being allocated at btmem_bc_next[].
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_MBUF_ALLOCD      -> More messages requested than first call
*     API_BC_MEMORY_OFLOW     -> Not enough memory to create messages
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT count)           // (i) Number of BC messages to allocate
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BC_MESSAGE  bc_buffer;          /* Hardware BM Message buffer image */
   BT_UINT     i;                  /* Loop counter */

   BT_U32BIT   bytes_required;     /* Number of bytes required for msgs +data */
   BT_U32BIT   addr;               /* Byte offset of current BC msg buffer    */
   BT_U32BIT   nextbc;             // Byte offset to next BC message buffer   */
   BT_U32BIT   data_addr1;         /* Byte offset to BC data buffer 1         */
   BT_U32BIT   data_addr2;         /* Byte offset to BC data buffer 2         */

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (bc_running[cardnum])
      return API_BC_RUNNING;

   if (bc_mblock_count[cardnum] != 0)
   {
      if (bc_mblock_count[cardnum] < count)
         return API_BC_MBUF_ALLOCD;
   } 

   if(channel_is_shared[cardnum])
      vbtReadRAM(cardnum,(char *)&btmem_pci1553_next[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_NEXT,sizeof(BT_U32BIT));

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageAlloc[cardnum] )
   {
      i = (*pUsrBC_MessageAlloc[cardnum])(cardnum, &count);
      if ( i == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( i == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageAlloc[cardnum] = NULL;
      else if ( i != API_CONTINUE )
         return i;
   }
#endif

   /* Compute the number of bytes required for message and data buffers */
   bytes_required = (BT_U32BIT)count * (BC_MBLOCK_SIZE(cardnum) +
                               (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]));

   /*******************************************************************
   *  Check for memory overflow
   *******************************************************************/

   /*  Round the starting address up to a multiple of 8 words/16 bytes */
   btmem_pci1553_next[cardnum] = (btmem_pci1553_next[cardnum]+15) & 0xFFFFFFF0L;
   nextbc = bytes_required + btmem_pci1553_next[cardnum];
   if ( nextbc > btmem_pci1553_rt_mbuf[cardnum] )
      return API_BC_MEMORY_OFLOW;
   if ( btmem_bc[cardnum] == 0 )
   {
      btmem_bc[cardnum]           = btmem_pci1553_next[cardnum];
      btmem_bc_next[cardnum]      = nextbc;
      btmem_pci1553_next[cardnum] = nextbc;
   }

   /*******************************************************************
   *  Initialize the count of messages available.
   *******************************************************************/
   if ( bc_mblock_count[cardnum] == 0 )
      bc_mblock_count[cardnum] = (BT_U16BIT)count;

   nextbc = btmem_bc[cardnum];

   /*******************************************************************
   *  Init the hardware pointer to the beginning of the new bus list
   *******************************************************************/
   vbtSetFileRegister(cardnum,RAMREG_BC_MSG_PTR  ,0x0000);
   vbtSetFileRegister(cardnum,RAMREG_BC_MSG_PTRSV,(BT_U16BIT)(nextbc >> hw_addr_shift[cardnum]));

   /*******************************************************************
   *  Build and output the no-op BC message blocks, linked together
   *   and containing pointers to the BC data blocks.
   *******************************************************************/
   memset(&bc_buffer, 0, sizeof(BC_MESSAGE));

   /* Set error injection address to default location (EI buffer 0) */
   bc_buffer.addr_error_inj = (BT_U16BIT)(BTMEM_EI >> 1);
   bc_buffer.gap_time = 15;  // Intermessage gap time in microseconds.

   /* Initialize the BC message blocks. */
   for ( i = 0; i < count; i++ )
   {
      /****************************************************************
      *  Fill in default values for all pointers in block.
      ****************************************************************/
      /* Set data buffer addresses */
      data_addr2 = data_addr1 = nextbc + BC_MBLOCK_SIZE(cardnum);

      if ( bc_num_dbufs[cardnum] != 1 ) /* Not single buffered. */
         data_addr2 += bc_size_dbuf[cardnum];
      bc_buffer.addr_data1 = (BT_U16BIT)(data_addr1 >> hw_addr_shift[cardnum]);
      bc_buffer.addr_data2 = (BT_U16BIT)(data_addr2 >> hw_addr_shift[cardnum]);

      /* Compute the next message address.    */
      addr = BC_MBLOCK_ADDR(cardnum, i+1);
      if ( i < (count-1) )
         bc_buffer.addr_next = (BT_U16BIT)(addr >> hw_addr_shift[cardnum]);
      else
         bc_buffer.addr_next = (BT_U16BIT)(btmem_bc[cardnum] >> hw_addr_shift[cardnum]);

      /* Write the buffers to the board so BusTools_BC_MessageWrite() */
      /*  et.al. can get the pointers to the data buffers.            */
      vbtWrite(cardnum, (LPSTR)&bc_buffer, nextbc, sizeof(BC_MESSAGE));
      nextbc = addr;
   }

   if(channel_is_shared[cardnum])
      vbtWriteRAM(cardnum,(char *)&btmem_pci1553_next[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_NEXT,sizeof(BT_U32BIT));

   return API_SUCCESS;
}

/****************************************************************
*
* PROCEDURE NAME - BusTools_BC_MessageGetaddr()
*
* FUNCTION
*     This routine converts a BC message id to a BT1553 address.
*     Note that the address is in bytes.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageGetaddr(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to compute address
   BT_U32BIT * addr)    // (o) Returned byte address of BC message
{
   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /************************************************
   *  Return computed address
   ************************************************/
   /* Address is base plus length of preceeding messages. */
   *addr = BC_MBLOCK_ADDR(cardnum, messageid);

   return API_SUCCESS;
}

/****************************************************************************
*                               
* PROCEDURE NAME - BusTools_BC_MessageGetid()
*   
* FUNCTION
*     This routine converts a BT1553 address to a BC message
*     id.  Note that the address is in bytes, as from the interrupt queue.
*     We account for the 8-word alignment for the PCI-1553.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_MBLOCK_NOMATCH   -> Address is not a BC message block
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageGetid(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT addr,          // (i) Byte address of BC message
   BT_UINT * messageid) // (o) Number of the BC message
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;
   BT_U32BIT  msg_blk_size;   // Size of a BC message plus data buffers.

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /*******************************************************************
   *  Calculate index based on starting address & size.
   *******************************************************************/
   msg_blk_size = BC_MBLOCK_SIZE(cardnum) +
                               (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);

   addr <<= (hw_addr_shift[cardnum]-1);
 
   *messageid =
   i = (BT_UINT)( (addr - btmem_bc[cardnum]) / msg_blk_size);

   if ( i < bc_mblock_count[cardnum] )
      return API_SUCCESS;

   return API_BC_MBLOCK_NOMATCH;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_MessageNoop()
*
* FUNCTION
*     This routine takes a BC message id and toggles the message
*     between a NOOP and an 1553 message.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*     API_BC_NOTMESSAGE       -> This is not a proper 1553-type message
*     API_BC_NOTNOOP          -> This is not a proper noop-type message
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageNoop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_UINT NoopFlag,        // (i) Flag, 1 -> Set to NOOP, 0 -> Return to msg
                            //           0xf -> Timed Noop
   BT_UINT WaitFlag)        // (i) Flag, 1 -> Wait for BC to not be executing msg,
                            //           0 -> Do not test for BC executing msg.
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT    msg_addr;     /* Address of BC message. */
//   BT_U32BIT    bc_address;   /* Address of meg being processed by BC */
   BT_U16BIT    temp;         /* Temp for swap. */
   BT_INT       tnop_stat=0;  /* this for timed nop on unsupported F/W. */

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /************************************************
   *  Compute address of BC message
   ************************************************/
   /* Address is base plus length of preceeding messages. */
   msg_addr = BC_MBLOCK_ADDR(cardnum, messageid);

   /*******************************************************************
   *  Figure out if this is a legal swap.
   *******************************************************************/
   /* Fetch the message control word from the board. */
   vbtRead(cardnum, (LPSTR)&temp, msg_addr, 2);

   if ( (temp & 0x0006) == 0x0000 )
      return API_BC_CANT_NOOP;

   if (NoopFlag)         /* Set 1553 message to noop */
   {
      temp &= BC_HWCONTROL_SET_NOP; /* Noop bit is active when zero. */
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x439)
      {
         if(NoopFlag == TIMED_NOOP)
            temp |= BC_HWCONTROL_SET_TNOP;
      }
      else
         tnop_stat=0x5;
   }
   else
   {
      temp |= BC_HWCONTROL_OP;      /* Noop is inactive when one. */
      temp &= BC_HWCONTROL_CLEAR_TNOP; /* Timed noop disable when cleared */
   }   
   
#if 0 // wait flag is deprecated.  This flag is not required there is need to perform this wait.
   /* Now wait until the BC is not processing the message of interest. */
   if ( WaitFlag )         /* Only wait if requested. */
   {
      while(1)
      {
         if(bc_running[cardnum] == 0)
            break; //BC is not running

         bc_address = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_BC_MSG_PTRSV))<<hw_addr_shift[cardnum];
         if ( msg_addr < bc_address )
            break;
         if ( msg_addr > (bc_address+sizeof(BC_MESSAGE)) )
            break;

      }
   }
#endif 

   /* Update the message control word on the board. */
   vbtWrite(cardnum, (LPSTR)&temp, msg_addr, 2);
   if(tnop_stat)
      return API_HARDWARE_NOSUPPORT;

   return API_SUCCESS;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_ControlWordUpdate()
*
* FUNCTION
*     This routine allows users to change some of the the BC Message
*     control parameters.  The following is the list of parameters:
*      Bus       - Select Bus A or B          -- BC_CONTROL_BUFFERA,  BC_CONTROL_BUFFERB
*      Buffer    - Select Buffer A or B       -- BC_CONTROL_CHANNELA, BC_CONTROL_CHANNELB
*      Interrupt - switch interrupt on or off -- BC_CONTROL_INTERRUPT
*      Retry     - switch retries on or off   -- BC_CONTROL_RETRY 
*      Int Queue - BC_CONTROL_INTQ_ONLY
*      NOP/Message - BC_CONTROL_NOP -- BC_CONTROL_MESSAGE
*     Each parameter in the list is processes so, if you do not select BC_CONTROL_RETRY
*     The retry option is cleared on this message
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*     API_BC_UPDATEMESSTYPE   -> This is not a proper 1553-type message
*     API_BC_BOTHBUFFERS      -> Can't use both buffers
*     API_BC_BOTHBUSES        -> Can't set both buses
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_ControlWordUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_U16BIT controlWord,   // (i) New Control Word settings
   BT_UINT WaitFlag)        // (i) Flag, 1 -> Wait for BC to not be executing msg,
                            //           0 -> Do not test for BC executing msg.
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT    msg_addr;     // Address of BC message. 
   BT_U32BIT    bc_address;   // Address of meg being processed by BC 
   BT_U16BIT    bc_cntl_wrd;  // BC control Word.

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /************************************************
   *  Compute address of BC message
   ************************************************/
   // Address is base plus length of preceeding messages. 
   msg_addr = BC_MBLOCK_ADDR(cardnum, messageid);

   /*******************************************************************
   *  Process Messages...
   *******************************************************************/
   // Fetch the message control word from the board. 
   vbtRead(cardnum, (LPSTR)&bc_cntl_wrd, msg_addr, 2);

   if ( (bc_cntl_wrd & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;

   //Set or clear retries 
   if(controlWord & BC_CONTROL_RETRY)
      bc_cntl_wrd |= BC_HWCONTROL_RETRY;
   else
      bc_cntl_wrd &= ~BC_HWCONTROL_RETRY;

   //Set or clear interrupts
   if(controlWord & BC_CONTROL_INTERRUPT)
      bc_cntl_wrd |= BC_HWCONTROL_INTERRUPT;
   else
      bc_cntl_wrd &= ~BC_HWCONTROL_INTERRUPT;

   //Set Buffer A or Buffer B
   if((controlWord & BC_CONTROL_BUFFERA) && 
      (controlWord & BC_CONTROL_BUFFERB)   )
      return API_BC_BOTHBUFFERS;

   if(controlWord & BC_CONTROL_BUFFERA)
      bc_cntl_wrd |= BC_HWCONTROL_BUFFERA;
   if(controlWord & BC_CONTROL_BUFFERB)
      if (bc_num_dbufs[cardnum] == 2)
         bc_cntl_wrd &= ~BC_HWCONTROL_BUFFERA;

   //Set Bus A or Bus B
   if((controlWord & BC_CONTROL_CHANNELA) && 
      (controlWord & BC_CONTROL_CHANNELB)   )
      return API_BC_BOTHBUSES;

   if(controlWord & BC_CONTROL_CHANNELB)
      bc_cntl_wrd |= BC_HWCONTROL_CHANNELB;
   if(controlWord & BC_CONTROL_CHANNELA)
      bc_cntl_wrd &= ~BC_HWCONTROL_CHANNELB;

   if(controlWord & BC_CONTROL_INTQ_ONLY)
      bc_cntl_wrd |= BC_HWCONTROL_INTQ_ONLY;
   else
      bc_cntl_wrd &= ~BC_HWCONTROL_INTQ_ONLY;

   if(controlWord & BC_CONTROL_MESSAGE)
      bc_cntl_wrd |= 0x1;     //Message
   else
      bc_cntl_wrd &= 0xfffe;  //NOP

   /* Now wait until the BC is not processing the message of interest. */
   if ( WaitFlag )         // Only wait if requested. 
   {
      if(bc_running[cardnum]==1) // Skip if the BC is not running
      {
         while(1)
         {
            bc_address = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_BC_MSG_PTR))<<hw_addr_shift[cardnum];
            if ( msg_addr < bc_address )
               break;
            if ( msg_addr > (bc_address+sizeof(BC_MESSAGE)) )
               break;
         }
      }
   }
   // Update the message control word on the board. 
   vbtWrite(cardnum, (LPSTR)&bc_cntl_wrd, msg_addr, 2);
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageRead()
*
*  FUNCTION
*     This routine is used to read the specified BC Message
*     Block from the board.  The data contained therein is
*     returned in the caller supplied structure.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageRead(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT mblock_id,       // (i) Number of BC message
   API_BC_MBUF * api_message) // (i) Pointer to buffer to receive msg
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT         id;          // Message number or error injection number
   BT_UINT         status;      // Function return status
   BT_UINT         wordno;      // Word number for conditional messages
   union
   {
      BT1553_STATUS status;           // enabled word counts (bit field)
      BT_U16BIT data;         // enabled mode codes
   }NullStatus;  // Null status word for Broadcast RT-RT messages

   BT_U32BIT       addr;        // Byte address of BC Message
   BT_U32BIT       addr_prev;   // Byte address of previous BC Message
   BC_MESSAGE      mblock;      // Local copy of hardware BC Message
   union{
     BC_CBUF         *cpbuf;       // Pointer used to ref msg as an IP conditional
     BC_MESSAGE      *mpbuf;
   }bcptr;
   BC_CBUF         *cbuf;
   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageRead[cardnum] )
   {
      status = (*pUsrBC_MessageRead[cardnum])(cardnum, &mblock_id, api_message);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageRead[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get BC Message Block from hardware
   *******************************************************************/
   addr = BC_MBLOCK_ADDR(cardnum, mblock_id);
   if(board_access_32[cardnum])
      vbtRead32(cardnum, (LPSTR)&mblock, addr, sizeof(mblock));
   else
      vbtRead(cardnum, (LPSTR)&mblock, addr, sizeof(mblock));

   /*******************************************************************
   *  Clear out caller's structure and fill in simple stuff
   *******************************************************************/
   /*memset(api_message+6,0,sizeof(API_BC_MBUF)-14-6);  // Not really needed */
   api_message->messno  = (BT_U16BIT)mblock_id;

   /* Only if timing trace is enabled */
   AddTrace(cardnum, NBUSTOOLS_BC_MESSAGEREAD, mblock_id, addr >> 1, 0, 0, 0);

   if ( (mblock.control_word & 0x0007) == 0x0002 || (mblock.control_word & 0x0007) == 0x0003 )  /* Normal 1553 message */
   {
      if ( (mblock.control_word & 0x0007) == 0x0003)
         api_message->control                = BC_CONTROL_MESSAGE;
      else
         api_message->control                = BC_CONTROL_MSG_NOP;

      if (mblock.control_word & BC_HWCONTROL_MFRAMEBEG)
         api_message->control             |= BC_CONTROL_MFRAME_BEG;
      if (mblock.control_word & BC_HWCONTROL_MFRAMEEND)
         api_message->control             |= BC_CONTROL_MFRAME_END;
      if (mblock.control_word & 0x0020)
         api_message->control             |= BC_CONTROL_RTRTFORMAT;
      if (mblock.control_word & 0x0400)
         api_message->control             |= BC_CONTROL_RETRY;
      if (mblock.control_word & BC_HWCONTROL_INTERRUPT)
         api_message->control             |= BC_CONTROL_INTERRUPT;

      if (mblock.control_word & BC_HWCONTROL_BUFFERA)
         api_message->control          |= BC_CONTROL_BUFFERA;
      else
         api_message->control          |= BC_CONTROL_BUFFERB;
      if (mblock.control_word & BC_HWCONTROL_CHANNELB)
         api_message->control          |= BC_CONTROL_CHANNELB;
      else
         api_message->control          |= BC_CONTROL_CHANNELA;

      if(mblock.control_word & BC_CONTROL_TIMED_NOP)
         api_message->control          |= BC_CONTROL_TIMED_NOP;

      addr = ((BT_U32BIT)mblock.addr_next) << 1;                   // *8 applied below
      if ( addr == 0 )
         api_message->messno_next = 0xFFFF;
      else
      {
         status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
         if (status)
             return status;
         api_message->messno_next   = (BT_U16BIT)id;
      }

      if(board_using_extended_timing[cardnum])
         api_message->long_gap = (BT_U32BIT)((mblock.gap_time2 << 16) | mblock.gap_time);
      else
         api_message->gap_time = mblock.gap_time;

      // Modify to correctly handle Broadcast RT-RT format.V4.01.ajh
      // Modified again for RT-RT and Broadcast RT-RT.V4.05.ajh
      api_message->mess_command1 = mblock.mess_command1;

      NullStatus.data = 0;
      if ( api_message->control & 0x0020 )       // RT-RT format
      {
         api_message->mess_command2 = mblock.mess_command2;
         if ( rt_bcst_enabled[cardnum] && (mblock.mess_command1.rtaddr == 31) )
         {
            api_message->mess_status1  = mblock.mess_status1;            // Swapped order
            api_message->mess_status2  = NullStatus.status;  //  in V4.05.
         }
         else
         {
            api_message->mess_status1  = mblock.mess_status1;
            api_message->mess_status2  = mblock.mess_status2;
         }
      }
      else
      {
         api_message->mess_status1  = mblock.mess_status1;
         api_message->mess_status2  = NullStatus.status; // Clear unused word.V4.04.ajh
      }
      //api_message->status        = mblock.status;
      api_message->status        = (((BT_U32BIT)mblock.mstatus[1])<<16) | mblock.mstatus[0];

      if(board_has_bc_timetag[cardnum])
         //api_message->time_tag = mblock.timetag;
         TimeTagConvert(cardnum, &(mblock.timetag), &(api_message->time_tag));

      if(board_using_msg_schd[cardnum])
      {
         api_message->rep_rate = mblock.addr_data2;
         api_message->start_frame = mblock.start_frame;
      }

      addr = ((BT_U32BIT)mblock.addr_error_inj) << 1;
      id = (BT_U16BIT)((addr - BTMEM_EI) / sizeof(EI_MESSAGE));

      addr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
      
      if(board_access_32[cardnum])
         vbtRead32(cardnum,(LPSTR)(api_message->data[0]),addr,(2*BT1553_BUFCOUNT));
      else
         vbtRead(cardnum,(LPSTR)(api_message->data[0]),addr,2*BT1553_BUFCOUNT);

      /* If we are single-buffered, no need to read the same buffer twice. */
      if ( mblock.addr_data1 != mblock.addr_data2 )
      {
         addr = ((BT_U32BIT)mblock.addr_data2) << hw_addr_shift[cardnum];
         if(board_access_32[cardnum])
            vbtRead32(cardnum,(LPSTR)(api_message->data[1]),addr,(2*BT1553_BUFCOUNT));
         else
            vbtRead(cardnum,(LPSTR)(api_message->data[1]),addr,2*BT1553_BUFCOUNT);
      }

      return API_SUCCESS;
   }

   /*******************************************************************
   *  Buffer is NOT a 1553 message -- figure it out here
   *******************************************************************/
   bcptr.mpbuf = &mblock;
   cbuf = bcptr.cpbuf;

   /************************************************
   *  Process Read BC Message
   *  Get branch and next jump to addresses
   ************************************************/
   addr = ((BT_U32BIT)cbuf->branch_msg_ptr) << 1;
   if(addr>0)
   {
      status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
      if (status)
         return status;
      api_message->messno_branch = (BT_U16BIT)id;
   }
   else
      api_message->messno_branch = (BT_U16BIT)0;

   addr = ((BT_U32BIT)cbuf->addr_next) << 1;    // Apply *8 below if needed.
   if ( addr == 0 )
      api_message->messno_next   = 0xFFFF;
   else
   {
      status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
      if (status)
         return status;
      api_message->messno_next   = (BT_U16BIT)id;
   }

   /*******************************************************************
   *  Now figure out what type of message this really is
   *******************************************************************/
   switch ( cbuf->control_word & 0x0006 )   /* Extract ctl1 and ctl0 */
   {
      case 0x0000:       /* no-op or simple branch message (same format) */
         api_message->time_tag = cbuf->timetag;
         if (api_message->messno_next != (api_message->messno + 1))
            api_message->control    = BC_CONTROL_BRANCH;
         else
         {
            api_message->control    = BC_CONTROL_NOP;
            if (mblock.control_word & BC_HWCONTROL_MFRAMEBEG)
               api_message->control             |= BC_CONTROL_MFRAME_BEG;
            if (mblock.control_word & BC_HWCONTROL_MFRAMEEND)
               api_message->control             |= BC_CONTROL_MFRAME_END;
            if (mblock.control_word & BC_HWCONTROL_SET_TNOP)
               api_message->control             |= BC_CONTROL_TIMED_NOP;
         }
         return API_SUCCESS;

      /************************************************
      *  Conditional branch
      ************************************************/
      case 0x0006:       /* Conditional Branch */
         if (mblock_id == 0)
            return API_BC_MESS1_COND;

         api_message->time_tag = cbuf->timetag;
         addr = ((BT_U32BIT)cbuf->branch_msg_ptr) << 1;
         status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
         if (status)
            return status;
         api_message->messno_branch = (BT_U16BIT)id;

         addr = ((BT_U32BIT)cbuf->addr_next) << 1;    // Apply *8 below if needed.
         if ( addr == 0 )
            api_message->messno_next   = 0xFFFF;
         else
         {
            status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
            if (status)
               return status;
            api_message->messno_next   = (BT_U16BIT)id;
         }

         api_message->data_value    = cbuf->data_pattern;
         api_message->data_mask     = cbuf->bit_mask;

         /* Read back the conditional counter values.V4.39.ajh */
         api_message->cond_count_val = cbuf->cond_count_val;
         api_message->cond_counter   = cbuf->cond_counter;

         addr = ((BT_U32BIT)cbuf->tst_wrd_addr1) << 1;      
         addr_prev = BC_MBLOCK_ADDR(cardnum, mblock_id-1);

         if (addr < addr_prev)
         {
            api_message->control    = BC_CONTROL_CONDITION2;
            api_message->address    = (BT_U16BIT)(addr >> hw_addr_shift[cardnum]);

            api_message->test_address = cbuf->tst_wrd_addr1 << 4 | cbuf->tst_wrd_addr2 << 1;

         }
         else
         {
            api_message->control    = BC_CONTROL_CONDITION;

            if (addr == addr_prev +
                            ((char*)&(mblock.mess_command1) - (char*)&mblock))
               wordno = 0;
            else if (addr == addr_prev +
                            ((char*)&(mblock.mess_command2) - (char*)&mblock))
               wordno = 1;
            else if (addr == addr_prev +
                             ((char*)&(mblock.mess_status1) - (char*)&mblock))
               wordno = 2;
            else if (addr == addr_prev +
                             ((char*)&(mblock.mess_status2) - (char*)&mblock))
               wordno = 3;
            else
               wordno = 4 +
                        (BT_U16BIT)((addr - (addr_prev + BC_MBLOCK_SIZE(cardnum))) / 2);

            if (wordno > 35)
               return API_BC_BAD_COND_ADDR;

            api_message->address = (BT_U16BIT)wordno;
         }
         return API_SUCCESS;

      /****************************************************************
      *   Last message in list(Stop BC message)
      ****************************************************************/

      case 0x0004:       /* Stop BC message */
         api_message->control = BC_CONTROL_LAST;
         api_message->time_tag = cbuf->timetag;
         return API_SUCCESS;
   }
   return API_BC_ILLEGALMESSAGE;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageReadData()
*
*  FUNCTION
*     This routine is used to read the data area of the specified message
*     block.  The data area to be read (there are two for each message
*     block) is determined by the current state of the ping-pong value
*     in the BC Message Block Control Word.`
*
*     If both message data buffer pointers are the same, just read
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageReadData(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) number of the BC message
   BT_U16BIT * buffer)  // (o) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MESSAGE mblock;    // Used to read the BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = BC_MBLOCK_ADDR(cardnum, mblock_id);
   vbtRead(cardnum,(LPSTR)&mblock,addr,6*2);

   if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;

   /* Extract the number of words to read */
   wCount = mblock.mess_command1.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  Read data from the data buffer which is in use.
   *  If there is only one data buffer, both pointers point to it.
   *******************************************************************/
   if (mblock.control_word & BC_HWCONTROL_BUFFERA)
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
   else
      daddr = ((BT_U32BIT)mblock.addr_data2) << hw_addr_shift[cardnum];
   /* Read the specified data buffer, using the specified word count */
   vbtRead(cardnum, (LPSTR)buffer, daddr, 2*wCount);
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageReadDataBuffer()
*
*  FUNCTION
*     This routine is used to read the data area of the specified message
*     block.  The data area to be read (there are two for each message
*     block) is determined by the buffer parameter passed.
*
*     If both message data buffer pointers are the same, just read
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageReadDataBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) number of the BC message
   BT_UINT   buffer_id,      // (i) Buffer ID 0=A 1=B
   BT_U16BIT * buffer)      // (o) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MESSAGE mblock;    // Used to read the BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if(buffer_id > 1)
      return API_BAD_PARAM;

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = BC_MBLOCK_ADDR(cardnum, mblock_id);
   vbtRead(cardnum,(LPSTR)&mblock,addr,6*2);

   if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;

   /* Extract the number of words to read */
   wCount = mblock.mess_command1.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  Read data from the data buffer which is in use.
   *  If there is only one data buffer, both pointers point to it.
   *******************************************************************/
   if (buffer_id==0)
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
   else
      daddr = ((BT_U32BIT)mblock.addr_data2) << hw_addr_shift[cardnum];
   /* Read the specified data buffer, using the specified word count */
   vbtRead(cardnum, (LPSTR)buffer, daddr, 2*wCount);
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageWrite()
*
*  FUNCTION
*     This routine is used to write the specified BC Message Block to the
*     board.  The data is retrieved from the caller supplied structure.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*     API_HARDWARE_NOSUPPORT  -> IP does not support conditional messages
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   API_BC_MBUF * api_message)  // (i) pointer to user's buffer containing msg
{
   /*******************************************************************
   *  Local variables
   ********************************************************************/
   BT_UINT    next_messno;  /* Next message's message number          */
   BT_UINT    id;           /* Compare or branch message number       */
   BT_UINT    messtype;     /* Message type to be generated           */
   BT_UINT    wordno;

   BT_U32BIT  addr;         /* General purpose byte address           */
   BT_U32BIT  bc_msg_addr;  /* Byte address of the current BC message block */
   BT_U32BIT  data_addr1;   /* First BC message data buffer byte address    */
   BT_U32BIT  data_addr2;   /* Second BC message data buffer byte address   */

   BC_MESSAGE mblock;       /* BC Message block structure              */
   union{
     BC_CBUF         *cpbuf;       // Pointer used to ref msg as an IP conditional
     BC_MESSAGE      *mpbuf;
   }bcptr;
   BC_CBUF    *cbuf;        /* Control message block structure pointer */
   BT_U32BIT  gap_time;     /* Local corrected gap time                */

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   {
      BT_UINT    status;       /* Status return from called functions   */
      if ( pUsrBC_MessageWrite[cardnum] )
      {
         status = (*pUsrBC_MessageWrite[cardnum])(cardnum, &mblock_id, api_message);
         if ( status == API_RETURN_SUCCESS )
            return API_SUCCESS;
         else if ( status == API_NEVER_CALL_AGAIN )
            pUsrBC_MessageWrite[cardnum] = NULL;
         else if ( status != API_CONTINUE )
            return status;
      }
   }
#endif

   /*******************************************************************
   *  We check the intermessage gap time or absolute time
   *******************************************************************/

   gap_time = api_message->gap_time;

   if(board_using_extended_timing[cardnum] == 0)
   {
      if(gap_time == 0) //if the gap is not set, set to default of 15 usec
         gap_time = 15;
   }

   /* Caluclate the Data Buffer addresses                             */
   bc_msg_addr = BC_MBLOCK_ADDR(cardnum, mblock_id);

   memset((char *)&mblock, 0, sizeof(mblock));

   data_addr2 = data_addr1 = bc_msg_addr + BC_MBLOCK_SIZE(cardnum);
   if ( bc_num_dbufs[cardnum] != 1 ) /* Not single buffered. */
      data_addr2 += bc_size_dbuf[cardnum];
   mblock.addr_data1 = (BT_U16BIT)(data_addr1 >> hw_addr_shift[cardnum]);
   mblock.addr_data2 = (BT_U16BIT)(data_addr2 >> hw_addr_shift[cardnum]);

   /*******************************************************************
   *  Figure out what kind of message this is, based on the 3 bit field.
   *******************************************************************/
   messtype = api_message->control & BC_CONTROL_TYPEMASK;

   if (messtype == BC_CONTROL_MESSAGE || messtype == BC_CONTROL_MSG_NOP)
   {
      /* Check for illegal bits */
      if ((api_message->control & BC_CONTROL_BUFFERA) &&
          (api_message->control & BC_CONTROL_BUFFERB))
         return API_BC_BOTHBUFFERS;
      if ((api_message->control & BC_CONTROL_CHANNELA) &&
          (api_message->control & BC_CONTROL_CHANNELB))
         return API_BC_BOTHBUSES;

      if(messtype == BC_CONTROL_MSG_NOP)
      {
         if(api_message->control & BC_CONTROL_TIMED_NOP) //make this a timed noop
            mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_NOP | BC_HWCONTROL_SET_TNOP;    // 1553 message Noop
         else
            mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_NOP;
      }
      else  
         mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_OP;    // 1553 message

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         mblock.control_word        |= BC_HWCONTROL_MFRAMEBEG;
      if (api_message->control & BC_CONTROL_MFRAME_END)
         mblock.control_word        |= BC_HWCONTROL_MFRAMEEND;
      if (api_message->control & BC_CONTROL_RTRTFORMAT)
         mblock.control_word        |= 0x0020;
      if (api_message->control & BC_CONTROL_RETRY)
         mblock.control_word        |= 0x0400;
      if (api_message->control & BC_CONTROL_INTERRUPT)
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
      if (api_message->control & BC_CONTROL_INTQ_ONLY)
      {
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word        |= BC_HWCONTROL_INTQ_ONLY;
      }
      if (api_message->control & BC_CONTROL_BUFFERA)
         mblock.control_word        |= BC_HWCONTROL_BUFFERA;
      if (api_message->control & BC_CONTROL_BUFFERB)
         mblock.control_word        |= BC_HWCONTROL_BUFFERB;

      if (api_message->control & BC_CONTROL_CHANNELA)
         mblock.control_word        |= BC_HWCONTROL_CHANNELA;
      if (api_message->control & BC_CONTROL_CHANNELB)
         mblock.control_word        |= BC_HWCONTROL_CHANNELB;

      mblock.mess_command1 = api_message->mess_command1; // All messages
      mblock.mess_command2 = api_message->mess_command2; // Only for RT-RT msgs

      
      if(board_using_extended_timing[cardnum])
      {
         if(api_message->long_gap == 0)
            api_message->long_gap = (BT_U32BIT)gap_time;
         mblock.gap_time = (BT_U16BIT)(api_message->long_gap & 0xffff);
         mblock.gap_time2 = (BT_U16BIT)((api_message->long_gap & 0x00ff0000)>> 16);        
      }
      else
         mblock.gap_time = api_message->gap_time;

      mblock.mess_status1  = api_message->mess_status1;
      mblock.mess_status2  = api_message->mess_status2;
      //mblock.status        = api_message->status;
      api_message->status  = (((BT_U32BIT)mblock.mstatus[1])<<16) | mblock.mstatus[0];

      addr = BTMEM_EI + api_message->errorid * sizeof(EI_MESSAGE);
      mblock.addr_error_inj = (BT_U16BIT)(addr >> 1); // EI pointers are word addrs

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
      }

      mblock.timetag.microseconds=0x0;
      mblock.timetag.topuseconds=0x0;

      if(board_using_msg_schd[cardnum])
      {
         mblock.addr_data2 = api_message->rep_rate;
         mblock.start_frame = api_message->start_frame;
      }

      /* Data buffer pointers already setup by BusTools_MessageAlloc() */
      /* Write the completed message to the board.                     */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      else
         vbtWrite(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));

      /* Write the first data buffer to the board  */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)(api_message->data[0]),data_addr1,2*BT1553_BUFCOUNT);
      else
         vbtWrite(cardnum,(LPSTR)(api_message->data[0]),data_addr1,2*BT1553_BUFCOUNT);

      /* If buffers are different, write the second data buffer to the board */
      //if ( data_addr1 != data_addr2 ) /* Not single buffered. */
      if(bc_num_dbufs[cardnum] != 1)
      {
         if(board_access_32[cardnum])
            vbtWrite32(cardnum,(LPSTR)(api_message->data[1]),data_addr2,(2*BT1553_BUFCOUNT));//round up to 
         else
            vbtWrite(cardnum,(LPSTR)(api_message->data[1]),data_addr2,2*BT1553_BUFCOUNT);
      }

      return API_SUCCESS;
   }

   /*******************************************************************
   *   Handle last message entry which generates a BC Stop message.
   *******************************************************************/
   if ( messtype == BC_CONTROL_LAST )
   {
      mblock.control_word = BC_HWCONTROL_LASTMESS | BC_HWCONTROL_OP;  /* Stop BC message (last message block) */

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         mblock.control_word |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word |= BC_HWCONTROL_INTQ_ONLY;
      }

	  if(api_message->control & BC_CONTROL_EXT_SYNC)  /* This is from BC_CONTROL_HALT */
	     mblock.control_word |= BC_CLR_EXT_SYNC;

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if (next_messno >= bc_mblock_count[cardnum])
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
      }

      /* Write the completed message to the board.                     */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      else
         vbtWrite(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));

      return API_SUCCESS;
   }

   /*******************************************************************
   *  BC Write Message:
   *  Handle timed no-op message 
   *******************************************************************/
   if (api_message->control & BC_CONTROL_TIMED_NOP)
   {
      mblock.control_word = BC_HWCONTROL_SET_TNOP;         /* Timed Noop message */

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         mblock.control_word |= BC_HWCONTROL_MFRAMEBEG;  /* V4.01 */

      if (api_message->control & BC_CONTROL_MFRAME_END)
         mblock.control_word |= BC_HWCONTROL_MFRAMEEND;  /* V2.41 */

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word        |= BC_HWCONTROL_INTQ_ONLY;
      }
      mblock.gap_time      = (BT_U16BIT)gap_time; // Set the gap time for this noop
      next_messno = api_message->messno_next;

      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
      }

#if 0 //Not implemented on this release
      if(board_using_msg_schd[cardnum])
      {
         mblock.addr_data2 = api_message->rep_rate;
         mblock.start_frame = api_message->start_frame;
      }
#endif //0 

      /* Write the completed message to the board.                     */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      else
         vbtWrite(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      return API_SUCCESS;
   }

   /*******************************************************************
   *  BC Write Message:
   *  Handle no-op message and unconditional branch to specified msg.
   *  Both generate a Noop message, which can have the End Minor Frame 
   *  bit set.
   *******************************************************************/
   if ((messtype == BC_CONTROL_NOP) || (messtype == BC_CONTROL_BRANCH))
   {
      mblock.control_word = BC_HWCONTROL_NOP;         /* Noop message */

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         mblock.control_word |= BC_HWCONTROL_MFRAMEBEG;  /* V4.01 */

      if (api_message->control & BC_CONTROL_MFRAME_END)
         mblock.control_word |= BC_HWCONTROL_MFRAMEEND;  /* V2.41 */

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word        |= BC_HWCONTROL_INTQ_ONLY;
      }

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
      }

      /* Write the completed message to the board.                     */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      else
         vbtWrite(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      return API_SUCCESS;
   }

   /*******************************************************************
   *   BC Write Message:
   *   Handle conditional messages (three kinds).
   *   All generate the same Conditional Message.
   *******************************************************************/
   if ( mblock_id == 0 )
      return API_BC_MESS1_COND;     /* First message cannot be a conditional. */

   bcptr.mpbuf = &mblock;
   cbuf = bcptr.cpbuf;
   
   cbuf->control_word = 0x0006 | 1;    /* Conditional message */
   if (api_message->control & BC_CONTROL_INTERRUPT) /* add interrupt */
         cbuf->control_word |= BC_HWCONTROL_INTERRUPT;

   /* Move the fixed data into the hardware-image scratch buffer. */
   cbuf->data_pattern = api_message->data_value;
   cbuf->bit_mask     = api_message->data_mask;
   /* Move the conditional count values.V4.39.ajh */
   cbuf->cond_count_val = api_message->cond_count_val;
   cbuf->cond_counter   = api_message->cond_counter;

   id = api_message->messno_branch;
   if (id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_BRANCH; 
   cbuf->branch_msg_ptr = (BT_U16BIT)
                        (BC_MBLOCK_ADDR(cardnum, id) >> hw_addr_shift[cardnum]);

   next_messno = api_message->messno_next;
   if ( next_messno == 0xFFFF ) /* Check for special end-of-aperiodic msg list */
      cbuf->addr_next = 0;
   else
   {
      if ( next_messno >= bc_mblock_count[cardnum] )
         return API_BC_ILLEGAL_NEXT;
      cbuf->addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
   }

   if ( (messtype == BC_CONTROL_CONDITION) ||
        (messtype == BC_CONTROL_CONDITION3) ) /* Bug Report A-000040.ajh */
   {
      /********************************************************************
      *  Type 1 or Type 3 Conditional Branch.  Set up address of compare,
      *   using the specified word number, relative to the previous or
      *    the specified message.
      ********************************************************************/
      wordno = api_message->address;
      if ( wordno > 35 )
         return API_BC_BAD_COND_ADDR;

      if (messtype == BC_CONTROL_CONDITION3)  /* Bug Report A-000040.ajh */
      {
         /* Fetch the address of the specified message. */
         id = api_message->messno_compare;
         if (id >= bc_mblock_count[cardnum])
            return API_BC_ILLEGALTARGET;
         addr = BC_MBLOCK_ADDR(cardnum, id);
      }
      else
      {
         /* Fetch the address of the previous message. */
         addr = BC_MBLOCK_ADDR(cardnum, mblock_id-1);
      }
      /* Adjust the address of previous msg, to specified word. */
      switch ( wordno )
      {
         case 0:    /* Command word of previous or specified message */
            addr += ((char*)&(mblock.mess_command1) - (char*)&mblock);
            break;
         case 1:    /* Command word #2 (RT-RT msgs only) of previous or specified message */
            addr += ((char*)&(mblock.mess_command2) - (char*)&mblock);
            break;
         case 2:    /* Status word of previous or specified message */
            addr += ((char*)&(mblock.mess_status1) - (char*)&mblock);
            break;
         case 3:    /* Status word #2 (RT-RT msgs only) of previous or specified message */
            addr += ((char*)&(mblock.mess_status2) - (char*)&mblock);
            break;
         default:   /* Data words #1 through #32 of first data buffer */
            addr += BC_MBLOCK_SIZE(cardnum) + 2 * (wordno-4);
            break;
      }

      // Convert byte address to word address and split into components.
      cbuf->tst_wrd_addr1 = (BT_U16BIT)((addr >> 4)       );  // V3.30.ajh
      cbuf->tst_wrd_addr2 = (BT_U16BIT)((addr >> 1) & 0x07);
   }
   else if ( messtype == BC_CONTROL_CONDITION2 )
   {
      /* Compare address is directly specified by caller. */
      
      // Convert byte address to word address and split into components.
      cbuf->tst_wrd_addr1 = (BT_U16BIT)((api_message->test_address >> 4)       );// V3.30.ajh
      cbuf->tst_wrd_addr2 = (BT_U16BIT)((api_message->test_address >> 1) & 0x07);
   }

   /* Write the completed message to the board.                     */
   if(board_access_32[cardnum])
      vbtWrite32(cardnum,(LPSTR)cbuf,bc_msg_addr,sizeof(BC_CBUF));
   else
      vbtWrite(cardnum,(LPSTR)cbuf,bc_msg_addr,sizeof(BC_CBUF));

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageUpdate()
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified message block.  The data area to be updated
*     (there are two for each message block) is determined
*     by the current state of the ping-pong value in the
*     BC Message Block Control Word.  If data buffer A is
*     currently being used, then buffer B is updated.  Likewise,
*     if data buffer B is currently being used, then buffer
*     A is updated.  After the data area is updated, the buffer
*     bit in the Control Word is updated.  The next time this
*     message is sent by the BC, it will use the new data.
*
*     If both message data buffer pointers are the same, just update
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
* 01/19/1998 Improve speed by only reading message buffer up to the
*            data buffer pointers.
* 07/07/1998 Improve speed by only writing number of words specified
*            by the 1553 command word.
* 11/20/1998 If single buffered, just output the data words.
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_U16BIT * buffer)  // (i) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MESSAGE mblock;    // Used to read the BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message
   BT_UINT    status;    // Status return from called functions

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageUpdate[cardnum] )
   {
      status = (*pUsrBC_MessageUpdate[cardnum])(cardnum, &mblock_id, buffer);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageUpdate[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = BC_MBLOCK_ADDR(cardnum, mblock_id);
   vbtRead(cardnum,(LPSTR)&mblock,addr,6*2);   /*sizeof(mblock)); V2.71.ajh */

   /*******************************************************************
   *  If this is not a 1553 message.return error
   *******************************************************************/
   if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;
   
   /* Extract the number of words to write */
   wCount = mblock.mess_command1.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  If there is only one data buffer, just update it.
   *******************************************************************/
   if ( mblock.addr_data1 == mblock.addr_data2 )
   {
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
      /* Update the single data buffer, using the specified word count */
      vbtWrite(cardnum, (LPSTR)buffer, daddr, 2*wCount);
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Two data buffers are defined.
   *  Output new data buffer to buffer not in use, then swap buffers.
   *******************************************************************/
   if (mblock.control_word & BC_HWCONTROL_BUFFERA)
   {
      daddr = ((BT_U32BIT)mblock.addr_data2) << hw_addr_shift[cardnum];
      mblock.control_word &= ~BC_HWCONTROL_BUFFERA;
   }
   else
   {
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
      mblock.control_word |= BC_HWCONTROL_BUFFERA;
   }
   /* Update the specified data buffer, using the specified word count */
   vbtWrite(cardnum, (LPSTR)buffer, daddr, 2*wCount);

   /*******************************************************************
   *  Update message block command word (2 bytes)
   *******************************************************************/
   vbtWrite(cardnum,(LPSTR)&mblock,addr,1*2);
   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageUpdateBuffer()
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified message block and Buffer.  The data area to be updated
*     is determined by the buffer parameter passed to this function.  
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
* 01/19/1998 Improve speed by only reading message buffer up to the
*            data buffer pointers.
* 07/07/1998 Improve speed by only writing number of words specified
*            by the 1553 command word.
* 11/20/1998 If single buffered, just output the data words.
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageUpdateBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer)      // (i) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MESSAGE mblock;    // Used to read the BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message
   BT_UINT    status;    // Status return from called functions

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if (bc_num_dbufs[cardnum] == 1)// Only use 1 buffer
      if(buffer_id == 1)
         buffer_id =0;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageUpdate[cardnum] )
   {
      status = (*pUsrBC_MessageUpdate[cardnum])(cardnum, &mblock_id, buffer);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageUpdate[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = BC_MBLOCK_ADDR(cardnum, mblock_id);
   vbtRead(cardnum,(LPSTR)&mblock,addr,6*2);   /*sizeof(mblock)); V2.71.ajh */

   /*******************************************************************
   *  If this is not a 1553 message, return error.
   *******************************************************************/
   if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;
   
   /* Extract the number of words to write */
   wCount = mblock.mess_command1.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  If there is only one data buffer, just update it.
   *******************************************************************/
   if ( mblock.addr_data1 == mblock.addr_data2 )
   {
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
      /* Update the single data buffer, using the specified word count */
      vbtWrite(cardnum, (LPSTR)buffer, daddr, 2*wCount);
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Two data buffers are defined.
   *  Output new data buffer to selected buffer.
   *******************************************************************/
   if (buffer_id == 1)
   {
      daddr = ((BT_U32BIT)mblock.addr_data2) << hw_addr_shift[cardnum];
   }
   else
   {
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
   }

   /* Update the specified data buffer, using the specified word count */
   vbtWrite(cardnum, (LPSTR)buffer, daddr, 2*wCount);

   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_SetFrameRate
*
*  FUNCTION
*     This routine dynamically update the BC frame time.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BM_Init not yet called
*     API_BC_BADFREQUENCY     -> Invalid Frame time
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_SetFrameRate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT frame_time)    // (i) New Frame Time in uSecs
{
   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if ((frame_time > 1638375L) || (frame_time < 250))
      return API_BC_BADFREQUENCY;

   /*******************************************************************
   *  Store minor frame register -- frame time is in
   *   units of 25 micro-seconds
   *******************************************************************/
   frametime[cardnum] = (BT_U16BIT)(frame_time/25);   /* Must be less than 65535. */
   
   vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME, frametime[cardnum]);
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_Start
*
*  FUNCTION
*     This routine handles starting the BC at a specified message number.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BM_Init not yet called
*     API_BC_RUNNING          -> BM currently running
*     API_BUSTOOLS_BADCARDNUM -> Invalid card number
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_Start(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT mblock_id)       // (i) BC Message number to start processing at
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT  value;       /* General Purpose variable */
   BT_INT mode_warning;
   BT_U16BIT hwrData;

   /************************************************
   *  Check initial conditions
   ************************************************/
   AddTrace(cardnum, NBUSTOOLS_BC_START, mblock_id, bc_running[cardnum], 0, 0, 0);
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Handle "BC-Start" operation:
   *     re-load BC message start pointer
   *     start BC
   *******************************************************************/
   // Detect single function board and attempt to start second function.V4.26
   if(!bit_running[cardnum])
   {
      if(board_is_dual_function[cardnum] == 1)
      {
         mode_warning = API_DUAL_FUNCTION_ERR;
         if ( _HW_1Function[cardnum] && (rt_running[cardnum]) )
           return API_DUAL_FUNCTION_ERR;
      }
      else
      {
         mode_warning = API_SINGLE_FUNCTION_ERR;
         if ( _HW_1Function[cardnum] && (bm_running[cardnum] | rt_running[cardnum]) )
            return API_SINGLE_FUNCTION_ERR;
      }
   }

   SignalUserThread(cardnum, EVENT_BC_START, 0, 0);

   value = (BT_U16BIT)(BC_MBLOCK_ADDR(cardnum, mblock_id) >> hw_addr_shift[cardnum]);
   vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTR  , 0x0000);
   vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV, value);
   vbtSetFileRegister(cardnum, RAMREG_ENDFLAGS, 0);

   // Clear the minor frame overflow bit.
   vbtSetFileRegister(cardnum, RAMREG_ORPHAN,
                  (BT_U16BIT)(vbtGetFileRegister(cardnum, RAMREG_ORPHAN) &
                                             ~RAMREG_ORPHAN_MINORFRAME_OFLOW));

   // Set the minor frame time into the hardware register 0x0E!
//   vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME, frametime[cardnum]);

   /* If ext_trig_bc == 0, no external triggering of BC. */
   /*                == 1, BC started by external trigger. */
   /*                ==-1, each BC minor frame started by external trigger. */
   /* On a 486/DX4-100 it takes 17 us to get the bus started. */
   if ( ext_trig_bc[cardnum] == 0 )
   {
      api_writehwreg(cardnum,HWREG_BC_EXT_SYNC,0x0000);  /* Disable HW BC trigger */
      api_writehwreg_or(cardnum,HWREG_CONTROL1,CR1_BCRUN); /* Just run the BC */
      //Test for single/dual mode warning
      hwrData = vbtGetHWRegister(cardnum,HWREG_CONTROL1);
      if(hwrData & CR1_SMODE)
         return mode_warning;
   }
   else
   {
      api_writehwreg(cardnum,HWREG_BC_EXT_SYNC,CR1_BC_EXT_SYNC); /* Enable HW BC trig */
   }
   /* The HWREG_RESPONSE register must be programmed after one of the */
   /*  three run bits has been set.  Does the trigger bit count?? */
   api_writehwreg(cardnum,HWREG_RESPONSE,wResponseReg4[cardnum]);

   bmrec_timetag[cardnum].microseconds = 0;      /* Timetag for simulated messages only */
   bmrec_timetag[cardnum].topuseconds = 0;
   bc_running[cardnum]    = 1;      /* Try to remember that the BC is running */

   channel_status[cardnum].bc_run=1;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_StartStop
*
*  FUNCTION
*     This routine handles turning the BC on or off.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BM_Init not yet called
*     API_BC_RUNNING          -> BM currently running
*     API_BC_NOTRUNNING       -> BM not currently running
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_StartStop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT startflag)       // (i) flag=1 to start the BC, 0 to stop it 0xF BC_START_BIT
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT  value;       /* Determine if BC busy */
   time_t     lTime = 0;   /* Timeout value */
   BT_INT     status;

   /************************************************
   *  Check initial conditions
   ************************************************/
   AddTrace(cardnum, NBUSTOOLS_BC_STARTSTOP, startflag, bc_running[cardnum], bt_inited[cardnum], 0, 0);
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   {
      int        i;           /* Loop counter */
      if ( pUsrBC_StartStop[cardnum] )
      {
         i = (*pUsrBC_StartStop[cardnum])(cardnum, &startflag);
         if ( i == API_RETURN_SUCCESS )
            return API_SUCCESS;
         else if ( i == API_NEVER_CALL_AGAIN )
            pUsrBC_StartStop[cardnum] = NULL;
         else if ( i != API_CONTINUE )
            return i;
      }
   }
#endif

   /*******************************************************************
   *  Handle "BC-Start" operation:
   *     re-load BC message start pointer
   *     start BC
   *******************************************************************/
   if ( startflag )
   {
      if(startflag==BC_START_TT_RESET)
      {
         /*  Initialize the H/W Bus Monitor time counter. */
         BusTools_TimeTagInit(cardnum);
      }
      if(startflag == 0xf)
         bit_running[cardnum]=1;
      /* Start the Bus Controller beginning with the first message in the list. */
      status = BusTools_BC_Start(cardnum, 0);
      if(startflag == 0xf)
         bit_running[cardnum]=0;
      return status;
   }

   /*******************************************************************
   *  Handle "stop" operation -- loop until the BC_BUSY bit is clear.
   *  If it takes too long, just turn off the BC anyway.
   *******************************************************************/
   SignalUserThread(cardnum, EVENT_BC_STOP, 0, 0);
   /* Turn off the Bus Controller Enable external trigger bit first, */
   /*  if it is on. */

   api_writehwreg(cardnum,HWREG_BC_EXT_SYNC,0x0000);  /* Disable HW BC trigger */

   lTime = time(NULL) + 2;   /* Setup a time-out value of 1-2 seconds.V4.01 */
   /* This is to allow the frame to complete before stopping the BC */
   do{
      MSDELAY(1); //Wait a millisecond
      value = api_readhwreg(cardnum,HWREG_CONTROL1);
      /* Finish if either BC Run or BC Busy bits are clear */
      if ( (value & (CR1_BCBUSY|CR1_BCRUN)) != (CR1_BCBUSY|CR1_BCRUN) )
         break;
   }
   while ( time(NULL) < lTime );

   // Turn off the BC Run bit and the BC Busy.  
   api_writehwreg_and(cardnum,HWREG_CONTROL1,(BT_U16BIT)(~(CR1_BCRUN|CR1_BCBUSY)));

   bc_running[cardnum] = 0;

   /*******************************************************************
   *  If an error was detected -- return error code
   *******************************************************************/
   value = api_readhwreg(cardnum,HWREG_CONTROL1);

   if ( value & CR1_BCBUSY )
      return API_BC_HALTERROR;
   
   channel_status[cardnum].bc_run=0;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_Trigger()
*
*  FUNCTION
*     This routine is used to setup the BC external trigger mode.
*     The BC supports three trigger modes:
*
*     BC_TRIGGER_IMMEDIATE  - BC starts running immediately
*     BC_TRIGGER_ONESHOT    - BC is triggered by external source,
*                             and free runs after the trigger.
*     BC_TRIGGER_REPETITIVE - Each minor frame is started by the
*                             external trigger.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_Trigger(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_INT    trigger_mode)  // (i0 BC_TRIGGER_IMMEDIATE - BC starts immediately.
                            //  -> default              - BC starts immediately.
                            //  -> BC_TRIGGER_ONESHOT   - BC trig ext TTL source.
                            //  -> BC_TRIGGER_REPETITIVE- Each minor frame is
                            //                            started by the TTL trig.
                            //  -> BC_TRIGGER_USER        Allows user to configure Frame triggering
{
   /***********************************************************************
   *  Check initial conditions
   **********************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /*******************************************************************
   *  Set the global BC Trigger Flag based on the trigger_mode parameter.
   *******************************************************************/
   /* If ext_trig_bc == 0, no external triggering of BC. */
   /*                == 1, BC started by external trigger. */
   /*                ==-1, each BC minor frame started by external trigger. */
   switch ( trigger_mode )
   {
      case BC_TRIGGER_IMMEDIATE:  /* BC starts running immediately. */
      default:
           ext_trig_bc[cardnum] = BC_TRIGGER_IMMEDIATE;
           break;
      case BC_TRIGGER_ONESHOT:    /* BC is triggered by external source, */
                                  /*   and free runs after trigger. */
           ext_trig_bc[cardnum] = BC_TRIGGER_ONESHOT;
           break;
      case BC_TRIGGER_REPETITIVE: /* Each minor frame is started by the */
                                  /*   external trigger. */
           ext_trig_bc[cardnum] = BC_TRIGGER_REPETITIVE;
           break;
      case BC_TRIGGER_USER:       /* User defined trigger */
                                  /*   external trigger. */
           ext_trig_bc[cardnum] = BC_TRIGGER_USER;
           break;
   }
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -    DumpBCmsg()
*
*  FUNCTION
*       This procedure outputs a dump of BC Messages.  It is a local helper
*       function for the BusTools_DumpMemory user-callable function.
*
****************************************************************************/
#if defined(FILE_SYSTEM)
void DumpBCmsg(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT    i;                        // Loop index
   BT_UINT      j, k;                     // Loop index
   int          msgnum;                   // Message number
   int          shift_count;              // Shift PCI/ISA-1553 addresses
   int          num_ctrl_wrds;            // Number of BC Message control words
   BT_U32BIT    first;                    // Offset to first list word
   BT_U32BIT    last;                     // Offset to last list word
   BT_U16BIT    data[48];                 // Read 16+ words per line

   /* Dump the write-only BC setup parameters  */
   if(board_using_extended_timing[cardnum])
      fprintf(hfMemFile, "HWREG_RESPONSE Reg = %X, frametime(us) = %u(dec)\n",
             wResponseReg4[cardnum], frametime32[cardnum]);
   else
      fprintf(hfMemFile, "HWREG_RESPONSE Reg = %X, frametime(us) = %u(dec)\n",
             wResponseReg4[cardnum], frametime[cardnum]*25);

   /* This is where the specified buffers live */
   BusTools_GetAddr(cardnum, GETADDR_BCMESS, &first, &last);
   first /= 2;    // Convert to word offset.
   last  /= 2;    // Convert to word offset.
   msgnum = 0;
   shift_count = hw_addr_shift[cardnum] - 1;  // Just shift to word addresses
   num_ctrl_wrds = 24;

   for ( i = first; i < last; )
   {
      // Read the current line of 12 data words from the board.
      BusTools_MemoryRead(cardnum, i*2, 24*2, data);
      if ( (data[0] & 0x0006) == 0x0002 )
      {   // Normal 1553 message
         fprintf(hfMemFile, "Msg %3d @%04X:", msgnum, i);
         for ( j = 0; j < 2; j++ )
            fprintf(hfMemFile, " %4.4X", data[j]);
         if(board_has_bc_timetag[cardnum])
            fprintf(hfMemFile, " tt%04X%04X%04X", data[14], data[13], data[12]);
         fprintf(hfMemFile, " ei%3.3X", data[2]);
         fprintf(hfMemFile, " gap%d", data[3]);
         fprintf(hfMemFile, " F=%4.4X", data[4]<<shift_count);  // First Buffer address
         if ( (unsigned)(data[4]<<shift_count) == i+num_ctrl_wrds )
            fprintf(hfMemFile, "!");
         else
            fprintf(hfMemFile, "*");
         fprintf(hfMemFile, " S=%4.4X", data[5]<<shift_count);  // Second Buffer address
         if ( (unsigned)(data[5]<<shift_count) == i+num_ctrl_wrds + (bc_num_dbufs[cardnum]-1)*bc_size_dbuf[cardnum]/2 )
            fprintf(hfMemFile, "!");
         else
            fprintf(hfMemFile, "*");
         fprintf(hfMemFile, " 2c%4.4X", data[6]);                  // Second (RT-RT) Command word
         fprintf(hfMemFile, " tS%4.4X rS%4.4X", data[7], data[8]); // RT 1553 Status Words
         fprintf(hfMemFile, " %4.4X%4.4X", data[10], data[9]);     // Interrupt status
         if(board_using_msg_schd[cardnum])
           fprintf(hfMemFile, " SF=%d RR=%d ", data[15],data[5]);
         fprintf(hfMemFile, " nxt@%4.4X", data[11]<<shift_count);  // Next message addr
         if ( (unsigned)(data[11]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
            fprintf(hfMemFile, "!\n");
         else
            fprintf(hfMemFile, "*\n");
      }
      else
      {   // NOOP, STOP or Conditional
         if ( (data[0] & 0x0006) == 0x0000 )
         {  // NOOP Message
            if((data[0] & BC_HWCONTROL_SET_TNOP) == BC_HWCONTROL_SET_TNOP)
               fprintf(hfMemFile, "TNop %3d @%04X gap%d:", msgnum, i,data[3]);
            else
               fprintf(hfMemFile, "Nop %3d @%04X:", msgnum, i);
            for ( j = 0; j < 11; j++ )
               fprintf(hfMemFile, " %4.4X", data[j]);
            fprintf(hfMemFile, " nxt@%5.5X", data[11]<<shift_count);   // Next msg addr
            if ( (unsigned)(data[11]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
               fprintf(hfMemFile, "!\n");
            else
               fprintf(hfMemFile, "*\n");
         }
         else if ( (data[0] & 0x0006) == 0x0004 )
         {  // STOP Message
            fprintf(hfMemFile, "Stp %3d @%04X:", msgnum, i);
            for ( j = 0; j < 11; j++ )
               fprintf(hfMemFile, " %4.4X", data[j]);
            fprintf(hfMemFile, " nxt@%5.5X", data[11]<<shift_count);   // Next msg addr
            if ( (unsigned)(data[11]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
               fprintf(hfMemFile, "!\n");
            else
               fprintf(hfMemFile, "*\n");
         }
         else if ( (data[0] & 0x0006) == 0x0006 )
         {  // CONDITIONAL Message
            fprintf(hfMemFile, "Con %3d @%04X:", msgnum, i);
            fprintf(hfMemFile, " %4.4X", data[0]);
            fprintf(hfMemFile, " tst=%5.5X/%4.4X", (data[1]<<shift_count)+data[2],data[2]);
            for ( j = 3; j < 10; j++ )
               fprintf(hfMemFile, " %4.4X", data[j]);
            fprintf(hfMemFile, " brn@%4.4X", data[10]<<shift_count);     // Branch msg addr
            if ( (unsigned)(data[10]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
               fprintf(hfMemFile, "!");
            else
               fprintf(hfMemFile, "*");
            fprintf(hfMemFile, " nxt@%4.4X", data[11]<<shift_count);   // Next msg addr
            if ( (unsigned)(data[11]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
               fprintf(hfMemFile, "!\n");
            else
               fprintf(hfMemFile, "*\n");
         }
      }
      // Step over the message control words to the data buffers.
      i += num_ctrl_wrds;

      // For each buffer (either one or two)...
      for ( k = 0; k < bc_num_dbufs[cardnum]; k++ )
      {
         fprintf(hfMemFile, "Data %2d @%04X:", k, i);

         BusTools_MemoryRead(cardnum, i*2, 16*2, data);
         for ( j = 0; j < 16; j++ )
            fprintf(hfMemFile, " %4.4X", data[j]);
         fprintf(hfMemFile, "\n");
         i += 16;
         // 33 or 40 words per buffer
         fprintf(hfMemFile, "Data %2d @%04X:", k, i);
         BusTools_MemoryRead(cardnum, i*2, (bc_size_dbuf[cardnum]-32), data);
         //for ( j = 0; j < (unsigned)(bc_size_dbuf[cardnum]/2-16); j++ )
         for ( j = 0; j < 16; j++ )
            fprintf(hfMemFile, " %4.4X", data[j]);
         fprintf(hfMemFile, "\n");
         i += bc_size_dbuf[cardnum]/2 - 16;
      }
      msgnum++;
   }
}
#endif


#ifndef _CVI_
/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BC_ReadNextMessage()
*
*   FUNCTION
*       This function reads the next BC message that meets the criteria set
*       by the passed parameters.  The arguments include a timeout value in
*       in milliseconds. If there are no Rt messages put onto the queue this
*
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_MBLOCK_NOMATCH;  -> Bad BC Messno
*       API_BC_READ_TIMEOUT     -> Timeout before data read
*       API_HW_IQPTR_ERROR      -> interrupt Queue pointer error
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_ReadNextMessage(int cardnum,BT_UINT timeout,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_INT status;
   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.

   BT_U16BIT iq_addr;                  // Hardware Interrupt Queue Pointer
   BT1553_COMMAND cmd;
   BT_U32BIT beg;                      // Beginning of the interrupt queue
   BT_U32BIT end;                      // End of the interrupt queue

   BT_UINT   messno;
  
   BT_U32BIT mess_addr;
   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;
   BT_U32BIT  msg_blk_size;  // Size of a BC message block, including data bufs
   BT_U32BIT start;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   //start = timeGetTime();
   start = CEI_GET_TIME();

   // Get range of byte addresses for interrupt queue 
   beg = BTMEM_IQ;
   end = BTMEM_IQ_NEXT - 1;

   iq_addr = vbtGetFileRegister(cardnum,INT_QUE_PTR_REG/2);

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   iqptr_sw = ((BT_U32BIT)iq_addr) << 1;

   /************************************************
   *  Loop until timeout
   ************************************************/

   do
   {
      /* Read current location in interupt queue */
	  iq_addr = vbtGetFileRegister(cardnum,INT_QUE_PTR_REG/2);

      // Convert the hardware word address to a byte address.
      iqptr_hw = ((BT_U32BIT)iq_addr) << 1;
      // If the pointer is outside of the interrupt queue abort.V4.09.ajh
      if ( (iqptr_hw < beg) || (iqptr_hw > end) )
         return API_HW_IQPTR_ERROR;

      /**********************************************************************
      *  Process all HW Interrupt Queue entries that have been written
      *  Start with the SW interrupt pointer from the last time.
      **********************************************************************/

      while ( iqptr_sw != iqptr_hw )
      {
         /*******************************************************************
         *  Get the 3 word interrupt block pointed to by iqptr_sw.
         *******************************************************************/
         vbtRead_iq(cardnum,(LPSTR)&intqueue, iqptr_sw, sizeof(intqueue));

         iqptr_sw = ((BT_U32BIT)intqueue.nxt_int) << 1; // Chain to next entry

         // We only care about BC interrupt here.
         if ( intqueue.t.mode.bc )
         {
            mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
            /*******************************************************************
            *  Calculate index based on starting address & size.
            *******************************************************************/
            msg_blk_size = BC_MBLOCK_SIZE(cardnum) +
                                        (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);

            mess_addr <<= 3;     // Align to 8-word boundry.
            messno = (BT_UINT)( (mess_addr - btmem_bc[cardnum]) / msg_blk_size);

            if ( messno >= bc_mblock_count[cardnum] )
               return API_BC_MBLOCK_NOMATCH;
            vbtRead(cardnum, (LPSTR)&cmd, mess_addr+2, sizeof(BT1553_COMMAND));
            if((rt_addr == DONT_CARE) || (rt_addr & (bit<<cmd.rtaddr)))
			{
			   if((subaddress == DONT_CARE) || (subaddress & (bit<<cmd.subaddr)))
			   {
                  if((tr == DONT_CARE) || (tr == cmd.tran_rec))
				  {
                     status = BusTools_BC_MessageRead(cardnum, messno, pBC_mbuf);
				     return status;
				  }
			   }
			}

         }
      }
   }while((CEI_GET_TIME() - start) < timeout);
   
   return API_BC_READ_TIMEOUT;
}
#endif

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BM_ReadLastMessage()
*
*   FUNCTION
*       This function reads the last RT message that meets the criteria set
*       by the passed parameters.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_ReadLastMessage(int cardnum,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   BT_INT status;
   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.
   BT1553_COMMAND cmd;
   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_U32BIT iqptr_cur;
   BT_U32BIT mess_addr;

   BT_U32BIT msg_blk_size;
   BT_UINT   messno;

   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;
   BT_UINT queue_entry=6;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   iqptr_hw = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % sizeof(IQ_MBLOCK) != 0 ) )
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_bc_last[cardnum];
   iqptr_cur = iqptr_hw;
   
   /************************************************
   *  Loop until all the message are checked
   ************************************************/

   while ( iqptr_sw != iqptr_cur )
   {
      /*******************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
	  if(iqptr_cur == BTMEM_IQ)
         iqptr_cur = BTMEM_IQ_NEXT - queue_entry;
      else
	     iqptr_cur = iqptr_cur - queue_entry;      
	   
	   vbtRead(cardnum,(LPSTR)&intqueue, iqptr_cur, sizeof(intqueue));

      // We only care about RT interrupt here.
      if ( intqueue.t.mode.bc )
      {
         mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
         /*******************************************************************
         *  Calculate index based on starting address & size.
         *******************************************************************/
         msg_blk_size = BC_MBLOCK_SIZE(cardnum) +
                                     (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);

         mess_addr <<= 3;     // Align to 8-word boundry.
         messno = (BT_UINT)( (mess_addr - btmem_bc[cardnum]) / msg_blk_size);

         if ( messno >= bc_mblock_count[cardnum] )
            return API_BC_MBLOCK_NOMATCH;
         vbtRead(cardnum, (LPSTR)&cmd, mess_addr+2, sizeof(BT1553_COMMAND));

         if((rt_addr == DONT_CARE) || (rt_addr & (bit<<cmd.rtaddr)))
		 {
			if((subaddress == DONT_CARE) || (subaddress & (bit<<cmd.subaddr)))
			{
               if((tr == DONT_CARE) || (tr == cmd.tran_rec))
			   {
                  status = BusTools_BC_MessageRead(cardnum, messno, pBC_mbuf);
				  if(status)
                     return status;
				  else
					 break;
			   }
			}
		 }
      }
   }
   iqptr_bc_last[cardnum] = iqptr_hw;
   if(iqptr_sw == iqptr_hw)
	   return API_BC_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BC_ReadLastMessageBlock()
*
*   FUNCTION
*       This function reads all the RT messages that meets the criteria set
*       by the passed parameters that are stored in the interrupt queue between
*       the queue head pointer and the tail pointer.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_ReadLastMessageBlock(int cardnum,BT_INT rt_addr_mask, BT_INT subaddr_mask,
                                                       BT_INT tr, BT_UINT *mcount,API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_UINT   messno;
   BT_UINT   msg_cnt;
   BT_U32BIT mess_addr;
   BT1553_COMMAND cmd;
   BT_U32BIT msg_blk_size;

   BT_U32BIT bit = 1;
   BT_INT DONT_CARE = -1;
 
   *mcount = msg_cnt = 0; // Clear out the count before returning any error

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   iqptr_hw = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;
   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % sizeof(IQ_MBLOCK) != 0 ))   
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_bc_last[cardnum];
   /************************************************
   *  Loop until
   ************************************************/
   
   while ( iqptr_sw != iqptr_hw )
   {
      /*******************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
      vbtRead_iq(cardnum,(LPSTR)&intqueue, iqptr_sw, sizeof(intqueue));
      iqptr_sw = ((BT_U32BIT)intqueue.nxt_int) << 1; // Chain to next entry

      // We only care about BC interrupts here.
      if ( intqueue.t.mode.bc )
      {
         mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;

         /*******************************************************************
         *  Calculate index based on starting address & size.
         *******************************************************************/

         msg_blk_size = BC_MBLOCK_SIZE(cardnum) +
                                     (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);


         mess_addr <<= 3;     // Align to 8-word boundry.
         messno = (BT_UINT)( (mess_addr - btmem_bc[cardnum]) / msg_blk_size);

         if ( messno >= bc_mblock_count[cardnum] )
            return API_BC_MBLOCK_NOMATCH;

         vbtRead(cardnum, (LPSTR)&cmd, mess_addr+2, sizeof(BT1553_COMMAND));

         if((rt_addr_mask == DONT_CARE) || (rt_addr_mask & (bit<<cmd.rtaddr)))
         {
            if((subaddr_mask == DONT_CARE) || (subaddr_mask & (bit<<cmd.subaddr)))
            {
               if((tr == DONT_CARE) || (tr == cmd.tran_rec))
               {
                  BusTools_BC_MessageRead(cardnum, messno, &pBC_mbuf[msg_cnt]);
                  msg_cnt++;
               }  
            }
         }
      }
   }
   iqptr_bc_last[cardnum] = iqptr_sw;
   *mcount = msg_cnt;
   if(msg_cnt == 0)
	   return API_BC_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BC_Checksum1760()
*
*   FUNCTION
*       This function Calculates a 1760 checksum for the data in and stores  
*       the result in the next location in the API_BC_MBUF.  
*
*   RETURNS
*       API_SUCCESS             -> success
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_Checksum1760(API_BC_MBUF *mbuf, BT_U16BIT *cksum)
{
	BT_U16BIT checksum, wd, temp, i, shiftbit, wdcnt;

    wdcnt = mbuf->mess_command1.wcount;

	// Start at zero.
	checksum = 0;

	// Process each word in the data buffer.
	for (wd = 0; wd < (wdcnt-1); wd++) {
		temp = *mbuf->data[wd];

		// Cyclically right-shift the word by the word index.
		for (i=0; i<wd; i++) {
			shiftbit = temp & 0x0001;
			temp = temp >> 1;
			if (shiftbit) temp |= 0x8000;
		}

		// XOR the shifted word into the checksum.
		checksum ^= temp;
	}

	// Cyclically left-shift the checksum by the word index.
	for (i=0; i<wd; i++) {
		shiftbit = checksum & 0x8000;
		checksum = checksum << 1;
		if (shiftbit) checksum |= 0x0001;
	}
    
    *cksum = checksum;
    *mbuf->data[wdcnt-1] = checksum;
	return API_SUCCESS;
}

#ifndef _LVRT_
/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BC_AutoIncrMessageData()
*
*   FUNCTION
*       This function sets up an interrupt to automatically increment a data
*       value ins a partular BC message.  You supply the message number, start
*       value, increment value, increment rate, and max value.  You need to
*       both to start and stop the auto incrementing.  
*
*   RETURNS
*       API_SUCCESS         
*       API_BUSTOOLS_BADCARDNUM
*       API_BUSTOOLS_NOTINITED
*       API_BC_NOTMESSAGE
*       
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_AutoIncrMessageData(BT_INT cardnum,BT_INT messno,BT_INT data_wrd,
                                                      BT_U16BIT start, BT_U16BIT incr, 
                                                      BT_INT rate, BT_U16BIT max, BT_INT sflag)
{
   int    rt, tr, sa;
   int    status;

   API_BC_MBUF api_message;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /* Make sure we get an interrupt on this message */

   /***********************************************************************
   *  If the BC is starting, register a thread for the board.
   *  If the BC is shutting down, unregister the thread.
   **********************************************************************/
   if (sflag)
   {
      if(max == 0)
         max = 0xffff;

      if(pIntFIFO[cardnum][messno] != NULL)
      {
         return API_BC_AUTOINC_INUSE;
      }
           
      pIntFIFO[cardnum][messno] = (API_INT_FIFO *)malloc(sizeof(API_INT_FIFO));
      if(pIntFIFO[cardnum][messno] == NULL)
         return API_MEM_ALLOC_ERR;

      status = BusTools_BC_MessageRead(cardnum,messno,&api_message);
      if(status)
         return status;
      
      if ( (api_message.control & BC_CONTROL_TYPEMASK) != BC_CONTROL_MESSAGE )
         return API_BC_NOTMESSAGE;

      api_message.control &= ~BC_CONTROL_INTERRUPT;    // Turn off the interrupt
      api_message.control |= BC_CONTROL_INTERRUPT;     // Turn on the interrupt

      if( api_message.control & BC_CONTROL_BUFFERA)
         api_message.data[0][data_wrd] = start;
      if( api_message.control & BC_CONTROL_BUFFERB)
         api_message.data[1][data_wrd] = start;

      status = BusTools_BC_MessageWrite(cardnum,messno,&api_message);
      if(status)
         return status; 

      // Setup the FIFO structure for this board.
      memset(pIntFIFO[cardnum][messno], 0, sizeof(API_INT_FIFO));
      pIntFIFO[cardnum][messno]->function     = bc_auto_incr;
      pIntFIFO[cardnum][messno]->iPriority    = THREAD_PRIORITY_ABOVE_NORMAL;
      pIntFIFO[cardnum][messno]->dwMilliseconds = INFINITE;
      pIntFIFO[cardnum][messno]->iNotification  = 0;       // Dont care about startup or shutdown
      pIntFIFO[cardnum][messno]->FilterType     = EVENT_BC_MESSAGE;
      pIntFIFO[cardnum][messno]->nUser[0] = messno;  // Set the first user parameter to message number.
      pIntFIFO[cardnum][messno]->nUser[1] = data_wrd;
      pIntFIFO[cardnum][messno]->nUser[2] = incr;
      pIntFIFO[cardnum][messno]->nUser[3] = rate;
      pIntFIFO[cardnum][messno]->nUser[4] = 1; // use for counter
      pIntFIFO[cardnum][messno]->nUser[5] = max;
      pIntFIFO[cardnum][messno]->nUser[6] = start;
      
      for ( rt=0; rt < 32; rt++ )
         for (tr = 0; tr < 2; tr++ )
            for (sa = 0; sa < 32; sa++ )
               pIntFIFO[cardnum][messno]->FilterMask[rt][tr][sa] = 0x0;  // Enable all messages

      pIntFIFO[cardnum][messno]->FilterMask[api_message.mess_command1.rtaddr][api_message.mess_command1.tran_rec]
                          [api_message.mess_command1.subaddr] = 0xffffffff;

      // Call the register function to register and start the BC thread.
      status = BusTools_RegisterFunction(cardnum, pIntFIFO[cardnum][messno], 1);
      if ( status )
         return status;
   }
   else
   {
      // Call the register function to unregister and stop the BC thread.
      if(pIntFIFO[cardnum][messno] == NULL)
         return API_NULL_PTR;
      status = BusTools_RegisterFunction(cardnum, pIntFIFO[cardnum][messno], 0);
      free(pIntFIFO[cardnum][messno]);
      pIntFIFO[cardnum][messno]=NULL;
   }
   return status;      // Have the API function continue normally
}

/*===========================================================================*
 * User ENTRY POINT:        B C _ A U T O _ I N C R
 *===========================================================================*
 *
 * FUNCTION:    bc_auto_incr()
 *
 * DESCRIPTION: This function increments the pre-defined data word.
 *
 * It will return:
 *   API_SUCCESS - Thread continues execution
 *===========================================================================*/

BT_INT _stdcall bc_auto_incr(
   BT_UINT cardnum,
   struct api_int_fifo *sIntFIFO)
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_U16BIT   data[33];       // Data buffer we update
   BT_INT      tail;           // FIFO Tail index
   BT_UINT     messno;         // Message number to be updated
   BT_INT      status;

   /***********************************************************************
   *  Loop on all entries in the FIFO.  Get the tail pointer and extract
   *   the FIFO entry it points to.   When head == tail FIFO is empty
   ***********************************************************************/
   tail = sIntFIFO->tail_index;
   while ( tail != sIntFIFO->head_index )
   {
      // Extract the buffer ID from the FIFO
      messno = sIntFIFO->fifo[tail].bufferID;
      if(messno == (BT_UINT)sIntFIFO->nUser[0])
      {   
         //  and read the message data from the board.
         status = BusTools_BC_MessageReadData(cardnum, messno, data);
         if ( status )
            return status;

         // Update the data buffer
         if(sIntFIFO->nUser[4] == sIntFIFO->nUser[3])
         {
            data[sIntFIFO->nUser[1]] += sIntFIFO->nUser[2];
            data[sIntFIFO->nUser[1]] = (data[sIntFIFO->nUser[1]] % sIntFIFO->nUser[5]);
            if(data[sIntFIFO->nUser[1]] == 0)
               data[sIntFIFO->nUser[1]] = sIntFIFO->nUser[6];
            sIntFIFO->nUser[4] = 1;
         }
         else
            sIntFIFO->nUser[4]++;

         // Now write the data back to the message buffer:
         status = BusTools_BC_MessageUpdate(cardnum, messno, data);
         if ( status )
            return status;
      }
      // Now update and store the tail pointer.
      tail++;                         // Next entry
      tail &= sIntFIFO->mask_index;   // Wrap the index
      sIntFIFO->tail_index = tail;    // Save the index
   }
   return API_SUCCESS;
}
#endif //_LVRT_
