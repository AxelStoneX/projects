/*============================================================================*
 * FILE:                          B M . C 
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
 *             This file contains the API routines which support BM
 *             operation of the BusTools board.
 *             These functions assume that the BusTools_API_Init-() function
 *             has been successfully called.
 *     
 * USER ENTRY POINTS:
 *    BusTools_BM_Checksum1760   - Calculate 1760 chechsum on data in a BM buffer
 *    BusTools_BM_GetUserData    - Read the BM user data value from the file register 
 *    BusTools_BM_FilterRead     - read the specified BM Control Buffer
 *    BusTools_BM_FilterWrite    - creates/updates Control Buffer for RT/SA/tr.
 *    BusTools_InterMessageGap   - compute intermessage gap time
 *    BusTools_InterMessageGapLV - compute intermessage gap time Lab View
 *    BusTools_BM_Init           - performs full initialization of BM
 *    BusTools_BM_MessageAlloc   - alloc and init BM Message Buffers
 *    BusTools_BM_MessageGetaddr - converts BM message id to a board address
 *    BusTools_BM_MessageGetid   - converts a board address to a BM msg id
 *    BusTools_BM_MessageRead    - transfers contents of a BM Message Buffer
 *    BusTools_BM_MessageReadBlock   
 *                               - transfers all BM_MBUFs received since the
 *                                 last call to the routine from the API buffer
 *    BusTools_BM_ReadLastMessage- Reads the last message in the interrupt queue
 *    BusTools_BM_ReadLastMessageBlock- Reads the last group of message in the interrupt queue.
 *    BusTools_BM_ReadNextMessage- Reads the next message in the interrupt queue.
 *    BusTools_BM_SetRT_RT_INT   - Sets interrupt message in a RT-RT message
 *    BusTools_BM_SetUserData    - Sets the BM User data in the file register.
 *    BusTools_BM_StartStop      - turns the BM on or off
 *    BusTools_BM_TriggerRead    - reads from the BM Trigger Buffer
 *    BusTools_BM_TriggerWrite   - writes to the BM Trigger Buffer
 *    BusTools_InterMessageGap   - computes gap between two BM messages
 *    BusTools_ErrorCountClear   - Clear the message & error counters
 *    BusTools_ErrorCountGet     - Get the message and error counters.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    BM_MsgReadBlock            - transfers all BM_MBUFs received since the
 *                                 last call to the routine to local BM buffer.
 *    DumpBM                     - Helper function for BusTools_DumpMemory()
 *
 * INTERNAL ROUTINES:
 *    BM_MessageConvert   - convert raw BM msg buf to API_BM_MBUF struct
 *    ErrorCountUpdate    - Update message and error counters
 *    BM_BusLoading       - Update the API Bus Statistics buffer
 *    BM_BusLoadingFilter - Update the Filtered Bus Loading Statistics buffer
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  01/12/1999   Modify BM_MessageConvert() to clear 1553 status word, status
               word status and response time for broadcast message.  Fix
               BusTools_BM_MessageGetid() for the PCI-1553.V3.01.ajh
  05/11/1999   Modified ErrorCountUpdate() to not count 1553 status word bits if
               this is a broadcast message (since there is no 1553 status word
               for broadcast messages).V3.06.ajh
  10/11/1999   Fixed RT-RT message intermessage gap time calculation.V3.22.ajh
  10/19/1999   Fixed status_c1 in BM_MessageConvert.V3.22.ajh
  10/30/1999   Modify for 48-bit hardware time tags.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  02/22/2000   Added back door support for the new BM Trigger Output on every
               BM message interrupt.V4.01.ajh
  02/24/2000   Moved the call to TimeTagClearFlag() from the ReadBlock function
               into vbtNotify.V4.01.ajh
  06/19/2000   Changed BM_StartStop to return API_SINGLE_FUNCTION_ERR if either the
               RT or the BM is running, and _HW_1Function[cardnum] is set.V4.05.ajh
  06/22/2000   Added support for the User Interface DLL.  Added a second default
               BM cbuf and initialized it.  Changed BM_FilterWrite to use two
               default buffers: first enables everything, second disables all
               word counts.V4.06.ajh
  10/04/2000   Changed BusTools_InterMessageGap to correctly account for the
               fact that MIL-STD-1553 gap times are not bus dead times.V4.16.ajh
  10/11/2000   Changed BusTools_InterMessageGap to correctly account for
               broadcast messages, and test a new interrupt status bit that
               reports if a SA 31 message is a mode code.V4.17.ajh
  11/07/2000   Modified BM_StartStop to not call the dump function if the
               file name starts with a null.V4.20.ajh
  12/04/2000   Changed BM_Start to report single function warning after
               starting the BM.V4.26.ajh
  12/14/2000   Changed the way BM_MessageRead computes message numbers.V4.30.ajh
  01/02/2001   Modified BM_MessageConvert to read response time as a word.V4.30.ajh
  01/17/2001   Fixed segment wrap around problem in BM_FilterWrite.V4.31.ajh
  03/27/2001   Added V3.11 BM trigger on data to support IP-1553 V6 PROMs.V4.31.ajh
  03/28/2001   Added code to perform bus loading and error recording.V4.32.ajh
  04/18/2001   Changed BM Trigger to support V3.20 firmware with trigger on data
               support.V4.38.ajh
  06/07/2001   Set the activity bit for the transmitting RT of an RT-RT msg.V4.40.ajh
  09/24/2001   Modified struct api_bus_stats to add Accumulated Interrupt Status
               per [RT][TR][SA][BUS].V4.41.ajh
  02/15/2002   Add support for high level interrupt queue processing.v4.48
  05/28/2002   Change mode code handling for 1553Z mode 
  01/27/2003   Fix sugned/unsigned mismatch.
  10/01/2003   32-Bit API
  10/01/2003   BM Trigger update
  11/19/2007   Added BM DMA code that is conditionally compiled.  The current release
               does not compile this code.
  09/10/2008   Allow for BM trigger on bits in the interrupt status word.
*/

/*---------------------------------------------------------------------------*
 *       INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"

/*---------------------------------------------------------------------------*
 *          LOCAL VARIABLES, NOT REFERENCED OUTSIDE OF THIS MODULE
 *---------------------------------------------------------------------------*/
static BT_U32BIT bm_ecount[MAX_BTA][32];// Cummulative error counters
static BT_U32BIT bm_rec_prev[MAX_BTA];  // Previous spot in BM_MBUF's (to detect wrap)
//static BT_UINT   bm_count[MAX_BTA];     // Current number of BM HW buffers in circle  NOW IN GLOBALS.H
//static BT_UINT   nBM_MBUF_len[MAX_BTA]; // Actual length(bytes) of a HW BM_MBUF, including pad
static BT_UINT   bm_isBusLoading[MAX_BTA]; // Flag is set if bus loading being computed
static BT_U32BIT iqptr_bm_last[MAX_BTA];      // Queue pointer.
static BT_U32BIT bmq_ptr_last[MAX_BTA];

#ifdef VXW_PCI_THALES
static BT_U32BIT bmq_ptr_save[MAX_BTA];
#endif //VXW_PCI_THALES
/****************************************************************************
*  The following two entries handle the BM message numbering scheme:
*  'bm_messno' is the message number currently assigned to 'bm_messaddr'.
*  It is changed by 'BM_MsgReadBlock' as required.
****************************************************************************/

static BT_U32BIT bm_messaddr[MAX_BTA];  // Current HW BM buffer address offset.
static BT_U32BIT bm_messno[MAX_BTA];    // Current HW message number.
#if defined(DO_BUS_LOADING)
static API_BUS_STATS bus_stats[MAX_BTA];// Bus statistics
#endif //(DO_BUS_LOADING)

/****************************************************************************
*  Bit definitions for "bm_hw_queue_ovfl" byte array.
****************************************************************************/

#define BM_API_OFLOW         0x01  /* If set, BM API FIFO has overflowed */
#define BM_API_OFLOW_MSG     0x02  /* If set, API oflow status returned  */
#define BM_HW_OFLOW          0x10  /* If set, BM HW FIFO has overflowed  */
#define BM_HW_OFLOW_MSG      0x20  /* If set, HW oflow status returned   */
#define BM_REG_BAD           0x40  /* If set, BM msg pointer reg bad     */
#define BM_REG_BAD_MSG       0x80  /* If set, BM msg reg status returned */

/****************************************************************************
*  Interrupt enables and status reporting masks. V4.16
****************************************************************************/
#define BM_INTERRUPT_ENABLES_VALID 0x8FFFFFFF  /* Valid BM Interrupts */
#define BM_INTERRUPT_STATUS_VALID1 0xBFFFFFFF  /* Valid BM Message Interrupt status */
#define BM_INTERRUPT_STATUS_VALID2 0xFFFFFFFF  /* Valid BM Block Read Interrupt status */

#ifdef BM_EXPRESS
#include "bm_only_functions.c"
#endif //BM_EXPRESS

/****************************************************************************
*
*  BC/BM MEMORY MAP (all in first segment)
*
*     Start End
*     Addr  Addr  Size    
*     (hex) (hex) (dec)    Description
*     ---------------------------------------------------------------------
*     0000  003F    64     Hardware Registers 
*     0040  007F    64     RAM Registers (for the microcode)
*     0080  0094    21     BM Trigger Buffer
*     0095  0099     5     BM Default Control Buffer
*     009A  0399   768     IQ Buffer (256 entries x 3 words each)
*     039A  07FB  1122     EI Buffers (33 entries x 34 words each)
*     07FC  07FF     5     Unused
*     0800  0FFF  2048     BM Filter Buffer. Must be on 2K boundry.
*     1000  ----  varies   BM Control Buffers, variable size up to 10K words.
*     1000  FFFF           Available for BM data buffers, BC message buffers.
*
*  All references in preceeding table are word addresses and word counts.
*  Note that while the board uses word address and word counts, the host
*  uses byte addresses and byte counts.
*
****************************************************************************/

/****************************************************************
*
*  PROCEDURE - BusTools_BM_FilterRead
*
*  FUNCTION
*     This procedure reads the specified BM Control Buffer
*     from the hardware.  The information is returned in the
*     caller supplied structure.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_NOTINITED        -> BM_Init not yet called
*     API_BM_ILLEGAL_ADDR     -> BM illegal address specified
*     API_BM_ILLEGAL_SUBADDR  -> BM illegal subaddress specified
*     API_BM_ILLEGAL_TRANREC  -> BM illegal trans/rec flag specified
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_FilterRead(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT rtaddr,
   BT_UINT subaddr,
   BT_UINT tr,
   API_BM_CBUF * api_cbuf)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_U32BIT  addr_fbuf;         // address within BM filter buffer
   BT_U16BIT  waddr;             // word address of BM control buffer
   BT_U32BIT  addr;              // byte address of BM control buffer
   BM_CBUF    cbuf;              // local hw compatible BM control buffer

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_BM_ILLEGAL_ADDR;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_BM_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_BM_ILLEGAL_TRANREC;

   /*******************************************************************
   *  Get cbuf from hardware, first reading the BM Filter Buffer(fbuf)
   *  entry for this RT/TR/SA combination to get the cbuf address.
   *******************************************************************/

   addr_fbuf = BTMEM_BM_FBUF +        // Byte addr of the beginning of fbuf.
       (rtaddr << 7) + (tr << 6) + (subaddr << 1 ); // Byte offset of entry.

   // Read BM Filter Buffer entry (e.g., cbuf word address) for RT/TR/SA.
   vbtRead(cardnum, (LPSTR)&waddr, addr_fbuf, sizeof(waddr));

   addr = (waddr << 1) + BTMEM_BM_BEGIN;    // Convert to byte addr of cbuf
   vbtRead(cardnum, (LPSTR)&cbuf, addr, sizeof(cbuf));

   /*******************************************************************
   *  Copy information from local buffer to caller's buffer
   *******************************************************************/

   api_cbuf->t.wcount      = flips(cbuf.wcount);
   api_cbuf->pass_count    = cbuf.pass_count2;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_BM_FilterWrite
*
*  FUNCTION
*     This procedure creates/updates the BM Control Buffer structure for the
*     specified subunit (RT addr/subaddr/tr).  If this is the first time this
*     routine has been called for the specified subunit (since a "BM_Init"),
*     the BM Control Buffer is allocated in the BT memory.  If not, the existing
*     Control Buffer is updated with the new information.
*
*     The API maintains two default BM control buffers: the first one contains
*     FFFF FFFF (enable all messages) and the second one contains 0000 0000
*     (disable all messages).
*
*     If the buffer is being set to FFFF FFFF 0001, then a new buffer is not
*     allocated.  This routine points the current entry to the first default
*     CBUF and returns SUCCESS.
*
*     If the buffer is being set to 0000 0000 0001, then a new buffer is not
*     allocated.  This routine points the current entry to the second default
*     CBUF and returns SUCCESS.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_NOTINITED        -> BM_Init not yet called
*     API_BM_MEMORY_OFLOW     -> BM memory overflow
*     API_BM_ILLEGAL_ADDR     -> BM illegal address specified
*     API_BM_ILLEGAL_SUBADDR  -> BM illegal subaddress specified
*     API_BM_ILLEGAL_TRANREC  -> BM illegal trans/rec flag specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_FilterWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   rtaddr,
   BT_UINT   subaddr,
   BT_UINT   tr,
   API_BM_CBUF * api_cbuf)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_U32BIT addr_fbuf;         // Address within BM filter buffer
   BT_U16BIT waddr;             // Word address of BM control buffer
   BT_U32BIT addr;              // Byte address of BM control buffer
   BM_CBUF   cbuf;              // Local hw compatible BM control buffer

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   if (rtaddr >= RT_ADDRESS_COUNT)
      return API_BM_ILLEGAL_ADDR;

   if (subaddr >= RT_SUBADDR_COUNT)
      return API_BM_ILLEGAL_SUBADDR;

   if (tr >= RT_TRANREC_COUNT)
      return API_BM_ILLEGAL_TRANREC;

   /*******************************************************************
   *  Get address of current entry in fbuf, then get the address of
   *   the current cbuf.
   *******************************************************************/
   // Get address of current entry in fbuf.
   addr_fbuf = BTMEM_BM_FBUF +        // Byte addr of the beginning of fbuf.
       (rtaddr << 7) + (tr << 6) + (subaddr << 1 ); // Byte offset of entry.

   // Read BM Filter Buffer entry to get cbuf word address.
   vbtRead(cardnum, (LPSTR)&waddr, addr_fbuf, sizeof(BT_U16BIT));

   /*******************************************************************
   *  If this CBUF is the same as the first default CBUF, do not
   *   allocate another CBUF, just point the FBUF entry at the
   *   first default CBUF and return success.V4.06.ajh
   *******************************************************************/
   if ( (api_cbuf->pass_count == 1) &&
        (api_cbuf->t.wcount == 0xFFFFFFFFL) )
   {
      if ( waddr != (BT_U16BIT)(BTMEM_BM_CBUF_DEF >> 1) )  // If not the default,
      {
         // Point this filter buffer entry at the first default CBUF
         // Update CBUF's address in FBUF
         // The byte address in FBUF for this CBUF is in "addr_fbuf"
         waddr = (BT_U16BIT)(BTMEM_BM_CBUF_DEF >> 1);
         vbtWrite(cardnum, (LPSTR)&waddr, addr_fbuf, sizeof(BT_U16BIT));
      }
      return API_SUCCESS;
   }

   /*******************************************************************
   *  If this CBUF is the same as the second default CBUF, do not
   *   allocate another CBUF, just point the FBUF entry at the
   *   second default CBUF and return success.V4.06.ajh
   *******************************************************************/
   if ( (api_cbuf->pass_count == 1) &&
        (api_cbuf->t.wcount == 0x00000000L) )
   {
      if ( waddr != (BT_U16BIT)((BTMEM_BM_CBUF_DEF+sizeof(BM_CBUF)) >> 1) )
      {
         // Point this filter buffer entry at the first default CBUF
         // Update CBUF's address in FBUF
         // The byte address in FBUF for this CBUF is in "addr_fbuf"
         waddr = (BT_U16BIT)((BTMEM_BM_CBUF_DEF+sizeof(BM_CBUF)) >> 1);
         vbtWrite(cardnum, (LPSTR)&waddr, addr_fbuf, sizeof(BT_U16BIT));
      }
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Copy information from caller's buffer to local hw compatible cbuf
   *******************************************************************/
   cbuf.wcount      = flips(api_cbuf->t.wcount);
   cbuf.pass_count  = api_cbuf->pass_count;
   cbuf.pass_count2 = api_cbuf->pass_count;

   /*******************************************************************
   *  If this is the first time any new CBUF has been written,
   *  store the CBUF address (for memory retrievals)
   *******************************************************************/
   if ( btmem_bm_cbuf[cardnum] == 0 )
      btmem_bm_cbuf_next[cardnum] = btmem_bm_cbuf[cardnum] = btmem_tail1[cardnum];

   /*******************************************************************
   *  If this is the first time for the specified subunit since BM_Init()
   *   (e.g., this RT/SA/TR FBUF entry points at the default CBUF),
   *  allocate the CBUF and update FBUF address to point to the new CBUF
   *******************************************************************/

   if ( waddr != (BT_U16BIT)(BTMEM_BM_CBUF_DEF >> 1) )  // If not the default,
   {
      /*******************************************************************
      *  Store new cbuf in hardware.
      *******************************************************************/
      addr = (waddr << 1) + BTMEM_BM_BEGIN;    // Byte addr of cbuf
      vbtWrite(cardnum, (LPSTR)&cbuf, addr, sizeof(cbuf));
   }
   else
   {
      /*******************************************************************
      *  Allocate a CBUF and update FBUF address to point to the CBUF.
      *******************************************************************/
      addr = btmem_tail1[cardnum];

      // Check for memory overflow.  Addresses above 64 K words not supported!
      if ((addr + sizeof(BM_CBUF)) > 0x20000L)
         return API_BM_MEMORY_OFLOW;

      // Update next avail location in memory
      btmem_tail1[cardnum] += sizeof(BM_CBUF);

      // Output CBUF to the board.
      vbtWrite(cardnum,(LPSTR)&cbuf,addr,sizeof(cbuf));

      // Update CBUF's address in FBUF
      // The byte address in FBUF for this CBUF is in "addr_fbuf"
      waddr = (BT_U16BIT)(addr >> 1);  // Word address of the new CBUF.
      vbtWrite(cardnum, (LPSTR)&waddr, addr_fbuf, sizeof(BT_U16BIT));

      // Show end of CBUF area
      btmem_bm_cbuf_next[cardnum] = btmem_tail1[cardnum];
   }
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BM_Init
*
*  FUNCTION
*     This procedure performs full initialization of BM functionality.
*     Note that BusTools_API_Init must have already been called
*     (in order to initialize the board and reset memory allocations).
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_INITED           -> BM_Init already called
*     API_BM_RUNNING          -> BM currently running
*     API_BC_RUNNING          -> BC is running
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT enable_a,
   BT_UINT enable_b)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    rtaddr,tr,subaddr;  // Indexes to BM_FBUF entries.
   BT_U16BIT  value;              // Temp for enable/disable BM channels
   BT_U32BIT  addr;               // General byte address.
   BM_CBUF    cbuf;
   BT_U16BIT  fbuf_entry;         // Current BM_FBUF entry.

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(channel_is_shared[cardnum])
   {
      vbtReadRAM8(cardnum,(LPSTR)&bm_inited[cardnum], BTMEM_CH_SHARE + SHARE_BM_INITED ,1);
      vbtReadRAM8(cardnum,(LPSTR)&bm_running[cardnum],BTMEM_CH_SHARE + SHARE_BM_RUNNING,1);
      vbtReadRAM8(cardnum,(LPSTR)&rt_running[cardnum],BTMEM_CH_SHARE + SHARE_RT_RUNNING,1);
      vbtReadRAM8(cardnum,(LPSTR)&bc_running[cardnum],BTMEM_CH_SHARE + SHARE_BC_RUNNING,1);
      if(bm_inited[cardnum])
         return API_BM_INITED;
   }
   else
   {

      if (bc_running[cardnum])
        return API_BC_RUNNING;
 
      if (bm_running[cardnum])
         return API_BM_RUNNING;

      if (rt_running[cardnum])
         return API_RT_RUNNING;
   }

   /*******************************************************************
   *  Reset the board, shut off the BC, BM and RT, clear BC Busy.
   *  Part of BM initialization is total reallocation of SEG1 memory.  
   *  This is also done by API Init
   *******************************************************************/ 
   if((enable_a < 0xf) && (enable_b < 0xf))
      BusTools_InitSeg1(cardnum);
 
   // Turn off bus loading calculations.
   bm_isBusLoading[cardnum] = 0;
   BM_INT_ON_RTRT_TX[cardnum] = 0;
   MultipleBMTrigEnable[cardnum] = 0;
   
   /*******************************************************************
   *  Fill in default Bus Monitor Control Buffers (default CBUFs)
   *******************************************************************/
   cbuf.wcount      = 0xFFFFFFFFL;  // All word counts enabled
   cbuf.pass_count  = 1;
   cbuf.pass_count2 = 1;
   vbtWrite(cardnum,(LPSTR)&cbuf,BTMEM_BM_CBUF_DEF,sizeof(cbuf));
   cbuf.wcount      = 0x00000000L;  // All word counts disabled

   vbtWrite(cardnum,(LPSTR)&cbuf,BTMEM_BM_CBUF_DEF+sizeof(cbuf),sizeof(cbuf));

   /*******************************************************************
   *  Clear out BM Filter Buffer; point all entries to the default CBUF
   *******************************************************************/
   fbuf_entry = (BT_U16BIT)(BTMEM_BM_CBUF_DEF >> 1);
   for ( rtaddr = 0; rtaddr < BM_ADDRESS_COUNT; rtaddr++ )
   {
      for ( tr = 0; tr < BM_TRANREC_COUNT; tr++ )
      {
         for ( subaddr = 0; subaddr < BM_SUBADDR_COUNT; subaddr++ )
         {
            addr = BTMEM_BM_FBUF + // Byte addr of the current fbuf entry.
                   (rtaddr << 7) + (tr << 6) + (subaddr << 1);
            // Write BM Filter Buffer entry (e.g., default cbuf word address)
            //   for the current RT/TR/SA.
            vbtWrite(cardnum, (LPSTR)&fbuf_entry, addr, sizeof(fbuf_entry));
         }
      }
   }
   
   /*******************************************************************
   *  Clear out error counters
   *******************************************************************/
   BusTools_ErrorCountClear(cardnum);

   /*******************************************************************
   *  Enable/disable Bus Monitor channels
   *******************************************************************/
   value = vbtGetFileRegister(cardnum, RAMREG_ORPHAN);

   if((_HW_FPGARev[cardnum] & 0xfff) < 0x499)
   {
      value &= (BT_U16BIT)~RAMREG_ORPHAN_BM_DISABLE_A;
      value &= (BT_U16BIT)~RAMREG_ORPHAN_BM_DISABLE_B;
   }

   vbtSetFileRegister(cardnum, RAMREG_ORPHAN, (BT_U16BIT)value);

   /*******************************************************************
   *  Clear count of BM messages, BM running flag, and recorder window.
   *******************************************************************/
   bm_count[cardnum]    = 0;
   bm_running[cardnum]  = 0;

   /*******************************************************************
   *  Set the inerrupt queue pointer last to begging of queue
   *******************************************************************/
   iqptr_bm_last[cardnum] = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;

#ifdef USE_BM_DMA
   /*******************************************************************
   *  Set up the DMA Buffers
   *******************************************************************/
   vbtDmaInit(cardnum);
#endif

   /*******************************************************************
   *   Initialize timetag correction factors, init H/W BM time counter.
   *   Moved to BusTools_BM_StartStop() Start function.V3.30.ajh
   *******************************************************************/
   bm_inited[cardnum] = 1;        // Report BM initialization complete.

   if(channel_is_shared[cardnum])
      vbtWriteRAM8(cardnum,(LPSTR)&bm_inited[cardnum],BTMEM_CH_SHARE + SHARE_BM_INITED,1); 

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_BM_MessageAlloc
*
*  FUNCTION
*     This routine allocates and initializes the specified
*     number of BM Message Buffers in the BT memory.  If the
*     specified number of Message Buffers will not fit, (or
*     the caller specified "-1" buffers), the routine will
*     allocate as many buffers as will fit into the remaining
*     memory in the SEG1 segment.
*
*     The routine returns the number of Message Buffers actually
*     allocated to the caller.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_NOTINITED        -> BM_Init not yet called
*     API_BM_RUNNING          -> BM currently running
*     API_BM_MEMORY_OFLOW     -> BM memory overflow
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BM_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mbuf_count,
   BT_UINT   * mbuf_actual,
   BT_U32BIT enable)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_UINT      i;                  // Loop index
   BT_U32BIT    mbuf_possible;      // Number of MBUF's we have room for.
   BT_U16BIT    mbuf_reg;           // First buffer address; register format;
   BT_U32BIT    addr;               // Address of next MBUF in the chain.
   BT_U32BIT    bytes_available;    // Bytes available in segment.
   BM_MBUF      mbuf;               // MBUF used to initialize the board.

   /************************************************
   *  Check initial and error conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   if (bm_inited[cardnum] == 2)
      return API_BM_MSG_ALLOC_CALLED;

   if (bm_running[cardnum])
      return API_BM_RUNNING;

   if(channel_is_shared[cardnum])
   {
      vbtReadRAM(cardnum,(char *)&btmem_bm_mbuf_next[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_BM_MBUF_NEXT,4);
      vbtReadRAM(cardnum,(char *)&btmem_pci1553_next[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_NEXT,4);
   } 

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBM_MessageAlloc[cardnum] )
   {
      i = (*pUsrBM_MessageAlloc[cardnum])(cardnum, &mbuf_count, mbuf_actual, &enable);
      if ( i == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( i == API_NEVER_CALL_AGAIN )
         pUsrBM_MessageAlloc[cardnum] = NULL;
      else if ( i != API_CONTINUE )
         return i;
   }
#endif

   /*******************************************************************
   *  Figure out how many MBUF's will fit into available memory.
   *******************************************************************/

   // Round up the starting address to a multiple of 8 words (16 bytes).
   btmem_pci1553_next[cardnum] = (btmem_pci1553_next[cardnum] + 15) & 0xFFFFFFF0;
   addr = btmem_pci1553_next[cardnum];  
   nBM_MBUF_len[cardnum] = (sizeof(BM_MBUF)+15) & ~0x000F;   // Pad to multiple of 8 words
   bytes_available = btmem_pci1553_rt_mbuf[cardnum] - addr;  // To top of memory.

   if ( bytes_available < 3*nBM_MBUF_len[cardnum] )
      return API_BM_MEMORY_OFLOW;        // Must be able to allocate 3 buffers!

   /* Save the starting location of the BM Message Buffers */

   btmem_bm_mbuf[cardnum] = addr;
   mbuf_possible = (BT_U16BIT)(bytes_available / nBM_MBUF_len[cardnum]);

   if ( mbuf_count < 3 )              // Always allocate at least 3 buffers.
      mbuf_count = 3;

   if ( mbuf_count > mbuf_possible )  // Covers the case of mbuf_count == ffff
      mbuf_count = (BT_UINT)mbuf_possible;

   /*******************************************************************
   *  Allocate the required number of buffers.  Initialize to zero,
   *   especially the timetag, except for the interrupt enables.
   *******************************************************************/
   enable &= BM_INTERRUPT_ENABLES_VALID;       // Zero any invalid bits

   memset((void*)&mbuf, 0, sizeof(BM_MBUF));
   // Eliminate any problems with mis-aligned longs by doing a memcpy:
   // int_status is used by the software(emulation) version of driver.V4.21
   flip(&enable);
   memcpy((void *)&mbuf.int_enable, (void *)&enable, sizeof(enable));
   memcpy((void *)&mbuf.int_status, (void *)&enable, sizeof(enable));
   for (i = 0; i < mbuf_count; i++)
   {
      // Insert the WORD or 8-WORD (PCI-1553) address of the next buffer.
      if (i == mbuf_count-1)
         mbuf.next_mbuf = (BT_U16BIT)(btmem_bm_mbuf[cardnum] >> hw_addr_shift[cardnum]);
      else
         mbuf.next_mbuf = (BT_U16BIT)((addr + nBM_MBUF_len[cardnum]) >> hw_addr_shift[cardnum]);

      vbtWriteRAM(cardnum, (LPSTR)&mbuf, addr, sizeof(BM_MBUF));
      addr += nBM_MBUF_len[cardnum];
   }
   btmem_bm_mbuf_next[cardnum] = addr;
   btmem_pci1553_next[cardnum] = addr;

   /*******************************************************************
   *  Return actual number of buffers allocated to caller.
   *******************************************************************/
   bm_count[cardnum] = mbuf_count;
   *mbuf_actual      = mbuf_count;

   /*******************************************************************
   *  Store address of beginning of BM_MBUF's in hardware register.
   *  Use either a WORD address or an 8-WORD address if PCI-1553 board.
   *******************************************************************/

   mbuf_reg = (BT_U16BIT)(btmem_bm_mbuf[cardnum] >> hw_addr_shift[cardnum]);

   vbtSetFileRegister(cardnum, RAMREG_BM_PTR_SAVE, mbuf_reg);
   vbtSetFileRegister(cardnum, RAMREG_BM_MBUF_PTR, mbuf_reg);

   bmq_ptr_last[cardnum] = mbuf_reg;

   bm_inited[cardnum] = 2;      // Report buffers have been allocated.
   if(channel_is_shared[cardnum])
   {
      vbtWriteRAM8(cardnum,(LPSTR)&bm_inited[cardnum],BTMEM_CH_SHARE + SHARE_BM_INITED,1);
      vbtWriteRAM(cardnum,(char *)&btmem_bm_mbuf_next[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_BM_MBUF_NEXT,4);
      vbtWriteRAM(cardnum,(char *)&btmem_pci1553_next[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_NEXT,4);
   } 

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BM_MessageConvert
*
*  FUNCTION
*     This routine converts a raw BM message buffer into the
*     API_BM_MBUF structure.  The contents of the control word
*     in the message buffer must be used when interpreting the
*     buffer.
*
*  RETURNS
*     nothing.
*
*  Note: This routine takes about 35-130 us on a 486DX4/100,
*        depending on the message length (32 word RT-RT is the 
*        worst case).  This assumes that this routine is copying
*        data directly from the board.  If the data exists in a
*        PC-resident buffer, this routine is faster.
*
* 01/12/1999 Clear 1553 status word for broadcast message.V3.00.ajh
* 01/14/1999 Clear all unused status words for all msg types.V3.02.ajh
* 10/30/1999 Modify for 48-bit time tag.V3.30.ajh
****************************************************************************/
static void BM_MessageConvert(
   BT_UINT     cardnum,         // (i) card number (0 based)
   BT_U32BIT   mbuf_hw_offset,  // (i) Offset to the BM message on the board
   API_BM_MBUF * mbuf_user)// (o) Pointer to user-supplied output buffer
{
   /*******************************************************************
   *  Local variables.
   *******************************************************************/
   BT_INT          count;      // Count of data words in this message.
   BT_INT          i,j;        // Word counters
   BT_INT          mode;       // Indentifies the particular 1553 message.
   BT_INT          saddr;      // Subaddress (for mode code determination).
   BT_U16BIT       wData;      // Used to force a WORD read from the board.
   union{
      BT_U32BIT      *lpZero;     // Used to zero unused parameters.
      BT1553_COMMAND *cmd;
   }mem;
   BT1553_COMMAND  command1;   // First 1553 command word.
   BT_U16BIT      *data;       // Pointer to the variable portion of message.
   BM_MBUF        *mbuf_PCI;   // Pointer to the RAW data


   /*******************************************************************
   *  Get a pointer to the board which maps the current message.
   *  By using the pointer we only transfer the words we need to read.
   *******************************************************************/

#ifdef USE_BM_DMA
   mbuf_PCI = (BM_MBUF *)mbuf_hw_offset;
#else
   char  local_board_buffer[sizeof(BM_MBUF)]; // Scratch buffer

   mbuf_PCI = (BM_MBUF *)vbtGetPagePtr(cardnum, mbuf_hw_offset,
                                           sizeof(BM_MBUF),
                                           local_board_buffer);
#endif

   /*******************************************************************
   *  Transfer common fixed position data (first 9/10 words).
   *  mbuf_user->messno must be filled in by caller.
   *******************************************************************/
   mbuf_user->int_status = mbuf_PCI->int_status;

#ifdef _BM_USER_DATA_
   mbuf_user->user1 = mbuf_PCI->user_data1;
   mbuf_user->user2 = mbuf_PCI->user_data2;
#endif // _BM_USER_DATA_

   flip(&mbuf_user->int_status);   // for Big Endian targets

   /*******************************************************************
   *  Move the time tag (3 words for all boards)
   *  Move the fixed definition words which are past the time tag.
   *  Get the pointer to the variable definition words.V3.30.ajh
   *******************************************************************/

   TimeTagConvert(cardnum, &(mbuf_PCI->time_tag), &(mbuf_user->time));
   command1 = mbuf_user->command1 = mbuf_PCI->command1;
   mbuf_user->status_c1           = mbuf_PCI->status_c1;
   data =          (BT_U16BIT *)mbuf_PCI->data;

   AddTrace(cardnum, NBM_MESSAGECONVERT, mbuf_hw_offset,
            mbuf_user->time.topuseconds, mbuf_user->time.microseconds, 0, 0);

   /*******************************************************************
   *  Clear parameters in user's buffer which otherwise might not be cleared.
   *    This code is faster than memset() (6 instr).V3.02/V3.22.ajh.
   *******************************************************************/
   mem.cmd = &(mbuf_user->command2);
   memset(mem.lpZero,0,16);

   /*******************************************************************
   *  Determine message type -- mode word has following bits:
   *     1 -> RT->BC (0 -> BC->RT)
   *     2 -> BROADCAST
   *     4 -> MODECODE
   *     8 -> RT->RT
   ******************************************************************/

   // What about high word errors??????
   if ( (count = command1.wcount) == 0 )
      count = 32;

   mode = command1.tran_rec;         // Set LSB if TRANSMIT, else clear LSB.
   if ( rt_bcst_enabled[cardnum] && (command1.rtaddr == 31) ) // 3/19/98 ajh
      mode |= 2;                     // Set if Broadcast

   saddr = command1.subaddr;         // 3/19/98 ajh
   if ( saddr == 0 )
      mode |= 4;                     // Set if Mode Code
   else if ( rt_sa31_mode_code[cardnum] && (saddr == 31) )
      mode |= 4;                     // Set if Mode Code

   if (mbuf_user->int_status & BT1553_INT_RT_RT_FORMAT)
      mode |= 8;                     // Set if RT to RT transfer

   /*******************************************************************
   *  Handle rest of message according to message type decoded above.
   *  This case statement moves the data from the board into the user
   *  buffer.  This code is targeted toward 32-bits: Oh, well...
   *******************************************************************/

   i = 0;
   switch( mode )
   {
      case 0:     // BC->RT message
         for (j=0; j<count; j++)
         {
            mbuf_user->value[j]    = data[i++];
            wData                  = data[i++];       // Force a WORD read but
            mbuf_user->status[j]   = (BT_U8BIT)wData; //  only store a BYTE.
         }
         //i = 2*count;
         wData                     = data[i++];       // Force a WORD read but
         mbuf_user->response1.time = (BT_U8BIT)wData; // only store a BYTE.V4.30
         mbuf_user->status1        = *((BT1553_STATUS *)&data[i++]);
         mbuf_user->status_s1      = data[i];
         break;

      case 1:     // RT->BC message
         wData                     = data[0];         // Force a WORD read but
         mbuf_user->response1.time = (BT_U8BIT)wData; // only store a BYTE.V4.30
         mbuf_user->status1        = *((BT1553_STATUS *)&data[1]);
         mbuf_user->status_s1      = data[2] ;
         i = 3;
         for (j=0; j<count; j++)
         {
            mbuf_user->value[j]    = data[i++];
            wData                  = data[i++];       // Force a WORD read but
            mbuf_user->status[j]   = (BT_U8BIT)wData; //  only store a BYTE.
         }
         break;

      case 2:     // BROADCAST BC->RT message [10]
         for (j=0; j<count; j++)
         {
            mbuf_user->value[j]    = data[i++];
            wData                  = data[i++];       // Force a WORD read but
            mbuf_user->status[j]   = (BT_U8BIT)wData; //  only store a BYTE.
         }
         break;

      case 4:     // Modecode (receive -- always has one data word in 155B Mode and 0 in 1553A mode)
         if(bt_op_mode[cardnum] == 0x0)
		 {
		    mbuf_user->value[0]       = data[0];
            wData                     = data[1];         // Force a WORD read but
            mbuf_user->status[0]      = (BT_U8BIT)wData; // only store a BYTE.
            wData                     = data[2];         // Force a WORD read but		 
            mbuf_user->response1.time = (BT_U8BIT)wData; // only store a BYTE.V4.30
            mbuf_user->status1        = *((BT1553_STATUS    *)&data[3]);
            mbuf_user->status_s1      =                        data[4] ;
         }
		 else
		 {
            wData                     = data[0];         // Force a WORD read but		 
            mbuf_user->response1.time = (BT_U8BIT)wData; // only store a BYTE.V4.30
            mbuf_user->status1        = *((BT1553_STATUS    *)&data[1]);
            mbuf_user->status_s1      =                        data[2] ;
         }
         break;

      case 5:     // Modecode (transmit -- either with or without data)
         wData                     = data[0];         // Force a WORD read but
         mbuf_user->response1.time = (BT_U8BIT)wData; // only store a BYTE.V4.30
         mbuf_user->status1        = *((BT1553_STATUS    *)&data[1]);
         mbuf_user->status_s1      =                        data[2] ;
         mbuf_user->value[0]       = data[3];         // Move data anyway
         wData                     = data[4];         // Force a WORD read but
         mbuf_user->status[0]      = (BT_U8BIT)wData; //  only store a BYTE.
         break;

      case 6:     // BROADCAST Modecode (receive -- always has one data word)
         mbuf_user->value[0]       = data[0];
         wData                     = data[1];         // Force a WORD read but
         mbuf_user->status[0]      = (BT_U8BIT)wData; //  only store a BYTE.
         break;

      case 8:     // RT->RT command data[0] is word [10] for PCI1553
         mbuf_user->command2       = *((BT1553_COMMAND   *)&data[0]);
         mbuf_user->status_c2      =                        data[1] ;
         wData                     = data[2];         // Force a WORD read but
         mbuf_user->response1.time = (BT_U8BIT)wData; // only store a BYTE.V4.30
         mbuf_user->status1        = *((BT1553_STATUS    *)&data[3]);
         mbuf_user->status_s1      =                        data[4] ;
         i = 5;
         for (j=0; j<count; j++)
         {
            mbuf_user->value[j]    = data[i++];
            wData                  = data[i++];       // Force a WORD read but
            mbuf_user->status[j]   = (BT_U8BIT)wData; //  only store a BYTE.
         }
         //  i = 2*count + 5;
         wData                     = data[i++];       // Force a WORD read but
         mbuf_user->response2.time = (BT_U8BIT)wData; // only store a byte.
         mbuf_user->status2        = *((BT1553_STATUS *)&data[i++]);
         mbuf_user->status_s2      =                     data[i] ;
         break;

      case 10:    // BROADCAST RT->RT command.V4.01.ajh
         mbuf_user->command2       = *((BT1553_COMMAND   *)&data[0]);
         mbuf_user->status_c2      =                        data[1] ;
         wData                     = data[2];         // Force a WORD read but
         mbuf_user->response1.time = (BT_U8BIT)wData; // only store a BYTE.V4.30
         mbuf_user->status1        = *((BT1553_STATUS    *)&data[3]);
         mbuf_user->status_s1      =                        data[4] ;
         // Second command status word (status2 and status_s2)(receive status)
         //  is NULL for Broadcast RT-RT.

         i = 5;
         for (j=0; j<count; j++)
         {
            mbuf_user->value[j]    = data[i++];
            wData                  = data[i++];       // Force a WORD read but
            mbuf_user->status[j]   = (BT_U8BIT)wData; //  only store a BYTE.
         }
         break;

      case 7:     // BROADCAST Modecode (transmit -- only without data)
         break;

      case 3:     // ILLEGAL (BROADCAST RT->BC message)
      case 9:     // ILLEGAL (RT->RT with first command being transmit)
      case 11:    // ILLEGAL (BROADCAST RT->RT, first command being transmit)
      case 12:    // ILLEGAL (RT->RT and Modecode)
      case 13:    // ILLEGAL (RT->RT and Modecode)
      case 14:    // ILLEGAL (RT->RT and Broadcast and Modecode)
      case 15:    // ILLEGAL (RT->RT and Broadcast and Modecode)
         //return;       // API_BM_ILLEGAL_MESSAGE;
         break;
   }
   return;               // API_SUCCESS;
}

/****************************************************************
*
* PROCEDURE NAME - BusTools_BM_MessageGetaddr()
*
* FUNCTION
*     This routine converts a BM message id to a BT address.
*     Note that the address is a byte address (not a BusTools
*     word address such as from the interrupt queue).
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_MessageGetaddr(
   BT_UINT   cardnum,           // (i) card number (0 based)
   BT_UINT mbuf_id,
   BT_U32BIT * addr)
{
   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] != 2)
      return API_BM_NOTINITED;

   if (mbuf_id >= bm_count[cardnum])
      return API_BM_ILLEGAL_MBUFID;

   /*******************************************************************
   *  Calculate and return the address of the buffer given
   *   the ID number which starts with zero:
   *******************************************************************/
   *addr = btmem_bm_mbuf[cardnum] + (BT_U32BIT)mbuf_id * nBM_MBUF_len[cardnum];
   return API_SUCCESS;
}

/****************************************************************
*
* PROCEDURE NAME - BusTools_BM_MessageGetid()
*
* FUNCTION
*     This routine converts a BT1553 address to a BM message
*     id.  Note that the address is a byte address (not a BusTools
*     word address) such as from the interrupt queue.  This address
*     is internally multiplied by 8 if this is the PCI-1553!
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_MessageGetid(
   BT_UINT   cardnum,           // (i) card number (0 based)
   BT_U32BIT     addr,
   BT_UINT * messageid)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_UINT i;

   /************************************************
   *  Check initial conditions
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] != 2)
      return API_BM_NOTINITED;

   /***********************************************************************
   *  Calculate the index of the buffer, given the buffer address,
   *  the starting address of all buffers, and the length of a single
   *  buffer.  btmem_bm_mbuf contains [MAX_BTA] elements, e.g., the
   *  starting addresses of the BM buffers in the [MAX_BTA] cards....ajh
   * Convert byte address to *8 address if this is the PCI-1553 card.
   ***********************************************************************/
   addr <<= hw_addr_shift[cardnum] - 1;
   *messageid = i = (BT_U16BIT)( (addr - btmem_bm_mbuf[cardnum]) / nBM_MBUF_len[cardnum] );

   if ( ((addr - btmem_bm_mbuf[cardnum]) % nBM_MBUF_len[cardnum]) ||
         (i > bm_count[cardnum])  )
      return API_BM_MBUF_NOMATCH;
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - BusTools_BM_MessageRead
*
*  FUNCTION
*     This routine transfers the contents of the specified BM
*     Message Buffer to the caller supplied structure.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_NOTINITED        -> BM_Init not yet called
*     API_BM_ILLEGAL_MBUFID   -> BM illegal mbuf_id
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_MessageRead(
   BT_UINT   cardnum,           // (i) card number (0 based)
   BT_UINT   mbuf_id,
   API_BM_MBUF * api_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT addr;
   long      message_number;

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] != 2)
      return API_BM_NOTINITED;

   if (mbuf_id >= bm_count[cardnum])
      return API_BM_ILLEGAL_MBUFID;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   {
      BT_INT status;
      if ( pUsrBM_MessageRead[cardnum] )
      {
         status = (*pUsrBM_MessageRead[cardnum])(cardnum, &mbuf_id, api_mbuf);
         if ( status == API_RETURN_SUCCESS )
            return API_SUCCESS;
         else if ( status == API_NEVER_CALL_AGAIN )
            pUsrBM_MessageRead[cardnum] = NULL;
         else if ( status != API_CONTINUE )
            return status;
      }
   }
#endif

   /*******************************************************************
   *  Read the specified message buffer.  First calculate the address
   *  of the buffer, given the ID number which starts with zero:
   *******************************************************************/
   addr = btmem_bm_mbuf[cardnum] + (BT_U32BIT)mbuf_id * nBM_MBUF_len[cardnum];

#ifdef USE_BM_DMA 
   vbtSetPLXRegister32(cardnum,PLX_DMA_BOARD_ADDR0, addr+0x100000);
   vbtSetPLXRegister8(cardnum,PLX_DMA_STATUS0, 0x3);
   BM_MessageConvert(cardnum, (BT_U32BIT)bt_devmap[cardnum].laddr, api_mbuf);
#else

   /*******************************************************************
   *  Fetch the message from the board, convert it, store it in api_mbuf.
   *******************************************************************/
   if(board_is_paged[cardnum])
     vbtAcquireFrameRegister(cardnum, 1);   // Acquire frame register
   
   BM_MessageConvert(cardnum, addr, api_mbuf);
#endif

   api_mbuf->int_status &= BM_INTERRUPT_STATUS_VALID1;
   
   if(board_is_paged[cardnum])
      vbtAcquireFrameRegister(cardnum, 0);   // Release frame register

   /*******************************************************************
   *  Fill in message number in top of message.  Properly
   *   handle the wrap around of the message buffers.V4.30
   *******************************************************************/
   // Compute the offset from current BM message to the specified message.
   message_number = mbuf_id - ((bm_messaddr[cardnum] - btmem_bm_mbuf[cardnum]) /
                                nBM_MBUF_len[cardnum]) + 1;
   if ( message_number >= (long)bm_count[cardnum] )
      message_number = 0;

   // Compute the message number of the specified message
   message_number += bm_messno[cardnum] - 1;

   // If the message number does not make any sense, increment it by the
   //  number of messages in the BM buffer list.
   if ( message_number < 0 )
      message_number += bm_count[cardnum];

   // Return the computed message number to the caller
   api_mbuf->messno = message_number;

   AddTrace(cardnum, NBUSTOOLS_BM_MESSAGEREAD, mbuf_id,
            api_mbuf->messno, *(BT_INT *)&api_mbuf->command1, 0, 0);
   return API_SUCCESS;
}

/**********************************************************************
*
*  PROCEDURE - BusTools_BM_MessageReadBlock
*
*  FUNCTION
*     This routine transfers all BM_MBUF's received since the last call
*     to the routine.  The caller supplies the buffer into which the
*     data will be stored (an array of API_BM_MBUF's used as a circular
*     buffer).  This routine returns the number of messages actually
*     transferred.  Note that no more than 64 Kbytes will be copied per
*     call to this routine.
*
*     In addition, this is the routine which updates the cummulative
*     error counters (see the BusTools_BM_ecount_xxx routines).
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_NOTINITED        -> BM_Init not yet called
*     API_BM_WRAP_AROUND      -> BM buffer has wrapped-around
*
***********************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_MessageReadBlock(
   BT_UINT   cardnum,           // (i) card number (0 based)
   API_BM_MBUF * api_mbuf, // (i) address of caller's array of BM_MBUF's
   BT_UINT size,                // (i) number of BM_MBUF's in array
   BT_UINT curpos,              // (i) next avail location in array
   BT_UINT * ret_count)     // (o) number of BM_MBUF's transferred
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT     count;            // Number of messages transfered.
   BT_INT      nTail, nHead;
   API_BM_MBUF *apiaddr;    // Sequencing address within API array.

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] != 2)
      return API_BM_NOTINITED;

   /*******************************************************************
   *  Get current position of the software within the API message queue,
   *  which is the tail pointer.
   *******************************************************************/
   nTail = nAPI_BM_Tail[cardnum];
   nHead = nAPI_BM_Head[cardnum];

   /*******************************************************************
   *  Transfer data until we reach nHead, or the user's buffer is full.
   *******************************************************************/
   apiaddr = (API_BM_MBUF *)lpAPI_BM_Buffers[cardnum];    // API buffer starts here.
   count   = 0;                 // Number of messages we gave to caller.
   while ( nHead != nTail )
   {
      // Copy the message from the API buffer to the user's buffer.
      // Modern compilers do nearly as well as the in-line assembly
      //  code we used in previous versions!  (Finally!)
      // 16-bit code is, well, not so good, but machines are faster.
      api_mbuf[curpos] = apiaddr[nTail];

      // Step to next API buffer.  Handle wrap of circular API buffer.
      nTail++;
      nTail &= NAPI_BM_BUFFERS - 1;

      count++;          // Increment count of msgs returned to caller.
      // We never want to return more than 64 K worth of data.
      if ( (count >= size) || (count >= BM_MAX_MSGS_PER_READ_BLOCK) )
         break;         // Exit the while loop, caller's buffer is full.

      // Step to next buffer in caller's buffer.  Handle wrap-around.
      curpos++;         // Increment index of message in caller's buffer.
      if ( curpos >= size )    // If index is outside of user's array,
         curpos = 0;           //  reset index to first message in buffer.
   }  // end of while ( nHead != nTail )
   nAPI_BM_Tail[cardnum] = nTail;    // Update tail pointer.

   /*******************************************************************
   *  Return the count of API_BM_MBUF's read to caller
   *******************************************************************/
   *ret_count = count;
   if ( (bm_hw_queue_ovfl[cardnum] & (BM_API_OFLOW|BM_API_OFLOW_MSG))
         == BM_API_OFLOW )
   {  // Report that the software buffer has overflowed, only once.
      bm_hw_queue_ovfl[cardnum] |= BM_API_OFLOW_MSG;
      return API_BM_WRAP_AROUND;      // We only report overflow once.
   }
   else if ( (bm_hw_queue_ovfl[cardnum] & (BM_HW_OFLOW|BM_HW_OFLOW_MSG))
              == BM_HW_OFLOW )
   {  // Report that the hardware buffer has overflowed, only once.
      bm_hw_queue_ovfl[cardnum] |= BM_HW_OFLOW_MSG;
      return API_BM_HW_WRAP_AROUND;
   }
   else if ( (bm_hw_queue_ovfl[cardnum] & (BM_REG_BAD|BM_REG_BAD_MSG))
              == BM_REG_BAD )
   {  // Report that the hardware buffer has overflowed, only once.
      bm_hw_queue_ovfl[cardnum] |= BM_REG_BAD_MSG;
      return API_BM_POINTER_REG_BAD;
   }
   else
      return API_SUCCESS;
}

/**********************************************************************
*
*  PROCEDURE - ErrorCountUpdate
*
*  FUNCTION
*     This routine updates the bm_ecount[][] array with the current
*     message's status.
*     Call this function with a pointer to the array for the current
*     cardnum to be updated.
*
*  RETURNS
*     nothing
***********************************************************************/
static void ErrorCountUpdate(
   BT_U32BIT   * bm_ecount_cardnum, // (i) pointer to bm_ecount[cardnum]
   BT_U32BIT     int_status,        // (i) value of BM interrupt status
   BT1553_STATUS status1)           // (i) value of first 1553 status word
{
   int    i;        // Loop counter.

   // Array elements 30 and 31 are not used, so use [31] for message count.
   bm_ecount_cardnum[31]++;                     // Message Count.V4.01.ajh

   // Typically only the End Of Message Interrupt bit is set (0x00010000)
   if ( int_status & BT1553_INT_END_OF_MESS )   // EOM Interrupt set?
   {
      bm_ecount_cardnum[16]++;                  // Count it and
      int_status &= ~BT1553_INT_END_OF_MESS;    //  clear it...
   }
   if ( int_status & BT1553_INT_NO_RESP )       // No Response Interrupt set?
   {
      bm_ecount_cardnum[26]++;                  // Count it and
      int_status &= ~BT1553_INT_NO_RESP;        //  clear it...
   }
   else if ( int_status & BT1553_INT_BROADCAST )
   {
      bm_ecount_cardnum[17]++;                  // Count it and
      int_status &= ~BT1553_INT_BROADCAST;      //  clear it...
   }
   else
   {
      if ( status1.sr )                    // 1553 Status: Service Request
         bm_ecount_cardnum[23]++;
      if ( status1.bcr )                   // 1553 Status: Broadcast Received
         bm_ecount_cardnum[24]++;
      if ( status1.busy )                  // 1553 Status: Busy
         bm_ecount_cardnum[25]++;
      if ( status1.me )                    // 1553 Status: Message Error
         bm_ecount_cardnum[27]++;
      if ( status1.sf )                    // 1553 Status: Subsystem Flag
         bm_ecount_cardnum[28]++;
      if ( status1.tf )                    // 1553 Status: Terminal Flag
         bm_ecount_cardnum[29]++;
   }
   for (i = 0; i < 23; i++)     // Only first 23 are set by the BM Hardware,
   {                            //  except for [26] which is handled above...
      if ( int_status & 1 )
         bm_ecount_cardnum[i]++;
      int_status >>= 1;
      if ( int_status == 0 )
         break;                 // If zero, nothing left to count.
   }
}

/****************************************************************************
*
*  PROCEDURE - BM_BusLoading
*
*  FUNCTION
*     This routine computes the amount of time a message takes on the bus.  It
*     totals the command, status and data word times with the response times,
*     then adds a fixed estimate of the intermessage gap time.
*
*     The resulting time in microseconds is used to update a bunch of counters
*     that keep track of the bus loading per RT, SA, et.al.  This function is
*     intended to be fast and not especially accurate in the case of errors!
*
*  RETURNS
*     Nothing.
****************************************************************************/
#if !defined(DO_BUS_LOADING)
#define BM_BusLoading(p1,p2)
#else
static void BM_BusLoading(
   API_BM_MBUF   * msg,     // (i) Current BM message
   API_BUS_STATS * stats)   // (o) Pointer to stats buffer to update
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int        dTime;           // Estimate of the time taken by the message
   int        mode;            // Message type code
   int        rtaddr;          // RT address
   int        tr;              // T/R bit
   int        saddr;           // Subaddress
   int        count;           // Count of data words in message
   int        bus;             // 0 for primary bus, 1 for secondary bus
   int        Response1;       // First measured response time, us.
   int        Response12;      // First plus Second measured response time, us.

   /*******************************************************************
   *  Determine the message type -- mode word has following bit values:
   *     0 -> BC->RT      1 -> RT->BC     2 -> BROADCAST
   *     4 -> MODECODE    8 -> RT->RT
   ******************************************************************/
   // What about high word errors??????
   if ( (count = msg->command1.wcount) == 0 )
      count = 32;

   // Setup the response times, scaled to microseconds and forget rounding.
   // Get rid of the overlap between response time and bus activity time.
   Response1  = (msg->response1.time >> 1) - 2;
   Response12 = (msg->response1.time + msg->response2.time) - 4;

   // Check interrupt status for broadcast.
   rtaddr = msg->command1.rtaddr;
   tr     = msg->command1.tran_rec;
   saddr  = msg->command1.subaddr;

   // Determine what kind of message this is.
   mode = tr;          // Set LSB if transmit, else clear LSB.

   if ( msg->int_status & BT1553_INT_BROADCAST )
      mode |= 2;

   // Check the mode code bit to see if this is a SA 31 mode code, if not
   //  implemented in this firmware check the SA==0 to determine if the
   //  message is a mode code.  Old firmware versions that do not set this
   //  mode code status bit will not correctly report SA 31 mode codes.
   if ( (saddr == 0) || (msg->int_status & BT1553_INT_MODE_CODE) )
      mode |= 4;                     // Set if Mode Code at SA 0 or SA 31

   if ( msg->int_status & BT1553_INT_RT_RT_FORMAT )
      mode |= 8;                     // Set for RT-RT messages

   /*******************************************************************
   * Handle rest of message according to message type decoded above.
   * This case statement computes the bus time used by the specific
   *  message type and length as decoded above.  A single word on the
   *  1553 bus takes 20 us, but there is a 2 us difference between a
   *  measured gap and the bus active time.
   *******************************************************************/
   switch ( mode )
   {
      case 0:     // BC->RT message
         dTime  = 20 + 20*count;// Account for the cmd and data words.
         if ( msg->int_status & BT1553_INT_NO_RESP )
            break;
         dTime += Response1;    // Response time.
         dTime += 20;           // Account for the status word.
         break;

      case 1:     // RT->BC message
         dTime  = 20;           // Account for the cmd word.
         if ( msg->int_status & BT1553_INT_NO_RESP )
            break;
         dTime += 20;           // Account for the status word.
         dTime += Response1;    // Response time.
         if ( msg->status1.me || msg->status1.busy )
            break;              // No data if there was an error.
         dTime += 20*count;     // Account for the data words.
         break;

      case 2:     // BROADCAST BC->RT message
         dTime  = 20 + 20*count;// Account for the cmd and data words.
         break;

      case 4:     // Modecode (receive -- always has one data word)
         dTime  = 20+20 ;       // Account for the cmd + data words.
         if ( msg->int_status & BT1553_INT_NO_RESP )
            break;
         dTime += Response1;    // Response time.
         dTime += 20;           // Account for status word.
         break;

      case 5:     // Modecode (transmit -- either with or without data)
         dTime  = 20;           // Account for the cmd word.
         if ( msg->int_status & BT1553_INT_NO_RESP )
            break;
         dTime += Response1;    // Response time.
         dTime += 20;           // Account for status word.
         if ( msg->status1.me || msg->status1.busy )
            break;              // No data if there was an error.
         if ( (count>=16) & (count<32) ) // Is there is a data word?
            dTime += 20;        // Account for the data word.
         break;

      case 6:     // BROADCAST Modecode (receive -- always has one data word)
         dTime  = 20+20 ;       // Account for the cmd + data words.
         break;

      case 7:     // BROADCAST Modecode (transmit -- only without data)
         dTime  = 20;           // Account for the command word.
         break;

      case 8:     // RT->RT command
         // The BM sets one bit for NR on EITHER first RT or Second RT.
         // This means that we cannot determine WHICH RT did not respond,
         //  and therefore cannot calculate the time the message took.
         // So just wing it...
         dTime  = 20+20;        // Account for the two cmd words.
         if ( msg->int_status & BT1553_INT_NO_RESP )
            break;
         dTime += Response12;   // Response times.
         dTime += 20;           // Account for the first status word.
         if ( msg->status1.me || msg->status1.busy )
            break;              // No data if there was an error.
         dTime += 20*count;     // Account for the data words.
         dTime += 20;           // Account for the second status word.
         break;

      case 10:    // BROADCAST RT->RT command
         dTime  = 20+20;        // Account for the two cmd words.
         if ( msg->int_status & (BT1553_INT_NO_RESP | BT1553_INT_ME_BIT) )
            break;
         dTime += Response1;    // Response time.
         dTime += 20;           // Account for the status word from sending RT.
         if ( msg->status1.me || msg->status1.busy )
            break;                   // No data if there was an error.
         dTime += 20*count;     // Account for the data words.
         break;

      case 3:     // ILLEGAL (BROADCAST RT->BC message)
      case 9:     // ILLEGAL (RT->RT with first command being transmit)
      case 11:    // ILLEGAL (BROADCAST RT->RT, first command being transmit)
      case 12:    // ILLEGAL (RT->RT and Modecode)
      case 13:    // ILLEGAL (RT->RT and Modecode)
      case 14:    // ILLEGAL (RT->RT and Broadcast and Modecode)
      case 15:    // ILLEGAL (RT->RT and Broadcast and Modecode)
         break;       // API_BM_ILLEGAL_MESSAGE;
   }
   // Remainder of function revised V4.41.ajh
   // Update the bus loading arrays with the count of microseconds.
   dTime += 6;        // Add in something for the intermessage gap.
   stats->BusCountsRTSA[rtaddr][saddr] += dTime;
   stats->BusCountsRT[rtaddr]          += dTime;
   stats->BusCounts                    += dTime;

   // Update the command/error counts/flags structure.
   //  This update is for the receive side of an RT-RT message...
   bus = msg->int_status & BT1553_INT_CHANNEL ? 1 : 0;

   if ( msg->int_status & BT1553_INT_RT_RT_FORMAT )
      stats->StatusWordRt[rtaddr] |= *(BT_U16BIT *)&msg->status2;  // Current receive RT status word [RT] (logical OR of status words)
   else
      stats->StatusWordRt[rtaddr] |= *(BT_U16BIT *)&msg->status1;  // Current RT status word [RT] (logical OR of status words)

   stats->CommandCountRtBus[rtaddr][bus]++;  // Current Command Counts [RT][BUS]

   if ( msg->int_status & 0x0000F7FFL )
   {
      stats->ErrorCountRtBus[rtaddr][bus]++;  // Current Error Counts [RT][BUS]
      stats->ErrorCountRtSaBus[rtaddr][saddr][bus]++; // Current Error Counts [RT][SA][BUS]
      stats->ErrorsRTtrSABus[rtaddr][tr][bus] |= 1<<saddr; // Current Error Flags [RT][TR][BUS](one bit per SA)
   }
   if ( msg->int_status & BT1553_INT_NO_RESP )
   {
      stats->NrCountRtBus[rtaddr][bus]++;             // Current No Response Counts [RT][BUS]
      stats->NrCountRtSaBus[rtaddr][saddr][bus]++;    // Current No Response Counts [RT][SA][BUS]
      stats->NRRTtrSABus[rtaddr][tr][bus] |= 1<<saddr; // Current No Response Flags [RT][TR][BUS] (one bit per SA)
   }
   if ( msg->status1.me )
   {
      stats->MeCountRtBus[rtaddr][bus]++;           // Current Message Error Counts [RT][BUS]
      stats->MeCountRtSaBus[rtaddr][saddr][bus]++;  // Current Message Error Counts [RT][SA][BUS]
   }
   stats->ErrorBitMapRt[rtaddr]          |= msg->int_status; // Current Error Bit Map [RT]  (logical OR of errors)
   stats->ErrorBitMapRtSa[rtaddr][saddr] |= msg->int_status; // Current Error Bit Map [RT][SA]  (logical OR of errors)
   stats->CommandCountRtSaBus[rtaddr][saddr][bus]++;       // Current Command Counts [RT][SA][BUS]
   stats->ActivityRTtrSABus[rtaddr][tr][bus] |= 1<<saddr;  // Current Activity Flags [RT][TR][BUS](one bit per SA)
   stats->ActivityRTtrBus[tr][bus]           |= 1<<rtaddr; // Current Activity Flags [TR][BUS] (one bit per RT)

   // If this is an RT-RT message, update the transmitting RT.
   if ( msg->int_status & BT1553_INT_RT_RT_FORMAT )
   {
      rtaddr = msg->command2.rtaddr;
      saddr  = msg->command2.subaddr;
      tr     = 1;
      stats->BusCountsRTSA[rtaddr][saddr] += dTime;
      stats->BusCountsRT[rtaddr]          += dTime;

   if ( (msg->int_status & BT1553_INT_BROADCAST) == 0 )
      stats->StatusWordRt[rtaddr] |= *(BT_U16BIT *)&msg->status1; // Current transmit RT status word [RT] (logical OR of status words)
   stats->CommandCountRtBus[rtaddr][bus]++;           // Current Command Counts [RT][BUS]

   if ( msg->int_status & 0x0000F7FFL )
   {
      stats->ErrorCountRtBus[rtaddr][bus]++;          // Current Error Counts [RT][BUS]
      stats->ErrorCountRtSaBus[rtaddr][saddr][bus]++; // Current Error Counts [RT][SA][BUS]
      stats->ErrorsRTtrSABus[rtaddr][tr][bus] |= 1<<saddr; // Current Error Flags [RT][TR][BUS](one bit per SA)
   }
   if ( msg->int_status & BT1553_INT_NO_RESP )
   {
      stats->NrCountRtBus[rtaddr][bus]++;             // Current No Response Counts [RT][BUS]
      stats->NrCountRtSaBus[rtaddr][saddr][bus]++;    // Current No Response Counts [RT][SA][BUS]
      stats->NRRTtrSABus[rtaddr][tr][bus] |= 1<<saddr; // Current No Response Flags [RT][TR][BUS] (one bit per SA)
   }
   if ( msg->status1.me )
   {
      stats->MeCountRtBus[rtaddr][bus]++;             // Current Message Error Counts [RT][BUS]
      stats->MeCountRtSaBus[rtaddr][saddr][bus]++;    // Current Message Error Counts [RT][SA][BUS]
   }
   stats->ErrorBitMapRt[rtaddr]          |= msg->int_status; // Current Error Bit Map [RT]  (logical OR of errors)
   stats->ErrorBitMapRtSa[rtaddr][saddr] |= msg->int_status; // Current Error Bit Map [RT][SA]  (logical OR of errors)
   stats->CommandCountRtSaBus[rtaddr][saddr][bus]++;  // Current Command Counts [RT][SA][BUS]
   stats->ActivityRTtrSABus[rtaddr][tr][bus] |= 1<<saddr;  // Current Activity Flags [RT][TR][BUS](one bit per SA)
   stats->ActivityRTtrBus[tr][bus]           |= 1<<rtaddr; // Current Activity Flags [TR][BUS] (one bit per RT)
   }
   return;
}
#endif


/****************************************************************************
*
*  PROCEDURE - BM_BusLoadingFilter
*
*  FUNCTION
*     This routine filters the bus loading values recorded by BM_BusLoading.
*
*     The activity counts are used to update the BusLoading values.  This
*     function executes every 210 ms, and computes the bus loading using
*     the following equation, where BusLoading is percent loading and
*     BusCounts is the number of microseconds used by 1553 traffic during
*     the sampling period:
*
*     BusLoading = BusLoading * 3   +  (BusCounts * 100) / 210,000 us
*                  --------------------------------------------------
*                                      4
*
*     Since 100/210000 ~= 1/2048, we can use a shift of 11 to scale BusCounts,
*        this degrades the accuracy by 2100/2048 = ~+2.5 percent (high).
*     Note that we scale the above constants (and subtract one from the shift
*      count for 'BusCounts') to maintain half percent resolution in the result.
*
*  RETURNS
*     Nothing.
****************************************************************************/
#if defined(DO_BUS_LOADING)
void BM_BusLoadingFilter(
   BT_UINT   cardnum)           // (i) card number (0 based)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int           rt;            // RT Address loop counter
   int           sa;            // RT Subaddress loop counter
   API_BUS_STATS *stats;        // Pointer to stats buffer to update
   static int    TimeToUpdate[MAX_BTA] = {0,};

   // If bus loading calculations are turned off return.
   if ( bm_isBusLoading[cardnum] == 0 )
      return;

   if ( --TimeToUpdate[cardnum] > 0 )
      return;
   TimeToUpdate[cardnum] = 21;

   // Get the pointer to the current bus statistics buffer
   stats = &bus_stats[cardnum];

   // Update all of the bus loading percentages, clearing the activity counts.
   stats->BusLoading = ( (stats->BusLoading*12) + (stats->BusCounts>>8) ) >> 4;
   stats->BusCounts  = 0;
   for ( rt = 0; rt < 32; rt++ ) {
      stats->BusLoadingRT[rt] = (BT_U8BIT) (
            ( (stats->BusLoadingRT[rt]*12) + (stats->BusCountsRT[rt]>>8) ) >> 4
                                            );
      stats->BusCountsRT[rt] = 1023;            // Do some rounding
      for ( sa = 0; sa < 32; sa++ ) {
         stats->BusLoadingRTSA[rt][sa] = (BT_U8BIT) (
         ( (stats->BusLoadingRTSA[rt][sa]*12) + (stats->BusCountsRTSA[rt][sa]>>8) ) >> 4
                                                     );
         stats->BusCountsRTSA[rt][sa] = 1023;   // Do some rounding
      }
   }
}
#endif

/****************************************************************************
*
*  PROCEDURE - BM_MsgReadBlock
*
*  FUNCTION
*     This routine transfers all BM_MBUF's recorded since the last call
*     to the routine.  The API supplies the buffer into which the data
*     will be stored (an array of API_BM_MBUF's used as a circular
*     buffer).  This routine updates the buffer head pointer to reflect
*     the number of messages actually transferred from the hardware to
*     the API buffer.
*     If either the hardware queue wraps around, or the API buffer wraps
*     around, this routine will return an error message.
*
*  GLOBALS:
*     BT_INT nAPI_BM_Buffers       is the size of our buffer.
*     API_BM_MBUF lpAPI_BM_Buffers[cardnum] points to our BM buffer.
*     BT_INT nAPI_BM_Head[cardnum] is the head pointer.
*     BT_INT nAPI_BM_Tail[cardnum] is the tail pointer.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BM_WRAP_AROUND      -> BM API buffer has wrapped-around
*     API_BM_WRAP_AROUND1     -> BM hardware buffer has wrapped-around
****************************************************************************/

void BM_MsgReadBlock(
   BT_UINT   cardnum)           // (i) card number (0 based)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT   bmaddr;           // HW buffer sequencing address
   BT_U32BIT   bmaddr_cur;       // Addr of next BM_MBUF in HW to be written
   BT_U16BIT   temp;             // Used for checking HW buffer wrap-around
   BT_INT      nHead, nPrevHead; // Local buffer head pointers.
   API_BM_MBUF *useraddr;   // Sequencing address within API array
   API_BM_MBUF *oflowaddr;  // Overflow message within API array
   int         hw_overflow = 0;  // HW buffer overflow flag
#if defined(DO_BUS_LOADING)
   API_BUS_STATS * stats = &bus_stats[cardnum];  // Pointer to the bus loading buffer
#endif

   /*******************************************************************
   *  Get current position of hardware within BM_MBUF queue, and
   *   convert the BM board address to a byte address.
   *******************************************************************/
   bmaddr_cur = vbtGetFileRegister(cardnum, RAMREG_BM_PTR_SAVE);
   bmaddr_cur <<= hw_addr_shift[cardnum];

   /* Only if timing trace is enabled */
   AddTrace(cardnum, BM_MSGREADBLOCK, 0x000F, bmaddr_cur>>1,
            bm_rec_prev[cardnum]>>1, 0, 0);

   /*******************************************************************
   *  Read the BM buffer pointer from the HW, and verify that it is OK.
   *******************************************************************/
   if ( (bmaddr_cur < btmem_bm_mbuf[cardnum]) ||
        (bmaddr_cur >= btmem_bm_mbuf_next[cardnum]) ||
        ((bmaddr_cur - btmem_bm_mbuf[cardnum]) % nBM_MBUF_len[cardnum] != 0) )
   {
      static int error_reported = 0;
      if ( !error_reported )
      {
#ifdef FILE_SYSTEM
         error_reported = 1;
         sprintf(szMsg,"Register 4C not within/on BM Message Buffers\n"
                 "Cardnum=%i, 2*Reg 4C=%x begin=%x end=%x(Byte Offsets)\n"
                 "Dumping memory to C:\\BUSAPIBM.DMP", cardnum, bmaddr_cur,
                  btmem_bm_mbuf[cardnum], btmem_bm_mbuf_next[cardnum]);
         BusTools_DumpMemory(cardnum, 0xFFFFFFFF, "C:\\BUSAPIBM.DMP", szMsg);
#endif //FILE_SYSTEM

      }
      bm_hw_queue_ovfl[cardnum] |= BM_REG_BAD;
      return;
   }

   /*******************************************************************
   *   Check for hardware queue overflow -- if the hardware bit location
   *   we previously set is zero ("bm_rec_prev[cardnum]"), then we have
   *   a BM HW Buffer wrap-around.
   *   Use the "BM Overflow" bit in the BM Msg Status dword.  We set the
   *   bit in the last message we process, then when we return to read
   *   again we see if the bit has been cleared by the HW.  Ugly but
   *   it works.  If the bit is clear, report overflow to the user.
   *******************************************************************/
   temp = vbtGetRegister(cardnum, (bm_rec_prev[cardnum]+ 0x08)/2);
   if ( (temp & 0x4000) == 0 )
   {
      AddTrace(cardnum, BM_MSGREADBLOCK, 0x000F, 0x000F,
               0x000F, bm_rec_prev[cardnum]/2, bmaddr_cur/2);
      // HW message queue overflow has been detected.
      hw_overflow = 1;
      bm_hw_queue_ovfl[cardnum] |= BM_HW_OFLOW; // HW buffer overflow
  
      SignalUserThread(cardnum, EVENT_HW_OVERFLO, 0, 0);
      // Give the best guess of the minumum number of messages lost.
      bm_messno[cardnum] += bm_count[cardnum];
#ifdef FILE_SYSTEM
      sprintf(szMsg, "Dump on BM HW buf overflow @ BM buffer %X bmaddr_cur = %X",
              bm_rec_prev[cardnum]/2, bmaddr_cur/2);
      BusTools_DumpMemory(cardnum, 0xFFFFFFFF, "BMHWofl.dmp", szMsg);
#endif //FILE_SYSTEM

   }

   /*******************************************************************
   *  Transfer data until we reach bmaddr_cur...even if the API buffer
   *  fills up and overflows.  This allows us to count the msgs lost...
   *******************************************************************/

   bmaddr    =  bm_messaddr[cardnum];   // Where we left off before.V4.30
   nHead     = nAPI_BM_Head[cardnum];   // Store msgs starting at head.
   oflowaddr = NULL;                    // Overflow not detected.
   while ( bmaddr != bmaddr_cur )       // While SW ptr != HW ptr ...
   {
      // Fetch the message from the board, convert it and
      //  store it in lpAPI_BM_Buffers[cardnum][nHead].
      useraddr = (API_BM_MBUF *)&lpAPI_BM_Buffers[cardnum][nHead*sizeof(API_BM_MBUF)];   
      // Skip the read if the previous message overflowed the buffer,
      //  since this message must be thrown away also!  Reading the
      //  message is very time-consuming, and we are out of time!
      if ( oflowaddr == NULL )
      {
#ifdef USE_BM_DMA 
         vbtSetPLXRegister32(cardnum,PLX_DMA_BOARD_ADDR0, bmaddr+0x100000);
         vbtSetPLXRegister8(cardnum,PLX_DMA_STATUS0, 0x3);
         BM_MessageConvert(cardnum, (BT_U32BIT)bt_devmap[cardnum].laddr, useraddr);
#else
         BM_MessageConvert(cardnum, bmaddr, useraddr);
#endif
         useraddr->int_status &= BM_INTERRUPT_STATUS_VALID2;
         // Typically only the End Of Message Interrupt bit is set (0x00010000)
         ErrorCountUpdate(bm_ecount[cardnum], useraddr->int_status,
                                              useraddr->status1);
         // If bus loading calculations are turned on update the counters.
#if defined(DO_BUS_LOADING)
         if ( bm_isBusLoading[cardnum] )
            BM_BusLoading(useraddr, stats);
#endif
      }
      useraddr->messno = bm_messno[cardnum]++; // Store and update msg num.
      bm_rec_prev[cardnum] = bmaddr;           // Save (possibly) last loc read.

      // Mark this message with the overflow flag if a HW overflow occured.
      if ( hw_overflow )
      {
         hw_overflow = 0;
         useraddr->int_status |= BT1553_INT_BM_OVERFLOW; // Tag buffer o-flow.
      }
      
      // Step to next buffer on the card that will be used by the HW.
      bmaddr += nBM_MBUF_len[cardnum];    // Step to next BM HW buffer.
      if ( bmaddr >= btmem_bm_mbuf_next[cardnum] )
         bmaddr = btmem_bm_mbuf[cardnum]; // Wrap the hardware buffer around.

      // Check for overflow of API buffer.  If overflow, overwrite this msg.
      nHead += 1;           // Next msg might be overflow.
      nHead &= NAPI_BM_BUFFERS - 1;         // Reset head pointer if wrap around.
      if ( nHead == nAPI_BM_Tail[cardnum] ) // Check for FIFO overflow.
      {  // The BM FIFO has overflowed. Next msg overwrites this msg in FIFO.
         bm_hw_queue_ovfl[cardnum] |= BM_API_OFLOW;  // Remember SW overflow.
         // Now we need to tag the PREVIOUS message with the overflow flag.
         // This is because we will (sooner or later) overwrite THIS message
         //  with another, and the FIFO will not be full, so IT will NOT get
         //  the OVERFLOW tag set.  The overflow tag will therefor be lost...
         nHead = nPrevHead = nAPI_BM_Head[cardnum];  // Current head message.
         nPrevHead--;                       // Step to previous head message
         nPrevHead &= NAPI_BM_BUFFERS;      //     and handle wrap around.
         oflowaddr = (API_BM_MBUF *)&lpAPI_BM_Buffers[cardnum][nPrevHead*sizeof(API_BM_MBUF)];
         oflowaddr->int_status |= BT1553_INT_BM_OVERFLOW; // Tag buffer o-flow.
      }
      else
      {  // The BM FIFO has NOT overflowed...Step to next API buffer.
         nAPI_BM_Head[cardnum] = nHead;  // Save incremented head pointer with wrap.
      }
   }  // end while ( bmaddr != bmaddr_cur )   // While SW ptr != HW ptr ...

   bm_messaddr[cardnum] = bmaddr;

   /*******************************************************************
   *  Set the overflow bit, so we can detect HW buffer wrap-around.
   *******************************************************************/
   vbtReadModifyWrite(cardnum, RAM, bm_rec_prev[cardnum]+0x08, 0x4000, 0x4000);

   /*******************************************************************
   *  Signal the user thread if the API buffer has overflowed.
   *******************************************************************/
  if ( oflowaddr )
      SignalUserThread(cardnum, EVENT_API_OVERFLO, 0, 0);
   return;
}

/****************************************************************
*
*  PROCEDURE - BusTools_BM_StartStop
*
*  FUNCTION
*     This routine handles turning the BM on or off.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_NOTINITED        -> BM_Init not yet called
*     API_BM_RUNNING          -> BM currently running
*     API_BM_NOTRUNNING       -> BM not currently running
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_StartStop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT startflag)       // (i) 0=stop
                            //     1=start
                            //     3=start + reset time tag
                            //     0xf BM_START_BIT
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U16BIT addr, hwrData;
   BT_INT    status;

   /************************************************
   *  Check initial conditions
   ************************************************/
   AddTrace(cardnum, NBUSTOOLS_BM_STARTSTOP, startflag, bm_running[cardnum], 0, 0, 0);
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] != 2)
      return API_BM_NOTINITED;

   if ((startflag == 0) && (bm_running[cardnum] == 0))
      return API_BM_NOTRUNNING;

   if (startflag && bm_running[cardnum])
      return API_BM_RUNNING;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBM_StartStop[cardnum] )
   {
      status = (*pUsrBM_StartStop[cardnum])(cardnum, &startflag);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBM_StartStop[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Start or Stop the BM
   *******************************************************************/
   if ( startflag )
   {
      if(startflag != 0xf)
      {
         // Detect single function board and attempt to start second function.V.26.ajh
         if(board_is_dual_function[cardnum] == 0)
         {
            if ( _HW_1Function[cardnum] && (bc_running[cardnum] | rt_running[cardnum]) )
               return API_SINGLE_FUNCTION_ERR;
         }
      }
      bm_messno[cardnum]    = 1;
      bm_messaddr[cardnum]  = btmem_bm_mbuf[cardnum];
      // Get the address of the previous message
      BusTools_BM_MessageGetaddr(cardnum, bm_count[cardnum]-1, &bm_rec_prev[cardnum]);
      // Set the bit in the previous message so we can detect HW buf overflow
      vbtReadModifyWrite(cardnum, RAM, bm_rec_prev[cardnum]+ 0x08, 0x4000, 0x4000);
      nAPI_BM_Tail[cardnum] = nAPI_BM_Head[cardnum] = 0;  // API FIFO is empty.

      addr = (BT_U16BIT)(btmem_bm_mbuf[cardnum] >> hw_addr_shift[cardnum]);

      vbtSetFileRegister(cardnum,RAMREG_BM_PTR_SAVE,addr);
      vbtSetFileRegister(cardnum,RAMREG_BM_MBUF_PTR,addr);

      api_writehwreg_or(cardnum,HWREG_CONTROL1,CR1_BMRUN);
      
      //test the single mode warning bit
      hwrData = vbtGetHWRegister(cardnum,HWREG_CONTROL1);
      if(hwrData & CR1_SMODE)
         return API_SINGLE_FUNCTION_ERR;

      bm_running[cardnum] = 1;  // We are really running now.
      SignalUserThread(cardnum, EVENT_BM_START, 0, 0);  

      if(startflag == 0x3) // reset time tag
         BusTools_TimeTagInit(cardnum);

      channel_status[cardnum].bm_run=1;
   }
   else
   {
      // Turn off bus loading calculations.
      bm_isBusLoading[cardnum] = 0;
      api_writehwreg_and(cardnum,HWREG_CONTROL1,(BT_U16BIT)(~CR1_BMRUN));

      SignalUserThread(cardnum, EVENT_BM_STOP, 0, 0);

      bm_running[cardnum] = 0;
#ifdef FILE_SYSTEM
      if ( DumpOnBMStop[cardnum])
      {
         BusTools_DumpMemory(cardnum, 0xFFFFFFFF, "DumpOnBMStop.txt",
                             "Default Dump on BM Stop enabled in busapi32.ini file");
      }
#endif //FILE_SYSTEM

      channel_status[cardnum].bm_run=0;
   }
   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE - BM_TriggerWritePost320
*
*  FUNCTION
*     This routine transfers the contents of the caller supplied structure
*     to the post version 3.20 firmware BM Trigger Buffer.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_NOTINITED        -> BM_Init not yet called
*     API_BM_RUNNING          -> BM currently running
*
****************************************************************************/

static BT_INT BM_TriggerWritePost320(
   BT_UINT   cardnum,            // (i) card number (0 based)
   API_BM_TBUF * api_tbuf)
{
   /************************************************
   *  Local variables
   ************************************************/
   int          i;
   int          WordCount;
   BM_TBUF_ENH  etbuf;

   /*******************************************************************
   *  Clear the trigger buffer hardware before setting up the new stuff.
   *******************************************************************/
   memset(&etbuf,0,sizeof(etbuf));   // Zero the entire buffer.
   vbtWrite(cardnum,(LPSTR)&etbuf,BTMEM_BM_TBUF,sizeof(etbuf));

   /*******************************************************************
   *  Create the standard trigger buffer for output.
   *******************************************************************/
   etbuf.start_trigger_event = 0x0;
   etbuf.stop_trigger_event  = 0x0;

   if ( api_tbuf->trig_ext == 1 )
   {
      etbuf.trigger_header = BM_ETBUF_EXT;  // External trigger enable
   }
   else if ( api_tbuf->trig_ext == 2 )
   {
      etbuf.trigger_header = BM_ETBUF_EVERY; // External output on BM message
   }
   else // api_tbuf->trig_ext == 0
   {
      if ( api_tbuf->control1 )
      {
         etbuf.trigger_header = BM_ETBUF_START;
         for ( i = 0; i < 4; i++ )
         {
            switch ( api_tbuf->capture[i].type )
            {
               case 0:     // No trigger
                  etbuf.start_event[i].trigger_offset = 0;
                  api_tbuf->capture[i].mask=0;
                  api_tbuf->capture[i].value=0;
                  api_tbuf->capture[i].count=0;
                  break;
               case 1:     // Trigger on command word.  The command word offset
                           //  is not a function of T/R; for RT-RT use first cmd word
                  etbuf.start_event[i].trigger_offset = 8;
                  break;
               case 2:     // Trigger on status word.  The offset is a function
                           //  of T/R & word count; ignor Broadcast & RT-RT;
                           // It must be linked to a command word.
                  if ( (WordCount = api_tbuf->capture[0].value & 0x003F) == 0 )
                     WordCount = 32;
                  if ( api_tbuf->capture[0].value & 0x0400 )
                     etbuf.start_event[i].trigger_offset = 11; // Transmit
                  else
                     etbuf.start_event[i].trigger_offset = (BT_U16BIT)(10 +
                                                   2*WordCount + 1);
                  break;
               case 3:     // Trigger on data word.  The offset is a function of
                           //  T/R; it must be linked to a command.
                  if ( api_tbuf->capture[0].value & 0x0400 )
                     etbuf.start_event[i].trigger_offset = (BT_U16BIT)(13 +
                                                   2*api_tbuf->capture[i].word);
                  else
                     etbuf.start_event[i].trigger_offset = (BT_U16BIT)(10 +
                                                   2*api_tbuf->capture[i].word);
                  break;
               case 4:     // Trigger on time (not implemented)
                  etbuf.start_event[i].trigger_offset = 0;
                  break;
               case 5:    //Trigger on the bm_interrupt status 1
                  etbuf.start_event[i].trigger_offset = 3;
                  break;  
               case 6:    //Trigger on the bm_interrupt status 2
                  etbuf.start_event[i].trigger_offset = 4;
                  break;
            }
            etbuf.start_event[i].trigger_event_bit_mask = api_tbuf->capture[i].mask;
            etbuf.start_event[i].trigger_event_value    = api_tbuf->capture[i].value;
            etbuf.start_event[i].trigger_event_count    = api_tbuf->capture[i].count;
            etbuf.start_event[i].trigger_initial_count  = api_tbuf->capture[i].count;
         }
      }
      if ( api_tbuf->control2 )
      {
         etbuf.trigger_header |= BM_ETBUF_STOP;
         for ( i = 0; i < 4; i++ )
         {
            switch ( api_tbuf->stop[i].type )
            {
               case 0:     // No trigger
                  etbuf.stop_event[i].trigger_offset = 0;
                  break;
               case 1:     // Trigger on command word.  The command word offset
                           //  is not a function of T/R; for RT-RT use first cmd word
                  etbuf.stop_event[i].trigger_offset = 8;
                  break;
               case 2:     // Trigger on status word.  The offset is a function
                           //  of T/R & word count; ignor Broadcast & RT-RT;
                           // It must be linked to a command word.
                  if ( (WordCount = api_tbuf->stop[0].value & 0x003F) == 0 )
                     WordCount = 32;
                  if ( api_tbuf->stop[0].value & 0x0400 )
                     etbuf.stop_event[i].trigger_offset = 11; // Transmit
                  else
                     etbuf.stop_event[i].trigger_offset = (BT_U16BIT)(10 +
                                                   2*WordCount + 1);
                  break;
               case 3:     // Trigger on data word.  The offset is a function of
                           //  T/R; it must be linked to a command.
                  if ( api_tbuf->stop[0].value & 0x0400 )
                     etbuf.stop_event[i].trigger_offset = (BT_U16BIT)(13 +
                                                   2*api_tbuf->stop[i].word);
                  else
                     etbuf.stop_event[i].trigger_offset = (BT_U16BIT)(10 +
                                                   2*api_tbuf->stop[i].word);
                  break;
               case 4:     // Trigger on time (not implemented)
                  etbuf.stop_event[i].trigger_offset = 0;
                  break;
               case 5:    //Trigger on the bm_interrupt status 1
                  if((_HW_FPGARev[cardnum] & 0xfff) >= 0x422)
                     etbuf.start_event[i].trigger_offset = 3;
                  else
                     return API_FEATURE_SUPPORT;
                  break;
               case 6:    //Trigger on the bm_interrupt status 2
                  if((_HW_FPGARev[cardnum] & 0xfff) >= 0x422)
                     etbuf.start_event[i].trigger_offset = 4;
                  else
                     return API_FEATURE_SUPPORT;
                  break;
            }
            etbuf.stop_event [i].trigger_event_bit_mask = api_tbuf->stop   [i].mask;
            etbuf.stop_event [i].trigger_event_value    = api_tbuf->stop   [i].value;
            etbuf.stop_event [i].trigger_event_count    = api_tbuf->stop   [i].count;
            etbuf.stop_event [i].trigger_initial_count  = api_tbuf->stop   [i].count;
         }
      }

/**********************************************************************
*  BM Trigger Buffer definition
*
*     4 - Start Trigger Events (only 3 Active) A - B - C - D(Not Used)
*     4 - Stop Trigger Events (only 3 Active) E - F - G - H(Not Used)
* 
*
*     control: dialog box entries for order of events:
*                         START SIDE                  STOP SIDE
*                    0 -> unconditionally             never
*                    1 -> if A                        if E
*                    2 -> if A AND B                  if E AND F
*                    3 -> if A OR B                   if E OR F
*                    4 -> if A with B (same message)  if E with F (same message) 
*                    5 -> if B armed by A             if F armed by E
*                    6 -> if A with B and C           if E with F and G
*                    7 -> if A with B or C            if E with F or G
*                    8 -> if C armed by A with B      if G armed by E with F
*                    9 -> if A with B armed by C      if E with F armed by G
*                   10 -> if A and B and C            if E and F and G
*                   11 -> if A or B or C              if E or E or G
*     type:    trigger type:
*                    0 -> none
*                    1 -> command
*                    2 -> status
*                    3 -> data
*
*     mask:    16 bit mask (applied prior to compare)
*     value:   16 bit value to compare
*     word:    word number (data trigger - 3) or offset (user offset trigger - 5)
*     count:   number of times event must occur before event_occured bit is set
*
**********************************************************************/

      switch ( api_tbuf->control1 ) // Setup start trigger event type
      {
         case 0:  // Trigger unconditionally
            if ( !(api_tbuf->trig_ext) )    // If external trigger not enabled,
               etbuf.trigger_header |= BM_ETBUF_INTE; //  set trigger occured.
            break;
         case 1:  // Trigger if A
            etbuf.start_trigger_event = 0x10;
            break;
         case 2:  // Trigger if A AND B
            etbuf.start_trigger_event = 0x30;
            break;
         case 3:  // Trigger if A OR B
            etbuf.start_trigger_event = 0xB0;
            break;
         case 4:  // Trigger if A then B
            etbuf.start_trigger_event = 0x20;
            break;
         case 5:  // Trigger if B armed A
            etbuf.start_trigger_event = 0xa0;
            break;
         case 6:  // Trigger if A with B and C
            etbuf.start_trigger_event = 0x40;
            break;
         case 7:  // Trigger if A with B or C
            etbuf.start_trigger_event = 0x50;
            break;
         case 8:  // Trigger if C armed by A with B
            etbuf.start_trigger_event = 0x60;
            break;
         case 9:  // Trigger if A with B armed by C
            etbuf.start_trigger_event = 0x70;
            break;
         case 10:  // Trigger if A and B and C
            etbuf.start_trigger_event = 0x80;
            break;
         case 11:  // Trigger if A or B or C
            etbuf.start_trigger_event = 0x90;
            break;
      }

      switch ( api_tbuf->control2 )
      {
         case 0:  // Stop never
            break;
         case 1:  // Trigger if A
            etbuf.stop_trigger_event = 0x10;
            break;
         case 2:  // Trigger if A AND B
            etbuf.stop_trigger_event = 0x30;
            break;
         case 3:  // Trigger if A OR B
            etbuf.stop_trigger_event = 0xB0;
            break;
         case 4:  // Trigger if A then B
            etbuf.stop_trigger_event = 0x20;
            break;
         case 5:  // Trigger if B armed A
            etbuf.stop_trigger_event = 0xa0;
            break;
         case 6:  // Trigger if A with B and C
            etbuf.stop_trigger_event = 0x40;
            break;
         case 7:  // Trigger if A with B or C
            etbuf.stop_trigger_event = 0x50;
            break;
         case 8:  // Trigger if C armed by A with B
            etbuf.stop_trigger_event = 0x60;
            break;
         case 9:  // Trigger if A with B armed by C
            etbuf.stop_trigger_event = 0x70;
            break;
         case 10:  // Trigger if A and B and C
            etbuf.stop_trigger_event = 0x80;
            break;
         case 11:  // Trigger if A or B or C
            etbuf.stop_trigger_event = 0x90;
            break;

      }
   }
   if ( api_tbuf->trig_err )
      etbuf.trigger_header |= BM_ETBUF_ERROR;

   if ( api_tbuf->trig_ext_output )  // Non-zero enables external trigger output
      etbuf.trigger_header |= BM_ETBUF_OUTPUT;

   if ( BMTrigOutput[cardnum] )       // Back door to trig on every BM message
      etbuf.trigger_header |= BM_ETBUF_EVERY;   // Trigger TTL output on every BM message.

   /*******************************************************************
   *  Store etbuf in hardware
   *******************************************************************/
   vbtWrite(cardnum,(LPSTR)&etbuf,BTMEM_BM_TBUF,sizeof(etbuf));

   BMTriggerSave[cardnum] = *(BM_TBUF_ENH *)&etbuf;
   if ( api_tbuf->trig_ext_output > 1 )   // "2" enables repetitive ext trig output
      MultipleBMTrigEnable[cardnum] = 1;
   else
      MultipleBMTrigEnable[cardnum] = 0;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_BM_TriggerWrite
*
*  FUNCTION
*     This routine transfers the contents of the caller
*     supplied structure to the BM Trigger Buffer.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BM_NOTINITED        -> BM_Init not yet called
*     API_BM_RUNNING          -> BM currently running
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_TriggerWrite(
   BT_UINT   cardnum,           // (i) card number (0 based)
   API_BM_TBUF * api_tbuf)  // (i) API trigger definition buffer
{
   /************************************************
   *  Check initial conditions
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   return BM_TriggerWritePost320(cardnum, api_tbuf);
}

/****************************************************************************
*
*  PROCEDURE - BusTools_ErrorCountClear
*
*  FUNCTION - Clear the message and error counters for specified board.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> card number out of range
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_ErrorCountClear(
   BT_UINT   cardnum)       // (i) card number (0 based)
{
   /************************************************
   *  Local variables
   ************************************************/

   int i;

   /************************************************
   *  Check initial conditions
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   /************************************************
   *  Clear out error counters
   ************************************************/

   for (i=0; i<32; i++)
      bm_ecount[cardnum][i] = 0;

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_ErrorCountGet
*
*  FUNCTION - Get the values of the message and error counters.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> bad card number
*
****************************************************************************/
#ifdef _64_BIT_
NOMANGLE CEI_UINT64 CCONV BusTools_ErrorCountGet(
#else
NOMANGLE BT_INT CCONV BusTools_ErrorCountGet(
#endif // _64_BIT_
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT * msgcount,
   BT_U32BIT * buf)
{
   /************************************************
   *  Local variables
   ************************************************/
   int i;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   /************************************************
   *  Check initial conditions
   ************************************************/
#if defined(DO_BUS_LOADING)
   if (  (msgcount == NULL) || (buf == NULL) )
   {
      // Turn on bus loading calculations.
      bm_isBusLoading[cardnum] = 1;
#ifdef _64_BIT_
      return (CEI_UINT64)&bus_stats[cardnum];
#else
      return (BT_INT)&bus_stats[cardnum];
#endif
   }
#endif

   /************************************************
   *  Transfer current counters to caller's buffer
   ************************************************/
   for ( i = 0; i < 32; i++ )
      buf[i] = bm_ecount[cardnum][i];

   *msgcount = bm_ecount[cardnum][31]; // Count of Messages in BM buffer.V4.01.ajh
#if defined(DO_BUS_LOADING) && 0
   buf[24] = bus_stats[cardnum].BusLoading;
   buf[25] = bus_stats[cardnum].BusLoadingRT[1];
#endif
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_InterMessageGap
*
*  FUNCTION
*     This routine computes the intermessage gap which exists between two
*     specified messages.  This is accomplished by first differencing the
*     two timetags, then subtracting the amount of time that the first
*     message took to on the bus.
*
*     This is not as simple as it first appears.  Messages are time tagged
*     at the mid sync of the command word, and the intermessage gap time is
*     defined as the time between the mid parity of the last word of the
*     previous message to the mid sync of the command word of the current
*     message.  The mid parity occurs 0.5 us before the end of the word, and
*     the mid sync occurs 1.5 us after the command word starts, so the
*     intermessage gap time is 2 us LESS than the bus dead time....
*     Response times are measured like gap times; 2 us greater than the bus
*     dead time.
*
*  RETURNS
*     Calculated Intermessage Gap Time in microseconds.
****************************************************************************/

NOMANGLE BT_U32BIT CCONV BusTools_InterMessageGap(
   API_BM_MBUF * first,
   API_BM_MBUF * second)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  dInterMsgGap;
   BT_U16BIT  count;
   BT_U16BIT  mode;
   BT_U16BIT  saddr;
   int        Response1;       // First measured response time, us.
   int        Response12;      // First plus Second measured response time, us.
   int        temp;

   /*******************************************************************
   *  Subtract the first message's timetag from the second message.
   *  We only subtract the low 32-bits because we only return a 32-bit
   *   value in microseconds.
   *******************************************************************/
   dInterMsgGap = second->time.microseconds - first->time.microseconds;
   if ( dInterMsgGap == 0 )
      return 0L;            // If both messages are @ same time, return 0.

   /*******************************************************************
   *  Determine the first message type -- mode word has following bits:
   *     0 -> BC->RT
   *     1 -> RT->BC
   *     2 -> BROADCAST
   *     4 -> MODECODE
   *     8 -> RT->RT
   ******************************************************************/

   // What about high word errors??????
   if ( (count = first->command1.wcount) == 0 )
      count = 32;

   // Setup the response times, scaled to microseconds and rounded to even.
   // Convert response times to "bus dead time"s by subtracting the half bit
   //  time at the beginning, and the 1.5 bit times at the end of the gap.V4.16.ajh
   Response1 = first->response1.time >> 1;
   if ( (first->response1.time & 0x0003) == 0x0001 )
      Response1++;           // Round the response time to even.
   temp = first->response1.time + first->response2.time;
   Response12 = temp >> 1;
   if ( (temp & 0x0003) == 0x0001 )
      Response12++;          // Round the response times to even.

   // Determine what kind of message this is.
   mode = first->command1.tran_rec;          // Set LSB if TRUE, else clear LSB.

   // Determine if broadcast.  We need to look at the interrupt status
   //  because otherwise we have no idea if this is a broadcast message.V4.17.ajh
   if ( (first->int_status & BT1553_INT_BROADCAST) && (first->command1.rtaddr == 31) )
      mode |= 2;

   saddr = first->command1.subaddr;
   // Check the mode code bit to see if this is a SA 31 mode code, if not
   //  implemented in this firmware check the SA==0 to determine if the
   //  message is a mode code.  Old firmware versions that do not set this
   //  mode code status bit will not correctly report SA 31 mode codes.V4.17.ajh
   if ( (saddr == 0) || (first->int_status & BT1553_INT_MODE_CODE) )
      mode |= 4;                     // Set if Mode Code at SA 0 or SA 31

   if ( first->int_status & BT1553_INT_RT_RT_FORMAT )
      mode |= 8;                     // Set for RT-RT messages

   /*******************************************************************
   * Handle rest of message according to message type decoded above.
   * This case statement computes the bus time used by the specific
   *  message type and length as decoded above.  A single word on the
   *  1553 bus takes 20 us, but there is a 2 us difference between a
   *  measured gap and the bus active time.
   *******************************************************************/
   switch ( mode )
   {
      case 0:     // BC->RT message
         dInterMsgGap -= 18;         // Account for the cmd word.
         dInterMsgGap -= 20*count;   // Account for the data words.
         if ( first->int_status & BT1553_INT_NO_RESP )
            break;
         dInterMsgGap -= Response1;  // Response time.
         dInterMsgGap -= 18;         // Account for the status word.
         break;

      case 1:     // RT->BC message
         dInterMsgGap -= 18;         // Account for the cmd word.
         if ( first->int_status & BT1553_INT_NO_RESP )
            break;
         dInterMsgGap -= 18;         // Account for the status word.
         dInterMsgGap -= Response1;  // Response time.
         if ( first->status1.me || first->status1.busy )
            break;                   // No data if there was an error.
         dInterMsgGap -= 20*count;   // Account for the data words.
         break;

      case 2:     // BROADCAST BC->RT message
         dInterMsgGap -= 18;         // Account for the command word.
         dInterMsgGap -= 20*count;   // Account for the data words.
         break;

      case 4:     // Modecode (receive -- always has one data word)
         dInterMsgGap -= 18;         // Account for the cmd word.
         dInterMsgGap -= 20;         // Account for data.
         if ( first->int_status & BT1553_INT_NO_RESP )
            break;
         dInterMsgGap -= Response1;    // Response time.
         dInterMsgGap -= 18;           // Account for status word.
         break;

      case 5:     // Modecode (transmit -- either with or without data)
         dInterMsgGap -= 18;         // Account for the cmd word.
         if ( first->int_status & BT1553_INT_NO_RESP )
            break;
         dInterMsgGap -= Response1;    // Response time.
         dInterMsgGap -= 18;           // Account for status word.
         if ( first->status1.me || first->status1.busy )
            break;                     // No data if there was an error.
         if ( (count>=16) & (count<32) ) // Is there is a data word?
            dInterMsgGap -= 20;          // Account for the data word.
         break;

      case 6:     // BROADCAST Modecode (receive -- always has one data word)
         dInterMsgGap -= 18;         // Account for the cmd word.
         dInterMsgGap -= 20;         // Account for data word.
         break;

      case 8:     // RT->RT command
         // The BM sets one bit for NR on EITHER first RT or Second RT.
         // This means that we cannot determine WHICH RT did not respond,
         //  and therefore cannot calculate the time the message took.
         // So just guess at it...
         dInterMsgGap -= 18+20;        // Account for the two cmd words.
         if ( dInterMsgGap < 40 )
            break;
         if ( first->int_status & BT1553_INT_NO_RESP )
            break;
         dInterMsgGap -= (Response12); // Response times.
         dInterMsgGap -= 18;           // Account for the first status word.
         if ( first->status1.me || first->status1.busy )
            break;                     // No data if there was an error.
         if ( dInterMsgGap > (unsigned)20*count )
            dInterMsgGap -= 20*count;// Account for the data words.
         if ( dInterMsgGap > 24 )
            dInterMsgGap -= 18;      // Account for the second status word.
         break;

      case 10:    // BROADCAST RT->RT command
         dInterMsgGap -= 18+20;      // Account for the two cmd words.
         if ( first->int_status & (BT1553_INT_NO_RESP | BT1553_INT_ME_BIT) )
            break;
         dInterMsgGap -= Response1;  // Response time.
         dInterMsgGap -= 18;         // Account for the status word from sending RT.
         if ( first->status1.me || first->status1.busy )
            break;                   // No data if there was an error.
         dInterMsgGap -= 20*count;   // Account for the data words.
         break;

      case 7:     // BROADCAST Modecode (transmit -- only without data)
         dInterMsgGap -= 18;         // Account for the command word.
         break;

      case 3:     // ILLEGAL (BROADCAST RT->BC message)
      case 9:     // ILLEGAL (RT->RT with first command being transmit)
      case 11:    // ILLEGAL (BROADCAST RT->RT, first command being transmit)
      case 12:    // ILLEGAL (RT->RT and Modecode)
      case 13:    // ILLEGAL (RT->RT and Modecode)
      case 14:    // ILLEGAL (RT->RT and Broadcast and Modecode)
      case 15:    // ILLEGAL (RT->RT and Broadcast and Modecode)
         break;       // API_BM_ILLEGAL_MESSAGE;
   }
   return dInterMsgGap;
}


/****************************************************************************
*
*  PROCEDURE NAME -    DumpBMmsg()
*
*  FUNCTION
*       This procedure outputs a dump of BM Messages.  It is a local helper
*       function for the BusTools_DumpMemory user-callable function.
*
****************************************************************************/
void DumpBMmsg(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT    i;                        // Loop index
   BT_UINT      j, k;                     // Loop index
   int          msgnum;                   // Message number
   BT_U32BIT    first;                    // Offset to first list word
   BT_U32BIT    last;                     // Offset to last list word
   BT_U16BIT    data[40];                 // Read 16+ words per line

   /* This is where the specified buffers live */
   BusTools_GetAddr(cardnum, GETADDR_BMMESSAGE, &first, &last);
   first /= 2;    // Convert to word offset.
   last  /= 2;    // Convert to word offset.
   msgnum = 0;
   for ( i = first; i < last; )
   {
      // Read the current line of 10 data words from the board.
      BusTools_MemoryRead(cardnum, i*2, 10*2, data);
      fprintf(hfMemFile, "Msg %3d @%04X:", msgnum, i);
      // Print the address of the next BM message
      fprintf(hfMemFile, " nxt%04X", data[0]<<(hw_addr_shift[cardnum]-1));
      if ( (unsigned)(data[0]<<(hw_addr_shift[cardnum]-1)) == i+nBM_MBUF_len[cardnum]/2 )
         fprintf(hfMemFile, "!");
      else
         fprintf(hfMemFile, "*");
      // Print the interrupt enable mask and interrupt status
      fprintf(hfMemFile, " ena%04X%04X int%04X%04X",
              data[2], data[1], data[4], data[3]);

      // Print the 45-bit time tag from the hardware
      fprintf(hfMemFile, " tt%04X%04X%04X", data[7], data[6], data[5]);
      // Print the command and status words
      fprintf(hfMemFile, " cmd%04X OK%04X\n", data[8], data[9]);

      // Now print the remaining words (9-83) in the message buffer.
      // Note that word 9 gets printed here for the 48-bit time tag
      //  boards, even though it was printed above...
      for ( k = 0; k < (86-9+1); )
      {
         fprintf(hfMemFile, "BM Dat  @%04X:", i+9+k);
         BusTools_MemoryRead(cardnum, (i+9+k)*2, 16*2, data);
         for ( j = 0; j < 16 && k < (86-9+1); j++, k++ )
            fprintf(hfMemFile, " %4.4X", data[j]);
         fprintf(hfMemFile, "\n");
      }
      // Step to the next BM Buffer and Message Number.
      i += nBM_MBUF_len[cardnum]/2;
      msgnum++;
   }
}

/****************************************************************************
*
*  PROCEDURE NAME -    DumpBMflt()
*
*  FUNCTION
*       This procedure outputs a dump of the BM filter buffer.  It is a local
*       helper function for the BusTools_DumpMemory user-callable function.
*
****************************************************************************/
#define FILT_PER_LINE  16
void DumpBMflt(             // Dump the BM filter list to output file
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT    i;                        // Loop index
   BT_UINT      j;                        // Loop index
   BT_U32BIT    first;                    // Offset to first list word
   BT_U32BIT    last;                     // Offset to last list word
   BT_U16BIT    data[FILT_PER_LINE];      // Read 16+ words per line
   int          state = 0;                // Display state
   int          non_default;              // Non-default entries detected
   BT_U32BIT    firstCbuf;                // Address of lowest allocated cbuf
   BT_U32BIT    lastCbuf;                 // Address of highest allocated cbuf

   // Get the beginning and end of the BM Filter buffer
   BusTools_GetAddr(cardnum, GETADDR_BMFILTER, &first, &last);
   first /= 2;   // Convert to word offset
   last  /= 2;   // Convert to word offset

   // Initialize the pointers to the non-default cbufs allocated by the user
   firstCbuf = 0x7FFFFFFL;                // Address of lowest allocated cbuf
   lastCbuf  = 0;                         // Address of highest allocated cbuf

   // Dump the BM Filter buffer region to the file.
   for ( i = first; i < last; i += FILT_PER_LINE )
   {
      // Read the current line of data words from the board.
      BusTools_MemoryRead(cardnum, i*2, FILT_PER_LINE*2, data);
      non_default = 0;
      for ( j = 0; j < FILT_PER_LINE; j++ )
      {  // Check for BOTH default cbufs: V4.27.ajh
         if ( (data[j] != (BT_U16BIT) (BTMEM_BM_CBUF_DEF >> 1) ) &&
              (data[j] != (BT_U16BIT)((BTMEM_BM_CBUF_DEF+sizeof(BM_CBUF)) >> 1)) )
         {
            non_default = 1;  // We have non-default entries to display
            if ( data[j] > lastCbuf  ) lastCbuf  = data[j];
            if ( data[j] < firstCbuf ) firstCbuf = data[j];
         }
      }
      if ( non_default || state == 0 )
      {
         if ( state > 1 ) fprintf(hfMemFile, "\n");
         fprintf(hfMemFile, "%X:", i);
         for ( j = 0; j < FILT_PER_LINE; j++ )
            fprintf(hfMemFile, " %4.4X", data[j]);
         fprintf(hfMemFile, "\n");
         state = 1;
      }
      else
      {
         fprintf(hfMemFile, ".");
         state++;
         if ( state > 64 )
         {
            state = 2;
            fprintf(hfMemFile, "\n");
         }
      }
   }
   fprintf(hfMemFile, "\n");

   // Dump the non-default BM control buffers to the file.V4.27.ajh
   // Hopefully they will not be scattered all over memory...
   if ( firstCbuf < lastCbuf )
   for ( i = firstCbuf; i < lastCbuf+4; i += FILT_PER_LINE )
   {
      // Read the current line of data words from the board.
      BusTools_MemoryRead(cardnum, i*2, FILT_PER_LINE*2, data);
      fprintf(hfMemFile, "%X:", i);
      for ( j = 0; j < FILT_PER_LINE; j++ )
         fprintf(hfMemFile, " %4.4X", data[j]);
      fprintf(hfMemFile, "\n");
   }
}

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
*       API_BM_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BM_READ_TIMEOUT     -> Timeout before data read
*       API_HW_IQPTR_ERROR      -> interrupt Queue pointer error
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_ReadNextMessage(int cardnum,BT_UINT timeout,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_BM_MBUF *pBM_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.

   BT_U16BIT iq_addr;                  // Hardware Interrupt Queue Pointer
   BT1553_COMMAND cmd;
   BT_U32BIT beg;                      // Beginning of the interrupt queue
   BT_U32BIT end;                      // End of the interrupt queue

   BT_UINT   messno;
   BT_U32BIT addr;
   long      message_number;
  
   BT_U32BIT mess_addr;
   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;

   BT_U32BIT start;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

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

         // We only care about BM interrupt here.
         if ( intqueue.t.mode.bm )
         {
            mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;

            mess_addr <<= hw_addr_shift[cardnum] - 1;
            messno = (BT_U16BIT)( (mess_addr - btmem_bm_mbuf[cardnum]) / nBM_MBUF_len[cardnum] );

            if ( ((mess_addr - btmem_bm_mbuf[cardnum]) % nBM_MBUF_len[cardnum]) ||
                 (messno > bm_count[cardnum])  )
               return API_BM_MBUF_NOMATCH;

            if(BM_INT_ON_RTRT_TX[cardnum])
               vbtRead(cardnum, (LPSTR)&cmd, mess_addr+20, sizeof(BT1553_COMMAND));
            else
               vbtRead(cardnum, (LPSTR)&cmd, mess_addr+16, sizeof(BT1553_COMMAND));

			if((rt_addr == DONT_CARE) || (rt_addr & (bit<<cmd.rtaddr)))
			{
			   if((subaddress == DONT_CARE) || (subaddress & (bit<<cmd.subaddr)))
			   {
                  if((tr == DONT_CARE) || (tr == cmd.tran_rec))
				  {
                        /*******************************************************************
                        *  Read the specified message buffer.  First calculate the address
                        *  of the buffer, given the ID number which starts with zero:
                        *******************************************************************/
                        addr = btmem_bm_mbuf[cardnum] + (BT_U32BIT)messno * nBM_MBUF_len[cardnum];

                        /*******************************************************************
                        *  Fetch the message from the board, convert it, store it in api_mbuf.
                        *******************************************************************/
                        if(board_is_paged[cardnum])
                           vbtAcquireFrameRegister(cardnum, 1);   // Acquire frame register
                        BM_MessageConvert(cardnum, addr, pBM_mbuf);
                        pBM_mbuf->int_status &= BM_INTERRUPT_STATUS_VALID1;
                        if(board_is_paged[cardnum])
                           vbtAcquireFrameRegister(cardnum, 0);   // Release frame register

                        /*******************************************************************
                        *  Fill in message number in top of message.  Properly
                        *   handle the wrap around of the message buffers.V4.30
                        *******************************************************************/
                        // Compute the offset from current BM message to the specified message.
                        message_number = messno - ((bm_messaddr[cardnum] - btmem_bm_mbuf[cardnum]) /
                                        nBM_MBUF_len[cardnum]) + 1;
                        if ( message_number >= (long)bm_count[cardnum] )
                           message_number = 0;

                        // Compute the message number of the specified message
                        message_number += bm_messno[cardnum] - 1;

                        // If the message number does not make any sense, increment it by the
                        //  number of messages in the BM buffer list.
                        if ( message_number < 0 )
                           message_number += bm_count[cardnum];

                        // Return the computed message number to the caller
                        pBM_mbuf->messno = message_number;
						return API_SUCCESS;
					}
				}
			}
         }
      }
	  
   }while((CEI_GET_TIME() - start) < timeout);

   return API_BM_READ_TIMEOUT;
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
*       API_BM_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BM_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_ReadLastMessage(int cardnum,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_BM_MBUF *pBM_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.
   BT1553_COMMAND cmd;
   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_U32BIT iqptr_cur;
   BT_UINT   messno;
   BT_U32BIT addr;
   long      message_number;  
   BT_U32BIT mess_addr;

   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;
   BT_UINT queue_entry=6;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   iqptr_hw = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;

   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % sizeof(IQ_MBLOCK) != 0 ) )
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_bm_last[cardnum];
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

      // We only care about BM interrupt here.
      if ( intqueue.t.mode.bm )
      {
         mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
         mess_addr <<= hw_addr_shift[cardnum] - 1;
         messno = (BT_U16BIT)( (mess_addr - btmem_bm_mbuf[cardnum]) / nBM_MBUF_len[cardnum] );

         if ( ((mess_addr - btmem_bm_mbuf[cardnum]) % nBM_MBUF_len[cardnum]) ||
              (messno > bm_count[cardnum])  )
            return API_BM_MBUF_NOMATCH;
         vbtRead(cardnum, (LPSTR)&cmd, mess_addr+16, sizeof(BT1553_COMMAND));

         if((rt_addr == DONT_CARE) || (rt_addr & (bit<<cmd.rtaddr)))
		 {
			if((subaddress == DONT_CARE) || (subaddress & (bit<<cmd.subaddr)))
			{
               if((tr == DONT_CARE) || (tr == cmd.tran_rec))
			   {
                  /*******************************************************************
                  *  Read the specified message buffer.  First calculate the address
                  *  of the buffer, given the ID number which starts with zero:
                  *******************************************************************/
                  addr = btmem_bm_mbuf[cardnum] + (BT_U32BIT)messno * nBM_MBUF_len[cardnum];

                  /*******************************************************************
                  *  Fetch the message from the board, convert it, store it in api_mbuf.
                  *******************************************************************/
                  if(board_is_paged[cardnum])
                     vbtAcquireFrameRegister(cardnum, 1);   // Acquire frame register
                  BM_MessageConvert(cardnum, addr, pBM_mbuf);
                  pBM_mbuf->int_status &= BM_INTERRUPT_STATUS_VALID1;
                  if(board_is_paged[cardnum])
                     vbtAcquireFrameRegister(cardnum, 0);   // Release frame register

                  /*******************************************************************
                  *  Fill in message number in top of message.  Properly
                  *   handle the wrap around of the message buffers.V4.30
                  *******************************************************************/
                  // Compute the offset from current BM message to the specified message.
                  message_number = messno - ((bm_messaddr[cardnum] - btmem_bm_mbuf[cardnum]) /
                                  nBM_MBUF_len[cardnum]) + 1;
                  if ( message_number >= (long)bm_count[cardnum] )
                     message_number = 0;

                  // Compute the message number of the specified message
                  message_number += bm_messno[cardnum] - 1;

                  // If the message number does not make any sense, increment it by the
                  //  number of messages in the BM buffer list.
                  if ( message_number < 0 )
                     message_number += bm_count[cardnum];

                  // Return the computed message number to the caller
                  pBM_mbuf->messno = message_number;               
                  break;
			   }
			}
		 }
      }
   }
   iqptr_bm_last[cardnum] = iqptr_hw;
   if(iqptr_sw == iqptr_hw)
	   return API_BM_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BM_ReadLastMessageBlock()
*
*   FUNCTION
*       This function reads the last RT messages that meets the criteria set
*       by the passed parameters.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BM_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BM_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_ReadLastMessageBlock(int cardnum,BT_INT rt_addr_mask,
									   BT_INT subaddr_mask,BT_INT tr, BT_UINT *mcount,API_BM_MBUF *pBM_mbuf)
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
   BT_U32BIT addr;
   long      message_number;

   BT_U32BIT bit = 1;
   BT_INT DONT_CARE = -1;

   *mcount = 0;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   iqptr_hw = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;
   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % sizeof(IQ_MBLOCK) != 0 ))   
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_bm_last[cardnum];

   /******************************************************************
   *  Loop until 
   *******************************************************************/
   msg_cnt = 0;
   while ( iqptr_sw != iqptr_hw )
   {
      /*******************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
      vbtRead_iq(cardnum,(LPSTR)&intqueue, iqptr_sw, sizeof(intqueue));
      iqptr_sw = ((BT_U32BIT)intqueue.nxt_int) << 1; // Chain to next entry

      // We only care about BM interrupts here.
      if ( intqueue.t.mode.bm )
      {
         mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
         mess_addr <<= hw_addr_shift[cardnum] - 1;
	     messno = (BT_U16BIT)( (mess_addr - btmem_bm_mbuf[cardnum]) / nBM_MBUF_len[cardnum] );

         if ( ((mess_addr - btmem_bm_mbuf[cardnum]) % nBM_MBUF_len[cardnum]) ||
              (messno > bm_count[cardnum])  )
            return API_BM_MBUF_NOMATCH;
         vbtRead(cardnum, (LPSTR)&cmd, mess_addr+16, sizeof(BT1553_COMMAND));
         
         if((rt_addr_mask == DONT_CARE) || (rt_addr_mask & (bit<<cmd.rtaddr)))
		 {
			if((subaddr_mask == DONT_CARE) || (subaddr_mask & (bit<<cmd.subaddr)))
			{
               if((tr == DONT_CARE) || (tr == cmd.tran_rec))
			   {

                  /*******************************************************************
                  *  Read the specified message buffer.  First calculate the address
                  *  of the buffer, given the ID number which starts with zero:
                  *******************************************************************/
                  addr = btmem_bm_mbuf[cardnum] + (BT_U32BIT)messno * nBM_MBUF_len[cardnum];
                  /*******************************************************************
                  *  Fetch the message from the board, convert it, store it in api_mbuf.
                  *******************************************************************/
                  if(board_is_paged[cardnum])
                     vbtAcquireFrameRegister(cardnum, 1);   // Acquire frame register

                  BM_MessageConvert(cardnum, addr, &pBM_mbuf[msg_cnt]);
                  pBM_mbuf[msg_cnt].int_status &= BM_INTERRUPT_STATUS_VALID1;
                  if(board_is_paged[cardnum])
                     vbtAcquireFrameRegister(cardnum, 0);   // Release frame register

                  /*******************************************************************
                  *  Fill in message number in top of message.  Properly
                  *   handle the wrap around of the message buffers.V4.30
                  *******************************************************************/
                  // Compute the offset from current BM message to the specified message.
                  message_number = messno - ((bm_messaddr[cardnum] - btmem_bm_mbuf[cardnum]) /
                                  nBM_MBUF_len[cardnum]) + 1;
                  if ( message_number >= (long)bm_count[cardnum] )
                     message_number = 0;

                  // Compute the message number of the specified message
                  message_number += bm_messno[cardnum] - 1;

                  // If the message number does not make any sense, increment it by the
                  //  number of messages in the BM buffer list.
                  if ( message_number < 0 )
                     message_number += bm_count[cardnum];

                  // Return the computed message number to the caller
                  pBM_mbuf[msg_cnt].messno = message_number;

				  msg_cnt++;
			   }
			}
		 }
      }
   }
   iqptr_bm_last[cardnum] = iqptr_sw;
   *mcount = msg_cnt;
   if(msg_cnt == 0)
	   return API_BM_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BM_Checksum1760()
*
*   FUNCTION
*       This function calculates a MIL-STD-1760 check for the BM buffer passed
*       and compares it to the lest data word in the message.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BM_1760_ERROR       -> Checksum do not match
*
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BM_Checksum1760(API_BM_MBUF mbuf, BT_U16BIT *cksum)
{
	BT_U16BIT checksum, wd, temp, i, shiftbit, wdcnt;

    wdcnt = mbuf.command1.wcount;

	// Start at zero.
	checksum = 0;

	// Process each word in the data buffer.
	for (wd = 0; wd < (wdcnt-1); wd++) {
		temp = mbuf.value[wd];

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

    if(mbuf.value[wdcnt-1] == checksum)
	   return API_SUCCESS;
    else
       return API_BM_1760_ERROR;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BM_SetRT_RT_INT
*
*  FUNCTION
*     This function set which RT on an RT-RT message is use to interrupt.  The 
*     default for the RT-RT interrupt is on the command1 (receive) RT.
*
*  RETURNS
*     API_SUCCESS             -> success
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_SetRT_RT_INT(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT iflag)           // (i) 0 = interrupt on Receive RT
                            //     1 = interrupt on Transmit RT
{
   if(iflag)
      BM_INT_ON_RTRT_TX[cardnum] = 1;
   else
      BM_INT_ON_RTRT_TX[cardnum] = 0;

   return API_SUCCESS;
}

/****************************************************************
*
* PROCEDURE NAME - BusTools_BM_GetUserData()
*
* FUNCTION
*     This routine reads user data to file register.  This data is
*     then written into the BM Message Buffer on each bm Message.
*     This allows the user to add information to each BM message.
*     There are two words the user can use, data1 and data2.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_GetUserData(
   BT_UINT   cardnum,           // (i) card number (0 based)
   BT_U16BIT   *data1,
   BT_U16BIT   *data2)
{

   /************************************************
   *  Check initial conditions
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] != 2)
      return API_BM_NOTINITED;

   if((_HW_FPGARev[cardnum] & 0xfff) < 0x399)
      return API_HARDWARE_NOSUPPORT;

   *data1 = vbtGetFileRegister(cardnum,RAMREG_USER1);
   *data2 = vbtGetFileRegister(cardnum,RAMREG_USER2);

   return API_SUCCESS;
}

/****************************************************************
*
* PROCEDURE NAME - BusTools_BM_SetUserData()
*
* FUNCTION
*     This routine writes user data to file register.  This data is
*     then written into the BM Message Buffer on each bm Message.
*     This allows the user to add information to each BM message.
*     There are two words the user can use, data1 and data2.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BM_SetUserData(
   BT_UINT   cardnum,           // (i) card number (0 based)
   BT_U16BIT   data1,
   BT_U16BIT   data2)
{

   /************************************************
   *  Check initial conditions
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] != 2)
      return API_BM_NOTINITED;

   if((_HW_FPGARev[cardnum] & 0xfff) < 0x399)
      return API_HARDWARE_NOSUPPORT;

   vbtSetFileRegister(cardnum,RAMREG_USER1,data1);
   vbtSetFileRegister(cardnum,RAMREG_USER2,data2);

   return API_SUCCESS;
}

#ifdef VXW_PCI_THALES
/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BM_ReadMessageBuffer
*
*  FUNCTION
*     This is function for Thales PowerPC processor.
*
*  RETURNS
*     API_SUCCESS             -> success
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BM_ReadMessageBuffer(int cardnum,BT_UINT *mcount,void *pBM_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT bmq_ptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT bmq_ptr_sw;                 // Previous HW interrupt queue ptr.
   BT_UINT msg_cnt,baseline;
   BT_INT status,i;

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh

   msg_cnt = 0;  
   bmq_ptr_hw = vbtGetFileRegister(cardnum,RAMREG_BM_PTR_SAVE);
   bmq_ptr_sw = bmq_ptr_last[cardnum];

   if(bmq_ptr_save[cardnum] == bmq_ptr_hw) // make sure the head pointer moved
   {
      *mcount = 0;
      return API_SUCCESS;
   }

   bmq_ptr_save[cardnum] = bmq_ptr_hw; 

   /******************************************************************
   *  Loop until 
   *******************************************************************/

   while( bmq_ptr_sw != bmq_ptr_hw)
   {
      /*******************************************************************
      *  read the raw BM buffer.
      *******************************************************************/

      for ( i = 0; i < sizeof(BM_MBUF)/4; i++ )
      {
         ((BT_U32BIT *)pBM_mbuf)[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][0]+(bmq_ptr_sw << hw_addr_shift[cardnum])))[i];
         fliplong(&(((BT_U32BIT *)pBM_mbuf)[i])); // for PMC on PowerPC
      }

      bmq_ptr_sw += sizeof(BM_MBUF)/4;
      if(bmq_ptr_sw << hw_addr_shift[cardnum] >= btmem_bm_mbuf_next[cardnum])
         bmq_ptr_sw = btmem_bm_mbuf[cardnum] >> hw_addr_shift[cardnum];

      ((BM_MBUF *)pBM_mbuf)++;
      msg_cnt++;
   }
   
   for ( i = 0; i < sizeof(BM_MBUF)/2; i++ )
   {
      ((BT_U32BIT *)pBM_mbuf)[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][0]+(bmq_ptr_sw << hw_addr_shift[cardnum])))[i];
      fliplong(&(((BT_U32BIT *)pBM_mbuf)[i])); // for PMC on PowerPC
   }

   msg_cnt++;
   bmq_ptr_last[cardnum] = bmq_ptr_sw + 11;
   *mcount = msg_cnt;

   return API_SUCCESS;
}
#endif//VXW_PCI_THALES


