/*============================================================================*
 * FILE:                            R T . C
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
 *             This file contains the API routines which support RT mode
 *             operation of the BusTools board, including the RT Mode
 *             Initialization, Configuration and execution routines.
 *             These functions assume that the BusTools_API_Init-() function
 *             has been successfully called.
 *
 * USER ENTRY POINTS: 
 *      BusTools_RT_AbufRead        - Read RT Address Buffer (4 words).
 *      BusTools_RT_AbufWrite       - Write only the user's "enables" word.
 *      BusTools_RT_AutoIncrMessageData - auto increment a data word
 *      BusTools_RT_CbufbroadRead   - Read Control Buffer "legal_wordcount"
 *                                    and number of MBUF's originally defined
 *                                    for a broadcast Subaddress.
 *      BusTools_RT_CbufbroadWrite  - Update/create Control Buffer for the
 *                                    specified RT address/subaddress (subunit)
 *                                    and allocate specified number of MBUF's
 *                                    for a broadcast subaddress.
 *      BusTools_RT_CbufRead        - Read Control Buffer "legal_wordcount"
 *                                    and number of MBUF's originally defined.
 *      BusTools_RT_CbufWrite       - Update/create Control Buffer for the
 *                                    specified RT address/subaddress (subunit)
 *                                    and allocate specified number of MBUF's.
 *      BusTools_RT_Checksum1760    - Calculate a 1760 checksum on the RT message buffer
 *      BusTools_RT_GetRTAddr       - Returns  hardwired RT address
 *      BusTools_RT_GetRTAddr1760   - Returns either latched or current RT address.
 *      BusTools_RT_Init            - Init RT, setup memory map, stop RT HW.
 *      BusTools_RT_MessageGetaddr  - Returns the address of a RT message buffer.
 *      BusTools_RT_MessageGetid    - Convert Message Buffer Address to subunit
 *                                    identification (RT address, subaddress,
 *                                    and tran/rec bit), and buffer ID.
 *      BusTools_RT_MessageRead     - Read RT Message Buffer, identified by
 *                                    RT address, subaddress, T/R bit, and
 *                                    buffer ID.
 *      BusTools_RT_MessageWrite    - Write data to RT Message Buffer.
 *      BusTools_RT_MessageWriteStatusWord - Write 1553 Status Word data to
 *                                    the specified RT Message Buffer.
 *      BusTools_RT_MessageWriteDef - Write data to default RT Message Buffer.
 *      BusTools_RT_MonitorEnable   - Enables the RT to run in monitor only mode
 *      BusTools_RT_StartStop       - Starts/stops the RT.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *      RT_CbufbroadAddr - Returns the address of a specified Broadcast CBUF.
 *
 * INTERNAL ROUTINES:
 *      none
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ---------    ---------------------------------------------------------------
  10/19/1999   Removed mess_status2 from _RT_MessageRead.V3.22.ajh
  11/15/1999   Fixed problems in the allocation of cbufs and mbufs by the
               BusTools_RT_CbufWrite() function which were introduced when the
               PCI-1553 support was added.  Changed BusTools_RT_MessageWriteDef
               to copy the user data into the RT mbuf.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  02/18/2000   Fixed IP-1553 code that was broken in Version 4.00.V4.01.ajh
  03/21/2000   Discovered a problem with the RT default buffer.  Buffer is both
               transmit and receive, but the code assumes receive.  The RT status
               word is not correctly returned for transmit messages.  The RT
               message read functions do not handle mode codes correctly, then
               do not correctly locate the RT status word.V4.01.ajh
  06/01/2000   Changed _RT_MessageRead to return a NULL status word for
               a broadcast RT transmit command.V4.04.ajh
  06/19/2000   V2.05 (1/98) of the PC-1553 broke the RT status word.  Modify
               _RT_MessageRead() to get the status word from the address
               buffer on an RT receive.  Changed RT_Start to return
               API_SINGLE_FUNCTION_ERR if either the BC or the BM is running,
               and _HW_1Function[cardnum] is set.V4.05.ajh
  06/22/2000   Added support for the User Interface DLL.V4.06.ajh
  08/03/2000   Added RAMREG_RT_DISA and RAMREG_RT_DISB to support LPU V3.05 and
               WCS V3.07.  Added RAMREG_RT_ITF 8/9/00.v4.09.ajh
  09/08/2000   Changed BusTools_RT_CbufWrite.V4.13.ajh
  11/17/2000   Changed _MessageRead to use the "firmware in buffer" bits, and to
               return a non-null status word for a bcr RT-RT transmit msg.V4.22.ajh
  12/04/2000   Changed RT_Start to report single function warning after
               starting the RT.V4.26.ajh
  12/07/2000   Changed RT_AbufWrite and RT_AbufRead to support Dynamic Bus
               Control mode codes.V4.27.ajh
  03/27/2001   Clear the new RT disable registers in _RT_Init().V4.31.ajh
  03/26/2001   Added ability to create multiple RT message buffers for broadcast
               messages.V4.39.ajh
  10/18/2001   Add the RT monitor mode support.
  01/15/2002   Added high level interrupt queue message processing functions v4.46
  02/15/2002   Added modular API support. v4.48
  01/27/2003   Fix signed/unsigned mismatch
  02/26/2003   Added support for hardwire RT addressing
  10/01/2003   32-Bit API
 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"


#define RTMASK ~(0x01000000L)
/*******************************************************************************
*  Define the interrupt status bits which are NOT valid for the RT.  We will
*  return the RTB0 bit because the user might want to know that the firmware
*  was in the buffer when it was read...
*******************************************************************************/
BT_INT _stdcall rt_auto_incr(BT_UINT cardnum,
                             struct api_int_fifo *sIntFIFO);

static API_INT_FIFO  *pRTIntFIFO[MAX_BTA][32][32];     // Thread FIFO structure pointer

// Zero indicates that the RT/TR/SA combination (subunit) has NOT been
//  initialized (and it points to the default CBUF/MBUF pair that was
//   created for the associated RT during RT initialization)...
// Else contains the BYTE address of the first message buffer in the
//  MBUF chain created by calling RT_CbufWrite.
static BT_U32BIT
   RT_mbuf_addr[MAX_BTA][RT_ADDRESS_COUNT][RT_TRANREC_COUNT][RT_SUBADDR_COUNT];

// Number of message buffers allocated for subunit.  Zero if subunit has not
//  been initialized (e.g., subunit points to the default MBUF for the RT
//   through the default CBUF), or _RT_CbufWrite created a CBUF that points to
//   the default MBUF for this RT.
static unsigned short
   RT_mbuf_alloc[MAX_BTA][RT_ADDRESS_COUNT][RT_TRANREC_COUNT][RT_SUBADDR_COUNT];

static BT_U32BIT iqptr_rt_last[MAX_BTA];

// GCC does not grok ANSI continuation lines (V4.24):
#define btmem_rt_mbuf_def(rt) (BTMEM_RT_MBUF_DEF + btmem_rt_begin[cardnum] + rt*sizeof(RT_MBUF))
#define btmem_rt_cbuf_def(rt) (BTMEM_RT_CBUF_DEF + btmem_rt_begin[cardnum] + rt*sizeof(RT_CBUF))

/*******************************************************************************
*  Allocate space (word offsets) for:
*  Start  End   Size              Note that btmem_rt_begin[cardnum] supplies
*  Addr   Addr  Words                       the high-order address bits!
*  (hex) (hex)  (dec)  
*  -----------------------------------------------------------------------------
*  0F800 0FFFF  2048  RT Filter Buffer on 2K boundry (BTMEM_RT_FBUF)
*                     - addressed by RTaddr||TR||Subaddress + base.
*                     - RTaddr < 31 points to RT Control Buffers (nonbroad)
*                     - RTaddr 31 entries point to RT Control Buffers (broad)
*                     - point rt0-30 entries at the default ctrl bufs
*                     - point rt31 entries at the broadcast entries
*                     - if sa31 mode code enabled, point sa 31 entries at sa 0
*
*  0F780 0F7FF   128  RT Address Buffer (BTMEM_RT_ABUF)(32*4)
*                     - addressed by RTaddr*4 + base.
*                     - contains enables, status word, last cmd word, BIT wrd
*
*  0F180 0F77F  1536  Default RT Msg Buffers (BTMEM_RT_MBUF_DEF)(32*48)
*                     - one buffer for each of 32 possible RT's
*                     - last buffer is used for broadcast messages
*
*  0F120 0F17F    96  Default RT Ctrl Bufs (nonbroad)(BTMEM_RT_CBUF_DEF)(32*3)
*                     - contains 32 bits of legal word counts + RT msg buf ptr
*                     - last buffer (RT 31) unused if broadcast enabled
*
*  0E160 0F11F  4032  RT Ctrl Bufs (broadcast)(BTMEM_RT_CBUFBROAD)(64*63)
*                     - one buffer for each possible RT's, both Trans & Receive
*                     - contains RT msg buf ptr + 31*32 bits of legal word cnts
*                     - allocate only 62 buffers if sa 31 mode code enabled
*                     - allocate no buffers if broadcast not enabled
*                     - "reserved" words are not allocated
*
*  00000 0E15F 57695  Available RT Memory
*                     - BTMEM_RT_BOT_AVAIL to BTMEM_RT_TOP_AVAIL
*                     - Contains Cbuf's and Mbuf's for PC-1553 and IP-1553,
*                     -  only the Cbuf's for the PCI-1553.  For the PCI-1553
*                        the Mbuf's are created starting at the highest address
*                        and working their way down toward the BC & BM buffers.
*
*  Notes:
*  - The broadcast control buffers are not needed if broadcast is not enabled,
*        and they are not created.
*******************************************************************************/


/****************************************************************************
*
*   PROCEDURE NAME - RT_CbufbroadAddr()
*
*   FUNCTION
*       This routine is used to obtain the address of a specified
*       Broadcast RT Control Buffer.  This address depends on the
*       state of broadcase enable and SA 31 mode code definition,
*       as well as the Subaddress and Transmit/receive bit.  The
*       RT address is 31 of course.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_ILLEGAL_ADDR     -> illegal RT address specified
*
****************************************************************************/
BT_U32BIT RT_CbufbroadAddr(
   BT_UINT cardnum,           // (i) card number
   BT_UINT sa,                // (i) Subaddress
   BT_UINT tr)                // (i) Transmit=1, Receive=0
{
   BT_U32BIT  addr;     // Address of specified RT Broadcast Control Buffer

   // Internal routine; no error checking should be necessary.

   // If broadcast not enabled, return the address above broadcast area.
   //if ( rt_bcst_enabled[cardnum] == 0 )
   //   return BTMEM_RT_CBUF_DEF + btmem_rt_begin[cardnum];

   // The address of the RT Control Buffer depends on the state of the
   //  Subaddress 31 mode code definition.  There are 64 buffers if SA 31
   //  mode code is disabled, 62 if SA 31 mode code is enabled.
   if ( rt_sa31_mode_code[cardnum] )
   {
      if ( sa == 31 )
         sa = 0;     // Map SA 31 to SA 0 if SA 31 mode codes are enabled.
      addr = BTMEM_RT_CBUFBSA31 + btmem_rt_begin[cardnum] +
                                  sa*sizeof(RT_CBUFBROAD) +
                                  tr*sizeof(RT_CBUFBROAD)*(RT_SUBADDR_COUNT-1);
   }
   else
   {
      addr = BTMEM_RT_CBUFBROAD + btmem_rt_begin[cardnum] +
                                  sa*sizeof(RT_CBUFBROAD) +
                                  tr*sizeof(RT_CBUFBROAD)*RT_SUBADDR_COUNT;
   }
   return addr;
}


/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_AbufRead()
*
*   FUNCTION
*       This routine is used to read the information currently stored
*       in the address buffer for the specified RT address.  All four
*       words of the address buffer are returned.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_ILLEGAL_ADDR     -> illegal RT address specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_AbufRead(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   API_RT_ABUF * abuf)    // (o) Pointer to buffer to receive abuf data
{
   /************************************************
   *  Local variables
   ************************************************/

   BT_U32BIT     addr;
   RT_ABUF_ENTRY aentry;
   BT_U32BIT     RTDisableA;   // Channel A RT disable flags for all RTs
   BT_U32BIT     RTDisableB;   // Channel B RT disable flags for all RTs

   /************************************************
   *  Check initial conditions
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddress >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   /*******************************************************************
   *  Read abuf entry from hardware, based on RT address.
   *******************************************************************/

   addr = BTMEM_RT_ABUF + btmem_rt_begin[cardnum] +
          rtaddress * sizeof(RT_ABUF_ENTRY);
   vbtRead(cardnum, (LPSTR)&aentry, addr, sizeof(RT_ABUF_ENTRY));

   /*******************************************************************
   *  Copy data from local buffer to user's buffer
   *******************************************************************/
   abuf->status   = aentry.status;
   abuf->command  = aentry.last_command;
   abuf->bit_word = aentry.bit_word;

   /*******************************************************************
   *  If this is the new PCI et.al. firmware, read the RAM registers
   *   for the RT disables, and return the new abuf flags.
   *******************************************************************/

   // Setup the Dynamic Bus Acceptance mode code flags:
   abuf->inhibit_term_flag = aentry.enables;

   // Read the current values of the RAM Registers RT Disables
   vbtRead(cardnum, (LPSTR)&RTDisableA, RAMREG_RT_DISA*2, 4);
   vbtRead(cardnum, (LPSTR)&RTDisableB, RAMREG_RT_DISB*2, 4);

   // Now return the Channel A RAM Register Disables:
   abuf->enable_a = (BT_U16BIT)(RTDisableA & (1 << rtaddress) ? 0 : 1);

   // Now return the Channel B RAM Register Disables:
   abuf->enable_b = (BT_U16BIT)(RTDisableB & (1 << rtaddress) ? 0 : 1);

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_AbufWrite()
*
*   FUNCTION
*       This routine is used to write into the address buffer
*       for the specified RT address.  Only the "enables" word
*       is transferred (the other items are not set by the user).
*
*       The RT Enable bits control whether the specified RT is active on the
*       bus on channel A and/or channel B.  While the RT enable bits in the
*       hardware rt_address_buffer are active low (0=enable), the RT enable
*       bits in the API_RT_ABUF structure are active high (1=enable).
*
*       The Inhibit Terminal Flag: When this bit is set the terminal flag bit
*       in the RT's status word will not be set on the 1553 status word output
*       onto the bus, regardless of the bit value in the rt_status_word.
*       This bit is cleared by the host during setup and toggled by the
*       hardware upon reception of mode codes 00110 and 00111.  Reception of
*       the "Override Inhibit Mode Command" will not affect the status word of
*       the mode command itself, but will allow the terminal flag to be set in
*       the status word of subsequent messages.
*
*       The RT Status Word is the status word returned by the RT.  You program
*       it during initialization but it may be altered at any time by the
*       software. The hardware may also set some of the status word bits in
*       accordance with MIL-STD-1553B.  Your software must initialize this
*       word before running the RT by clearing all of the bits except the RT
*       address.  The API will fill in the correct RT address when this function
*       is called.  You can change the RT address by using the error injection
*       features of the API (see BusTools_EI_EbufWrite).
*
*       The RT Last Command Word: When the RT detects a command word on the
*       1553 bus, it is stored here for use in the "Transmit Last Command Word"
*       mode command.  This is an internal word used by the hardware but may
*       be initialized by the software prior to running the RT.
*
*       The BIT word is the 16-bit word used by the Transmit Built-In-Test (BIT)
*       mode command.  The host will normally clear this word during setup and
*       update it during reception of an initiate self-test mode command.  This
*       capability requires application code to read the self_test bit in the
*       message_status word or set the self_test bit in the rt_interrupt_enables
*       word and have a self-test routine to exercise the hardware.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_ILLEGAL_ADDR     -> illegal RT address specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_AbufWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   API_RT_ABUF * abuf)    // (i) Pointer to user's abuf structure
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT     addr;
   RT_ABUF_ENTRY aentry;
   BT_U32BIT     RTDisableA;   // Channel A RT disable flags for all RTs
   BT_U32BIT     RTDisableB;   // Channel B RT disable flags for all RTs

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddress >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   /*******************************************************************
   *  Copy data from user's buffer to local abuf buffer.
   *  The format of the abuf depends on the firmware version.
   *******************************************************************/
   addr = BTMEM_RT_ABUF + btmem_rt_begin[cardnum] +
          rtaddress * sizeof(RT_ABUF_ENTRY);

   vbtRead(cardnum, (LPSTR)&aentry.enables,addr,sizeof(aentry.enables));

   aentry.enables&=0x1;

   aentry.last_command = abuf->command;
   aentry.bit_word     = abuf->bit_word;

   if ( abuf->inhibit_term_flag & 1 )           // V4.27
      aentry.enables |= RT_ABUF_ITF;            // Old style arguement
   else
      aentry.enables = abuf->inhibit_term_flag; // New style arguement

   aentry.status = (BT_U16BIT)((rtaddress << 11) |
                                  (abuf->status & RT_STATUSMASK_RES));

   /*******************************************************************
   *  Setup the RAM registers for RT disables, and setup abuf flags.
   *******************************************************************/

   // Setup the Dynamic Bus Acceptance mode code flags:
   if ( abuf->inhibit_term_flag & RT_ABUF_DBC_RT_OFF )
      aentry.enables |= RT_ABUF_DBC_RT_OFF;
   if ( abuf->inhibit_term_flag & RT_ABUF_DBC_ENA )
   {
      aentry.enables |= RT_ABUF_DBC_ENA | RT_ABUF_DISB; // This enables DBC
      DBC_Enable[cardnum] = 1;
   }

   // Read the current values of the RAM Registers RT Disables
   vbtRead(cardnum, (LPSTR)&RTDisableA, RAMREG_RT_DISA*2, 4);
   vbtRead(cardnum, (LPSTR)&RTDisableB, RAMREG_RT_DISB*2, 4);

   flip(&RTDisableA);
   flip(&RTDisableB);

   // Now update the Channel A RAM Register Disables:
   if ( abuf->enable_a == 0 )
      RTDisableA |= 1 << rtaddress;
   else
      RTDisableA &= ~(1 << rtaddress);

   // Now update the Channel B RAM Register Disables:
   if ( abuf->enable_b == 0 )
      RTDisableB |= 1 << rtaddress;
   else
      RTDisableB &= ~(1 << rtaddress);

   flip(&RTDisableA);
   flip(&RTDisableB);

   // Now write the updated values to the RAM Registers
   vbtWrite(cardnum, (LPSTR)&RTDisableA, RAMREG_RT_DISA*2, 4);
   vbtWrite(cardnum, (LPSTR)&RTDisableB, RAMREG_RT_DISB*2, 4);

   /*******************************************************************
   *  Write abuf entry to hardware
   *******************************************************************/
   addr = BTMEM_RT_ABUF + btmem_rt_begin[cardnum] +
          rtaddress * sizeof(RT_ABUF_ENTRY);

   vbtWrite(cardnum, (LPSTR)&aentry, addr, sizeof(RT_ABUF_ENTRY));

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_MonitorEnable()
*
*   FUNCTION This function set or resets the rt_monitor enable bit in the 
*   abuf entry for the specified RT address.
*
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_ILLEGAL_ADDR     -> illegal RT address specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_MonitorEnable(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   BT_UINT mode)              // (i) rt monitor mode 0 = disable; 1 = enable
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT     addr;
   BT_U16BIT     enables;

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddress >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   /*******************************************************************
   *  Read the abuf enables entry from the hardware and set/reset the 
   *  monitor bit
   *******************************************************************/
   addr = BTMEM_RT_ABUF + btmem_rt_begin[cardnum] +
          rtaddress * sizeof(RT_ABUF_ENTRY);

   vbtRead(cardnum, (LPSTR)&enables,addr,sizeof(enables));

   if (mode == RT_MONITOR_MODE)
	   enables |= RT_MONITOR_MODE;
   else
	   enables &=~RT_MONITOR_MODE; //reset the RT_MONITOR_ENABLE

   vbtWrite(cardnum, (LPSTR)&enables, addr, sizeof(enables));

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_CbufbroadRead() (BROADCAST ONLY)
*
*   FUNCTION
*       This routine reads the Control Buffer for the specified
*       RT address/subaddress.  Only the "legal_wordcount"
*       information is returned from the hardware.  This routine
*       also returns the number of MBUF's originally defined
*       for this subunit (this value will be '0' for any subunit
*       which has not yet been defined).
*
*       If either subaddress 0 or 31 is specified and sa 31 mode code
*       is enabled, simply read the sa 0 control buffer.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_ILLEGAL_ADDR     -> illegal RT address specified
*       API_RT_ILLEGAL_SUBADDR  -> illegal RT subaddress specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_CbufbroadRead(
   BT_UINT cardnum,           // (i) card number
   BT_UINT subaddr,           // (i) RT Subaddress
   BT_UINT tr,                // (i) Transmit=1, Receive=0
   API_RT_CBUFBROAD * apicbuf)  // (o) Pointer to user's RT control buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_UINT      i;
   BT_U32BIT    addr;
   BT_U16BIT    waddr;
   RT_CBUFBROAD cbufbroad;
   static int   AlreadyReported = 0;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_RT_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_RT_ILLEGAL_TRANREC;

   if ( rt_sa31_mode_code[cardnum] )
      if ( subaddr == 31 )
         subaddr = 0;     // Map SA 31 to SA 0 if SA 31 mode codes are enabled.

   if ( rt_bcst_enabled[cardnum] == 0 ) {
      if ( AlreadyReported++ )
         return API_SUCCESS;
      else
         return API_RT_BROADCAST_DISABLE;   // RT 31 broadcast is disabled.
   }

   /*******************************************************************
   *  Compute the location in fbuf on the card which points to the
   *  desired cbuf (specified RT/TR/SA), then read the cbuf from the
   *  hardware.  Note that broadcast is always associated with RT 31.
   *******************************************************************/
   addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
       (31 << 7) + (tr << 6) + (subaddr << 1); // Byte offset of entry.
   vbtRead(cardnum, (LPSTR)&waddr, addr, sizeof(BT_U16BIT));

   /*******************************************************************
   *  Get broadcast control buffer from hardware
   *******************************************************************/
   addr = (((BT_U32BIT)waddr) << 1) + btmem_rt_begin[cardnum];
   vbtRead(cardnum, (LPSTR)&cbufbroad, addr, sizeof(cbufbroad));

   /*******************************************************************
   *  Copy from hardware broadcast CBUF buffer to caller's buffer
   *******************************************************************/
   for (i = 0; i < 31; i++) {
      flip (&cbufbroad.legal_wordcount[i]);
      apicbuf->legal_wordcount[i] = cbufbroad.legal_wordcount[i];
   }
   // Return the number of mbuf's allocated in the caller's buffer.V4.39.ajh
   apicbuf->mbuf_count = RT_mbuf_alloc[cardnum][31][tr][subaddr];
   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_CbufbroadWrite() (BROADCAST ONLY)
*
*   FUNCTION
*       This routine updates the Broadcast Control Buffer information for
*       the specified RT subaddress/transmit/receive.  The Broadcast Control
*       Buffer contains a 32-bit word for each of RT 0-30.  Each word contains
*       the valid word count mask for the associated RT.
*
*       If either subaddress 0 or 31 is specified and sa 31 mode code
*       is enabled, simply write the sa 0 control buffer.
*
*       If the mbuf_count parameter is non-zero, and we have not already
*       allocated any (non-default) message buffers for this subaddress/TR,
*       allocate the specified number of mbuf's and point the cbufbroad to them.
*
*   RETURNS
*       API_SUCCESS              -> success
*       API_BUSTOOLS_BADCARDNUM  -> invalid card number specified
*       API_BUSTOOLS_NOTINITED   -> BusTools API not initialized
*       API_RT_NOTINITED         -> RT_Init not yet called
*       API_RT_ILLEGAL_SUBADDR   -> illegal RT subaddress specified
*       API_RT_ILLEGAL_TRANREC   -> illegal transmit/receive flag
*       API_RT_BROADCAST_DISABLE -> broadcast is disabled
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_CbufbroadWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT subaddr,           // (i) Subaddress to write
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   API_RT_CBUFBROAD * apicbuf) // (i) Buffer containing Control Buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_UINT      i;
   BT_U32BIT    addr;             // General purpose structure address, temp
   BT_U32BIT    cbuf_addr;        // Address of the current broadcast cbuf
   BT_U16BIT    waddr;
   RT_CBUFBROAD cbufbroad;
   BT_U32BIT    addr_mbuf;        // BYTE address of current MBUF
   BT_U32BIT    addr_first_mbuf;  // Byte address of first MBUF in the chain
   BT_UINT      mbuf_count;       // Number of message buffers to allocate.
   BT_U32BIT    bytes_needed;     // Number of bytes needed for CBUF + n*MBUFS.
   BT_U32BIT    bytes_available;  // Bytes available for allocation.
   RT_MBUF     mbuf;           // rt message buffer, IP/ISA/PCI-1553

   static int   AlreadyReported = 0;

   /************************************************
   *  Check initial conditions
   ************************************************/
   mbuf_count = (BT_UINT)apicbuf->mbuf_count;
   //if ( subaddr == 1 ) mbuf_count = 2;
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_RT_ILLEGAL_SUBADDR;
   if (tr >= RT_TRANREC_COUNT)
      return API_RT_ILLEGAL_TRANREC;

   if ( rt_bcst_enabled[cardnum] == 0 )
   {
      if ( AlreadyReported++ )
         return API_SUCCESS;
      else
         return API_RT_BROADCAST_DISABLE;   // RT 31 broadcast is disabled.
   }

   if(channel_is_shared[cardnum])
   {
      vbtReadRAM(cardnum,(char *)&btmem_pci1553_rt_mbuf[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_RT_MBUF,sizeof(BT_U32BIT));
   }

   if ( rt_sa31_mode_code[cardnum] )
	   if ( subaddr == 31 ) {
         subaddr = 0;     // Map SA 31 to SA 0 if SA 31 mode codes are enabled.
   }

   /*******************************************************************
   *  Compute the location in fbuf on the card which points to the
   *  desired cbuf (RT 31 specified TR/SA), read the fbuf pointer.
   *  Convert the pointer to a byte address then read the cbuf from the
   *  hardware.  Note that broadcast is always associated with RT 31.
   *******************************************************************/
   addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
       (31 << 7) + (tr << 6) + (subaddr << 1 ); // Byte offset of entry.
   vbtRead(cardnum, (LPSTR)&waddr, addr, sizeof(BT_U16BIT));

   // Now read the cbuf from the hardware at the address specified by fbuf.
   // This is necessary so we can get the current mbuf pointer.
   cbuf_addr = ((BT_U32BIT)waddr << 1) + btmem_rt_begin[cardnum];
   vbtRead(cardnum, (LPSTR)&cbufbroad, cbuf_addr, sizeof(cbufbroad));

   /*******************************************************************
   *  Set the user-specified values into the Broadcast CBUF.
   *******************************************************************/

   for ( i = 0; i < 31; i++ ) {
      cbufbroad.legal_wordcount[i] = flips(apicbuf->legal_wordcount[i]);
   }

   /*******************************************************************
   *  Write the updated broadcast control buffer back to the hardware.
   *******************************************************************/
   vbtWrite(cardnum, (LPSTR)&cbufbroad, cbuf_addr, sizeof(cbufbroad));

   /*******************************************************************
   *  If we do not need to allocate any message buffers we are done.
   *******************************************************************/
   if ( mbuf_count == 0 )
      return API_SUCCESS;          // Don't allocate any buffers
   if ( mbuf_count == RT_mbuf_alloc[cardnum][31][tr][subaddr] )
      return API_SUCCESS;          // Buffers already allocated
   if ( RT_mbuf_alloc[cardnum][31][tr][subaddr] != 0 )
      return API_RT_CBUF_EXISTS;   // Caller cannot change the number of MBUF's

   /***********************************************************************
   *  Caller wants to allocate some non-default message buffers.
   *   Allocate them and point this cbuf at the first buffer allocated.
   ************************************************************************
   *  MBUF's are allocated from btmem_pci1553_rt_mbuf
   *    (down to btmem_pci1553_next).
   ***********************************************************************/
   // Allocate the specified number of MBUFs
   addr = btmem_tail2[cardnum]; // Memory above this address is allocated.
   // Memory allocation for the ISA-, PCI- and PMC-1553.
   // MBUF's are allocated above the 64 KW boundry.
   bytes_needed = (BT_U32BIT)mbuf_count * sizeof(RT_MBUF);
   // Compute the memory available above 64 KW:
   btmem_pci1553_rt_mbuf[cardnum] &= ~0x000F;  // Round down to a 16-byte boundry.
   bytes_available = btmem_pci1553_rt_mbuf[cardnum]-btmem_pci1553_next[cardnum];
   if ( bytes_needed > bytes_available )
      return API_RT_MEMORY_OFLOW;

   btmem_pci1553_rt_mbuf[cardnum] -= bytes_needed;
   addr_first_mbuf = btmem_pci1553_rt_mbuf[cardnum];

   /***********************************************************************
   *  Link the new list of MBUF's together.
   ***********************************************************************/
   RT_mbuf_alloc[cardnum][31][tr][subaddr] = (BT_U16BIT)mbuf_count;

//========================================
   if ((subaddr == 0) && ( rt_sa31_mode_code[cardnum] ))
	   RT_mbuf_alloc[cardnum][31][tr][31] = (BT_U16BIT)mbuf_count;
//========================================

   // Initialize the default MBUF values.
   memset((void*)&mbuf, 0, sizeof(mbuf));
   mbuf.api.mess_id.rtaddr   = (BT_U16BIT)31;      // rt address field
   mbuf.api.mess_id.tran_rec = (BT_U16BIT)tr;      // transmit/receive bit
   mbuf.api.mess_id.subaddr  = (BT_U16BIT)subaddr; // subaddress field
   mbuf.api.mess_verify      = RT_VERIFICATION;    // Verification word
   mbuf.hw.ei_ptr = (BT_U16BIT)(BTMEM_EI >> 1);    // First EI buffer is zero.
   mbuf.hw.enable = flips(BT1553_INT_BROADCAST | BT1553_INT_END_OF_MESS);

   mbuf.hw.mess_command    = mbuf.api.mess_id; // 45-bit TT boards

   // Link the new MBUF's into a chain of length "mbuf_count".
   addr_mbuf = addr_first_mbuf;   // Start with the first MBUF in the chain.
   for (i = 0; i < mbuf_count; i++)
   {
      mbuf.api.mess_bufnum = (BT_U16BIT)i;
      // Link messages together, depending on position in list.
      addr = addr_mbuf + sizeof(RT_MBUF);   // Loop to next message?
      if ( i == (mbuf_count - 1) )
         addr = addr_first_mbuf;            // Loop back to first msg
      mbuf.hw.nxt_msg_ptr = (BT_U16BIT)(addr >> hw_addr_shift[cardnum]);
      vbtWrite(cardnum, (LPSTR)&mbuf, addr_mbuf, sizeof(mbuf));
      addr_mbuf += sizeof(RT_MBUF);
   }

   /***********************************************************************
   *  Update and write the broadcast control buffer back to the hardware.
   ***********************************************************************/
   RT_mbuf_addr[cardnum][31][tr][subaddr] = addr_first_mbuf;

//========================================
   if ((subaddr == 0) && ( rt_sa31_mode_code[cardnum] ))
	   RT_mbuf_addr[cardnum][31][tr][31] = addr_first_mbuf;
//========================================

   cbufbroad.message_pointer = (BT_U16BIT)(addr_first_mbuf >> hw_addr_shift[cardnum]);
   vbtWrite(cardnum, (LPSTR)&cbufbroad, cbuf_addr, sizeof(cbufbroad));

   if(channel_is_shared[cardnum])
   {
      vbtWriteRAM(cardnum,(char *)&btmem_pci1553_rt_mbuf[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_RT_MBUF,sizeof(BT_U32BIT));
   }

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_CbufRead() (NON-BROADCAST)
*
*   FUNCTION
*       This routine reads the Control Buffer for the specified
*       RT address/subaddress.  The "legal_wordcount" information
*       is returned from the hardware.  This routine
*       also returns the number of MBUF's originally defined
*       for this subunit (this value will be '0' for any subunit
*       which has not yet been defined).
*
*       If either subaddress 0 or 31 is specified and sa 31 mode code
*       is enabled, simply read the sa 0 control buffer.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_ILLEGAL_ADDR     -> illegal RT address specified
*       API_RT_ILLEGAL_SUBADDR  -> illegal RT subaddress specified
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_RT_CbufRead(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddr,            // (i) Remote Terminal address to be setup
   BT_UINT subaddr,           // (i) Subaddress to read
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   BT_UINT * mbuf_count,  // (o) Number of Message Buffers created
   API_RT_CBUF * apicbuf) // (o) User's buffer to receive Control Buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_U16BIT  waddr;
   BT_U32BIT  addr;
   RT_CBUF    cbuf;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_RT_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_RT_ILLEGAL_TRANREC;

   if ( rt_bcst_enabled[cardnum] != 0 )
      if (rtaddr == 31)
         return API_RT_CBUF_BROAD;

   if ( rt_sa31_mode_code[cardnum] )
      if ( subaddr == 31 )
         subaddr = 0;     // Map SA 31 to SA 0 if SA 31 mode codes are enabled.

   /*******************************************************************
   *  Compute the location in fbuf on the card which points to the
   *  desired cbuf, then read the cbuf from the hardware.
   *******************************************************************/
   addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
       (rtaddr << 7) + (tr << 6) + (subaddr << 1 ); // Byte offset of entry.
   vbtRead(cardnum, (LPSTR)&waddr, addr, sizeof(BT_U16BIT)); // Read Fbuf

   // Now read the cbuf from the hardware at the address specified by fbuf.
   addr = ((BT_U32BIT)waddr << 1) + btmem_rt_begin[cardnum];
   vbtRead(cardnum, (LPSTR)&cbuf, addr, sizeof(cbuf));       // Read Cbuf

   /*******************************************************************
   *  Copy legal word count and buffer count from the local buffer
   *  to the caller's buffer.
   *******************************************************************/
   apicbuf->legal_wordcount = ((BT_U32BIT)cbuf.legal_wordcount1 << 16)
                                        | cbuf.legal_wordcount0;
   *mbuf_count = RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr];

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_CbufWrite() (NON-BROADCAST)
*
*   FUNCTION
*       This routine initializes or updates the RT Control Buffer information
*       for the specified RT address/subaddress (a subunit).  This function
*       operates differently depending on how it was previously called for
*       the specified subunit and the current arguments.
*
*    1. If either sa 0 or sa 31 is specified, and sa 31 mode code
*       is enabled, than process the command as if sa 0 was specified.
*       When the command has been completed, point the sa 31 entry in
*       the RT Filter Buffer to the sa 0 cbuf.
*       -- This takes care of the MIL-STD-1553B option of responding
*          to a mode code on either SA 0 or SA 31.
*
*    2. If this sub unit points at the default MBUF for this RT, AND the
*       specified number of MBUF's is zero, AND the legal word count mask
*       is all "F"s, just retain the default CBUF and MBUF for this RT.
*       -- This is a no-operation case that enables all word counts, and
*          points the subunit to the default buffer for this RT.
*
*    3. Else if this CBUF does NOT point to the default MBUF, MODIFY the
*       current CBUF with the specified illegal word count mask.  Then, if
*       the mbuf_count parameter is less than the number of MBUFs allocated,
*       point the CBUF to the MBUF specified by the mbuf_count parameter.
*       If the mbuf_count parameter is equal to the number of MBUF's allocated,
*       then do not modify the current MBUF pointer in the CBUF.  If the
*       specified mbuf_count parameter is greater than the number of MBUF's
*       allocated return an error.
*       -- This allows you to change the llegal word count mask, and optionally
*          reset the RT data buffer list pointer to a specified buffer.
*
*    4. Lastly, if neither of the above is true, allocate a new CBUF and
*       the specified number of MBUF's.  Point the CBUF at the first new
*       MBUF in the list and update the FBUF address to point to the new
*       CBUF.  Link the MBUF's together, either as a circular list or as
*       a one-pass list (depending on the sign of the mbuf_count parameter). 
*       If the specified number of MBUF's is zero point the new CBUF with
*       the new legal word count mask at the default RT MBUF.
*       -- This allows you to setup a list of message buffers, and
*          specify the legal word count mask.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_ILLEGAL_ADDR     -> illegal RT address specified
*       API_RT_ILLEGAL_SUBADDR  -> illegal RT subaddress specified
*       API_RT_CBUF_BROAD       -> RT 31 is broadcast only
*       API_RT_CBUF_EXISTS      -> cannot change the number of MBUF's
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_CbufWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddr,            // (i) Remote Terminal address to be setup
   BT_UINT subaddr,           // (i) Subaddress to be setup
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   BT_INT  mbuf_count,        // (i) Num buffers, If negative, one pass through only.
                              //     Current buffer number if second call.
   API_RT_CBUF * apicbuf)     // (i) User-supplied CBUF; legal word count mask
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_INT     loop_to_top;      // Set if continuous loop, zero if one pass
   BT_INT     i;                // Loop index
   BT_U16BIT  waddr;            // WORD address
   BT_U32BIT  addr;             // General Purpose address
   BT_U32BIT  addr_mbuf;        // BYTE address of current MBUF
   BT_U32BIT  addr_cbuf;        // Byte address of CBUF
   BT_U32BIT  addr_first_mbuf;  // Byte address of first MBUF in the chain
   BT_U32BIT  bytes_needed;     // Number of bytes needed for CBUF + n*MBUFS.
   BT_U32BIT  bytes_available;  // Bytes available for allocation.

   RT_CBUF    cbuf;             // CBUF for writing to HW
   RT_MBUF    mbuf;             // rt message buffer, IP/ISA/PCI-1553


   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_RT_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_RT_ILLEGAL_TRANREC;

   if ( rt_bcst_enabled[cardnum] )
      if ( rtaddr == 31 )
   {
         return API_RT_CBUF_BROAD;
   }

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrRT_CbufWrite[cardnum] )
   {
      i = (*pUsrRT_CbufWrite[cardnum])(cardnum, &rtaddr, &subaddr, &tr,
                                       &mbuf_count, apicbuf);
      if ( i == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( i == API_NEVER_CALL_AGAIN )
         pUsrRT_CbufWrite[cardnum] = NULL;
      else if ( i != API_CONTINUE )
         return i;
   }
#endif

   if(channel_is_shared[cardnum])
   {
      vbtReadRAM(cardnum,(char *)&btmem_pci1553_rt_mbuf[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_RT_MBUF,sizeof(BT_U32BIT));
      vbtReadRAM(cardnum,(char *)&btmem_tail1[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_TAIL1,sizeof(BT_U32BIT));
   }

   /*******************************************************************
   *  1. We map subaddress 31 and subaddress 0 to the same buffer list,
   *     if SA 31 has been enabled as a mode code subaddress.
   *******************************************************************/
   if ( rt_sa31_mode_code[cardnum] )
   {
      if ( subaddr == 31 )
      {
         subaddr = 0;      // Map SA 31 to SA 0 if SA 31 mode codes are enabled.
         if ( RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] )
            return API_SUCCESS;    // SA 0 already initialized! V4.01.ajh
      }
   }
   
   /***********************************************************************
   *  If mbuf_count is negative, than setup the data buffers for a single
   *   pass.  If positive, last buffer loops back to first buffer.
   ***********************************************************************/
   if ( mbuf_count < 0 )
   {
      loop_to_top = 0;
      mbuf_count  = -mbuf_count;
   }
   else
      loop_to_top = 1;

   /***********************************************************************
   * If this sub unit has not been allocated any MBUF's before, it must be
   *  using the default CBUF and MBUF, which contains a legal word count
   *  mask of 0xFFFFFFFF.
   * 2. So if this sub unit has not been allocated any MBUF's before, AND
   *    the specified number of MBUF's is zero, AND the legal word count
   *    mask is all "F"s, keep the default CBUF, pointing at
   *    the default MBUF for this RT.
   ***********************************************************************/
   if ( (RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] == 0) ||
        (RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] == btmem_rt_mbuf_def(rtaddr)) )
   {
      if ( (mbuf_count == 0) && (apicbuf->legal_wordcount == 0xFFFFFFFF) )
         return API_SUCCESS;
   }

   /*******************************************************************
   *  3. If a previous call allocated some MBUFs, we want to update the
   *     CBUF with the new legal word count mask, AND we want to point
   *     the CBUF at the MBUF whose number is specified by the
   *     mbuf_count parameter.
   *******************************************************************/
   if ( (RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] != 0) &&
        (RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] != btmem_rt_mbuf_def(rtaddr)) )
   {
      // We cannot point at a MBUF that does not exist!
      if ( (unsigned)mbuf_count > RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
         return API_RT_CBUF_EXISTS;    // Cannot increase MBUFs in subunit! V4.13.ajh

      // Read the entry in the FBUF to get the pointer to the old CBUF.
      addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
             (rtaddr << 7) + (tr << 6) + (subaddr << 1 ); // Byte offset of entry.
      vbtRead(cardnum, (LPSTR)&waddr, addr, sizeof(BT_U16BIT));

      // Compute the old CBUF address then read the CBUF so we can update it.
      addr_cbuf = ((BT_U32BIT)waddr) << 1;
      vbtRead(cardnum, (LPSTR)&cbuf, addr_cbuf, sizeof(cbuf));

      // Update the old CBUF with the new legal word count mask
      cbuf.legal_wordcount0 = (BT_U16BIT)apicbuf->legal_wordcount;
      cbuf.legal_wordcount1 = (BT_U16BIT)(apicbuf->legal_wordcount >> 16);

      // Point the old CBUF at the specified MBUF for this RT.
      if ( (unsigned)mbuf_count < RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
      {
         addr = RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] + mbuf_count*sizeof(RT_MBUF);
         cbuf.message_pointer = (BT_U16BIT)(addr >> hw_addr_shift[cardnum]);
      }
      // Now write the updated CBUF and return success
      vbtWrite(cardnum, (LPSTR)&cbuf, addr_cbuf, sizeof(cbuf));
      return API_SUCCESS;
   }
   
   /***********************************************************************
   * 4. Allocate the CBUF and the specified number of MBUF's (might be zero)
   *    and update the FBUF address to point to the new CBUF.
   *  If the specified number of MBUF's is zero point the new CBUF with the
   *    new legal word count mask at the default RT MBUF, otherwise the
   *    new CBUF points to the beginning of the new list of MBUFs. V4.01.ajh
   ************************************************************************
   *  Note that CBUF's are allocated from btmem_tail2 down for all cards.
   *  MBUF's are allocated just above the associated CBUF for the
   *    ISA, and from btmem_pci1553_rt_mbuf
   *    (down to btmem_pci1553_next) for the PCI-1553 et.al.
   *  Added test for mbuf_count and legal_wordcount in Version 3.30.ajh
   ***********************************************************************/
   // Allocate the CBUF and the specified number of MBUFs (if any were specified)
   addr = btmem_tail2[cardnum]; // Memory above this address is allocated.

   // Memory allocation for the ISA-, PCI- and PMC-1553.  CBUF must be
   //  allocated below 64 KW, while the MBUF's are allocated above
   //  the 64 KW boundry.
   // First allocate the CBUF from memory in the first 64 KW.
   bytes_available = addr-btmem_tail1[cardnum]; // Avail seg 1.
   if ( sizeof(RT_CBUF) > bytes_available )
      return API_RT_MEMORY_OFLOW;

   btmem_tail2[cardnum] -= sizeof(RT_CBUF);    // Allocation is top down.
   addr_cbuf = btmem_tail2[cardnum];           // Memory above here allocated.

   // Next allocate the MBUF's from memory above the first 64 KW.
   bytes_needed = (BT_U32BIT)mbuf_count * sizeof(RT_MBUF);
   // Compute the memory available above 64 KW:
   btmem_pci1553_rt_mbuf[cardnum] &= ~0x000F;  // Round down to a 16-byte boundry.
   bytes_available = btmem_pci1553_rt_mbuf[cardnum]-btmem_pci1553_next[cardnum];
   if ( bytes_needed > bytes_available )
      return API_RT_MEMORY_OFLOW;

   btmem_pci1553_rt_mbuf[cardnum] -= bytes_needed;
   addr_first_mbuf = btmem_pci1553_rt_mbuf[cardnum];

   /***********************************************************************
   *  If we generated a list of new MBUF's link them together.
   ***********************************************************************/
   RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] = (BT_U16BIT)mbuf_count;
   if ( mbuf_count )
   {
      // Initialize the default MBUF values.
      memset((void*)&mbuf, 0, sizeof(mbuf));
      mbuf.api.mess_id.rtaddr   = (BT_U16BIT)rtaddr;  // rt address field
      mbuf.api.mess_id.tran_rec = (BT_U16BIT)tr;      // transmit/receive bit
      mbuf.api.mess_id.subaddr  = (BT_U16BIT)subaddr; // subaddress field
      mbuf.api.mess_verify      = RT_VERIFICATION;    // Verification word
      mbuf.hw.ei_ptr = (BT_U16BIT)(BTMEM_EI >> 1);    // First EI buffer is zero.

      mbuf.hw.mess_command = mbuf.api.mess_id; // 45-bit TT boards

      // Link the new MBUF's into a chain of length "mbuf_count".
      addr_mbuf = addr_first_mbuf;   // Start with the first MBUF in the chain.
      for (i = 0; i < mbuf_count; i++)
      {
         mbuf.api.mess_bufnum = (BT_U16BIT)i;
         // Link messages together, depending on position in list.
            addr = addr_mbuf + sizeof(RT_MBUF);   // Loop to next message?
         if ( i == (mbuf_count - 1) )
         {
            if ( loop_to_top )
               addr = addr_first_mbuf;         // Loop back to first msg
            else
               addr = addr_mbuf;               // Loop back to last (this) msg
         }
         mbuf.hw.nxt_msg_ptr = (BT_U16BIT)(addr >> hw_addr_shift[cardnum]);
         vbtWrite(cardnum, (LPSTR)&mbuf, addr_mbuf, sizeof(mbuf));
         addr_mbuf += sizeof(RT_MBUF);
      }
   }
   else
   {
      /*****************************************************************
      * If no MBUFs were created, point at the default MBUF that was
      *   created by RT_Init for this RT.
      *****************************************************************/
      addr_first_mbuf = btmem_rt_mbuf_def(rtaddr);
   }

   /********************************************************************
   * Point this new CBUF at the default MBUF for this RT, or point the
   *  CBUF at the new MBUFs we just created.
   * Then write the new CBUF to the hardware.
   ********************************************************************/
   RT_mbuf_addr[cardnum][rtaddr][tr][subaddr]  = addr_first_mbuf;
   cbuf.legal_wordcount0 = (BT_U16BIT)apicbuf->legal_wordcount;
   cbuf.legal_wordcount1 = (BT_U16BIT)(apicbuf->legal_wordcount >> 16);
   cbuf.message_pointer = (BT_U16BIT)(addr_first_mbuf >> hw_addr_shift[cardnum]);
   vbtWrite(cardnum, (LPSTR)&cbuf, addr_cbuf, sizeof(cbuf));

   /********************************************************************
   *  Update the entry in the FBUF to point to the new CBUF.
   ********************************************************************/
   addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
           (rtaddr << 7) + (tr << 6) + (subaddr << 1 ); // Byte offset of entry.
   waddr = (BT_U16BIT)(addr_cbuf >> 1);
   vbtWrite(cardnum, (LPSTR)&waddr, addr, sizeof(BT_U16BIT));

   /********************************************************************
   *  If this is sa 0 and sa 31 mode code is enabled, point the SA 31
   *   FBUF entry at this CBUF/MBUF string.
   ********************************************************************/
   if ( (subaddr == 0) && (rt_sa31_mode_code[cardnum]) )
   {
      // Update the SA 31 entry in the fbuf to point to the newly created cbuf.
      addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
             (rtaddr << 7) + (tr << 6) + (31 << 1 ); // Byte offset of entry.
      waddr = (BT_U16BIT)(addr_cbuf >> 1);
      vbtWrite(cardnum, (LPSTR)&waddr, addr, sizeof(BT_U16BIT));
      RT_mbuf_addr[cardnum][rtaddr][tr][31] =
                                          RT_mbuf_addr[cardnum][rtaddr][tr][0];
      RT_mbuf_alloc[cardnum][rtaddr][tr][31] =
                                          RT_mbuf_alloc[cardnum][rtaddr][tr][0];
   }

   if(channel_is_shared[cardnum])
   {
      vbtWriteRAM(cardnum,(char *)&btmem_pci1553_rt_mbuf[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_RT_MBUF,sizeof(BT_U32BIT));
   }
 
   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_Init()
*
*   FUNCTION
*       This function performs full initialization of RT functionality
*       on the board.  The following actions are performed:
*           - check for any illegal conditions (exit with error if
*             these conditions aren't true):
*               - bustools must be initialized
*               - no RT's can be running
*           - setup memory in hardware:
*               - available memory starts at beg of segment (address = 0/10000)
*               - allocate RT filter buffer (2048 words)
*               - allocate RT address buffers (32*4 words)
*               - allocate default RT message buffers (32*48 words)
*               - allocate default RT control buffers (64/62*63 words)(broadcast)
*               - allocate default RT control buffers (32*3 words)(non-broadcast)
*           - setup PC data structures
*           - show no RT's running

*       This gives each possible RT a default CBUF that points to an MBUF.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_RUNNING          -> RT currently operating
*       API_RT_MEMORY_OFLOW     -> RT memory filled
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_Init(
    BT_UINT cardnum,          // (i) card number
    BT_UINT testflag)         // (i) flag; ignored!
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT      rtaddr,tr,subaddr; // Indexes of the current RT_FBUF entry
   BT_U16BIT    cbuf_ptr;          // Pointer to cbuf in RT_FBUF entry
   BT_U32BIT    addr;              // Address of RT_FBUF entry

   BT_U32BIT    cbuf_addr;         // Byte address of rt control buffer
   BT_U32BIT    cbufbroad_addr;    // Byte address of rt control buffer (broad)
   BT_U32BIT    mbuf_addr;         // Byte address of rt message buffer

   RT_ABUF      abuf;              // rt address buffer (128 words)
   RT_CBUF      cbuf;              // rt control buffer (3 words)
   RT_CBUFBROAD cbufbroad;         // broadcast rt control buffer (63 words)
   RT_MBUF     mbuf;            // rt message buffer, IP/ISA/PCI-1553

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(channel_is_shared[cardnum])
   {
      vbtReadRAM8(cardnum,(LPSTR)&rt_inited[cardnum],BTMEM_CH_SHARE + SHARE_RT_INITED,1);
      if(rt_inited[cardnum])
         return API_RT_INITED;
   }

   /*******************************************************************
   *  clear the RAM registers for the RT disables.V4.31.ajh
   *******************************************************************/

   // Clear the RAM registers that contain the RT disables
   addr = 0xFFFFFFFFL;
   vbtWrite(cardnum, (LPSTR)&addr, RAMREG_RT_DISA*2, 4);
   vbtWrite(cardnum, (LPSTR)&addr, RAMREG_RT_DISB*2, 4);

   /*******************************************************************
   *  Fill in allocated space (in reverse order)
   *       default control buffer (broadcast) (if broadcast enabled)
   *       default control buffer (non-broadcast)
   *       default message buffers
   *       address buffer
   *       filter buffer
   *******************************************************************/
   if ( rt_bcst_enabled[cardnum] != 0 )
   {
      // Broadcast is enabled, create the RT Broadcast Control Buffers.
      // If SA31 is a mode code, point fbuf SA 0 and fbuf SA 31 to the same
      //  physical RT Broadcast Control Buffer.  We will only create 31*2
      //  broadcast cbufs instead of 32*2 cbufs.
      btmem_tail2[cardnum] =
      btmem_rt_top_avail[cardnum] = RT_CbufbroadAddr(cardnum, 0, 0);
      if ( btmem_tail1[cardnum] >= btmem_tail2[cardnum] )
         return API_RT_MEMORY_OFLOW;

      for ( rtaddr = 0; rtaddr < 31; rtaddr++ )
         cbufbroad.legal_wordcount[rtaddr] = 0xFFFFFFFFL; // Enable all word counts

      // Create the RT Control Buffers for Broadcast consisting of
      //  64 or 62 buffers containing 63 words per buffer.
      // Note that the RT_CbufbroadAddr() function maps SA 31 to SA 0 if
      //  SA 31 mode codes are enabled.
      for (subaddr = 0; subaddr < RT_SUBADDR_COUNT; subaddr++)
         for (tr = 0; tr < RT_TRANREC_COUNT; tr++)
         {
            cbufbroad_addr = RT_CbufbroadAddr(cardnum, subaddr, tr);
            cbufbroad.message_pointer = (BT_U16BIT)(btmem_rt_mbuf_def(31) >> hw_addr_shift[cardnum]);
            vbtWrite(cardnum, (LPSTR)&cbufbroad, cbufbroad_addr, sizeof(cbufbroad));
         }
   }
   else
   {
      // Broadcast is disabled, so do not create RT Broadcast Control Buffers.
      btmem_tail2[cardnum] =
      btmem_rt_top_avail[cardnum] = BTMEM_RT_TOP_NOBRO + btmem_rt_begin[cardnum];
      if ( btmem_tail1[cardnum] >= btmem_tail2[cardnum] )
         return API_RT_MEMORY_OFLOW;
   }

   // Create the Default RT Control Buffers for Non-Broadcast consisting of
   //  32 buffers containing 3 words per buffer.  Enable all word counts.
   cbuf.legal_wordcount0 = cbuf.legal_wordcount1 = 0xFFFF;
   for (rtaddr = 0; rtaddr < RT_ADDRESS_COUNT; rtaddr++)
   {
      cbuf_addr = btmem_rt_cbuf_def(rtaddr);
      cbuf.message_pointer = (BT_U16BIT)(btmem_rt_mbuf_def(rtaddr) >> hw_addr_shift[cardnum]);
      vbtWrite(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));
   }

   // Create the Default RT Message Buffers consisting of 32 buffers containing
   //  48 words per buffer.  The RT31 buffer receives all broadcast messages!!!
   memset((void*)&mbuf, 0, sizeof(mbuf));          // Initialize to zero.
   mbuf.hw.ei_ptr = (BT_U16BIT)(BTMEM_EI >> 1);    // EI buffer is all zero.
   mbuf.hw.enable = flips(BT1553_INT_BROADCAST | BT1553_INT_END_OF_MESS);
   for ( rtaddr = 0; rtaddr < RT_ADDRESS_COUNT; rtaddr++ )
   {
      mbuf_addr = btmem_rt_mbuf_def(rtaddr);
      /* Message Buffer points to itself  */
      mbuf.hw.nxt_msg_ptr  = (BT_U16BIT)(mbuf_addr >> hw_addr_shift[cardnum]);
      mbuf.hw.mess_command.rtaddr    = (BT_U16BIT)rtaddr; // 45-bit TT boards
      // Initialize the API-specific data in the buffer.
      mbuf.api.mess_id.rtaddr = (BT_U16BIT)rtaddr;// rt address field
      //mbuf.api.mess_id.wcount    = 0;                // word count/mode code field
      //mbuf.api.mess_id.subaddr   = 0;                // subaddress field
      //mbuf.api.mess_id.tran_rec  = 0;                // transmit/receive bit
      mbuf.api.mess_verify    = RT_VERIFICATION;  // Verification word
      vbtWrite(cardnum, (LPSTR)&mbuf, mbuf_addr, sizeof(mbuf));
   }

   // Create the RT Address Buffer; it contains a 4-word entry for
   //  each of 32 RT addresses for a total of 128 words, on a 32-bit boundry.
   // Its address is just below the RT Filter Buffer.
   for ( rtaddr = 0; rtaddr < RT_ADDRESS_COUNT; rtaddr++ )
   {
      abuf.aentry[rtaddr].enables      = 0;
      abuf.aentry[rtaddr].status       = (BT_U16BIT)(rtaddr << 11);
      abuf.aentry[rtaddr].last_command = 0;
      abuf.aentry[rtaddr].bit_word     = 0;
   }
   vbtWrite(cardnum, (LPSTR)&abuf, BTMEM_RT_ABUF + btmem_rt_begin[cardnum],
            sizeof(RT_ABUF));

   // Create the RT Filter Buffer; it contains one word for each RT, TR and
   //  SA combination for a total of 2048 words, on a 2048 word boundry.
   // Each entry points to one of the default RT Control Buffers.  This buffer
   //  is located at the top of the RT memory segment.
   // Note that this array must be setup differently depending on the context:
   // 1.  SA 31 is not a mode code -> SA 31 maps unique RT Control Buffers
   // 2.  SA 31 is a mode code -> SA 31 maps the same RT Control Buffers as SA 0
   // 3.  Broadcast is enabled -> RT 31 maps Broadcast RT Control Buffers
   // 4.  Broadcast is not enabled -> RT 31 maps normal RT Control Buffers
   for (rtaddr = 0; rtaddr < RT_ADDRESS_COUNT; rtaddr++)
   {
      if ( (rtaddr == 31) && (rt_bcst_enabled[cardnum]) )
      {
         // Process the Broadcast entries for RT 31.
         for (tr = 0; tr < RT_TRANREC_COUNT; tr++)
         {
            for (subaddr = 0; subaddr < RT_SUBADDR_COUNT; subaddr++)
            {
               // Get the address of the broadcast cbuf.  If this is SA 31
               //  and SA 31 is a mode code, map to SA 0 in RT_CbufbroadAddr().
               cbufbroad_addr = RT_CbufbroadAddr(cardnum, subaddr, tr);
               cbuf_ptr = (BT_U16BIT)(cbufbroad_addr >> 1);
               addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
                      (rtaddr << 7) + (tr << 6) + (subaddr << 1);
               vbtWrite(cardnum, (LPSTR)&cbuf_ptr, addr, sizeof(cbuf_ptr));
            }
         }
      }
      else
      {
         // Process the Non-Broadcast entries.  All entries for a given
         //  rtaddress point to the same RT control buffer.
         cbuf_addr = btmem_rt_cbuf_def(rtaddr);
         cbuf_ptr  = (BT_U16BIT)(cbuf_addr >> 1);
         for (tr = 0; tr < RT_TRANREC_COUNT; tr++)
         {
            // Update the transmit and receive entries for this RT in the
            //  fbuf to point to the default cbuf for this RT.
            for (subaddr = 0; subaddr < RT_SUBADDR_COUNT; subaddr++)
            {
               addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
                      (rtaddr << 7) + (tr << 6) + (subaddr << 1);
               vbtWrite(cardnum, (LPSTR)&cbuf_ptr, addr, sizeof(cbuf_ptr));
            }
         }
      }
   }

   /*******************************************************************
   *  Set RAM Registers with the base addresses of the
   *   RT Address Buffer and the RT Filter Buffer (lower 64 KW).
   *******************************************************************/

   vbtSetFileRegister(cardnum, RAMREG_RT_ABUF_PTR, (BT_U16BIT)(BTMEM_RT_ABUF >> 1));
   vbtSetFileRegister(cardnum, RAMREG_RT_FBUF_PTR, (BT_U16BIT)(BTMEM_RT_FBUF >> 1));

   /*******************************************************************
   *  Initialize the RT subunit control structure to indicate that
   *   all subunits have not been initialized.
   *******************************************************************/
   for ( rtaddr = 0; rtaddr < RT_ADDRESS_COUNT; rtaddr++ )
   {
      for ( tr = 0; tr < RT_TRANREC_COUNT; tr++ )
      {
         for ( subaddr = 0; subaddr < RT_SUBADDR_COUNT; subaddr++ )
         {
            RT_mbuf_addr[cardnum][rtaddr][tr][subaddr]  = 0;
            RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] = 0;
         }
      }
   }
   /*******************************************************************
   *  Show that the RT has been initialized
   *******************************************************************/
   iqptr_rt_last[cardnum] = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;

   rt_inited[cardnum] = 1;     // Report initialization complete.
   DBC_Enable[cardnum] = 0;    // Dynamic bus control not enabled,
                               //  vbtNotify does not need to test for it.
   if(channel_is_shared[cardnum])
      vbtWriteRAM8(cardnum,(LPSTR)&rt_inited[cardnum],BTMEM_CH_SHARE + SHARE_RT_INITED,1); 

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_MessageGetaddr
*
*   FUNCTION
*       Return the physical board offset of the specified RT Message
*       Buffer.  The caller specifies the MBUF number for a specific
*       RT subunit.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_MBUF_ILLEGAL     -> illegal MBUF number specified
*       API_RT_MBUF_NOTDEFINED  -> specified MBUF not yet defined
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_MessageGetaddr(
   BT_UINT cardnum,           // (i) Card number
   BT_UINT rtaddr,            // (i) RT Address
   BT_UINT subaddr,           // (i) RT Subaddress
   BT_UINT tr,                // (i) Transmit or Receive
   BT_UINT mbuf_id,           // (i) Message Buffer ID (0-based)
   BT_U32BIT * mbuf_offset)  // (o) Offset to message buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT   addr;

   /*******************************************************************
   *  Check error conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_RT_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_RT_ILLEGAL_TRANREC;

   if ( mbuf_id )
      if ( mbuf_id >= RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
         return API_RT_ILLEGAL_MBUFID;

   /*******************************************************************
   *  Return the address of the specified (RT#, T/R, SA#, buffer#)
   *   RT MBUF message buffer.V4.01.ajh
   *******************************************************************/
   if ( mbuf_id >= RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
      addr = btmem_rt_mbuf_def(rtaddr);
   else
      addr = RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] + mbuf_id*sizeof(RT_MBUF);

   mbuf_offset[0] = addr;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_RT_MessageGetid
*
*  FUNCTION
*     This routine returns the subunit identification (rt address,
*     sub address and tran/rec bit) for the specified message buffer
*     address. This address is assumed to be a left-shifted by 1
*     value from the interrupt queue; it does not have the segment
*     value added to it, nor does it have the *8 factor for the PCI-1553.
*     These corrections are added in this function.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_RT_NOTINITED        -> RT_Init not yet called
*     API_RT_MBUF_NOMATCH     -> no match for specified address
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_MessageGetid(
   BT_UINT cardnum,           // (i) Card number
   BT_U32BIT     addr,        // (i) Address (byte address) of message to locate
   BT_UINT * rtaddr,      // (o) RT Terminal address
   BT_UINT * subaddr,     // (o) RT Subaddress
   BT_UINT * tr,          // (o) Transmit or Receive
   BT_UINT * mbuf_id)     // (o) Message buffer number (0-based)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   RT_MBUF_API    two;   // Buffer which reads the RT API MBUF data
   union{
     BT1553_COMMAND msg;   // Temp which holds 1553 cmd word from RT MBUF
     BT_U16BIT msg_value;
   }mess;

   /*******************************************************************
   *  Check error conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   /*******************************************************************
   *  Because we know ALL rt info is in seg2, force address to seg2,
   *  unless, of course, the rt info is NOT in seg2...
   *******************************************************************/
   // Handle the 8-word alignment for the PCI-1553.
   addr <<= (hw_addr_shift[cardnum]-1);

   /*******************************************************************
   *  Read the buffer and extract the RT#, T/R, SA# and Buffer # from
   *   the API data stored on the board.  This code is duplicated in
   *   the file NOTIFY.C in the local function SignalUserThread().
   *******************************************************************/
   // Get the RT command word to determine the RT, SA and TR.
   // This code added V4.01.ajh

   vbtRead(cardnum, (LPSTR)&mess.msg, addr+9*2, sizeof(BT1553_COMMAND));

   // Read the API-specific data from the end of the RT Message buffer
   //  to determine the RT MBUF number.
   vbtRead(cardnum, (LPSTR)&two, addr+sizeof(RT_MBUF_HW), sizeof(two));
   *mbuf_id = two.mess_bufnum;

   // If the RT command word is non-zero, use it to determine the RT#, the
   //  SA# and the T/R bit.  Otherwise use the API mess_id.
   if ( mess.msg_value )
   {
      *rtaddr  = mess.msg.rtaddr;
      *subaddr = mess.msg.subaddr;
      *tr      = mess.msg.tran_rec;
   }
   else
   {
      *rtaddr  = two.mess_id.rtaddr;
      *subaddr = two.mess_id.subaddr;
      *tr      = two.mess_id.tran_rec;
   }

   if ( two.mess_verify != RT_VERIFICATION && two.mess_verify != VERIFICATION_RT )
      return API_RT_MBUF_NOMATCH;

   /* Only if timing trace is enabled */
   AddTrace(cardnum, NBUSTOOLS_RT_MESSAGEGETID,
            *rtaddr, *subaddr, *tr, *mbuf_id, addr >> 1);
   return API_SUCCESS;
}


/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_MessageRead
*
*   FUNCTION
*       Read data from the specified RT Message Buffer.  The caller
*       specifies the MBUF number for a specific RT subunit.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_MBUF_ILLEGAL     -> illegal MBUF number specified
*       API_RT_MBUF_NOTDEFINED  -> specified MBUF not yet defined
*   Modified to insure that the interrupt status words are valid.V2.89.ajh
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_MessageRead(
   BT_UINT cardnum,         // (i) card number
   BT_UINT rtaddr,          // (i) RT Terminal Address of message to read
   BT_UINT subaddr,         // (i) RT Subaddress of message to read
   BT_UINT tr,              // (i) Receive or Transmit buffer is to be read
   BT_UINT mbuf_id,         // (i) ID number of message buffer (0-based)
   API_RT_MBUF_READ * apimbuf) // (o) User's buffer to receive data
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   union{
      BT_U16BIT   null_value;   // 1553 status word we return if no response
      BT1553_STATUS null_status;
   }stat;
                              //  or broadcast
   BT_UINT     i;             // Loop index
   BT_U16BIT   wcount;        // Number of 1553 data words
   BT_U32BIT   addr;          // Computed offset to specified message
   BT_U16BIT  *pData;         // Pointer to the data and status word following
                              //  the mess_status1 parameter in the buffer.
   RT_MBUF     mbuf;         // Local copy of the HW buffer, IP/ISA/PCI-1553   

   stat.null_value=0;
   /*******************************************************************
   *  Check error conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_RT_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_RT_ILLEGAL_TRANREC;

   if ( mbuf_id )
	   if ( mbuf_id >= RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
         return API_RT_ILLEGAL_MBUFID;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrRT_MessageRead[cardnum] )
   {
      i = (*pUsrRT_MessageRead[cardnum])(cardnum, &rtaddr, &subaddr, &tr, &mbuf_id, apimbuf);
      if ( i == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( i == API_NEVER_CALL_AGAIN )
         pUsrRT_MessageRead[cardnum] = NULL;
      else if ( i != API_CONTINUE )
         return i;
   }
#endif

   /*******************************************************************
   *  Get the address of the specified (RT#, T/R, SA#, buffer#)
   *   RT MBUF message buffer.V4.01.ajh
   *******************************************************************/
   if ( mbuf_id >= RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
      addr = btmem_rt_mbuf_def(rtaddr);
   else
      addr = RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] + mbuf_id*sizeof(RT_MBUF);

   /*******************************************************************
   *  Read the specified RT message buffer. After all of the data has
   *   been read, test rtb0 (firmware in buffer).  If it is set,
   *   then re-read the buffer one time only.  If the bit is still set,
   *   let the user decide what he wants to do about it.V4.22
   *******************************************************************/
   if(board_access_32[cardnum])
      vbtRead32(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));
   else
      vbtRead(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));

   flip(&mbuf.hw.status);  // Endian code

   /*******************************************************************
   *  Transfer fixed offset data from local buffer to caller's buffer.
   *  Mask off RTB1 only....
   *******************************************************************/
   if((_HW_FPGARev[cardnum] & 0x0fff) < 0x411)
      apimbuf->status = mbuf.hw.status & RTMASK;   // Interrupt Status
   else
      apimbuf->status = mbuf.hw.status;   // Interrupt Status

   /*******************************************************************
   *  Transfer the time tag and the RT command word from local buffer
   *   to caller's buffer.  Modified to handle mode codes.V4.01.ajh
   *******************************************************************/
   // All boards which support 45-bit HW time tags go here:

   TimeTagConvert(cardnum, &(mbuf.hw.time_tag), &(apimbuf->time));
   apimbuf->mess_command = mbuf.hw.mess_command;
   if ( tr )     // Handle broadcast transmit commands.V4.22.ajh
   {
      // Broadcast mode codes without data are transmit;
      if ( mbuf.hw.status & BT1553_INT_BROADCAST )
      {
         // Broadcast RT-RT, the transmitter DOES have a valid status word
         if ( mbuf.hw.status & BT1553_INT_RT_RT_FORMAT )
            apimbuf->mess_status = mbuf.hw.mess_status1;      // BCST RT-RT transmit
         else
            apimbuf->mess_status =  stat.null_status;   // BCST mode code transmit.
      }
      else
         apimbuf->mess_status = mbuf.hw.mess_status1;  // Non-BCST transmit.
   }
   pData = mbuf.hw.mess_data;    // Pointer to the remainder of the data

   /*******************************************************************
   *  Get the word count so we can copy the data to the user's buffer.
   *******************************************************************/
   wcount = apimbuf->mess_command.wcount;
   if ( (subaddr == 0) || ((subaddr == 31) && rt_sa31_mode_code[cardnum]) )
   {
      if ( (wcount >= 16) && (bt_op_mode[cardnum] == 0x0))  // V4.52 
         wcount = 1;       // Mode Code with data word
      else                                               
         wcount = 0;       // Mode Code without data word
   }
   else
   {
      if ( wcount == 0 )
         wcount = 32;
   }

   /*******************************************************************
   *  Copy the valid data words to the user's data buffer.V4.01.ajh
   *******************************************************************/
   for ( i = 0; i < wcount; i++ )
       apimbuf->mess_data[i] = pData[i];

   /*******************************************************************
   *  1553 Message status word position depends upon message type.
   *  We handled the RT transmit case above...
   *******************************************************************/
   stat.null_value = (BT_U16BIT)(rtaddr<<11);
   if ( apimbuf->status & BT1553_INT_NO_RESP )
   {
      apimbuf->mess_status = stat.null_status; // Null status word.
   }
   else if ( tr == 0 )  // Receive message
   {
      if ( mbuf.hw.status & BT1553_INT_ME_BIT )
         apimbuf->mess_status = *((BT1553_STATUS*)&(pData[1]));
      else if ( (mbuf.hw.status & (BT1553_INT_BROADCAST | BT1553_INT_RT_RT_FORMAT))
                ==  BT1553_INT_BROADCAST )
         apimbuf->mess_status = stat.null_status; // Null status word.
      else
         apimbuf->mess_status = *((BT1553_STATUS*)&(pData[wcount]));
   }

   /* Only if timing trace is enabled */
   AddTrace(cardnum, NBUSTOOLS_RT_MESSAGEREAD,
            rtaddr, subaddr, tr,
            *(BT_U16BIT *)&apimbuf->mess_command,
            *(BT_U16BIT *)&apimbuf->mess_status);
   return API_SUCCESS;
}


/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_MessageWrite
*
*   FUNCTION
*       Write data to the specified RT Message Buffer.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_MBUF_ILLEGAL     -> illegal MBUF number specified
*       API_RT_MBUF_NOTDEFINED  -> specified MBUF not yet defined
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_MessageWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddr,            // (i) RT Terminal Address
   BT_UINT subaddr,           // (i) Subaddress
   BT_UINT tr,                // (i) Transmit=1, Receive=0
   BT_UINT mbuf_id,           // (i) RT Message Buffer number
   API_RT_MBUF_WRITE * apimbuf)  // (i) Pointer to user's RT mbuf structure
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_UINT     i;             // Loop counter for moving the data words
   BT_U32BIT   addr;          // Message Buffer offset on the board
   BT_U32BIT   eiaddr;        // Error Injection Buffer offset on the board
   RT_MBUF     mbuf;          // Local copy of the HW buffer, IP/ISA/PCI-1553

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_RT_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_RT_ILLEGAL_TRANREC;

   if ( mbuf_id )
   {
      if ( mbuf_id >= RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
      {
         return API_RT_ILLEGAL_MBUFID;
      }
   }

   /*******************************************************************
   *  Get the address of the specified (RT#, T/R, SA#, buffer#)
   *   RT MBUF message buffer.V4.01.ajh
   *******************************************************************/
   if ( mbuf_id >= RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
      addr = btmem_rt_mbuf_def(rtaddr);
   else
      addr = RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] + mbuf_id*sizeof(RT_MBUF);

   /*******************************************************************
   *  Read specified message buffer (mostly to get the link address,
   *   and the API-specific words at the end of the buffer, but also
   *   to get the time-tag, command and status words).
   *******************************************************************/
   if(board_access_32[cardnum])
      vbtRead32(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));
   else
      vbtRead(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));

   /*******************************************************************
   *  Transfer information from caller's buffer to local buffer.
   *******************************************************************/
   eiaddr = BTMEM_EI + apimbuf->error_inj_id * sizeof(EI_MESSAGE);

   mbuf.hw.ei_ptr = (BT_U16BIT)(eiaddr >> 1);
   mbuf.hw.enable = flips(apimbuf->enable); //flips this but save the value
   mbuf.hw.status = 0;

   for (i = 0; i < 32; i++)
      mbuf.hw.mess_data[i] = apimbuf->mess_data[i];

   /*******************************************************************
   *  Write new specified message buffer.
   *******************************************************************/

   flip(&mbuf.hw.status);

   if(board_access_32[cardnum])
      vbtWrite32(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));
   else
      vbtWrite(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));
   
   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_MessageWriteDef
*
*   FUNCTION
*       Write data to the default Message Buffer for the
*       specified RT.  This function also builds a table of
*       "Broadcast Interrupt Enables" for each RT.  This table
*       is used in vbtNotify() to PostMessage() to the proper
*       RT windows.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_BADCARDNUM -> cardnum >= MAX_BTA
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_ILLEGAL_ADDR     -> RT address > 31
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_MessageWriteDef(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddr,            // (i) RT Terminal Address
   API_RT_MBUF_WRITE * apimbuf)  // (i) Pointer to user's RT Message Buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT addr;
   BT_U32BIT eiaddr;
   int       i;               // Loop counter for moving the data words.
   RT_MBUF   mbuf;       // Local copy of the HW buffer, IP/ISA/PCI-1553

   /*******************************************************************
   *  Check error conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   /*******************************************************************
   *  Read specified message buffer (mostly to get the link address
   *   and the API-specific data)
   *******************************************************************/

   addr = btmem_rt_mbuf_def(rtaddr);

   if(board_access_32[cardnum])
      vbtRead32(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));
   else
      vbtRead(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));   

   /*******************************************************************
   *  Transfer information from caller's buffer to local buffer:
   *  Error Injection buffer address, interrupt enables and data words.
   *******************************************************************/
   eiaddr = BTMEM_EI + apimbuf->error_inj_id * sizeof(EI_MESSAGE);
   mbuf.hw.ei_ptr = (BT_U16BIT)(eiaddr >> 1);

   flip(&mbuf.hw.status);
   // Make sure the broadcast interrupt enable is set for RT31...
   if ( (rtaddr == 31) && rt_bcst_enabled[cardnum] )
      mbuf.hw.status = mbuf.hw.enable = apimbuf->enable | BT1553_INT_BROADCAST;
   else
      mbuf.hw.status = mbuf.hw.enable = apimbuf->enable;

   // Now transfer the data words.
   for ( i = 0; i < 32; i++ )
      mbuf.hw.mess_data[i] = apimbuf->mess_data[i];

   /*******************************************************************
   *  Write new specified message buffer
   *******************************************************************/

   flip(&mbuf.hw.status);
   if(board_access_32[cardnum])
      vbtWrite32(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));
   else
      vbtWrite(cardnum, (LPSTR)&mbuf, addr, sizeof(mbuf));

   /*******************************************************************
   *  Update the table of broadcast enable's for the given RT.
   *******************************************************************/

   if ( apimbuf->enable & BT1553_INT_BROADCAST )
      BroadcastIntEnable[cardnum][rtaddr] = 1;
   else
      BroadcastIntEnable[cardnum][rtaddr] = 0;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_MessageWriteStatusWord
*
*   FUNCTION
*       Write 1553 Status Word data to the specified RT Message Buffer.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_MBUF_ILLEGAL     -> illegal MBUF number specified
*       API_RT_MBUF_NOTDEFINED  -> specified MBUF not yet defined
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_MessageWriteStatusWord(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddr,            // (i) RT Terminal Address
   BT_UINT subaddr,           // (i) Subaddress
   BT_UINT tr,                // (i) Transmit=1, Receive=0
   BT_UINT mbuf_id,           // (i) RT Message Buffer number
   BT_U16BIT wStatusWord,     // (i) 1553 RT Status word
   BT_UINT wFlag)             // (i) Flag=RT_SET or RT_NOCHANGE
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT       addr;
   RT_MBUF_API     api_mbuf;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_RT_ILLEGAL_ADDR;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_RT_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_RT_ILLEGAL_TRANREC;

   if ( mbuf_id )
      if ( mbuf_id >= RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
      {
         return API_RT_ILLEGAL_MBUFID;
      }

   /*******************************************************************
   *  Get the address of the specified (RT#, T/R, SA#, buffer#)
   *   RT MBUF message buffer.V4.01.ajh
   *******************************************************************/
   if ( mbuf_id >= RT_mbuf_alloc[cardnum][rtaddr][tr][subaddr] )
      addr = btmem_rt_mbuf_def(rtaddr);
   else
      addr = RT_mbuf_addr[cardnum][rtaddr][tr][subaddr] + mbuf_id*sizeof(RT_MBUF);

   // Get the API-specific portion of the MBUF
   addr += sizeof(RT_MBUF_HW);     // Skip over the hardware buffer.
   vbtRead(cardnum, (LPSTR)&api_mbuf, addr, sizeof(api_mbuf));

   /*******************************************************************
   *  Transfer information from caller's buffer to local buffer.
   *******************************************************************/

   // Do not allow user to set reserved bits. V2.23.ajh
   wStatusWord = (BT_U16BIT)(wStatusWord & RT_STATUSMASK_RES);

   if ( wFlag )
      api_mbuf.mess_statuswd = (BT_U16BIT)(wStatusWord | RT_SET);
   else
      api_mbuf.mess_statuswd = (BT_U16BIT)(wStatusWord & ~RT_SET);

   /*******************************************************************
   *  Write new specified message buffer.
   *******************************************************************/

   vbtWrite(cardnum, (LPSTR)&api_mbuf, addr, sizeof(api_mbuf));
   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_StartStop()
*
*   FUNCTION
*       This function starts/stops the specified RT.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_RT_RUNNING          -> RT currently operating
*       API_RT_NOTRUNNING       -> RT not operating
*       API_RT_ILLEGAL_ADDR     -> illegal RT address specified
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_StartStop(
   BT_UINT cardnum,           // (i) card number
   BT_UINT startflag)         // (i) Flag=1 to start the RT, =0 to stop it
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_INT    status, mode_warning;
   BT_U16BIT hwrData;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   AddTrace(cardnum, NBUSTOOLS_RT_STARTSTOP, startflag, rt_running[cardnum], 0, 0, 0);
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   if (startflag && rt_running[cardnum])
      return API_RT_RUNNING;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrRT_StartStop[cardnum] )
   {
      status = (*pUsrRT_StartStop[cardnum])(cardnum, &startflag);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrRT_StartStop[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Either start or stop the RT
   *******************************************************************/

   if (startflag)
   {	 
      if(startflag != 0xf)
      {
         // Detect single or dual function board and attempt to start second function.
         if(board_is_dual_function[cardnum] == 1)
         {
            mode_warning = API_DUAL_FUNCTION_ERR;
            if ( _HW_1Function[cardnum] && (bc_running[cardnum]) )
               return API_DUAL_FUNCTION_ERR;
         }
         else
         {
            mode_warning = API_SINGLE_FUNCTION_ERR;
            if ( _HW_1Function[cardnum] &&
               (bc_running[cardnum] | bm_running[cardnum]) )
               return API_SINGLE_FUNCTION_ERR;
         }
      }
     
      if((hwRTAddr[cardnum] == -1) || (hwRTAddr[cardnum] == BTD_RTADDR_PARITY))
      {
         if(CurrentCardSlot[cardnum] == 0)
         {
            vbtSetDiscrete(cardnum,DISREG_HW_RTADDR,0x4);
            vbtSetDiscrete(cardnum,DISREG_HW_RTADDR,0x0);
         }

         if(CurrentCardSlot[cardnum] == 1)
         {
	        vbtSetDiscrete(cardnum,DISREG_HW_RTADDR,0x20);
            vbtSetDiscrete(cardnum,DISREG_HW_RTADDR,0x0);
         }
      }

      api_writehwreg_or(cardnum, HWREG_CONTROL1, CR1_RTRUN);
      // The HWREG_RESPONSE register must be programmed after one of the
      //  three run bits has been set.
      api_writehwreg(cardnum,HWREG_RESPONSE,wResponseReg4[cardnum]);

      //Test the single mode warning bit
      hwrData = vbtGetHWRegister(cardnum,HWREG_CONTROL1);
      if(hwrData & CR1_SMODE)
         return mode_warning;

      SignalUserThread(cardnum, EVENT_RT_START, 0, 0);
      rt_running[cardnum] = 1;
      channel_status[cardnum].rt_run=1;



   }
   else
   {
      rt_running[cardnum] = 0;
      SignalUserThread(cardnum, EVENT_RT_STOP, 0, 0);
      api_writehwreg_and(cardnum, HWREG_CONTROL1, (BT_U16BIT)(~CR1_RTRUN));
      channel_status[cardnum].rt_run=0;
   }
   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*   PROCEDURE NAME - DumpRTptrs()
*
*   FUNCTION
*       This function dumps the RT message block pointers.
*
*   RETURNS
*       nothing
*
*****************************************************************************/

#if defined(FILE_SYSTEM)
void DumpRTptrs(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   int     rt;              // Loop indexes
   int     tr;
   int     sa;

   fprintf(hfMemFile, "\nRT Message Buffer Pointers (Word Addresses)\n");
   for ( rt = 0; rt < 32; rt++ )
   {
      for ( tr = 0; tr < 2; tr++ )
      {
         fprintf(hfMemFile, "RT=%2d%c ", rt, tr ? 'T' : 'R');
         for ( sa = 0; sa < 32; sa++)
         {
            if ( RT_mbuf_addr[cardnum][rt][tr][sa] )
               fprintf(hfMemFile, "%5.5X ", RT_mbuf_addr[cardnum][rt][tr][sa]/2);
            else
               fprintf(hfMemFile, "..... ");
            if ( (sa % 8) == 7 )
               fprintf(hfMemFile, sa == 31 ? "\n" : "\n       ");
         }
      }
   }

   fprintf(hfMemFile, "\nRT Message Buffer Counts (hex)\n");
   for ( rt = 0; rt < 32; rt++ )
   {
      for ( tr = 0; tr < 2; tr++ )
      {
         fprintf(hfMemFile, "RT=%2d%c ", rt, tr ? 'T' : 'R');
         for ( sa = 0; sa < 32; sa++)
         {
            if ( RT_mbuf_alloc[cardnum][rt][tr][sa] )
               fprintf(hfMemFile, "%2.2X ", RT_mbuf_alloc[cardnum][rt][tr][sa]);
            else
               fprintf(hfMemFile, "... ");
            if ( (sa % 16) == 15 )
               fprintf(hfMemFile, sa == 31 ? "\n" : "\n       ");
         }
      }
   }

   fprintf(hfMemFile, "\n");
}
#endif

#ifndef _CVI_
/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_ReadNextMessage()
*
*   FUNCTION
*       This function reads the next RT message that meets the criteria set
*       by the passed parameters.  The arguments include a timeout value in
*       in milliseconds. If there are no Rt messages put onto the queue this
*
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_RT_READ_TIMEOUT     -> Timeout before data read
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_ReadNextMessage(int cardnum,BT_UINT timeout,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_RT_MBUF_READ *pRT_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   int       status;

   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.

   BT_U16BIT iq_addr;                  // Hardware Interrupt Queue Pointer

   BT_U32BIT beg;                      // Beginning of the interrupt queue
   BT_U32BIT end;                      // End of the interrupt queue
   BT_UINT   rtaddr;
   BT_UINT   subaddr;
   BT_UINT   transrec;
   BT_UINT   messno;
  
   BT_U32BIT mess_addr;
   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;

   BT_U32BIT start;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

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
      *  by the firmware. Start with the SW interrupt pointer from the last time.
      **********************************************************************/

      while ( iqptr_sw != iqptr_hw )
      {
         /*******************************************************************
         *  Get the 3 word interrupt block pointed to by iqptr_sw.
         *******************************************************************/

         vbtRead_iq(cardnum,(LPSTR)&intqueue, iqptr_sw, sizeof(intqueue));

         iqptr_sw = ((BT_U32BIT)intqueue.nxt_int) << 1; // Chain to next entry

         // We only care about RT interrupt here.
         if ( intqueue.t.mode.rt )
         {
            mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
            status = BusTools_RT_MessageGetid(cardnum,mess_addr,&rtaddr,
                                                 &subaddr,&transrec,&messno);
            if (status)
               return status;

         if((rt_addr == DONT_CARE) || (rt_addr & (bit<<rtaddr)))
		 {
			if((subaddress == DONT_CARE) || (subaddress & (bit<<subaddr)))
			{
               if((tr == DONT_CARE) || (tr == (BT_INT)transrec))
					{
            
                     status = BusTools_RT_MessageRead(cardnum,rtaddr,subaddr,
                                                      transrec,messno,
                                                      pRT_mbuf);
					}
				}
			}

         }
      }
   }while((CEI_GET_TIME() - start) < timeout);
   return API_RT_READ_TIMEOUT;
}
#endif

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_ReadLastMessage()
*
*   FUNCTION
*       This function reads the last RT message that meets the criteria set
*       by the passed parameters.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_RT_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_ReadLastMessage(int cardnum,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_RT_MBUF_READ *pRT_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   int       status;
   int       found;
   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_U32BIT iqptr_cur;
   BT_UINT   rtaddr;
   BT_UINT   subaddr;
   BT_UINT   transrec;
   BT_UINT   messno;
   BT_U32BIT mess_addr;

   BT_INT DONT_CARE = -1;
   BT_U32BIT bit = 1;
   BT_UINT queue_entry=6;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   iqptr_hw = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;

   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % sizeof(IQ_MBLOCK) != 0 ) )
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_rt_last[cardnum];
   iqptr_cur = iqptr_hw;
   found = 0;

   /***********************************************
   *  Loop until all the message are checked
   ************************************************/
   while ( iqptr_sw != iqptr_cur )
   {
      /******************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
	  
	  if(iqptr_cur == BTMEM_IQ)
         iqptr_cur = BTMEM_IQ_NEXT - queue_entry;
      else
	     iqptr_cur = iqptr_cur - queue_entry;

      vbtRead(cardnum,(LPSTR)&intqueue, iqptr_cur, sizeof(intqueue));

      // We only care about RT interrupt here.
      if ( intqueue.t.mode.rt )
      {
         mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
         status = BusTools_RT_MessageGetid(cardnum,mess_addr,&rtaddr,
                                                 &subaddr,&transrec,&messno);
         if (status)
            return status;
         if((rt_addr == DONT_CARE) || (rt_addr & (bit<<rtaddr)))
		 {
			if((subaddress == DONT_CARE) || (subaddress & (bit<<subaddr)))
			{
               if((tr == DONT_CARE) || (tr == (BT_INT)transrec))
			   {
                  found = 0x1;
                  status = BusTools_RT_MessageRead(cardnum,rtaddr,subaddr,
                                                   transrec,messno,pRT_mbuf);
				  if(status)
                     return status;
				  else
                     break;
			   }
			}
		 }
      }
   }
   iqptr_rt_last[cardnum] = iqptr_hw;
   if(!found)
	   return API_RT_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_ReadLastMessageBlock
*
*   FUNCTION
*       Reads the new RT message in the interrupt queue since the last time 
*       this function was called.  If this is the first time all the messages
*       in the buffer are read.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> bad card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_ReadLastMessageBlock(int cardnum,BT_INT rt_addr_mask,
									   BT_INT subaddr_mask,BT_INT tr, BT_UINT *mcount,API_RT_MBUF_READ *pRT_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   int       status;
   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.
   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_UINT   rtaddr;
   BT_UINT   subaddr;
   BT_UINT   transrec;
   BT_UINT   messno;
   BT_UINT   msg_cnt;
   BT_U32BIT mess_addr;

   BT_U32BIT bit = 1;
   BT_INT DONT_CARE = -1;

   *mcount = 0;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;
   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   iqptr_hw = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;
   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % sizeof(IQ_MBLOCK) != 0 ))   
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_rt_last[cardnum];   
   /************************************************
   *  Loop until 
   ************************************************/
   msg_cnt = 0;
   while ( iqptr_sw != iqptr_hw )
   {
      /*******************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
      vbtRead_iq(cardnum,(LPSTR)&intqueue, iqptr_sw, sizeof(intqueue));
      iqptr_sw = ((BT_U32BIT)intqueue.nxt_int) << 1; // Chain to next entry

      // We only care about RT interrupt here.
      if ( intqueue.t.mode.rt )
      {
         mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
         status = BusTools_RT_MessageGetid(cardnum,mess_addr,&rtaddr,
                                                 &subaddr,&transrec,&messno);
         if (status)
            return status;
         if((rt_addr_mask == DONT_CARE) || (rt_addr_mask & (bit<<rtaddr)))
		 {
			if((subaddr_mask == DONT_CARE) || (subaddr_mask & (bit<<subaddr)))
			{
               if((tr == DONT_CARE) || (tr == (BT_INT)transrec))
			   {
                  status = BusTools_RT_MessageRead(cardnum,rtaddr,subaddr,
                                                      transrec,messno,
                                                      &pRT_mbuf[msg_cnt]);
				  if(status)
					  return status;
				  msg_cnt++;
			   }
			}
		 }
      }
   }
   iqptr_rt_last[cardnum] = iqptr_sw;
   *mcount = msg_cnt;
   if(msg_cnt == 0)
	   return API_RT_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_GetRTAddr()
*
*   FUNCTION
*       This function returns the address set by the hardwaire address line
*       When hardwired RT addressing is enabled
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_RUNNING          -> RT currently operating
*       API_RT_MEMORY_OFLOW     -> RT memory filled
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_GetRTAddr(
    BT_UINT cardnum,          // (i) card number
    BT_INT *rtaddr)           // (o) hardwire RT address lines value
{

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(board_has_hwrtaddr[cardnum] == 0)
      return API_HARDWARE_NOSUPPORT;
     
   if(hwRTAddr[cardnum] == -1)
      return API_NO_HARDWIRE_RT;

   if(hwRTAddr[cardnum] == BTD_RTADDR_PARITY)
      return BTD_RTADDR_PARITY;

   *rtaddr = hwRTAddr[cardnum];
   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_GetRTAddr1760()
*
*   FUNCTION
*       This function returns the latched address set by the hardwire address
*       lines, when hardwired RT addressing is enabled.  It also allows the 
*       user to read the unlatched data on the lines.
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_RT_RUNNING          -> RT currently operating
*       API_RT_MEMORY_OFLOW     -> RT memory filled
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_GetRTAddr1760(
    BT_UINT cardnum,          // (i) card number
    BT_UINT aflag,            // (i)  = 0 - return latched data
                              //     >= 1 - read current data.
    BT_INT *rtaddr)           // (o) hardwire RT address lines value
{ 
   BT_U16BIT rdata;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(board_has_hwrtaddr[cardnum] == 0)
      return API_HARDWARE_NOSUPPORT; 

   if(aflag)
   {
      rdata = vbtGetDiscrete(cardnum,DISREG_HW_RTADDR); 
      if(CurrentCardSlot[cardnum] == 0)
      {
         if((rdata & 0x3) == 0x0)
         {
            hwRTAddr[cardnum] = -1;
         }
         else if((rdata & 0x3) == 0x1)
         {
            vbtSetDiscrete(cardnum,DISREG_RTADDR_RD1,RTADDR_SELECT1);
            hwRTAddr[cardnum] = (vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1))&0x1f;
         }
         else
         {  
	        return BTD_RTADDR_PARITY;
         }
      }
      else if(CurrentCardSlot[cardnum] == 1)
      {  
         if((rdata & 0x18) == 0x0)
         {
            hwRTAddr[cardnum] = -1;
         }
         else if((rdata & 0x18) == 0x8)
         {
            vbtSetDiscrete(cardnum,DISREG_RTADDR_RD1,RTADDR_SELECT2);
            hwRTAddr[cardnum] = ((vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1)) & 0x1f00)>>8;
         }
         else
         {
	        return BTD_RTADDR_PARITY;
         }
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }

   *rtaddr = hwRTAddr[cardnum];
   
   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_Checksum1760()
*
*   FUNCTION
*       This function Calculates a 1760 checksum for the data and stores  
*       the result in the next location in the API_RT_MBUF_WRITE.  
*
*   RETURNS
*       API_SUCCESS             -> success
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_Checksum1760(API_RT_MBUF_WRITE *mbuf, BT_U16BIT *cksum, int wdcnt)
{
	BT_U16BIT checksum, wd, temp, i, shiftbit;

	// Start at zero.
	
    checksum = 0;

	// Process each word in the data buffer.
	for (wd = 0; wd < (wdcnt-1); wd++) {
		temp = mbuf->mess_data[wd];

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
    mbuf->mess_data[wdcnt-1] = checksum;
	return API_SUCCESS;
}

#ifndef _LVRT_
/****************************************************************************
*
*   PROCEDURE NAME - BusTools_RT_AutoIncrMessageData()
*
*   FUNCTION
*       This function sets up an interrupt to automatically increment a data
*       value ins a partular RT transmit message.  You supply the message number, start
*       value, increment value, increment rate, and max value.  You need to
*       both to start and stop the auto incrementing.  
*
*   RETURNS
*       API_SUCCESS         
*       API_BUSTOOLS_BADCARDNUM
*       API_BUSTOOLS_NOTINITED
*       API_RT_NOTINITED
*       
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_RT_AutoIncrMessageData(BT_INT cardnum,BT_INT rtaddr,BT_INT subaddr,
									                  BT_INT data_wrd,BT_U16BIT start, BT_U16BIT incr, 
                                                      BT_INT rate, BT_U16BIT max, BT_INT sflag)
{
   int    rt, tr, sa, i;
   int    status;

   API_RT_MBUF_READ rt_rbuf;
   API_RT_MBUF_WRITE rt_wbuf;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_inited[cardnum] == 0)
      return API_RT_NOTINITED;

   /* Make sure we get an interrupt on this message */

   /***********************************************************************
   *  If the rt is starting, register a thread for the board.
   *  If the rt is shutting down, unregister the thread.
   **********************************************************************/
   if (sflag)
   {
      if(max == 0)
         max = 0xffff;

      if(pRTIntFIFO[cardnum][rtaddr][subaddr] != NULL)
         return API_RT_AUTOINC_INUSE;

      pRTIntFIFO[cardnum][rtaddr][subaddr] = (API_INT_FIFO *)malloc(sizeof(API_INT_FIFO));
      if(pRTIntFIFO[cardnum][rtaddr][subaddr] == NULL)
         return API_MEM_ALLOC_ERR;

      status = BusTools_RT_MessageRead(cardnum, rtaddr, subaddr, 1,0, &rt_rbuf ); // always transmit always the first buffer
      if(status)
         return status;

      for(i=0;i<32;i++)
        rt_wbuf.mess_data[i] = rt_rbuf.mess_data[i];

      rt_wbuf.mess_data[data_wrd] = start; // setup the start data
      rt_wbuf.error_inj_id = 0;
      rt_wbuf.enable = BT1553_INT_END_OF_MESS;

      status = BusTools_RT_MessageWrite(cardnum, rtaddr, subaddr, 1,0, &rt_wbuf ); // always transmit the first buffer
      if(status)
         return status; 

      // Setup the FIFO structure for this board.
      memset(pRTIntFIFO[cardnum][rtaddr][subaddr], 0, sizeof(API_INT_FIFO));
      pRTIntFIFO[cardnum][rtaddr][subaddr]->function     = rt_auto_incr;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->iPriority    = THREAD_PRIORITY_ABOVE_NORMAL;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->dwMilliseconds = INFINITE;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->iNotification  = 0;       // Dont care about startup or shutdown
      pRTIntFIFO[cardnum][rtaddr][subaddr]->FilterType     = EVENT_RT_MESSAGE;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->nUser[0] = rtaddr;  // Set the first user parameter to message number.
      pRTIntFIFO[cardnum][rtaddr][subaddr]->nUser[1] = data_wrd;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->nUser[2] = incr;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->nUser[3] = rate;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->nUser[4] = 1; // use for counter
      pRTIntFIFO[cardnum][rtaddr][subaddr]->nUser[5] = max;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->nUser[6] = start;
      pRTIntFIFO[cardnum][rtaddr][subaddr]->nUser[7] = subaddr;   
      
      for ( rt=0; rt < 32; rt++ )
         for (tr = 0; tr < 2; tr++ )
            for (sa = 0; sa < 32; sa++ )
               pRTIntFIFO[cardnum][rtaddr][subaddr]->FilterMask[rt][tr][sa] = 0x0;  //disable all messages

      pRTIntFIFO[cardnum][rtaddr][subaddr]->FilterMask[rtaddr][1][subaddr] = 0xffffffff;

      // Call the register function to register and start the BC thread.
      status = BusTools_RegisterFunction(cardnum, pRTIntFIFO[cardnum][rtaddr][subaddr], 1);
      if ( status )
         return status;
   }
   else
   {
      // Call the register function to unregister and stop the BC thread.
     if(pRTIntFIFO[cardnum][rtaddr][subaddr] == NULL)
         return API_NULL_PTR;
      status = BusTools_RegisterFunction(cardnum, pRTIntFIFO[cardnum][rtaddr][subaddr], 0);
      free(pRTIntFIFO[cardnum][rtaddr][subaddr]);
      pRTIntFIFO[cardnum][rtaddr][subaddr] = NULL;
   }
   return status;      // Have the API function continue normally
}

/*===========================================================================*
 * User ENTRY POINT:        R T _ A U T O _ I N C R
 *===========================================================================*
 *
 * FUNCTION:    rt_auto_incr()
 *
 * DESCRIPTION: This function increments the pre-defined data word.
 *
 * It will return:
 *   API_SUCCESS - Thread continues execution
 *===========================================================================*/

BT_INT _stdcall rt_auto_incr(
   BT_UINT cardnum,
   struct api_int_fifo *sIntFIFO)
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_INT      tail;           // FIFO Tail index
   BT_INT      status,i,wcount;
   API_RT_MBUF_READ rt_rbuf;
   API_RT_MBUF_WRITE rt_wbuf;
   BT_INT rtaddr,subaddr,trans,bufferID;

   /***********************************************************************
   *  Loop on all entries in the FIFO.  Get the tail pointer and extract
   *   the FIFO entry it points to.   When head == tail FIFO is empty
   ***********************************************************************/
   tail = sIntFIFO->tail_index;
   while ( tail != sIntFIFO->head_index )
   {
      bufferID = sIntFIFO->fifo[tail].bufferID;
      rtaddr  = sIntFIFO->fifo[tail].rtaddress;        // RT address
      trans    = sIntFIFO->fifo[tail].transrec;         // Transmit/Receive
      subaddr = sIntFIFO->fifo[tail].subaddress;       // Subaddress number

      if((rtaddr  == sIntFIFO->nUser[0]) &&
         (subaddr == sIntFIFO->nUser[7]) &&
         (trans == 1))
      {
         //  and read the message data from the board.
         status = BusTools_RT_MessageRead(cardnum, rtaddr, subaddr, 1, bufferID, &rt_rbuf ); // always transmit always the first buffer
         if ( status )
            return status;

         if (rt_rbuf.mess_command.wcount == 0)
            wcount = 32;
         else
            wcount = rt_rbuf.mess_command.wcount;
              
         for(i=0;i<wcount;i++)
           rt_wbuf.mess_data[i] = rt_rbuf.mess_data[i];
 
         // Update the data buffer
         if(sIntFIFO->nUser[4] == sIntFIFO->nUser[3])
         {
           rt_wbuf.mess_data[sIntFIFO->nUser[1]] += sIntFIFO->nUser[2];
           rt_wbuf.mess_data[sIntFIFO->nUser[1]] = rt_wbuf.mess_data[sIntFIFO->nUser[1]] % sIntFIFO->nUser[5];
           if(rt_wbuf.mess_data[sIntFIFO->nUser[1]] == 0)
           rt_wbuf.mess_data[sIntFIFO->nUser[1]] = sIntFIFO->nUser[6];
           sIntFIFO->nUser[4] = 1;
         }
         else
            sIntFIFO->nUser[4]++;

         bufferID = (bufferID+1)%RT_mbuf_alloc[cardnum][rtaddr][trans][subaddr];
         rt_wbuf.error_inj_id = 0;
         rt_wbuf.enable = BT1553_INT_END_OF_MESS;

         // Now write the data back to the message buffer:
         status = BusTools_RT_MessageWrite(cardnum, rtaddr, 
                                           subaddr, 1, bufferID, &rt_wbuf ); // always transmit always the first buffer
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
#endif // _LVRT_
