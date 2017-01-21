/*============================================================================*
 * FILE:                        B T D R V . C
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
 *             Low-level board access component.
 *             This file is used for all O/S, the difference
 *             is the mapping of the board into the program's address space.
 *             This module performs the actual memory-mapped reads and writes
 *             from/to the BusTools board.
 *
 *             The only user interface to these functions is (and should be)
 *             through the higher-level functions provided by the API.
 *
 *             All routines within this module assume that a successful call
 *             to vbtSetup() has been performed before they are called, and
 *             that their arguements are all valid.  Error checking within
 *             this module is minimal to provide the best possible performance.
 *
 *             This module also supports a simulation or demo mode, which
 *             simulates the operation of a board without having the hardware
 *             actually installed.  This mode is triggered by a flag passed
 *             to the vbtSetup() function by the BusTools_API_Init() or the
 *             BusTools_API_InitExtended() function.
 *
 * DESCRIPTION:  See vbtSetup() for a description of the functions performed
 *               by this module.
 *
 * API ENTRY POINTS: (only used by BUSAPI.DLL, BUSAPI32.DLL and BUSAPIDx.LIB):
 *    vbtGetRegister       Returns the value of specified adapter register.
 *    vbtGetHWRegister     Returns the value of the specified H/W register.
 *    vbtGetFileRegister   Returns the value of the specified file register.
 *    vbtRead              Reads range in 16 bits (1 word) of memory.
 *    vbtRead32            Reads range in 32 bits (2 words) of memory 
 *    vbtRead_iq           Reads data from the interrupt queue.
 *    vbtReadModifyWrite   Reads and updates a word of adapter memory.
 *    vbtSetRegister       Sets the value of the specified register.
 *    vbtSetHWRegister     Sets the value of the specified H/W register.
 *    vbtSetFileRegister   Sets the value of the specified File register.
 *    vbtSetup             Sets specified adapter for read/write access. 
 *    vbtShutdown          Disables access to specified adapter.
 *    vbtWrite             Writes data from buffer into adapter memory.
 *    vbtGetPagePtr        Gets a pointer to a specified BM message.
 *    vbtReadTimeTag       Reads the time tag counter.
 *    vbtWriteTimeTag      Write to the time tag counter.
 *
 * MAJOR INTERNAL ROUTINES:
 *    vbtBoardAccessSetup  Setup adapter access pointers, test board.
 *    vbtPageDataCopy      Transfers data to/from a single page.
 *
 * FUNCTIONS USED BY IPD, PCMCIA and ISA DEFINITION MODULES
 *    vbtMapPage           Given an adapter offset, switches to correct page
 *                         and returns pointer to specified offset in page.
 *    vbtSetFrame          Sets a new value into the frame register.
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  03/31/1999   Finished the hardware interrupt support code.V3.03.ajh
  04/26/1999   Fixed PC-1553 code to support mapping at E000:0000.  Fixed
               lowlevel.c interface problems with PC-1553 when using offset
               device numbers from the BUSAPI32.INI file.V3.05.ajh
  07/23/1999   Moved functions from btdrv.c into hwsetup.c and ipsetup.c.V3.20.ajh
  08/30/1999   Added ability to change polling interval through .ini file.V3.20.ajh
  12/30/1999   Changed code to clean up the conditional compiles.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  02/23/2000   Completed support for the IP-1553 PROM Version 5.V4.01.ajh
  07/31/2000   Fixed error in vbtBoardAccessSetup for the ISA-1553.V4.08.ajh
               Fix vbtSetup to not call GetPrivateProfileInt in Win3.11 to
               read the board address(it returns only 16-bits).V4.09.ajh
  08/19/2000   Modify vbtSetup in DOS mode to set bt_inuse[cardnum].  Modify
               vbtShutdown to handle the ISA-1553 in Win 3.11.V4.11.ajh
  11/06/2000   Fix vbtSetup in TNT mode to initialize DumpOnBMStop[cardnum]
               so the dump function is not automatically called.V4.20.ajh
  01/07/2002   Added supprt for Quad-PMC and Dual Channel IP V4.46 rhc
  10/22/2003   Add support for 32-Bit memory access
  11/25/2003   Fixed SlotOpened problem.
  02/19/2004   BusTools_API_Open and PCCard-D15553
  12/30/2005   Modified for improve portability and common vbtSetup
  12/30/2005   Add vbtRead_iq function for more efficient processing of IQ data.
  08/30/2006   Add AMC-1553 Support
  12/07/2006   Remove platform specific include and initalization.
  11/19/2007   Added code for vbtSetPLXRegister32 and vbtSetPLXRegister8.The 
               functions were added in support of DMA.
  06/29/2009   Add support for the RXMC-1553
 */

/*---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#define _BT_GLOBALS_  /* This module allocates the memory for the driver      */
#include "btdrv.h"
#include "lowlevel.h"

#if 0
#define debugMessageBox(a, b, c, d)  MessageBox((a), (b), (c), (d))
#else
#define debugMessageBox(a, b, c, d)
#endif

/*---------------------------------------------------------------------------*
 *                    Static Data Base
 *---------------------------------------------------------------------------*/
static int SlotOpened[MAX_BTA][MAX_BTA] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},};   // Contains cardnum+1 if entry open.

/*===========================================================================*
 * LOCAL ENTRY POINT:            v b t M a p P a g e
 *===========================================================================*
 *
 * FUNCTION:    This function is used to setup for access to dual-port memory.
 *
 * DESCRIPTION: Given an adapter offset, calculates correct page and returns
 *              a pointer to specified offset in the page.  Pages can be up
 *              to 1024 Kb long, thus requiring 32 bit arithmetic.
 *
 *              Note that this function is very time critical.
 *
 *      It will return:
 *              byte pointer to specified element in page as mapped by
 *              "framereg".  This pointer is valid for "pagebytes".
 *===========================================================================*/
LPSTR vbtMapPage(
   BT_UINT    cardnum,       // (i) card number
   BT_U32BIT  offset,        // (i) byte address within adapter memory
   BT_U32BIT *pagebytes,     // (o) number of bytes remaining in page
   BT_U16BIT *framereg)      // (o) value of frame register which maps page
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   LPSTR      lppage;        // Pointer to the requested byte address.
   BT_U32BIT  rawframe;      // Raw frame number (high bits of the offset).
   int        ptrnum;        // Index of the pointer which maps this page.
   BT_U32BIT  offsetinpage;  // Offset of requested byte address in page.

   /*************************************************************************
   *  Given a byte offset within adapter memory, setup the page, the
   *   pointer within the page, and the number of bytes which remain
   *   within the page, following the returned pointer. Note that the
   *   number of bytes remaining in the page could be as much as 1024K!
   * We convert a user-specified byte offset "offset" from the beginning of
   *  the board into the following parameters:
   *
   *  bt_FrameReg[] - The actual value needed to program the frame register.
   *  ptrnum        - Index into bt_pageaddr[cardnum][ptrnum].
   *  offsetinpage  - Value to add to bt_pageaddr[cardnum][ptrnum] to get the
   *                  actual host pointer.
   *  pagebytes     - Number of bytes in page that can be accessed via ptr.
   *  lppage        - Host pointer to specified offset within BusTools board.
   *
   * This conversion is performed as follows:
   *************************************************************************/
   // The lower bits of the offset map the location in the page.
   offsetinpage = offset & bt_OffsetMask[cardnum];
   // Compute the number of bytes remaining in the page.
   *pagebytes   = bt_OffsetMask[cardnum] - offsetinpage + 1;
   // Extract the upper bits of the offset; they are the page/frame number.
   rawframe     = offset & ~bt_OffsetMask[cardnum];
   // Shift the page/frame number to match the frame register.
   *framereg    = (BT_U16BIT)(rawframe >> bt_FrameShift[cardnum]);
   // Take the page/frame number and compute the bt_PageAddr[] index.
   ptrnum       = (int)(rawframe >> bt_PtrShift[cardnum]);
   // The host pointer is the base address of the frame plus the offset
   //   into the frame.
   lppage       = bt_PageAddr[cardnum][ptrnum] + (BT_UINT)offsetinpage;

   /*************************************************************************
   * Where: 
   *
   *  bt_OffsetMask[cardnum] - Mask that extracts page offset from board offset
   *  bt_FrameShift[cardnum] - Value to shift raw frame to get actual frame reg
   *  bt_PtrShift[cardnum]   - Value to shift raw frame to get pointer index.
   *************************************************************************/
  return lppage;
}

/*===========================================================================*
 * LOCAL ENTRY POINT:            v b t S e t F r a m e
 *===========================================================================*
 *
 * FUNCTION:    This function sets the BusTools board frame register to the
 *              specified value.  This function should only be called directly
 *              during initialization, since it does not acquire or release
 *              ownership of the Frame Register.
 *
 * DESCRIPTION: The frame register of the PC-, IP- and ISA-1553 is used to
 *              select the page within dual-port memory to be read or written
 *              by the host.
 *              Since there is only one frame register, access to it must be
 *              interlocked so that a thread or interrupt cannot modify the
 *              register while another thread or interrupt function is still
 *              accessing the board.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *              BTD_ERR_BADWCS if error reading back the WCS
 *===========================================================================*/
void vbtSetFrame(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT frame)
{
   if ( frame == bt_FrameReg[cardnum] )
      return;                            // Frame register already set properly.

   bt_FrameReg[cardnum] = frame;         // Remember the setting.

   if ( bt_inuse[cardnum] < 0 )
       return;      // Software versions of the driver have no frame register.

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   // Define the code needed;  The code varies depending on what      +
   //  type of board/carrier/environment we are using.                +
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   // Dispatch to the proper code based on the board type.
   if ( (CurrentCardType[cardnum] == ISA1553) || 
        (CurrentCardType[cardnum] == PCC1553) )  // Single- or multi-function
   {   // ISA-1553.  The fourth page pointer maps the Hardware Interface Regs.
       // The frame register is one of these registers.
       if ( CurrentCardSlot[cardnum] == SLOT_A )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_PAGE_1*2) = frame;
       else
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_PAGE_2*2) = frame;
   }
   else if ( (CurrentCardType[cardnum] == Q1041553) || 
             (CurrentCardType[cardnum] == PCCD1553) ) 
   {   // Q104-1553 ISA.  The fourth page pointer maps the Hardware Interface Regs.
       // The frame register is one of these registers.
       if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_1*2) = frame;
       else if( CurrentCardSlot[cardnum] == CHANNEL_2 )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_2*2) = frame;
       else if( CurrentCardSlot[cardnum] == CHANNEL_3 )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_3*2) = frame;
       else if( CurrentCardSlot[cardnum] == CHANNEL_4 )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_4*2) = frame;
   }
   return;
}


/*===========================================================================*
 * LOCAL ENTRY POINT:        v b t A c q u i r e F r a m e
 *===========================================================================*
 *
 * FUNCTION:    This function first acquires control of the frame, then it
 *              reads the frame register and saves it, then sets it to the
 *              value supplied by the caller.  The value read is returned
 *              to the caller.
 *              This function should be called in pairs with vbtReleaseFrame()
 *              to first acquire then release the ownership of the frame
 *              register.
 *
 * DESCRIPTION: The frame register of the PC-/ISA/PCC/PCCD-1553 is used to
 *              select the page within dual-port memory to be read or written
 *              by the host.
 *              Since there is only one frame register, access to it must be
 *              interlocked so that a thread or interrupt cannot modify the
 *              register while another thread or interrupt function is still
 *              accessing the board.
 *
 *              Note that this function is very time critical.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *              BTD_ERR_BADWCS if error reading back the WCS
 *===========================================================================*/
static BT_U16BIT vbtAcquireFrame(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT frame)         // (i) actual value to load into frame register
{
   BT_U16BIT  ret_frame=0;    // Value of the frame register, returned to caller.

   /*******************************************************************
   *  Define the code needed:  the code varies depending on what
   *   type of board/carrier and environment we are using.
   *  PCI-1553 does not call this function.
   *******************************************************************/

   if ( bt_inuse[cardnum] < 0 )
      return 0;      // Software versions of the driver have no frame register.

   CEI_MUTEX_LOCK(&hFrameCritSect[cardnum]);

   if ( frame == bt_FrameReg[cardnum] )
      return frame;                      // Frame register already set properly.


   bt_FrameReg[cardnum] = frame;         // Remember the new setting.

   // Dispatch to the proper code based on the board type.
   if ( CurrentCardType[cardnum] == ISA1553 || CurrentCardType[cardnum] == PCC1553)  // Single- or multi-function
   {   // ISA-1553.  The fourth page pointer maps the Hardware Interface Regs.
       // The frame register is one of these registers.
       if ( CurrentCardSlot[cardnum] == SLOT_A )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_PAGE_1*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_PAGE_1*2) = frame;
       }
       else
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_PAGE_2*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_PAGE_2*2) = frame;
       }
   }
   else if ( CurrentCardType[cardnum] == Q1041553 || CurrentCardType[cardnum] == PCCD1553 )  // Single- or multi-function
   {   // Q104-1553 ISA.  The fourth page pointer maps the Hardware Interface Regs.
       // The frame register is one of these registers.
       if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_1*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_1*2) = frame;
       }
       else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_2*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_2*2) = frame;
       }
       else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_3*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_3*2) = frame;
       }
       else if ( CurrentCardSlot[cardnum] == CHANNEL_4 )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_4*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_4*2) = frame;
       }
   }

   return (BT_U16BIT)(ret_frame & bt_FrameMask[cardnum]);
}


/*===========================================================================*
 * LOCAL ENTRY POINT:       v b t R e l e a s e F r a m e
 *===========================================================================*
 *
 * FUNCTION:    This function reloads the BusTools board frame register with
 *              the specified value, then releases it.  This function should
 *              be called in pairs with vbtAcquireFrame() to first acquire then
 *              release the ownership of the frame register...
 *
 * DESCRIPTION: The frame register of the PC-, IP- and ISA-1553 is used to
 *              select the page within dual-port memory to be read or written
 *              by the host.
 *              Since there is only one frame register, access to it must be
 *              interlocked so that a thread or interrupt cannot modify the
 *              register while another thread or interrupt function is still
 *              accessing the board.
 *
 *              Note that this function is very time critical.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *              BTD_ERR_BADWCS if error reading back the WCS
 *===========================================================================*/
 static void vbtReleaseFrame(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT frame)
{
   if ( bt_inuse[cardnum] < 0 )
      return;      // Software versions of the driver have no frame register.

   CEI_MUTEX_UNLOCK(&hFrameCritSect[cardnum]);   // Leave Critical Section.

   return;
}

#ifdef PLX_DEBUG
NOMANGLE BT_INT CCONV BusTools_PLX_Dump(cardnum)
{
   int i;
   BT_U32BIT * plx_reg;

   if( (CurrentCardType[cardnum] != QPCI1553) &&
       (CurrentCardType[cardnum] != QPCX1553) &&
       (CurrentCardType[cardnum] != QCP1553) )
      return 9876;
       
   plx_reg = (BT_U32BIT *)bt_iobase[cardnum];
   for(i=0;i<200;i++)
      printf("PLX REG[%x] = %lx\n",i,plx_reg[i]);

   return 0;
}
#endif //PLX_DEBUG


/*===========================================================================*
 * EXTERNAL ENTRY POINT:  v b t S e t u p
 *===========================================================================*
 *
 * FUNCTION:    Setup specified adapter for read/write access.
 *
 * DESCRIPTION: Specified adapter is initialized, or the software demo mode
 *              is initialized.
 *
 *      It will return:
 *              BTD_OK if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtSetup(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT phys_addr,     // (i) 32 bit base address of board
   BT_U32BIT ioaddr,        // (i) 32 bit I/O address
   BT_UINT   HWflag)        // (i) hw/sw flag: 0->sw, 1->hw, 2->HW interrupt enable
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int       status;
   int       i;

   AddTrace(cardnum, NVBTSETUP, bt_inuse[cardnum], phys_addr, ioaddr, 0, 0);
   /*******************************************************************
   *  Basic parameter and state verification
   *******************************************************************/
   if ( bt_inuse[cardnum] )
      return BTD_ERR_INUSE;       // Return card already initialized.

   /*******************************************************************
   *  Save the I/O base register address, and set page to 0
   *******************************************************************/
   _HW_WCSRev[cardnum]  = 0;           //
   _HW_PROMRev[cardnum] = 0;           //  for both WCS and PROM versions.
   bt_iobase[cardnum] = ioaddr;        // Used to setup the page frame reg.
   // We cannot change the frame register until we have mapped the board...
   bt_FrameReg[cardnum] = 0xFFFF;      // Current frame setting is unknown.
   for (i = 0; i < BT_NUM_PAGES; i++)
      bt_PageAddr[cardnum][i] = NULL;  // Initialize the Host Address Pointers.

   if ( phys_addr < MAX_BTA )
      api_device[cardnum] = phys_addr;    // V4.00.ajh
   else
      api_device[cardnum] = cardnum;

   hw_int_enable[cardnum] = HWflag;

   // Clear out the Register Function pointer array.
   RegisterFunctionOpen(cardnum);

   // Setup the API Bus Monitor buffer for this card.
   lpAPI_BM_Buffers[cardnum] = CEI_MALLOC((NAPI_BM_BUFFERS)*sizeof(API_BM_MBUF));

   nAPI_BM_Head[cardnum] = nAPI_BM_Tail[cardnum] = 0;    // FIFO is empty.
   if ( lpAPI_BM_Buffers[cardnum] == NULL )
      return BTD_ERR_NOMEMORY;

#ifdef _USER_INIT_
   /*******************************************************************
   *  software simulation 
   *******************************************************************/
   if(CurrentPlatform[cardnum] == PLATFORM_USER)
      strcpy(bt_UserDLLName[cardnum],(char *)CurrentMemMap[cardnum]);
#endif //_USER_INIT_

   if ( HWflag == 0 )
   {
#ifdef DEMO_CODE
      // Simulation, allocate 256 Kbytes of RAM
      bt_inuse[cardnum] = -1;      // Set software version of driver.V4.30.ajh

      if ( (status = vbtDemoSetup(cardnum)) != BTD_OK )
      {
         bt_inuse[cardnum] = 0;    // Driver not initialized.V4.30.ajh
         return status;
      }
#else
      return API_NO_BUILD_SUPPORT;
#endif //DEMO_CODE
   }
   else
   {
      // See if the device and slot we are trying to open is already open.
      if ( SlotOpened[api_device[cardnum]][CurrentCardSlot[cardnum]] )
      {
            return API_CHANNEL_OPEN_OTHER;
      }

      // Initialize a Critical Section to protect the HW frame register.
      // We do this even for boards that do not have a frame register,
      //  except for demo mode.
      CEI_MUTEX_CREATE(&hFrameCritSect[cardnum]);

      //  Hardware Version, setup paged access to BusTools Adapter.
      bt_inuse[cardnum] = 1;         // Set hardware version of driver.
      if ( (status = vbtBoardAccessSetup(cardnum,phys_addr)) != BTD_OK )
      {
         vbtShutdown(cardnum);       // Release mapped memory, etc.
         return status;
      }
   }

   /*******************************************************************
   *  Setup the timer callback polling routine, error reporting thread
   *   and hardware interrupt functions.
   *******************************************************************/
   SlotOpened[api_device[cardnum]][CurrentCardSlot[cardnum]] =
              cardnum+1;

   API_InterruptInit(cardnum);
   if((hw_int_enable[cardnum] >= API_DEMO_MODE) && (hw_int_enable[cardnum] < API_MANUAL_INT))
   {
      status = vbtInterruptSetup(cardnum, hw_int_enable[cardnum],
                                 api_device[cardnum]);
      if ( status )
         vbtShutdown(cardnum);
   }

   return status;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:           v b t S h u t d o w n
 *===========================================================================*
 *
 * FUNCTION:    Disables access to specified adapter.
 *
 * DESCRIPTION: The specified adapter is marked shutdown, all allocated memory
 *              is freed, and the memory-mapping selectors are released.  The
 *              timer callback is distroyed if this is the last operational
 *              adapter.
 *
 *      It will return:
 *              Nothing.
 *===========================================================================*/

void vbtShutdown( BT_UINT   cardnum)       // (i) card number (0 based)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int     bt_inuse_local;

   /*******************************************************************
   *  Basic parameter and state verification
   *******************************************************************/
   AddTrace(cardnum, NVBTSHUTDOWN, bc_running[cardnum],
                     bm_running[cardnum], rt_running[cardnum], 0, 0);

   bt_inuse_local      = bt_inuse[cardnum];
   bm_running[cardnum] = 0;         // BM is shutdown.
   bt_inuse[cardnum]   = 0;         // Card is shutdown.

   if ( bt_inuse_local == 0 )
      return;                       // Card is already shutdown.
   /*******************************************************************
   *  If the software version of the driver is running, we must release
   *   the memory we allocated to simulate the board.  If the hardware
   *   version is running, the protected mode selectors must be released.
   *  Only selectors which map memory above 1 meg must be released.
   *******************************************************************/
   if ( bt_inuse_local < 0 )  // -1=SW version.
   {
      /*******************************************************************
      *  Release the Bus Monitor buffer we allocated.
      *******************************************************************/
      if ( lpAPI_BM_Buffers[cardnum] != NULL )
      {
         CEI_FREE(lpAPI_BM_Buffers[cardnum]);
         lpAPI_BM_Buffers[cardnum] = NULL;
      }
      vbtDemoShutdown(cardnum);
      return;
   }

   RegisterFunctionClose(cardnum);  // Close down user threads.
   if((hw_int_enable[cardnum] >= API_DEMO_MODE) && (hw_int_enable[cardnum] < API_MANUAL_INT))
     vbtInterruptClose(cardnum);      // Close down interrupt and thread processing.

   /*******************************************************************
   *  Clear all of the DLL function addresses.
   *******************************************************************/
#if defined(_USER_DLL_)
   pUsrAPI_Close[cardnum]        = NULL;
   pUsrBC_MessageAlloc[cardnum]  = NULL;
   pUsrBC_MessageRead[cardnum]   = NULL;
   pUsrBC_MessageUpdate[cardnum] = NULL;
   pUsrBC_MessageWrite[cardnum]  = NULL;
   pUsrBC_StartStop[cardnum]     = NULL;
   pUsrBM_MessageAlloc[cardnum]  = NULL;
   pUsrBM_MessageRead[cardnum]   = NULL;
   pUsrBM_StartStop[cardnum]     = NULL;
   pUsrRT_CbufWrite[cardnum]     = NULL;
   pUsrRT_MessageRead[cardnum]   = NULL;
   pUsrRT_StartStop[cardnum]     = NULL;
#endif

   /*******************************************************************
   *  For the PCI/PMC/ISA-1553, power off the channel we are closing.
   *******************************************************************/
    if ((CurrentCardType[cardnum] == PCI1553) ||
        (CurrentCardType[cardnum] == PMC1553) ||
        (CurrentCardType[cardnum] == ISA1553)   )
   {
      if ( bt_PageAddr[cardnum][3] != NULL ) // Pointer must be valid...
      {
         if ( CurrentCardSlot[cardnum] == SLOT_A )
            bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0xBF; // Clear the power on bit
         else // if ( SupportedCardSlot[cardnum] == SLOT_B )
            bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0x7F;  // Clear the power on bit
      }
   }

   /*******************************************************************
   *  Release the Bus Monitor buffer we allocated.
   *******************************************************************/
   if ( lpAPI_BM_Buffers[cardnum] != NULL )
   {
      CEI_FREE(lpAPI_BM_Buffers[cardnum]);
      lpAPI_BM_Buffers[cardnum] = NULL;
   }

   CEI_MUTEX_DESTROY(&hFrameCritSect[cardnum]);

   SlotOpened[api_device[cardnum]][CurrentCardSlot[cardnum]] = 0;
   // In 95/98/NT/2000 free the critical section protecting the frame register.
   // We do this even for boards which do not need a critical section...

#ifdef _VMIC_
   if ( CurrentPlatform[cardnum] == PLATFORM_VMIC )
   {
      vbtFreeVMICAddress(cardnum);  // Free the VMIC pointers
   }
   return;
#endif //VMIC
  
#ifdef _USER_INIT_
   if ( CurrentPlatform[cardnum] == PLATFORM_USER )
   {
      // Call the user supplied function in the user specified DLL,
      //  and close it down.  Ignor errors since we are shutting down.
      vbtMapUserBoardAddress(cardnum, 0, 0, 0, bt_UserDLLName[cardnum]);
   }
#endif //_USER_INIT_

   if ( CurrentPlatform[cardnum] == PLATFORM_PC )
      vbtFreeBoardAddresses(&bt_devmap[api_device[cardnum]]);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t R e a d T i m e T a g
 *===========================================================================*
 *
 * FUNCTION:    Read the value of the 45-bit time tag counter register.
 *
 * DESCRIPTION: The value of the time tag register is read and returned.
 *              Care is taken to insure that the components do not wrap
 *              while reading the 3 16-bit words.  Byte offsets 0x2A-0x2F.
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

void vbtReadTimeTag(
   BT_UINT   cardnum,          // (i) card number (0 based)
   BT_U16BIT * timetag)    // (o) resulting 48-bit time value from HW
{
   BT_U16BIT volatile *hw_ttc; // Pointer to the time tag registers(3)
   BT_U16BIT volatile *hw_ttrd;// new time read register address
   BT_U16BIT jdata = 0x0;

  // Get a pointer to the ISA/PCI/PMC-1553 time tag counter hardware register

   hw_ttc = ((BT_U16BIT *)(bt_PageAddr[cardnum][1])+HWREG_READ_T_TAG);
   

   if(_HW_FPGARev[cardnum] > 0x385)
   {
      hw_ttrd  = ((BT_U16BIT *)(bt_PageAddr[cardnum][1])+HWREG_READ_T_TAG386);

      // Now read the time tag counter
      *(BT_U16BIT *)hw_ttc  = (BT_U16BIT)jdata;             // Initialize the time tag counter read register

#if defined (PPC_SYNC)
      IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
 eieio
 sync
#pragma endasm
#endif
     
      timetag[0] = *(BT_U16BIT *)hw_ttrd;     // Read the three words of the TT counter     
      hw_ttrd++;
      timetag[1] = *(BT_U16BIT *)hw_ttrd;     // The write latches the current value?      
      hw_ttrd++;
      timetag[2] = *(BT_U16BIT *)hw_ttrd;
     
   }
   else
   {
      // Now read the time tag counter
      hw_ttc[0]  = 0;             // Initialize the time tag counter read register
      timetag[0] = hw_ttc[0];     // Read the three words of the TT counter
      timetag[1] = hw_ttc[0];     // The write latches the current value?
      timetag[2] = hw_ttc[0];
   }

   flipw(&timetag[0]);
   flipw(&timetag[1]);
   flipw(&timetag[2]);
  
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e T i m e T a g
 *===========================================================================*
 *
 * FUNCTION:    Write the value of the 45-bit time tag load register.
 *
 * DESCRIPTION: The value of the time tag load register is set to the specified
 *              value.
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtWriteTimeTag(
   BT_UINT   cardnum,             // (i) card number
   BT1553_TIME * timetag)     // (i) 48-bit time value to load into register
{
   BT_U16BIT *hw_ttlc;        // Pointer to the Time Tag load registers (3)

   // Write the time tag load register depending on the board type.
   // Caller verified that this board has a writable time tag load register.

   // Get a pointer to the ISA/PCI/PMC-1553 time tag load register
   hw_ttlc = ((BT_U16BIT *)(bt_PageAddr[cardnum][1])+0x20/2);

#if defined (PPC_SYNC)
      IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
 eieio
 sync
#pragma endasm
#endif

   // Load the time tag load register with the specified value

   hw_ttlc[0] = flipws((BT_U16BIT)(timetag->microseconds));       // for PMC on PowerPC   //Endian code
   hw_ttlc[1] = flipws((BT_U16BIT)(timetag->microseconds >> 16)); // for PMC on PowerPC   //Endian code
   hw_ttlc[2] = flipws(timetag->topuseconds);                     // for PMC on PowerPC   //Endian code

}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e T i m e T a g I n c r
 *===========================================================================*
 *
 * FUNCTION:    Write the value of the 45-bit time tag load register.
 *
 * DESCRIPTION: The value of the time tag load register is set to the specified
 *              value.
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtWriteTimeTagIncr(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT incr)                // (i) increment value
{
   BT_U16BIT *hw_ttlc;        // Pointer to the Time Tag icrement load registers

   // Get a pointer to the time tag load increment register
   hw_ttlc = ((BT_U16BIT *)(bt_PageAddr[cardnum][1])+0x26/2);

#if defined (PPC_SYNC)
      IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
 eieio
 sync
#pragma endasm
#endif

   // Load the time tag load register with the specified value
   hw_ttlc[0] = flipws((BT_U16BIT)(incr));       //
   hw_ttlc[1] = flipws((BT_U16BIT)(incr >> 16)); //
}

//
/*===========================================================================*
 * EXTERNAL API ENTRY POINT:       v b t G e t P a g e P t r
 *===========================================================================*
 *
 * FUNCTION:    Given adapter offset, switches to correct DP memory page and
 *              returns a pointer to the specified offset within the page.
 *
 * DESCRIPTION: This function is intended to be used by the
 *              BM_MessageConvert() helper function ONLY.
 *              If the requested number of bytes is not present in the
 *              specified page, then the requested number of bytes are
 *              copied from the board (in two steps, of course) into a
 *              caller's buffer, and the address of this buffer is returned.
 *
 *      It will return:
 *              Page pointer.
 *===========================================================================*/
LPSTR vbtGetPagePtr(
   BT_UINT   cardnum,           // (i) card number.
   BT_U32BIT byteOffset,        // (i) offset within adapter memory.
   BT_UINT   bytesneeded,       // (i) number of bytes needed in page.
   LPSTR     local_board_buffer)   // (io) scratch buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT     pagebytes;     // Can be up to the full 1 MByte
   LPSTR         lppage;
   BT_U16BIT     framenum;

   // Dispatch on card type.  Access is always to dual port RAM!
   if ( (CurrentCardType[cardnum] == PCC1553)  ||
        (CurrentCardType[cardnum] == PCCD1553) ||
        (CurrentCardType[cardnum] == ISA1553)  ||
        (CurrentCardType[cardnum] == Q1041553) )
   {
      /****************************************************************
      * Adaptor requires paged access.
      * Given a byte offset within adapter memory, setup the page and
      * the pointer within the page, and the number of bytes
      * which remain within the page, following the returned pointer
      * (returned in "pagebytes")
      ****************************************************************/
      lppage = vbtMapPage(cardnum,byteOffset,&pagebytes,&framenum);

      /****************************************************************
      * If the current page does not contain all of the needed data, read
      *  the data from the board into our local buffer, and return the
      *  pointer to the local buffer.  We never check for a request for
      *  more data than will fit into the caller's buffer...
      * Certain PLATFORM_USER's cannot properly read 32-bit quantities.
      *  We just do the 16-bit copy for all such platforms, rather than
      *  try to keep track of which work and which don't work.
      ****************************************************************/

      if ( (bytesneeded > pagebytes) || (CurrentPlatform[cardnum] == PLATFORM_USER) )
      {
         vbtRead(cardnum, local_board_buffer, byteOffset, bytesneeded);
         return local_board_buffer;
      }

      /****************************************************************
      * Requested number of bytes does not cross a page boundry.  Setup
      *  the frame register, and return a pointer to the board.  Assume
      *  that the frame register has been locked by the caller!
      ****************************************************************/
      vbtSetFrame(cardnum,framenum);
      return lppage;
   }
   else // if ( (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {
      // Adaptor is flat-mapped, no frame register. Return a byte pointer
      //  to the specified byte offset in Dual Port MEMORY space.

#ifndef NON_INTEL_WORD_ORDER //
      return (bt_PageAddr[cardnum][0]+byteOffset);
#else
      if(board_access_32[cardnum])
         vbtRead32(cardnum, local_board_buffer, byteOffset, bytesneeded);
      else
         vbtRead(cardnum, local_board_buffer, byteOffset, bytesneeded);
      return local_board_buffer;
#endif
   }
}

//
/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t P a g e D a t a C o p y
 *===========================================================================*
 *
 * FUNCTION:    Copies data to or from a single page, up to number of bytes
 *              requested or to the end of the page.
 *
 *      It will return:
 *              Number of bytes read.
 *===========================================================================*/

static BT_UINT vbtPageDataCopy(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer to copy data to
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory
   BT_UINT   bytesToRead,   // (i) number of bytes to copy
   int       direction)     // (i) 0=read from board, 1=write to board
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   LPSTR      lppage;
   BT_U32BIT  pagebytes;  // Can be up to the full 256 Kb
   BT_UINT    bytestocopy;
   BT_U16BIT  framenum;
   BT_U16BIT  wCurrentFrame;
   BT_UINT i;

   /*******************************************************************
   * Given a byte offset within adapter memory, setup the page and
   *  the pointer within the page, and the number of bytes
   *  which remain within the page, following the returned pointer
   *  (returned in "pagebytes")
   *******************************************************************/
   lppage = vbtMapPage(cardnum,byteOffset,&pagebytes,&framenum);

   if ( bytesToRead > pagebytes )
      bytestocopy = (BT_UINT)pagebytes; 
   else
      bytestocopy = bytesToRead;

   // Save current frame, set frame to value returned by GetPagePtr.
   wCurrentFrame = vbtAcquireFrame(cardnum, framenum);

   // Code this routine using in-line assembly, to obtain max speed
   //  AND to insure that only WORD (or DWORD) reads are made from the board.

   if ( direction )
   {
      for ( i = 0; i < bytestocopy/2; i++ )
         ((BT_U16BIT *)lppage)[i] = ((BT_U16BIT *)lpbuffer)[i];
   }
   else
   {
      for ( i = 0; i < bytestocopy/2; i++ )
         ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)lppage)[i];
   }
   // Release and restore the frame register to the value we saved.
   vbtReleaseFrame(cardnum, wCurrentFrame);
   return(bytestocopy);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtRead(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    bytesread;          // bytes read
   BT_UINT    i;                  // Word counter

   // Dispatch on card and access type.
   if ( ((CurrentCardType[cardnum] == PCC1553)  && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == Q1041553) && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == ISA1553)  && (byteOffset >= 0x80*2)) )
   {
      // Paged access board, setup and read from page(s).
      do
      {
         bytesread = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToRead,0);

         lpbuffer     += bytesread;     // Increment user buffer pointer.
         byteOffset   += bytesread;     // Increment board address offset.
         bytesToRead  -= bytesread;     // Decrement bytes to be read.
      }
      while( bytesToRead );
   }
   else // if ( (CurrentCardType[cardnum] == ISA1553) ||
        //      (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {
      /****************************************************************
      * Non-paged access.  Map to proper address offset and read memory,
      *  based on the first offset given.  This is to support access to
      *  the HW and RAM registers via this function.
      * Code the move using in-line assembly to obtain max performance.
      ****************************************************************/
      if ( byteOffset < HWREG_COUNT*2 )
      {  // Hardware Register area.  Only the first register is readable,
         //  attempting to read any more will hang the PCI bus!
         //memset(lpbuffer, 0xFF, bytesToRead);
         //if ( byteOffset == 0 )
         //   *(BT_U16BIT *)lpbuffer = *(BT_U16BIT *)bt_PageAddr[cardnum][1];
         for ( i = 0; i < bytesToRead/2; i++ )
         {
           ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][1]+byteOffset))[i];
            flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PMC on PowerPC
           //bt_memcpy(lpbuffer, bt_PageAddr[cardnum][1]+byteOffset, bytesToRead);
         }
      }
      else if ( byteOffset < 0x80*2 )
      {  // RAM(File) Register area.  Can only be read as WORDS.
         for ( i = 0; i < bytesToRead/2; i++ )
         {
            ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][2]+byteOffset))[i];
            flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PMC on PowerPC
            // bt_memcpy(lpbuffer, bt_PageAddr[cardnum][2]+byteOffset, bytesToRead);
         }
      }
      else if ( byteOffset < 0x80000*2 ) // Byte offset indicates Memory area.
      {  // Dual-Port Memory area.  Can only be read as WORDS.
         for ( i = 0; i < bytesToRead/2; i++ )
         {
            ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
            flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PMC on PowerPC
            // bt_memcpy(lpbuffer, bt_PageAddr[cardnum][0]+byteOffset, bytesToRead);
         }
      }
      else
      {		  
         debugMessageBox(NULL, "Address out of bounds", "vbtRead", MB_OK | MB_SYSTEMMODAL);		 
         vbtShutdown(cardnum);
         exit(0);
      }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d R A M
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of RAM memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtReadRAM8(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;                  // Word counter

   for ( i = 0; i < bytesToRead; i++ )
   {
      lpbuffer[i] = ((char *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d R A M
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of RAM memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtReadRAM(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    bytesread;          // bytes read
   BT_UINT    i;                  // Word counter

   // Dispatch on card and access type.
   if ( CurrentCardType[cardnum] == PCC1553  ||
        CurrentCardType[cardnum] == PCCD1553 ||
        CurrentCardType[cardnum] == Q1041553 ||
        CurrentCardType[cardnum] == ISA1553 )
   {
      // Paged access board, setup and read from page(s).
      do
      {
         bytesread = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToRead,0);

         lpbuffer     += bytesread;     // Increment user buffer pointer.
         byteOffset   += bytesread;     // Increment board address offset.
         bytesToRead  -= bytesread;     // Decrement bytes to be read.
      }
      while( bytesToRead );
   }
   else 
   {
      for ( i = 0; i < bytesToRead/2; i++ )
      {
         ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
          flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PowerPC
      }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d H I F
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of HIF memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtReadHIF(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    bytesread;          // bytes read
   BT_UINT    i;                  // Word counter

   // Dispatch on card and access type.
   if ( (CurrentCardType[cardnum] == PCC1553)  ||
        (CurrentCardType[cardnum] == PCCD1553) ||
        (CurrentCardType[cardnum] == Q1041553) ||
        (CurrentCardType[cardnum] == ISA1553) )
   {
      // Paged access board, setup and read from page(s).
      do
      {
         bytesread = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToRead,0);

         lpbuffer     += bytesread;     // Increment user buffer pointer.
         byteOffset   += bytesread;     // Increment board address offset.
         bytesToRead  -= bytesread;     // Decrement bytes to be read.
      }
      while( bytesToRead );
   }
   else 
   {
      for ( i = 0; i < bytesToRead/2; i++ )
      {
         ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][3]+byteOffset))[i];
          flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PowerPC
      }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e H I F
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWriteHIF(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    byteswritten;       // bytes written
   BT_UINT    i;

   // Dispatch on card and access type.
   if ( ((CurrentCardType[cardnum] == PCC1553)  && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == Q1041553) && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == ISA1553)  && (byteOffset >= 0x80*2) ) )
   {
      // Paged access board, setup and write to page(s).
      while(bytesToWrite)
      {
         byteswritten = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToWrite,1);

         lpbuffer     += byteswritten;  // Increment user buffer pointer.
         byteOffset   += byteswritten;  // Increment board address offset.
         bytesToWrite -= byteswritten;  // Decrement bytes to be written.
      }
   }
   else // if ( (CurrentCardType[cardnum] == ISA1553) ||
        //      (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {

      for ( i = 0; i < bytesToWrite/2; i++ )
      {
          ((BT_U16BIT *)(bt_PageAddr[cardnum][3]+byteOffset))[i] =  flipws(((BT_U16BIT *)lpbuffer)[i]);
      }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d_iq
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtRead_iq(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    bytesread;          // bytes read
   BT_UINT    i;                  // Word counter

   // Dispatch on card and access type.
   if ( (CurrentCardType[cardnum] == PCC1553)  ||
        (CurrentCardType[cardnum] == PCCD1553) ||
        (CurrentCardType[cardnum] == Q1041553) ||
        (CurrentCardType[cardnum] == ISA1553)   )
   {
      // Paged access board, setup and read from page(s).
      do
      {
         bytesread = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToRead,0);

         lpbuffer     += bytesread;     // Increment user buffer pointer.
         byteOffset   += bytesread;     // Increment board address offset.
         bytesToRead  -= bytesread;     // Decrement bytes to be read.
      }
      while( bytesToRead );
   }
   else // if ( (CurrentCardType[cardnum] == ISA1553) ||
        //      (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {    // Dual-Port Memory area.  Can only be read as WORDS.
      for ( i = 0; i < bytesToRead/2; i++ )
      {
         ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
         flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PMC on PowerPC
      }
   }
}

//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d 3 2
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtRead32(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT i;
  
   if((bytesToRead % 4)==0)
   {
#ifdef WORD_SWAP
      for ( i = 0; i < bytesToRead/4; i++ )
      {
         ((BT_U32BIT *)lpbuffer)[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
      }
      for( i = 0; i < bytesToRead/2; i++ )
      {
         flipw(&(((BT_U16BIT *)lpbuffer)[i]));
      }
#else //WORD_SWAP
      for ( i = 0; i < bytesToRead/4; i++ )
      {
         ((BT_U32BIT *)lpbuffer)[i] = flips(((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i]);
      }
#endif //WORD_SWAP
   }
   else
   {
      channel_status[cardnum].byte_cnt_err=1;
   }   
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWrite(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    byteswritten;       // bytes written
   BT_UINT    i;

   // Dispatch on card and access type.
   if ( ((CurrentCardType[cardnum] == PCC1553)  && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == Q1041553) && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == ISA1553)  && (byteOffset >= 0x80*2) ) )
   {
      // Paged access board, setup and write to page(s).
      while(bytesToWrite)
      {
         byteswritten = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToWrite,1);

         lpbuffer     += byteswritten;  // Increment user buffer pointer.
         byteOffset   += byteswritten;  // Increment board address offset.
         bytesToWrite -= byteswritten;  // Decrement bytes to be written.
      }
   }
   else // if ( (CurrentCardType[cardnum] == ISA1553) ||
        //      (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {
      // Non-paged access board, write to the specified location...
      if ( byteOffset < 0x10*2 )        // Byte offset indicates HW Regs?
      {
         // Hardware register is being accessed (offset 0x400000).
         for ( i = 0; i < bytesToWrite/2; i++ )
         {
            ((BT_U16BIT *)(bt_PageAddr[cardnum][1]+byteOffset))[i] =  flipws(((BT_U16BIT *)lpbuffer)[i]);
         }
      }
      else if ( byteOffset < 0x80*2 )  // Byte offset indicates RAM Regs?
      {
         // RAM Register (File Register) is being accessed (offset 0x500000).
         for ( i = 0; i < bytesToWrite/2; i++ )
         {
            ((BT_U16BIT *)(bt_PageAddr[cardnum][2]+byteOffset))[i] = flipws(((BT_U16BIT *)lpbuffer)[i]);
         }
      }
      else if ( byteOffset <= 0x7FFFF*2 ) // Byte offset indicates Memory area.
      {
         // Non-paged access.  Map to proper address offset and write memory.
         // Code this routine using in-line assembly, to obtain max speed
         //  AND to insure that only WORD (or DWORD) writes are made from the board.
         for ( i = 0; i < bytesToWrite/2; i++ ) //Was bytesToWrite/4
         {
            ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = flipws(((BT_U16BIT *)lpbuffer)[i]);
         }
      }

      else
      {		  
         debugMessageBox(NULL, "Address out of bounds", "vbtWrite", MB_OK | MB_SYSTEMMODAL);	 
         vbtShutdown(cardnum);
         exit(0);
      }
   }
}


/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e R A M 8
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWriteRAM8(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;

   for ( i = 0; i < bytesToWrite; i++ ) //Was bytesToWrite/4
   {
      ((char *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = lpbuffer[i];
   }
}


/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e R A M
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWriteRAM(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    byteswritten;       // bytes written
   BT_UINT    i;

   // Dispatch on card and access type.
   if ( ((CurrentCardType[cardnum] == PCC1553)  && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == Q1041553) && (byteOffset >= 0x80*2)) ||
        ((CurrentCardType[cardnum] == ISA1553)  && (byteOffset >= 0x80*2) ) )
   {
      // Paged access board, setup and write to page(s).
      while(bytesToWrite)
      {
         byteswritten = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToWrite,1);

         lpbuffer     += byteswritten;  // Increment user buffer pointer.
         byteOffset   += byteswritten;  // Increment board address offset.
         bytesToWrite -= byteswritten;  // Decrement bytes to be written.
      }
   }
   else // if ( (CurrentCardType[cardnum] == ISA1553) ||
        //      (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {
      
      // Non-paged access.  Map to proper address offset and write memory.
      // Code this routine using in-line assembly, to obtain max speed
      //  AND to insure that only WORD (or DWORD) writes are made from the board.
      for ( i = 0; i < bytesToWrite/2; i++ ) //Was bytesToWrite/4
      {
         ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = flipws(((BT_U16BIT *)lpbuffer)[i]);
      }
   }
}


 /*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e 3 2
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWrite32(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;

   if((bytesToWrite % 4)==0)
   {
// lock Mutex here
#ifdef WORD_SWAP
      for( i = 0; i < bytesToWrite/2; i++ )
      {
         flipw(&(((BT_U16BIT *)lpbuffer)[i]));
      }
      for ( i = 0; i < bytesToWrite/4; i++ )
      {
         ((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = ((BT_U32BIT *)lpbuffer)[i];
      }
#else //WORD_SWAP
      for ( i = 0; i < bytesToWrite/4; i++ )
      {
         ((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = flips(((BT_U32BIT *)lpbuffer)[i]);
      }
#endif //WORD_SWAP
//Unlock mutex here
   }
   else
   {
      channel_status[cardnum].byte_cnt_err=1;
   }
}

//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t R e a d M o d i f y W r i t e
 *===========================================================================*
 *
 * FUNCTION:    Read and update a single word in adapter memory.
 *              This routine reads and restores the contents of the
 *              frame mapping register, so it is interrupt safe...
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              BTD_OK            if successful.
 *              BTD_ERR_PARAM     if board number too large.
 *              BTD_ERR_NOTSETUP  if board has not been setup.
 *===========================================================================*/

BT_U16BIT vbtReadModifyWrite(
   BT_UINT   cardnum,       // (i) card number
   BT_UINT   region,        // (i) HWREG,FILEREG, or RAM
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_U16BIT wNewWord,      // (i) new value to be written under mask
   BT_U16BIT wNewWordMask)  // (i) mask for new value
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT  regval;
   BT_U32BIT  pagebytes;      // Can be up to the full 256 Kb
   BT_U16BIT  wCurrentFrame;
   LPSTR      lppage = NULL;
   BT_U16BIT  framenum;

   // Dispatch on card and access type.
   if ( ((CurrentCardType[cardnum] == PCC1553) && (byteOffset >= 0x80*2)) ||
      ( (CurrentCardType[cardnum] == Q1041553) && (byteOffset >= 0x80*2)) ||
      ( (CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2)) ||
      ( (CurrentCardType[cardnum] == ISA1553)  && (byteOffset >= 0x80*2) ) )
   {
      /****************************************************************
      * Paged access board, setup and read/modify/write to page.
      * Given a byte offset within adapter memory, setup the page and
      *  the pointer within the page, and the number of bytes
      *  which remain within the page, following the returned pointer
      *  (returned in "pagebytes")
      ****************************************************************/
      lppage = vbtMapPage(cardnum,byteOffset,&pagebytes,&framenum);

      // Save current frame, set frame to value returned by GetPagePtr.
      wCurrentFrame = vbtAcquireFrame(cardnum, framenum);

      regval  = *(BT_U16BIT *)(lppage);

      *(BT_U16BIT *)(lppage) = (BT_U16BIT)((regval & ~wNewWordMask) |
                                           (wNewWord & wNewWordMask));

      // Restore the frame register to the value we saved.
      vbtReleaseFrame(cardnum, wCurrentFrame);
   }
   else // if ( (CurrentCardType[cardnum] == ISA1553) ||
        //      (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {
      // Non-paged access board, read/modify/write specified location...
      if ( region == HWREG)        // Byte offset indicates HW Reg?
      {
         // Hardware register is being accessed (offset 0x400000).
         lppage = bt_PageAddr[cardnum][1] + byteOffset;
      }
      else if ( region == FILEREG )  // Byte offset indicates RAM Reg?
      {
         // RAM Register (File Register) is being accessed (offset 0x500000).
         lppage = bt_PageAddr[cardnum][2] + byteOffset;
      }
      else if ( region == RAM ) // Byte offset indicates Memory area.
      {
         // Memory is being accessed (offset 0x200000).
         lppage = bt_PageAddr[cardnum][0] + byteOffset;
      }
      else
      {		  
         debugMessageBox(NULL, "Address out of bounds", "vbtReadModifyWrite", MB_OK | MB_SYSTEMMODAL);		 
         vbtShutdown(cardnum);
         exit(0);
      }

      // Now read/modify/write the proper location on the PCI-1553.
      regval  = *(BT_U16BIT *)(lppage);
      flipw(&regval); //for PMC on the PowerPC
      regval = (BT_U16BIT)((regval & ~wNewWordMask) |
                                        (wNewWord & wNewWordMask));

      flipw(&regval); //for PMC on the PowerPC
      *(BT_U16BIT *)(lppage) = regval;
      flipw(&regval);
   }
   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then read and the frame reg
 *              is restored to the saved value.  The value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U16BIT vbtGetRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   // Dispatch on card and access type.
   if ( ( (CurrentCardType[cardnum] == ISA1553) && (regnum  >= 0x80) )  ||
        ( (CurrentCardType[cardnum] == PCC1553) && (regnum  >= 0x80) )  ||
        ( (CurrentCardType[cardnum] == PCCD1553) && (regnum >= 0x80))   ||
        ( (CurrentCardType[cardnum] == Q1041553) && (regnum >= 0x80)) )
   {
      // Paged access board, setup and read two bytes from page.
      vbtPageDataCopy(cardnum,(LPSTR)&regval,regnum*2,2,0);
      return regval;
   }
   else // if ( (CurrentCardType[cardnum] == ISA1553) ||
        //      (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {
      // Non-paged access.  Map to proper address offset and read register.
      if ( regnum < 0x20 )
      {  // Hardware register is a WORD address.
         regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][1])+regnum);
         flipw(&regval); // for PMC on PowerPC
      }
      else if ( regnum < 0x80 )  // WORD offset indicates RAM Reg?
      {  // RAM/File Register is a WORD address.
         regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][2])+regnum);
         flipw(&regval); // for PMC on PowerPC
      }
      else
      {  // RAM offset is a WORD address.
         regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][0])+regnum);
         flipw(&regval); // for PMC on PowerPC
      }
      return regval;
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t F i l e R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then read and the frame reg
 *              is restored to the saved value.  The value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U16BIT vbtGetFileRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   // Non-paged access.  Map to proper address offset and read register.
   regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][2])+regnum);
   flipw(&regval); // for PMC on PowerPC
   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t C S C R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: Read the CSC and ACR registers
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U16BIT vbtGetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   // Non-paged access.  Map to proper address offset and read register.
   regval  = *((BT_U16BIT *)(bt_PageAddr[cardnum][3])+regnum);
   flipw(&regval); // for PMC on PowerPC
   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t H W R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then read and the frame reg
 *              is restored to the saved value.  The value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U16BIT vbtGetHWRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   // Non-paged access.  Map to proper address offset and read register.
   // Hardware register is a WORD address.
   regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][1])+regnum);
   flipw(&regval); // for PMC on PowerPC
   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t R e a d S e r i a l N u m b e r
 *===========================================================================*
 *
 * FUNCTION:    reads the serial number on selected boards.
 *
 * DESCRIPTION: This functions read the serial number from flash memory.
 *
 *      It will return:
 *              .API_SUCCESS
 *===========================================================================*/
BT_INT vbtReadSerialNumber(BT_UINT cardnum, BT_U16BIT *serial_number)
{

#define INVERTBITS(b)   (~(b))
#define REVERSEBITS(b)  (BitReverseTable[b])

static CEI_UCHAR BitReverseTable[256] =
{
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

   BT_U16BIT tdata;

   if(CurrentCardType[cardnum] == R15EC  || CurrentCardType[cardnum] == RXMC1553 )
   {
      BT_INT time_count=0;
      BT_U8BIT sn_lsb, sn_msb;

      //Read SN LSB
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x142/2] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x144/2] = flipws(0x1e);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = flipws(0x8);

      while(1)
      {
         tdata = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x140];
         if((tdata & 0x8)==0)
            break;
         MSDELAY(1);
         time_count++;
         if(time_count > 5)
            return API_TIMER_ERR;         
      }
      sn_lsb = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x148];
      
      //Read SN MSB
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x142/2] = flipws(0x1);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x144/2] = flipws(0x1e);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = flipws(0x8);

      while(1)
      {
         tdata = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x140];
         if((tdata & 0x8)==0)
            break;
         MSDELAY(1);
         time_count++;
         if(time_count > 5)
            return API_TIMER_ERR;         
      }
      sn_msb = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x148];     
   
      sn_lsb = REVERSEBITS(sn_lsb);
      sn_msb = REVERSEBITS(sn_msb);

      tdata = (CEI_UINT16)sn_lsb | (CEI_UINT16)(sn_msb << 8);
   }
   else if(CurrentCardType[cardnum] == AR15VPX || CurrentCardType[cardnum] == R15AMC || CurrentCardType[cardnum] == RPCIe1553)
   {
       BT_U32BIT indx,addr32,rdata32[4];
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32[0] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32[0] & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);

       for(indx = 0;indx<2;indx++)
       {
          addr32 = 0x80000000 + 0x520000 + indx;
          ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
          MSDELAY(1);
          
          rdata32[indx] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
          rdata32[indx] = little_endian_32(rdata32[indx]);
          rdata32[indx] = REVERSEBITS(rdata32[indx] & 0xff);
       }
#ifdef NON_INTEL_WORD_ORDER
       tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[1]&0xff) + (((BT_U16BIT)rdata32[0]&0xff)<<8));
#else
       tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[0]&0xff) + (((BT_U16BIT)rdata32[1]&0xff)<<8));
#endif
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   } 
   else
   {
      //clear status
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = flipws(0x50);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata  = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      //read array
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = flipws(0xff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x140a);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x0838);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3];
      flipw(&tdata); // for PMC on PowerPC
   }
   *serial_number = tdata;
   return API_SUCCESS;
}

//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t S e t R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then written and the frame reg
 *              is restored to the saved value.  The old value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void vbtSetRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U16BIT regval)        // (i) new value
{
   // Dispatch on card and access type.
   if ( ( (CurrentCardType[cardnum] == ISA1553)  && (regnum >= 0x80) ) ||
        ( (CurrentCardType[cardnum] == PCC1553)  && (regnum >= 0x80) ) ||
        ( (CurrentCardType[cardnum] == PCCD1553) && (regnum >= 0x80) ) ||
        ( (CurrentCardType[cardnum] == Q1041553) && (regnum >= 0x80)))
   {
      // Paged access board, setup and write two bytes from page.
      vbtPageDataCopy(cardnum,(LPSTR)&regval,regnum*2,2,1);
   }
   else // if ( (CurrentCardType[cardnum] == ISA1553) ||
        //      (CurrentCardType[cardnum] == PCI1553) ||
        //      (CurrentCardType[cardnum] == PMC1553)    )
   {
      // Non-paged access.  Map to proper address offset and write register.
      if ( regnum < HWREG_COUNT )
      {  // Hardware register is a WORD address.
         *((BT_U16BIT *)(bt_PageAddr[cardnum][1])+regnum) = flipws(regval);
         hwreg_value[cardnum][regnum] = regval;
      }
      else  if ( regnum < 0x80 )  // WORD offset indicates RAM Reg?
      {  // RAM(File) Register is a WORD address.
         *((BT_U16BIT *)(bt_PageAddr[cardnum][2])+regnum) = flipws(regval);
      }
      else
      {  // RAM offset is a WORD address.
         *((BT_U16BIT *)(bt_PageAddr[cardnum][0])+regnum) = flipws(regval);
      }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t S e t H W R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then written and the frame reg
 *              is restored to the saved value.  The old value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void vbtSetHWRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U16BIT regval)        // (i) new value
{
   // Hardware register is a WORD address.
   *((BT_U16BIT *)(bt_PageAddr[cardnum][1])+regnum) = flipws(regval);
   hwreg_value[cardnum][regnum] = regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t S e t F i l e R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then written and the frame reg
 *              is restored to the saved value.  The old value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void vbtSetFileRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U16BIT regval)        // (i) new value
{
   // Non-paged access.  Map to proper address offset and write register.
   *((BT_U16BIT *)(bt_PageAddr[cardnum][2])+regnum) = flipws(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t S e t P L X R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into PLX registers.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtSetPLXRegister(BT_UINT   cardnum,      // (i) card number
                       BT_U32BIT regnum,     // (i) host buffer (source)
                       BT_U16BIT regval)   // (o) byte offset within adapter (destination))
{
   *(BT_U16BIT *)(bt_iobase[cardnum]+regnum) = flipws(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t S e t P L X R e g i s t e r 3 2
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into PLX Register.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtSetPLXRegister32(BT_UINT   cardnum,  // (i) card number
                       BT_U32BIT regnum,     // (i) host buffer (source)
                       BT_U32BIT regval)     // (o) byte offset within adapter (destination))
{
   *(BT_U32BIT *)(bt_iobase[cardnum] + regnum) = flipws(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t S e t P L X R e g i s t e r 8
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into PLX Register.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtSetPLXRegister8(BT_UINT   cardnum,  // (i) card number
                       BT_U32BIT regnum,     // (i) host buffer (source)
                       BT_U8BIT regval)     // (o) PLX byte offset
{
   *(BT_U8BIT *)(bt_iobase[cardnum] + regnum) = regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t P L X R e g i s t e r 8
 *===========================================================================*
 *
 * FUNCTION:    Return data from PLX Register.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

BT_U8BIT vbtGetPLXRegister8(BT_UINT   cardnum,    // (i) card number
                            BT_U32BIT regnum)     // (i) PLX register address

{
   return( *(BT_U8BIT *)(bt_iobase[cardnum] + regnum));
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t P L X R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

BT_U16BIT vbtGetPLXRegister(
   BT_UINT   cardnum,      // (i) card number
   BT_U32BIT byteOffset)   // (o) byte offset within adapter (destination))
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   regval = *(BT_U16BIT *)(bt_iobase[cardnum]+byteOffset);
   flipw(&regval);
   return regval;
}

