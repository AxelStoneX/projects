 /*============================================================================*
 * FILE:                        I N I T . C
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
 *             This module contains the intialization, interrupt setup and
 *             memory access functions for the BusTools Application
 *             Programmer's Interface (API).
 *
 *             The first API call must initialize the channel.
 *             These routines call the low-level board setup
 *             function that maps the specified board and makes it ready for
 *             access, including loading the WCS as needed.
 *             The HW microcode registers are loaded and initialized by this
 *             module as required by the specified board.
 *
 * USER ENTRY POINTS: 
 *     BusTools_API_Close        - This procedure releases all API resources
 *     BusTools_API_InitExtended - Init API and board, registers(ALL)
 *     BusTools_API_Reset        - Provides a channel reset
 *     BusTools_API_ShareChannel - Share channel data for use by another application
 *     BusTools_API_JoinChannel  - Join an already running channel
 *     BusTools_API_QuitChannel  - Quit a shared channel
 *     BusTools_BoardHasIRIG     - Returns whether the board has IRIG
 *     BusTools_BoardIsMultiFunction - Return whether the board is single- dual- or multi-function.
 *     BusTools_DMA_Setup        - Configure PLX 9056 for Block DMA
 *     BusTools_ExtTriggerOut    - Pulses the external output trigger.
 *     BusTools_GetAddr          - Returns the start/end addresses of mem block
 *     BusTools_API_GetBaseAddr  - Returns the base address of the board
 *     BusTools_GetBoardType     - Returns the board type programmed for cardnum
 *     BusTools_GetChannelStatus - Return the Channel Status
 *     BusTools_GetChannelCount  - Returns the number of channel on a card
 *     BusTools_GetCSCRegs       - Return the data stored in the CSC and ACR registers 
 *     BusTools_GetFWRevision    - Returns the LPU, WCS and Build versions
 *     BusTools_GetPulse         - Returns the current value of the Heart Beat Register
 *     BusTools_GetRevision      - Returns the API version and board revision
 *     BusTools_GetSerialNumber  - Retunr the serial number for select boards.
 *     BusTools_MemoryAlloc      - Allocates memory for user's use
 *     BusTools_MemoryRead       - Reads the specified block of memory (deprecated)
 *     BusTools_MemoryRead2      - Reads the specified block of memory
 *     BusTools_MemoryWrite      - Writes the specified block of memory (deprecated)
 *     BusTools_MemoryWrite2     - Writes the specified block of memory
 *     BusTools_RAMRegisterRead  - Read specified RAM registers.
 *     BusTools_PCI_Reset        - Enable/Disable PCI resets of the 1553 board F/W 4.20
 *     BusTools_ReadVMEConfig    - Read the VME/QVME-1553 control reigsters
 *     BusTools_SetBroadcast     - Sets the flag for enabling broadcast
 *     BusTools_SetExternalSync  - Enables external sync of time-tag register
 *     BusTools_SetInternalBus   - Sets the flag for external or internal bus
 *     BusTools_SetOptions       - Sets Illegal command, Reset Timetag options
 *     BusTools_SetPolling       - Reset the polling interval
 *     BusTools_SetSa31          - Sets the flag for modecode on SA 31
 *     BusTools_SetTestBus       - Enables/Disables test bus
 *     BusTools_SetVoltage       - Sets the voltage hardware register
 *     BusTools_StatusGetString  - Returns a string describing a status value
 *     BusTools_WriteVMEConfig   - Writes to the VME/QVME-1553 control registers
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *     api_reset                 - Reset the hardware and restores hw registers
 *     api_writehwreg            - Writes word into the specified H/W register
 *     api_writehwreg_and        - And's word into the specified H/W register
 *     api_writehwreg_or         - Or's word into the specified H/W register
 *     api_readhwreg             - Reads word from specified H/W register
 *
 * INTERNAL ROUTINES:
 *     API_MemTest             - Memory test function...does not test WCS mem
 *     API_Init                - Init internal memory, registers and board(ALL)
 *     BusTools_InitSeg1       - Initialize segment 1 (BC and BM)
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  10/01/1999   Set the output voltage level and coupling at the end of _Init()
               because it was not being set for the PCI-1553.V3.20.ajh
  11/04/1999   Added Playback error message definitions.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  06/19/2000   Added single function warnings.V4.05.ajh
  12/12/2000   Changed _Init to correctly accept PC-1553 board I/O address.
               Moved the PC1553 version initialization code into HWSETUP.V4.28.ajh
  01/05/2001   Added support for the PCC-1553.V4.30.ajh
  04/18/2001   Changed the value of file register 3F to FFF0 for WCS V3.20 for
               non-IP-1553 products.V4.38.ajh
  01/07/2002   Added supprt for Quad-PMC and Dual Channel IP V4.46 rhc 
  02/11/2002   Change BusTools_API_Init to accomodate the IP-D1553 and QPMC
  03/15/2002   Improve initialization.
  06/06/2002   Added VME A16 Config read/write functions
  02/12/2003   Add support for QPCI-1553
  02/26/2003   Add suppport for discrete and differential I/O
  10/22/2003   Add data conversion function BusTools_DataGetString
  02/19/2004   PCCard-1553 Support, BusTools_API_OpenChannel
  03/11/2004   Add new Heart beat register support, H/W Only interrupts
  08/30/2004   Update discrete processing for QCP-1553
  08/18/2005   Add BusTools_GetDevInfo function
  12/30/2005   Improve portibility and efficiency
  08/30/2006   Add AMC-1553 support
  12/07/2006   Remove Windows specific init functions (move tp win32BoardSetup.c)
  11/19/2007   Remove BusTools_MemoryReadIDPROM.
  11/19/2007   Add BusTools_PCI_Reset
               Add support for R15EC in API_ReportMemErrors, BusTools_GetChannelCount, 
               Clear board_has_plx_dma in API_Init
               Added function BusTools_DMA_Setup
               Add option to monitor invalid command in BusTools_SetOptions. Intflag 
               bit 0x2 selects that option.
  02/25/2008   Increase the Polling option from 100 to 2000 milliseconds.
  03/27/2009   Add Ignore High Word Error for F/W 4.40 and greater
  06/29/2009   Add support for the RXMC-1553

*/

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#define _GLOBALS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"

/****************************************************************************
*
*  SEG #1 MEMORY MAP (BC & BM STUFF) (See Globals.h for current map)
*
*     Start End
*     Addr  Addr  Size
*     (hex) (hex) (dec)    Description
*     -------------------------------------------------------------
*     0000  000F    16     Hardware Registers
*     0010  007F   112     RAM Registers (for the microcode)
*     0080  0094    21     BM Trigger Buffer (45 words written)
*     0095  0098     4     BM Default Control Buffer (4 words req'd)
*     0099  041F   903     IQ Buffer (301 entries x 3 words each)
*     0420  07FF  1020     EI Buffer (30 entries x 34 words each)
*     0800  0FFF  2048     BM Filter Buffer
*     1000  ----
*
*  All references in preceeding table are word addresses and word
*  counts.  Note that all (most) addresses stored in API code are
*  byte addresses and byte counts.  All of the low-level board-access
*  routines expect byte addresses and byte counts.
*
****************************************************************************/
static const BT_U16BIT UcodeConstants[] = {
   0x0001,      // Microcode constant reg 60
   0x0002,      // Microcode constant reg 61
   0x0004,      // Microcode constant reg 62
   0x0008,      // Microcode constant reg 63
   0x0010,      // Microcode constant reg 64
   0x0020,      // Microcode constant reg 65
   0x0040,      // Microcode constant reg 66
   0x0080,      // Microcode constant reg 67
   0x0100,      // Microcode constant reg 68
   0x0200,      // Microcode constant reg 69
   0x0400,      // Microcode constant reg 6A
   0x0800,      // Microcode constant reg 6B
   0x1000,      // Microcode constant reg 6C
   0x2000,      // Microcode constant reg 6D
   0x4000,      // Microcode constant reg 6E
   0x8000,      // Microcode constant reg 6F
   0xFFFE,      // Microcode constant reg 70
   0xFFFD,      // Microcode constant reg 71
   0xFFFB,      // Microcode constant reg 72
   0xFFF7,      // Microcode constant reg 73
   0xFFEF,      // Microcode constant reg 74
   0xFFDF,      // Microcode constant reg 75
   0xFFBF,      // Microcode constant reg 76
   0xFF7F       // Microcode constant reg 77
                                           };

#ifdef SYSTEM_MONITOR  //System Monitor code for AR15-VPX
#include "system_monitor.c"
#endif

/**********************************************************************
*     Local Routines
**********************************************************************/


/**********************************************************************
*
* PROCEDURE NAME -    BusTools_API_Close
*
* FUNCTION   This procedure releases all API resources (i.e. memory,
*            board mapping pointers, interrupts, threads, etc.)
*
**********************************************************************/

NOMANGLE BT_INT CCONV BusTools_API_Close(
   BT_UINT cardnum )         // (i) card number
{
   /************************************************
   *  Local variables
   ************************************************/
   int    i;

   /*******************************************************************
   *  Handle case if we need to shutdown the world
   *******************************************************************/
   if ( cardnum >= 99 )
   {
      for ( i = 0; i < MAX_BTA; i++ )
      {
         /*******************************************************
         *  Enable vbtNotify() (the interrupt handler) to stop
         *   processing interrupts.
         *******************************************************/
         bt_interrupt_enable[i] = 0;
         if ( bc_running[i] )
            BusTools_BC_StartStop(i,0);
         if ( bm_running[i] )
            BusTools_BM_StartStop(i,0);
         if ( rt_running[i] )
            BusTools_RT_StartStop(i,0);

         bc_inited[i] = 0;
         bm_inited[i] = 0;
         rt_inited[i] = 0;
         bt_inited[i] = 0;

         if(CurrentCardType[i]==QPCI1553 || CurrentCardType[i]==QPCX1553)
            vbtSetHWRegister(i,0xd,QPCI_BIT_OFF);

         vbtShutdown(i);
         assigned_cards[i] = 0;
         CurrentCardType[i]=0;
      }
	  
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Check parameters
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Call the user interface dll entry point for Close, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrAPI_Close[cardnum] )
   {
      i = (*pUsrAPI_Close[cardnum])(cardnum);
      if ( i == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( i == API_NEVER_CALL_AGAIN )
         pUsrAPI_Close[cardnum] = NULL;
      else if ( i != API_CONTINUE )
         return i;
   }
#endif
   /*******************************************************************
   *  Enable vbtNotify() (the interrupt handler) to stop
   *   processing interrupts.
   *******************************************************************/
   bt_interrupt_enable[cardnum] = 0;
   
   /*******************************************************************
   *  Turn off all the parts (just in case)
   *******************************************************************/
   if ( bc_running[cardnum] )
      BusTools_BC_StartStop(cardnum,0);
   if ( bm_running[cardnum] )
      BusTools_BM_StartStop(cardnum,0); 
   if ( rt_running[cardnum] )
      BusTools_RT_StartStop(cardnum,0);

   if(CurrentCardType[cardnum]==QPCI1553 || CurrentCardType[cardnum]==QPCX1553)
      vbtSetHWRegister(cardnum,0xd,QPCI_BIT_OFF);

   bc_inited[cardnum] = 0;
   bm_inited[cardnum] = 0;
   rt_inited[cardnum] = 0;

   bt_inited[cardnum] = 0;
   vbtShutdown(cardnum);
   assigned_cards[cardnum] = 0;
   CurrentCardType[cardnum]=0;

   if(channel_is_shared[cardnum])
   {
      memset(&cshare[cardnum],0,sizeof(CHANNEL_SHARE));
      channel_is_shared[cardnum] = 0;
   }

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_API_ShareChannel
*
*  FUNCTION
*     This routine initializes the channel on the specified device.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_API_ShareChannel(BT_UINT cardnum)     // (i) 
{
   BT_UINT header;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(_HW_1Function[cardnum] == 1)  // requires multifunction board
      return API_SINGLE_FUNCTION_ERR;

   vbtReadRAM(cardnum,(char *)&header,BTMEM_CH_SHARE,sizeof(BT_UINT));
   if(header == CHANNEL_SHARED)
      return API_CHANNEL_SHARED;

   cshare[cardnum].header = CHANNEL_SHARED;
   cshare[cardnum].share_count = 0;

   cshare[cardnum].bc_inited  = bc_inited[cardnum];       // Non-zero indicates BC initialized
   cshare[cardnum].bc_running = bc_running[cardnum];      // Non-zero indicates BC is running
   cshare[cardnum].bm_inited  = bm_inited[cardnum];       // BM has been initialized
   cshare[cardnum].bm_running = bm_running[cardnum];      // BM is running flag
   cshare[cardnum].rt_inited  = rt_inited[cardnum];       // RT initialized flag.
   cshare[cardnum].rt_running = rt_running[cardnum];      // RT running flag.

   cshare[cardnum].CurrentPlatform = CurrentPlatform[cardnum];

   cshare[cardnum].CurrentCardSlot = CurrentCardSlot[cardnum]; 
   cshare[cardnum].CurrentCarrier = CurrentCarrier[cardnum];
   cshare[cardnum].CurrentCardType = CurrentCardType[cardnum];
   cshare[cardnum].hw_int_enable = hw_int_enable[cardnum];
   cshare[cardnum].bt_ucode_rev = bt_ucode_rev[cardnum];

   cshare[cardnum].board_has_irig = board_has_irig[cardnum]; 
   cshare[cardnum].board_has_testbus = board_has_testbus[cardnum]; 
   cshare[cardnum].board_has_discretes = board_has_discretes[cardnum]; 
   cshare[cardnum].board_has_differential = board_has_differential[cardnum]; 
   cshare[cardnum].board_access_32 = board_access_32[cardnum];
   cshare[cardnum].board_has_bc_timetag = board_has_bc_timetag[cardnum];
   cshare[cardnum].board_has_485_discretes = board_has_485_discretes[cardnum];
   cshare[cardnum].board_has_serial_number = board_has_serial_number[cardnum];
   cshare[cardnum].board_has_hwrtaddr = board_has_hwrtaddr[cardnum];

   cshare[cardnum].hwRTAddr = hwRTAddr[cardnum];

   cshare[cardnum].channel_status = channel_status[cardnum];

   cshare[cardnum].bt_op_mode = bt_op_mode[cardnum];

   cshare[cardnum].api_device = api_device[cardnum];
   cshare[cardnum].bt_interrupt_enable = bt_interrupt_enable[cardnum];

   cshare[cardnum].lpAPI_BM_Buffers = lpAPI_BM_Buffers[cardnum];

   cshare[cardnum].numDiscretes = numDiscretes[cardnum];
   cshare[cardnum].bt_dismask = bt_dismask[cardnum]; 

   cshare[cardnum]._HW_1Function = _HW_1Function[cardnum];
   cshare[cardnum]._HW_FPGARev = _HW_FPGARev[cardnum];
   cshare[cardnum]._HW_PROMRev = _HW_PROMRev[cardnum];
   cshare[cardnum]._HW_WCSRev = _HW_WCSRev[cardnum];

   cshare[cardnum].btmem_bm_mbuf = btmem_bm_mbuf[cardnum];
   cshare[cardnum].btmem_bm_mbuf_next = btmem_bm_mbuf_next[cardnum];
   cshare[cardnum].btmem_bm_cbuf = btmem_bm_cbuf[cardnum]; 
   cshare[cardnum].btmem_bm_cbuf_next = btmem_bm_cbuf_next[cardnum];  
   cshare[cardnum].btmem_bc = btmem_bc[cardnum]; 
   cshare[cardnum].btmem_bc_next = btmem_bc_next[cardnum]; 
   cshare[cardnum].btmem_tail1 = btmem_tail1[cardnum]; 
   cshare[cardnum].btmem_pci1553_next = btmem_pci1553_next[cardnum];
   cshare[cardnum].btmem_pci1553_rt_mbuf = btmem_pci1553_rt_mbuf[cardnum];

   vbtWriteRAM(cardnum,(char *)&cshare[cardnum],BTMEM_CH_SHARE,sizeof(CHANNEL_SHARE));
   channel_is_shared[cardnum] = 1;
   return 0;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_API_JoinChannel
*
*  FUNCTION
*     This routine initializes the channel on the specified device.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_API_JoinChannel(BT_UINT  *chnd, BT_UINT device, BT_UINT channel)     // (i) 
{
   BT_UINT  status, cardnum;
   char *hostaddr;        // Mapped address of the board in host space
   BT_INT scount;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   //clear the assigned cardnum table on initial start
   if(assigned_cards[MAX_BTA] != 0xbeef)
   {
       assigned_cards[MAX_BTA] = 0xbeef;
       for(cardnum = 0;cardnum<MAX_BTA;cardnum++)
          assigned_cards[cardnum] = 0;
   }

   //let assign the cardnum here
   for(cardnum = 0; cardnum<MAX_BTA; cardnum++)
   {
      if(assigned_cards[cardnum] == 0)
      {
         assigned_cards[cardnum] = 0xff;
         break;
      }
   }
   if(cardnum == MAX_BTA)
      return API_MAX_CHANNELS_INUSE;

   *chnd = cardnum;
   hostaddr = 0;

   if( (status = vbtMapBoardAddresses(device,&bt_devmap[device])) != BTD_OK)
      return status;

   if(bt_devmap[device].memSections == 1)
   {
      hostaddr = (char *)bt_devmap[device].memHostBase[0];
   }
   else if (bt_devmap[device].memSections == 2)
   {
      if ( (bt_devmap[device].memLengthBytes[0] != 0x80)  &&
           (bt_devmap[device].memLengthBytes[0] != 0x200) && 
           (bt_devmap[device].memLengthBytes[0] != 0x1000) )
         return BTD_ERR_NOACCESS;

      if ( bt_devmap[device].memLengthBytes[1] != 0x00800000 )
         return BTD_ERR_NOACCESS;
         
         hostaddr = (char *)bt_devmap[device].memHostBase[1];
         bt_iobase[cardnum] = (BT_U32BIT)bt_devmap[device].memHostBase[0];
   }
   boardBaseAddress[cardnum] = hostaddr;

   bt_PageAddr[cardnum][0] = hostaddr + CHAN1_QPCI + DATA_RAM_QPCI; // DP Memory
   bt_PageAddr[cardnum][1] = hostaddr + CHAN1_QPCI + HW_REG_QPCI;   // HW Registers
   bt_PageAddr[cardnum][2] = hostaddr + CHAN1_QPCI + REG_FILE_QPCI; // RAM Registers
   bt_PageAddr[cardnum][3] = hostaddr + CHAN1_QPCI;     // Host Interface Registers

   vbtReadRAM(cardnum,(char *)&cshare[cardnum],BTMEM_CH_SHARE,sizeof(CHANNEL_SHARE));
   if(cshare[cardnum].header != CHANNEL_SHARED)
      return API_CHANNEL_NOTSHARED;

   bt_inited[cardnum] = 1;
   bc_inited[cardnum] = cshare[cardnum].bc_inited;       // Non-zero indicates BC initialized
   bc_running[cardnum] = cshare[cardnum].bc_running;     // Non-zero indicates BC is running
   bm_inited[cardnum] = cshare[cardnum].bm_inited;       // BM has been initialized
   bm_running[cardnum] = cshare[cardnum].bm_running;     // BM is running flag
   rt_inited[cardnum] = cshare[cardnum].rt_inited;       // RT initialized flag.
   rt_running[cardnum] = cshare[cardnum].rt_running;     // RT running flag.

   CurrentPlatform[cardnum] = cshare[cardnum].CurrentPlatform; 
   CurrentCardSlot[cardnum] = cshare[cardnum].CurrentCardSlot;  
   CurrentCarrier[cardnum] = cshare[cardnum].CurrentCarrier;
   CurrentCardType[cardnum] = cshare[cardnum].CurrentCardType; 
   hw_int_enable[cardnum] = cshare[cardnum].hw_int_enable;
   bt_ucode_rev[cardnum] = cshare[cardnum].bt_ucode_rev;

   board_has_irig[cardnum] = cshare[cardnum].board_has_irig; 
   board_has_testbus[cardnum] = cshare[cardnum].board_has_testbus;  
   board_has_discretes[cardnum] = cshare[cardnum].board_has_discretes;  
   board_has_differential[cardnum] =  cshare[cardnum].board_has_differential; 
   board_access_32[cardnum] = cshare[cardnum].board_access_32;
   board_has_bc_timetag[cardnum] = cshare[cardnum].board_has_bc_timetag; 
   board_has_485_discretes[cardnum] = cshare[cardnum].board_has_485_discretes;
   board_has_serial_number[cardnum] = cshare[cardnum].board_has_serial_number; 
   board_has_hwrtaddr[cardnum] = cshare[cardnum].board_has_hwrtaddr;

   hwRTAddr[cardnum] = cshare[cardnum].hwRTAddr;
   channel_status[cardnum] = cshare[cardnum].channel_status; 
   bt_op_mode[cardnum] = cshare[cardnum].bt_op_mode;

   api_device[cardnum] = cshare[cardnum].api_device; 
   bt_interrupt_enable[cardnum] = cshare[cardnum].bt_interrupt_enable; 

   lpAPI_BM_Buffers[cardnum] = cshare[cardnum].lpAPI_BM_Buffers;

   numDiscretes[cardnum] = cshare[cardnum].numDiscretes; 
   bt_dismask[cardnum] = cshare[cardnum].bt_dismask;

   _HW_1Function[cardnum] = cshare[cardnum]._HW_1Function;
   _HW_FPGARev[cardnum] = cshare[cardnum]._HW_FPGARev;
   _HW_PROMRev[cardnum] = cshare[cardnum]._HW_PROMRev;
   _HW_WCSRev[cardnum] = cshare[cardnum]._HW_WCSRev;

   btmem_bm_mbuf[cardnum] = cshare[cardnum].btmem_bm_mbuf; 
   btmem_bm_mbuf_next[cardnum] = cshare[cardnum].btmem_bm_mbuf_next; 
   btmem_bm_cbuf[cardnum] = cshare[cardnum].btmem_bm_cbuf;
   btmem_bm_cbuf_next[cardnum] = cshare[cardnum].btmem_bm_cbuf_next;   
   btmem_bc[cardnum] =  cshare[cardnum].btmem_bc;
   btmem_bc_next[cardnum] =  cshare[cardnum].btmem_bc_next; 
   btmem_tail1[cardnum] =  cshare[cardnum].btmem_tail1;
   btmem_pci1553_next[cardnum] = cshare[cardnum].btmem_pci1553_next; 
   btmem_pci1553_rt_mbuf[cardnum] = cshare[cardnum].btmem_pci1553_rt_mbuf; 

   /* Now complete the final initialization to join */

   // Default Response Time Out=14.5us, late=12.5us.V4.01.ajh
   wResponseReg4[cardnum] = 0x1D19;
   
   // Clear out the Register Function pointer array.
   RegisterFunctionOpen(cardnum);

   nAPI_BM_Head[cardnum] = nAPI_BM_Tail[cardnum] = 0;

   // Initialize a Critical Section to protect the HW frame register.
   CEI_MUTEX_CREATE(&hFrameCritSect[cardnum]);
   bt_inuse[cardnum] = 1;         // Set hardware version of driver.

   CurrentMemKB[cardnum]   = BT_PCI_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM

   API_InterruptInit(cardnum);

   if((hw_int_enable[cardnum] >= API_DEMO_MODE) && (hw_int_enable[cardnum] < API_MANUAL_INT))
   {
      hw_int_enable[cardnum] = API_SW_INTERRUPT;  // Joining applciations can only do S/W itnerrupts  
      status = vbtInterruptSetup(cardnum, hw_int_enable[cardnum],
                                 api_device[cardnum]);
      if ( status )
         return status;
   }

   nBM_MBUF_len[cardnum] = (sizeof(BM_MBUF)+15) & ~0x000F;   // Pad to multiple of 8 words

   hw_addr_shift[cardnum] = 1+3;   // Shift byte address to get word
   ext_trig_bc[cardnum] = BC_TRIGGER_IMMEDIATE;       // Disable external BC triggering.
   /*******************************************************************
   *  Initialize RT operation mode flags...  By default we enable SA31
   *  as a Mode Code, and disable RT31 as broadcast, per US Air Force
   *  default requirements.
   *******************************************************************/
   if(bt_op_mode[cardnum] == 0x40)
   {
       rt_bcst_enabled[cardnum]   = 0;    // RT address 31 is NOT broadcast.
       rt_sa31_mode_code[cardnum] = 0;    // RT subaddress 31 is mode code.
       channel_status[cardnum].broadcast=0;
       channel_status[cardnum].SA_31=0;
   }
   else
   {
       rt_sa31_mode_code[cardnum] = 1;    // RT subaddress 31 is mode code.
       rt_bcst_enabled[cardnum]   = 1;    // RT address 31 is broadcast
       channel_status[cardnum].broadcast=1;
       channel_status[cardnum].SA_31=1;
   }
    
   TimeTagZeroModule(cardnum);        // Initialize time tag parameters

   //Update the share count
   vbtReadRAM(cardnum,(char *)&scount,BTMEM_CH_SHARE+SHARE_COUNT,sizeof(BT_INT));
   scount++;
   vbtWriteRAM(cardnum,(char *)&scount,BTMEM_CH_SHARE+SHARE_COUNT,sizeof(BT_INT));

   channel_is_shared[cardnum] = 1;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_API_QuitChannel
*
*  FUNCTION
*     This routine initializes the channel on the specified device.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_API_QuitChannel(BT_UINT cardnum, BT_UINT qFlag)     // (i) 
{
   BT_UINT scount;

   if(channel_is_shared[cardnum] == 0)
      return API_CHANNEL_NOTSHARED;

   if(qFlag & RT_QUIT)
   {
      rt_inited[cardnum] = 0;
      rt_running[cardnum] = 0;
      vbtWriteRAM8(cardnum,(LPSTR)&rt_inited[cardnum],BTMEM_CH_SHARE + SHARE_RT_INITED,1);
      vbtWriteRAM8(cardnum,(LPSTR)&rt_running[cardnum],BTMEM_CH_SHARE + SHARE_RT_RUNNING,1);
   }
   if(qFlag & BM_QUIT)
   {
      bm_inited[cardnum] = 0;
      bm_running[cardnum] = 0;
      vbtWriteRAM8(cardnum,(LPSTR)&bm_inited[cardnum],BTMEM_CH_SHARE + SHARE_BM_INITED,1);
      vbtWriteRAM8(cardnum,(LPSTR)&bm_running[cardnum],BTMEM_CH_SHARE + SHARE_BM_RUNNING,1);
   }
   if(qFlag & BC_QUIT)
   {
      bc_inited[cardnum] = 0;
      bc_running[cardnum] = 0;
      vbtWriteRAM8(cardnum,(LPSTR)&bc_inited[cardnum],BTMEM_CH_SHARE + SHARE_BC_INITED,1);
      vbtWriteRAM8(cardnum,(LPSTR)&bc_running[cardnum],BTMEM_CH_SHARE + SHARE_BC_RUNNING,1);

   }
   //Update the share count
   vbtReadRAM(cardnum,(char *)&scount,BTMEM_CH_SHARE+SHARE_COUNT,sizeof(BT_INT));
   scount--;
   vbtWriteRAM(cardnum,(char *)&scount,BTMEM_CH_SHARE+SHARE_COUNT,sizeof(BT_INT));

   if(qFlag & SHARE_QUIT)
   {
      cshare[cardnum].header = CHANNEL_NOT_SHARED;
      vbtWriteRAM(cardnum,(char *)&cshare[cardnum].header,BTMEM_CH_SHARE,sizeof(BT_UINT));
      channel_is_shared[cardnum] = 0;
   }

   channel_is_shared[cardnum] = 0;
   return API_SUCCESS;
}


/**********************************************************************
*
*  PROCEDURE NAME -    API_ReportMemErrors()
*
*  FUNCTION
*       This procedure outputs a dump of the failed memory locations
*       to a file when a memory error is detected.
*
*       first_time == 0, the output file is created & block written out.
*       first_time == 1, the block data is appended to the output file.
*       first_time == 2, the data block address is logged...no errors.
*       first_time == -1, the output file is closed and saved.
*
**********************************************************************/
#if defined(__BORLANDC__)
#pragma argsused
#endif
void API_ReportMemErrors(
   BT_UINT    cardnum,         // (i) card number (0 - based)
   int        type,            // (i) memory type being tested (0=WCS, 1=Regs, 2=DP)
   BT_U32BIT  BaseOffset,      // (i) board offset to current test block
   BT_U32BIT  length,          // (i) number of 16-bit words in the buffers
   BT_U16BIT *bufout,          // (i) array of data words read from UUT
   BT_U16BIT *bufin,           // (i) array of data words written to UUT
   BT_U16BIT *buf2,            // (i) array of data words read from UUT second time
   int       *first_time)      // (i) State information, see above
{
#if defined(FILE_SYSTEM)
   BT_U32BIT    i;                        // Loop index.
   time_t       tdate;                    // Time and date tag
   static int   num_written;              // number of records written
   static FILE *hfMemFile  = NULL;        // Error file handle is not open.

   switch ( first_time[0] )
   {
   case 0:               /* Open the output file and fall into case 1   */
      hfMemFile = fopen("BUSAPI.ERR", "a+t");   // Append+Text mode.V4.01.ajh
      if ( hfMemFile == NULL )
         return;                          // File could not be created.
      // Log the board type, etc.
      fprintf(hfMemFile, "\nCardnum = %d, ", cardnum);
      if ( CurrentCardType[cardnum] == PCI1553 )
         fprintf(hfMemFile, "PCI-1553 channel[0/1] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == VME1553 )
         fprintf(hfMemFile, "VME-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == QVME1553 )
         fprintf(hfMemFile, "QVME-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == VXI1553 )
         fprintf(hfMemFile, "VXI-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == PMC1553 )
         fprintf(hfMemFile, "PMC-1553 channel[0/1] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == QPMC1553 )
         fprintf(hfMemFile, "QPMC-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == QPM1553 )
         fprintf(hfMemFile, "QPM-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == AMC1553 )
         fprintf(hfMemFile, "AMC-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == R15AMC )
         fprintf(hfMemFile, "R15-AMC channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == RPCIe1553 )
         fprintf(hfMemFile, "RPCIe-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == QPCI1553 )
         fprintf(hfMemFile, "QPCI-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == QPCX1553 )
         fprintf(hfMemFile, "QPCX-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == QCP1553 )
         fprintf(hfMemFile, "QCP-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == IPD1553 )
         fprintf(hfMemFile, "IP-D1553 channel[1/2] = %d\n\n",
                            CurrentCardSlot[cardnum]);
       else if ((CurrentCardType[cardnum] == Q1041553) || 
                (CurrentCardType[cardnum] == Q1041553P) )
         fprintf(hfMemFile, "Q104-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == R15EC )
         fprintf(hfMemFile, "R15-EC channel[1/2] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == RXMC1553 )
         fprintf(hfMemFile, "RXMC-1553 channel[1/2] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else
         fprintf(hfMemFile, "ISA-1553 channel[0/1] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      fprintf(hfMemFile, " API Ver = %s/%s", API_VER, API_TYPE);
      // Time tag the output file.
      tdate = time(NULL);
      fprintf(hfMemFile, " Memory dump on %s\n\n", ctime(&tdate));

   case 1:               /* Write the specified data to the output file */
      if ( type == 0 )
         fprintf(hfMemFile, "WCS memory\n");
      else if ( type == 1 )
         fprintf(hfMemFile, "File Registers\n");
      else // if ( type == 2 )
         fprintf(hfMemFile, "Dual Port\n");
      num_written = 0;
   case 2:
      if (num_written > 2000 ) // V4.31.ajh
         break;                // If the board is toast stop.V4.09.ajh
      for ( i = 0; i < length; i++ )
      {
         if ( bufin[i] != bufout[i] )
         {  // Tag the error location:
            if ( buf2[i] != bufout[i] )
            fprintf(hfMemFile, "Byte/word offset = %5.5X/%5.5X, is = %4.4X,"
                               " should be = %4.4X, 2nd = %4.4X <---\n",
                               BaseOffset+i+i, (BaseOffset+i+i)/2,
                               bufin[i], bufout[i], buf2[i]);
            else
            fprintf(hfMemFile, "Byte/word offset = %5.5X/%5.5X, is = %4.4X,"
                               " should be = %4.4X, 2nd is OK <---\n",
                               BaseOffset+i+i, (BaseOffset+i+i)/2,
                               bufin[i], bufout[i]);
         }
         num_written++;
      }
      if ( length > 1 ) fprintf(hfMemFile, "\n");
      first_time[0] = 2; // The output file is already open
      break;

   case -1:              /* Close and save the output file              */
      if ( hfMemFile != NULL )      // If we created an output file
         fclose(hfMemFile);         //  close and save the debug data.
      first_time[0] = 0; // The output file has been closed
      break;
   }
#else
   BT_U32BIT    i;                        // Loop index.
   time_t       tdate;                    // Time and date tag
   static int   num_written;              // number of records written

   switch ( first_time[0] )
   {
   case 0:               /* Open the output file and fall into case 1   */
      printf("\nCardnum = %d, ", cardnum);
      if ( CurrentCardType[cardnum] == PCI1553 )
         printf("PCI-1553 channel[0/1] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == VME1553 )
         printf("VME-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == QVME1553 )
         printf("QVME-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == VXI1553 )
         printf("VXI-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == PMC1553 )
         printf("PMC-1553 channel[0/1] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == QPMC1553 )
            printf("QPMC-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == AMC1553 )
            printf("AMC-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == R15AMC )
            printf("R15-AMC channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == RPCIe1553 )
            printf("RPCIe-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == QPM1553 )
            printf("QPM-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == QPCI1553 )
         printf("QPCI-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == QPCX1553 )
         printf("QPCX-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == QCP1553 )
         printf("QCP-1553 channel[1/2/3/4] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == IPD1553 )
         printf("IP-D1553 channel[1/2/] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else if ( CurrentCardType[cardnum] == Q1041553 ||
                CurrentCardType[cardnum] == Q1041553P)
         printf("Q104-1553 PCI/ISA channel[1/2/] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == R15EC )
         printf("R15-EC channel[1/2] = %d\n\n",
                            CurrentCardSlot[cardnum]);
	  else if ( CurrentCardType[cardnum] == RXMC1553 )
         printf("RXMC-1553 channel[1/2] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      else
         printf("ISA-1553 channel[0/1] = %d\n\n",
                            CurrentCardSlot[cardnum]);
      printf(" API Ver = %s/%s", API_VER, API_TYPE);
      // Time tag the output file.
      tdate = time(NULL);
      printf(" Memory dump on %s\n\n", ctime(&tdate));

   case 1:               /* Write the specified data to the output file */
      if ( type == 0 )
         printf("WCS memory\n");
      else if ( type == 1 )
         printf("File Registers\n");
      else // if ( type == 2 )
         printf("Dual Port\n");
      num_written = 0;
   case 2:
      if (num_written > 20 ) // V4.31.ajh
         break;                // If the board is toast stop.V4.09.ajh
      for ( i = 0; i < length; i++ )
      {
         if ( bufin[i] != bufout[i] )
         {  // Tag the error location:
            if ( buf2[i] != bufout[i] )
            printf("Byte/word offset = %5.5X/%5.5X, is = %4.4X,"
                               " should be = %4.4X, 2nd = %4.4X <---\n",
                               BaseOffset+i+i, (BaseOffset+i+i)/2,
                               bufin[i], bufout[i], buf2[i]);
            else
            printf("Byte/word offset = %5.5X/%5.5X, is = %4.4X,"
                               " should be = %4.4X, 2nd is OK <---\n",
                               BaseOffset+i+i, (BaseOffset+i+i)/2,
                               bufin[i], bufout[i]);
         }
         num_written++;
      }
      if ( length > 1 ) printf("\n");
      first_time[0] = 2; // The output file is already open
      break;

   case -1:              /* Close and save the output file              */
      first_time[0] = 0; // The output file has been closed
      break;
   }
#endif
}


/**********************************************************************
*
*  PROCEDURE NAME - rand32 - random number generator
*
*  FUNCTION
*   rand32 uses a multiplicative congruential random number generator
*   with a period of around 2^32.  It returns successive 16-bit
*   pseudo-random numbers loaded into a user-supplied array.
*
**********************************************************************/
#define MULTIPLIER      0x015a4e35L
#define INCREMENT       1
static void rand32(
   long      *CurSeed,  // (i/o) Current seed for random generator
   BT_U16BIT *bufout,   // (o) pointer to resulting array of random numbers
   BT_INT     PageSize) // (i) number of random numbers to generate
{
   BT_INT    j;         // Loop counter

   for ( j = 0; j < PageSize; j++ )
   {
      CurSeed[0] = MULTIPLIER * CurSeed[0] + INCREMENT;
      bufout[j]  = (BT_U16BIT)CurSeed[0];
   }
}

/**********************************************************************
*
*  PROCEDURE NAME -    API_MemTest()
*
*  FUNCTION
*       This procedure tests the internal memory of the specified
*       board, returning with an error if any memory location fails.
*       At the conclusion of the test all DP memory is cleared to zero.
*
**********************************************************************/
#define MEM_TST_PG   512              /* Sixe of a memory test block */
BT_INT API_MemTest(BT_UINT cardnum)
{
   static BT_U16BIT  bufout[MEM_TST_PG];   // Memory test output buffer.
   static BT_U16BIT  bufin[MEM_TST_PG];    // memory test input buffer.
   static BT_U16BIT  buf2[MEM_TST_PG];     // memory test input buffer.

   long              i;                    // Memory test loop counter.
   int               j;                    // Memory test loop counter.
   long              CurSeed;              // Random number seed.
   BT_INT     return_status = API_SUCCESS;
   int        first_time = 0;       // Output file has not been created.

   void (*read_func)(BT_UINT cardnum, LPSTR lpbuffer, BT_U32BIT byteOffset, BT_UINT bytesToRead);

   void (*write_func)(BT_UINT cardnum, LPSTR lpbuffer,BT_U32BIT byteOffset, BT_UINT bytesToWrite);

   /*******************************************************************
   *  Test BusTools board memory (all but hw registers).  Simplify
   *   test by only writing the whole board, then reading it back.V2.80
   *******************************************************************/
   /*******************************************************************
   *  Test the File Registers memory area
   *******************************************************************/
   memset(bufout, 0, sizeof(bufout));

   // Zero Playback (time tag) and RAM Registers 0x0020    112
   vbtWrite(cardnum, (LPSTR)bufout, BTMEM_RAMREGS, RAMREG_COUNT*2);

   if(board_access_32[cardnum])
   {
      read_func = vbtRead32;
      write_func = vbtWrite32;
   }
   else
   {
      read_func = vbtRead;
      write_func = vbtWrite;
   }

   /*******************************************************************
   *  Test the Dual-Port memory area.  Write a unique pattern to the
   *   card, then read it all back.  Skip the space between the RAM
   *   Registers and the beginning of the second dual-port memory page.
   *******************************************************************/
   if ( first_time == 2 )     // If we have logged data,
      first_time = 1;         //  start with the new header.

   CurSeed = 0xDEADFACE;      // Seed the random number generator
   for ( i = 1; i < CurrentMemKB[cardnum]; i++ )
   {
      rand32(&CurSeed, bufout, MEM_TST_PG);  // Create the test pattern
      write_func(cardnum, (LPSTR)bufout, (DWORD)i*sizeof(bufout), sizeof(bufout));
   }
   // Read the pattern from the card and verify it.
   CurSeed = 0xDEADFACE;      // Restart the random number generator 
   for ( i = 1; i < CurrentMemKB[cardnum]; i++ )
   { 
      rand32(&CurSeed, bufout, MEM_TST_PG);  // Recreate the test pattern
      read_func(cardnum, (LPSTR)bufin, (DWORD)i*sizeof(bufin), sizeof(bufin));

      for ( j = 0; j < MEM_TST_PG; j++ )
      {
         if ( bufin[j] != bufout[j] )
         {
            read_func(cardnum, (LPSTR)buf2, (DWORD)i*sizeof(buf2), sizeof(buf2));
            API_ReportMemErrors(cardnum, 2, (DWORD)i*sizeof(bufin), MEM_TST_PG,
                                bufout, bufin, buf2, &first_time);
            return_status = API_BUSTOOLS_BADMEMORY;
            break;
         }
      }
      // Write out zeros to this 1K byte block.
      memset(bufout, 0, sizeof(bufout));
      write_func(cardnum,(LPSTR)bufout,(DWORD)i*sizeof(bufout),sizeof(bufout));
   }

   // Read the zeros from the card again and verify them.
   memset(bufout, 0, sizeof(bufout));        // Rebuild the test pattern
   for ( i = 1; i < CurrentMemKB[cardnum]; i++ )
   {
      read_func(cardnum,(LPSTR)bufin,(DWORD)i*sizeof(bufin),sizeof(bufin));
      for ( j = 0; j < MEM_TST_PG; j++ )
      {
         if ( bufin[j] != bufout[j] )
         {
            read_func(cardnum,(LPSTR)buf2,(DWORD)i*sizeof(buf2),sizeof(buf2));
            API_ReportMemErrors(cardnum, 2, (DWORD)i*sizeof(bufin), MEM_TST_PG,
                                bufout, bufin, buf2, &first_time);
            return_status = API_BUSTOOLS_BADMEMORY;
            break;
         }
      }
   }

   if ( return_status != API_SUCCESS )
   {
      first_time = -1;  // Close the output file, if one was created.
      API_ReportMemErrors(cardnum, 0, 0, 0, bufout, bufin, buf2, &first_time);
   }

   return return_status;
}

NOMANGLE void CCONV setFileRegisters(BT_INT cardnum)
{
   BT_INT cntr;
   /*******************************************************************
   *  Initialize the RAM registers and load the HW constant table.
   *******************************************************************/
   vbtSetFileRegister(cardnum,RAMREG_BC_INT_ENB1,0x0000);   // (0x40+0x03)
   vbtSetFileRegister(cardnum,RAMREG_BC_INT_ENB2,0x0000);   // (0x40+0x04)
   if(bt_op_mode[cardnum] == 0x40)
   {
      vbtSetFileRegister(cardnum,RAMREG_MODECODE1,  0xFFFF);   // (0x40+0x1C)  
      vbtSetFileRegister(cardnum,RAMREG_MODECODE2,  0xFFFF);   // (0x40+0x1D)
      vbtSetFileRegister(cardnum,RAMREG_MODECODE3,  0xFFFF);   // (0x40+0x1E)
      vbtSetFileRegister(cardnum,RAMREG_MODECODE4,  0xFFFF);   // (0x40+0x1F)
   }
   else
   {
      vbtSetFileRegister(cardnum,RAMREG_MODECODE1,  0x0000);   // (0x40+0x1C)  
      vbtSetFileRegister(cardnum,RAMREG_MODECODE2,  0xFFF2);   // (0x40+0x1D)
      vbtSetFileRegister(cardnum,RAMREG_MODECODE3,  0xFFFF);   // (0x40+0x1E)
      vbtSetFileRegister(cardnum,RAMREG_MODECODE4,  0xFFCD);   // (0x40+0x1F)
   }
 
   vbtSetFileRegister(cardnum,RAMREG_FLAGS,0x0400);     // flags reg 78
   vbtSetRegister(cardnum,0x0018,0xFEFF);           // Ucode cons reg 18
   vbtSetRegister(cardnum,0x0019,0xFDFF);           // Ucode cons reg 19
   vbtSetRegister(cardnum,0x001A,0xFBFF);           // Ucode cons reg 1A
   vbtSetRegister(cardnum,0x001B,0xF7FF);           // Ucode cons reg 1B
   vbtSetRegister(cardnum,0x001C,0xEFFF);           // Ucode cons reg 1C
   vbtSetRegister(cardnum,0x001D,0xDFFF);           // Ucode cons reg 1D
   vbtSetRegister(cardnum,0x001E,0xBFFF);           // Ucode cons reg 1E
   vbtSetRegister(cardnum,0x001F,0x7FFF);           // Ucode cons reg 1F
   // Here we have a PCI/PMC/ISA/IP.  If new LPU/WCS setup the RT disables registers:

   vbtSetFileRegister(cardnum,RAMREG_RT_DISA+0,0xFFFF); // Disable Ch A RT15 - 0
   vbtSetFileRegister(cardnum,RAMREG_RT_DISA+1,0xFFFF); // Disable Ch A RT31 - 15
   vbtSetFileRegister(cardnum,RAMREG_RT_DISB+0,0xFFFF); // Disable Ch B RT15 - 0
   vbtSetFileRegister(cardnum,RAMREG_RT_DISB+1,0xFFFF); // Disable Ch B RT31 - 15

   vbtSetFileRegister(cardnum,RAMREG_CLRWORD, 0x0000);  // Ucode cons reg 38
   vbtSetFileRegister(cardnum,RAMREG_SETWORD, 0xFFFF);  // Ucode cons reg 39
   vbtSetFileRegister(cardnum,RAMREG_MASK001F,0x001F);  // Ucode cons reg 3D
   vbtSetFileRegister(cardnum,RAMREG_MASK003F,0x003F);  // Ucode cons reg 3E

   for ( cntr = RAMREG_REG60; cntr <= RAMREG_REG77; cntr++ )
      vbtSetFileRegister(cardnum,cntr,UcodeConstants[cntr-RAMREG_REG60]);

   vbtSetFileRegister(cardnum,RAMREG_ENDFLAGS,0x0000);
}

/**********************************************************************
*
*  PROCEDURE NAME -    API_Init()
*
*  FUNCTION
*       This procedure initializes the internal memory & registers
*       according to the calling routines specified needs.
*
**********************************************************************/
NOMANGLE BT_INT CCONV API_Init(
   BT_UINT   cardnum,       // (i) card number (0 - 3)
   BT_U32BIT base_addr,     // (i) base address of board memory area
   BT_UINT   ioaddr,        // (i) io address of board
   BT_UINT * flag)          // (io) 0 -> software driver, 1 -> hardware driver,
                            //      2 -> HW interrupt enable
{
   #define MULTI_F (CR1_BMRUN | CR1_RTRUN)
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int      i,nmt,continue_on_mem_test_failure;              // Loop counter
   BT_INT   status;         // Pass/fail status from setup functions
   BT_U16BIT regval,version;

   /*******************************************************************
   *  Begin BusTools initialization.
   *******************************************************************/
   bt_interrupt_enable[cardnum] = 0;
   bc_inited     [cardnum] = 0;
   bc_running    [cardnum] = 0;
   bm_inited     [cardnum] = 0;
   bm_running    [cardnum] = 0;
   rt_inited     [cardnum] = 0;
   rt_running    [cardnum] = 0;

   board_has_irig[cardnum] = 0;
   board_has_testbus[cardnum] = 0;
   board_has_discretes[cardnum] = 0;
   board_has_differential[cardnum] = 0;
   board_access_32[cardnum] = 0;
   board_is_paged[cardnum] = 0;
   board_has_485_discretes[cardnum] = 0;
   board_has_bc_timetag[cardnum] = 0;
   board_has_plx_dma[cardnum] = 0;
   board_has_pio[cardnum] = 0;
   board_is_channel_mapped[cardnum]=0;
   board_using_msg_schd[cardnum] = 0;
   board_using_frame_start_timing[cardnum] = 0;
   board_using_extended_timing[cardnum] = 0;
   board_is_dual_function[cardnum] = 0x0;
   boardHasMultipleTriggers[cardnum] = 0x0;
   numPIO[cardnum] = 0;
   numDiscretes[cardnum] = 0;
   hwRTAddr[cardnum] = -1;
   supMF_OFlow[cardnum] = 0;
   BMTrigOutput[cardnum] = 0;
   procBmOnInt[cardnum] = 0;
   
   // Default Response Time Out=14.5us, late=12.5us.V4.01.ajh
   wResponseReg4[cardnum] = 0x1D19;

   if(*flag & API_NO_MEM_TST)
      nmt = 0xff;
   else
      nmt =0;

   if(*flag & API_CONT_ON_MEM_FAIL)
      continue_on_mem_test_failure = 0x07;
   else;
      continue_on_mem_test_failure = 0;

   if(*flag & API_A_MODE)
   {
      bt_op_mode[cardnum] = 0x40;  //Set 1553A operation
      channel_status[cardnum].run_mode=1;
   }
   else
   {
      bt_op_mode[cardnum] = 0x0;   //Set 1553B operation
      channel_status[cardnum].run_mode=0;
   }

   if(*flag & API_CHANNEL_MAP)
   {
      if(CurrentCardType[cardnum] == QPCI1553 ||
         CurrentCardType[cardnum] == QPCX1553 ||
         CurrentCardType[cardnum] == QPMC1553 ||
         CurrentCardType[cardnum] == QPM1553  ||
         CurrentCardType[cardnum] == AMC1553  ||
         CurrentCardType[cardnum] == R15AMC   ||
         CurrentCardType[cardnum] == RPCIe1553||
         CurrentCardType[cardnum] == QCP1553  ||
         CurrentCardType[cardnum] == Q1041553P )
         
         board_is_channel_mapped[cardnum]=CurrentCardSlot[cardnum]+1; // Set for select boards
      else
         return API_NO_CHANNEL_MAP;
   }
 
   *flag &= API_INT_MASK;

   channel_status[cardnum].int_mode=*flag;   // Set the int_mode to the flag value
   channel_status[cardnum].extbus=0;         // Defaults to internal bus
   channel_status[cardnum].mf_ovfl=0;        // No Minor_frame overflow now
   channel_status[cardnum].wcs_pulse=0;      // WCS pulse OK;
   channel_status[cardnum].int_fifo_count=0; // Init to 0 at start
   channel_status[cardnum].interr=0;
   channel_status[cardnum].addr_err=0;
   channel_status[cardnum].byte_cnt_err=0;
   channel_status[cardnum].irig_on=0;
   channel_status[cardnum].err_info = 0;
   /*******************************************************************
   *  Do the card specific initialization
   *******************************************************************/
   status = vbtSetup(cardnum,       // Card number
                     base_addr,     // 32 bit base address
                     ioaddr,        // IO address
                     *flag);

   if (status != BTD_OK)
   {
      assigned_cards[cardnum] = 0;
      return status;                // Any failure is fatal
   }

   // clear the 1553 control register and setup a few bit depending on conditions
   if(*flag & API_HW_INTERRUPT)
      regval = 0xc000;           //default int enable + transponder coupling
   else
      regval = 0x8000;           //default int disable + transponder coupling

   if(hwRTAddr[cardnum] == -1 || hwRTAddr[cardnum] == BTD_RTADDR_PARITY)
      regval |= 0x0100;  // set internal wrap

   api_writehwreg(cardnum, HWREG_CONTROL1, regval);

   if(CurrentCardType[cardnum]==QPCI1553 || CurrentCardType[cardnum]==QPCX1553)
      vbtSetHWRegister(cardnum,0xd,QPCI_BIT_OFF);

   /*******************************************************************
   *  Setup the shift count used to translate 16-bit WORD addresses
   *   on the board to and from byte addresses.  Shift by 1 or shift
   *   by 1+3 to handle the 8-word boundry addressing.
   *******************************************************************/
   hw_addr_shift[cardnum] = 1+3;   // Shift byte address to get word

   /*******************************************************************
   *  Write zeros to the simulated H/W registers.
   *******************************************************************/

   if ( *flag == 0 )
   {
      for ( i = 0; i < HWREG_COUNT2; i++ )
         api_writehwreg(cardnum, i, 0);
   }
   
   for(i=0;i<MAX_BTA;i++) //set the irig_bias to 0
      irig_bias[i] = 0;  // adjust IRIG-B time for circuit offset and delay

   ext_trig_bc[cardnum] = BC_TRIGGER_IMMEDIATE;       // Disable external BC triggering.
   api_writehwreg(cardnum,HWREG_BC_EXT_SYNC,0x0000);  // Disable HW BC trigger

   /*******************************************************************
   *  Setup the version ID information we return to the user.
   *******************************************************************/
 
   bt_ucode_rev[cardnum] = _HW_PROMRev[cardnum]*0x10000 +       // single digit
                           _HW_1Function[cardnum]*0x1000 +      // single digit
                           (_HW_FPGARev[cardnum]&0x0fff);       // three digits

   api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0); // Clear HW load of Time Tag mode
   api_writehwreg(cardnum, HWREG_RESPONSE, wResponseReg4[cardnum]); // 16us time out, 14us late.

   /*******************************************************************
   *  Test BusTools board memory (all but hw registers).  This test
   *   does NOT test the WCS (which was tested when the WCS was
   *   loaded).  This test writes the whole board with a unique
   *   pattern, then reads it all back leaving the board all zero.V2.80
   *******************************************************************/
   if(!nmt)
   {
      status = API_MemTest(cardnum);
      if ( status != API_SUCCESS)
      {
         if(continue_on_mem_test_failure)
         {
            channel_status[cardnum].err_info |= CHAN_MEM_TST_FAIL;
            status = 0;
         }
         else
         {   
            // Cleanup pointers, free mapping windows, allocated memory and threads.
            assigned_cards[cardnum] = 0;
            vbtShutdown(cardnum);
            return status;
         }
      }
   }

   /**********************************************************************
   * Read the WCS/LPU version ID from the LPU Revision Register.  The
   *  version is stored in HEX, but we want decimal...
   *********************************************************************/
   version = vbtGetHWRegister(cardnum,HWREG_WCS_REVISION);
   version = (version & 0x000F) + 10  * ((version >> 4) & 0x000F)
                                 + 100 * ((version >> 8) & 0x000F);
   _HW_WCSRev[cardnum]  = version;

   if(_HW_WCSRev[cardnum] >= 397)
      board_has_bc_timetag[cardnum] = 1;

   /*******************************************************************
   *  Initialize the RAM registers and load the HW constant table.
   *******************************************************************/
   setFileRegisters(cardnum);

   /*******************************************************************
   *  Setup first memory segment.  Setup error injection structure,
   *   fill in default values for Trigger Buffer, interrupt queue and
   *   pointer register, BM Filter Buffer and pointer register.
   *******************************************************************/
   BusTools_InitSeg1(cardnum);

   /*******************************************************************
   *  Clear out rest of address pointers for low memory
   *******************************************************************/
   bt_inited[cardnum] = 1;  // Report initialization complete.
                            // All btdrv.c function calls are legal.
   /*******************************************************************
   *  Initialize each of the "EI_MAX_ERROR_NUM" error injection
   *   buffers to zero, and write them starting at "BTMEM_EI".
   *******************************************************************/
   {
      BT_UINT    i;
      BT_U32BIT  addr;
      EI_MESSAGE error;
      memset((void*)&error, 0, sizeof(error));
      for ( i = 0; i < EI_MAX_ERROR_NUM; i++ )
      {
         addr = BTMEM_EI + i * sizeof(EI_MESSAGE);
         vbtWrite(cardnum, (LPSTR)&error, addr, sizeof(error));
      }
   }

   /*******************************************************************
   *  Set voltage to max, transformer coupled, PCI- and ISA-1553.V3.30.
   *******************************************************************/
   BusTools_SetVoltage(cardnum, 1980, 1);
   if(CurrentCardType[cardnum]==QPCI1553 || CurrentCardType[cardnum]==QPCX1553)
      vbtSetHWRegister(cardnum,0xd,QPCI_BIT_PASS);

   /*******************************************************************
   *  Complete misc initialization functions.
   *******************************************************************/
   TimeTagZeroModule(cardnum);        // Initialize time tag parameters
				  
   if((*flag & 0xf) < API_MANUAL_INT)
   {
      bt_interrupt_enable[cardnum] = 1;  // Enable vbtNotify, interrupt queue valid
   }

   /*******************************************************************
   *  Complete the reset of the 1553 functions on the board.
   *******************************************************************/

   if(bt_op_mode[cardnum] == 0x40)
   {
	   api_writehwreg_or(cardnum, HWREG_CONTROL1, 0x40);    //Set 1553a
       rt_bcst_enabled[cardnum]   = 0;    // RT address 31 is NOT broadcast.
       rt_sa31_mode_code[cardnum] = 0;    // RT subaddress 31 is mode code.
       channel_status[cardnum].broadcast=0;
       channel_status[cardnum].SA_31=0;
   }
   else
   {
       api_writehwreg_and(cardnum, HWREG_CONTROL1, ~0x40);     //reset 1553a
       api_writehwreg_or(cardnum, HWREG_CONTROL1, CR1_RT31BCST | CR1_SA31MC );  // Set Broadcast and SA 31 Mode Code
       rt_sa31_mode_code[cardnum] = 1;    // RT subaddress 31 is mode code.
       rt_bcst_enabled[cardnum]   = 1;    // RT address 31 is broadcast
       channel_status[cardnum].broadcast=1;
       channel_status[cardnum].SA_31=1;
   }
   
   if(status) //if there is an error then free up the cardnum
      assigned_cards[cardnum] = 0; 
       
   return status;
}

/**********************************************************************
*
*  PROCEDURE NAME -    BusTools_API_InitExtended()
*
*  FUNCTION
*       This procedure initializes the API to communicate with any of the
*       supported 1553 BusTools products, based on the arguements supplied
*       by the caller.
*
*       The caller specifies a series of arugements in the call which
*       determine which board/carrier/environment combination the API
*       will search for.
*
*       The caller specifies the:
* cardnum   // (i) card number (0 - 3) (device number in 32-bit land)
* base_addr // (i) linear base address of board or carrier memory area
* ioaddr    // (i) board or carrier I/O address
* flag      // (io) 0 -> request software driver, 1 -> hardware driver
* platform  // (i) execution platform: PLATFORM_PC, PLATFORM_VMIC, PLATFORM_VME
* cardType  // (i) 
* carrier   // (i) NATIVE or ....
* slot      // (i) SLOT_A, SLOT_B, SLOT_C, SLOT_D.
* mapping   // (i) carrier memory map: CARRIER_MAP_DEFAULT, CARRIER_MAP_A24,
*                                      CARRIER_MAP_A32
**********************************************************************/
NOMANGLE BT_INT CCONV BusTools_API_InitExtended(
  BT_UINT   cardnum,  // (i) card number (0 - 12) (device number in 32-bit land)
  BT_U32BIT base_addr,// (i) linear base address of board/carrier memory area or WinRT device number
  BT_UINT   ioaddr,   // (i) board or carrier I/O address
  BT_UINT * flag, // (io) 0 -> software driver, 1 -> hardware driver
                      //      2 -> HW interrupt enable
  BT_UINT   platform, // (i) execution platform: PLATFORM_PC, PLATFORM_VMIC, PLATFORM_VME
  BT_UINT   cardType, // (i) IP1553, IP1553MF, IP1553SF, or PCI1553
  BT_UINT   carrier,  // (i) NATIVE, or ....
  BT_UINT   slot,     // (i) SLOT_A, SLOT_B, SLOT_C, SLOT_D.
  BT_UINT   mapping)  // (i) carrier memory map: CARRIER_MAP_DEFAULT, CARRIER_MAP_A24,
                      //     CARRIER_MAP_A32,
                      //     or the address of the IP ID space (VME "NATIVE" only).
{
   /*******************************************************************
   *  BusTools can only be initialized once per card.
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   //clear the assigned cardnum table on initial start
   if(assigned_cards[MAX_BTA] != 0xbeef)
   {
       int i;
       assigned_cards[MAX_BTA] = 0xbeef;
       for(i = 0;i<MAX_BTA;i++)
          assigned_cards[i] = 0;
   }

   if(assigned_cards[cardnum] == 0xff)
      return API_CARDNUM_INUSE;

   assigned_cards[cardnum] = 0xff;

   if (bt_inited[cardnum])
      return API_BUSTOOLS_INITED;
   CurrentPlatform[cardnum] = platform;  // Platform (PC/VMIC/etc)
   CurrentCardType[cardnum] = cardType; // Card type (PC/IP/PCI/etc).
   CurrentCarrier[cardnum]  = carrier;  // Carrier type (ISA/PCI/VME/NATIVE/etc)
   CurrentCardSlot[cardnum] = slot;     // Card slot (SLOT_A, _B, _C, _D, etc)
   CurrentMemMap[cardnum]   = mapping;  // Carrier memory map (CARRIER_MAP_DEFAULT, etc)

   return API_Init(cardnum, base_addr, ioaddr, flag);
}

/**********************************************************************
*
*  PROCEDURE NAME -    BusTools_API_Reset()
*
*  FUNCTION
*       This procedure reset the internal memory & registers
*       according to the calling routines specified needs it does not
*       map the board or does board specific initialization.  
*
**********************************************************************/
NOMANGLE BT_INT CCONV BusTools_API_Reset(BT_UINT cardnum, BT_UINT reset_flag) 
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;
   
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(reset_flag == API_RESET_SEG1)
   {
      api_reset(cardnum);
      BusTools_InitSeg1(cardnum);
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Begin BusTools reset.
   *******************************************************************/
   if ( bc_running[cardnum] )
      BusTools_BC_StartStop(cardnum,0);
   if ( bm_running[cardnum] )
      BusTools_BM_StartStop(cardnum,0);
   if ( rt_running[cardnum] )
      BusTools_RT_StartStop(cardnum,0);

   bt_interrupt_enable[cardnum] = 0;
   bc_inited     [cardnum] = 0;
   bc_running    [cardnum] = 0;
   bm_inited     [cardnum] = 0;
   bm_running    [cardnum] = 0;
   rt_inited     [cardnum] = 0;
   rt_running    [cardnum] = 0;   
   
   // Default Response Time Out=14.5us, late=12.5us.V4.01.ajh
   wResponseReg4[cardnum] = 0x1D19;

   /*******************************************************************
   *  Do the card specific initialization
   *******************************************************************/
   regval = api_readhwreg(cardnum,HWREG_CONTROL1);
   regval &= 0x4000;
   regval |= 0x8100;
   api_writehwreg(cardnum, HWREG_CONTROL1, regval);

   if(CurrentCardType[cardnum]==QPCI1553 || CurrentCardType[cardnum]==QPCX1553)
      vbtSetHWRegister(cardnum,0xd,QPCI_BIT_OFF);

   /*******************************************************************
   *  Write zeros to the simulated H/W registers.
   *******************************************************************/
   ext_trig_bc[cardnum] = BC_TRIGGER_IMMEDIATE;   // Disable external BC triggering.

   api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0); // Clear HW load of Time Tag mode
   api_writehwreg(cardnum, HWREG_RESPONSE, wResponseReg4[cardnum]); // 16us time out, 14us late.

   /*******************************************************************
   *  Initialize the RAM registers and load the HW constant table.
   *******************************************************************/
   setFileRegisters(cardnum);   
   /*******************************************************************
   *  Setup first memory segment.  Setup error injection structure,
   *   fill in default values for Trigger Buffer, interrupt queue and
   *   pointer register, BM Filter Buffer and pointer register.
   *******************************************************************/
   BusTools_InitSeg1(cardnum);
   /*******************************************************************
   *  Clear out rest of address pointers for low memory
   *******************************************************************/
   bt_inited[cardnum] = 1;  // Report initialization complete.
                            // All btdrv.c function calls are legal.
   /*******************************************************************
   *  Initialize each of the "EI_MAX_ERROR_NUM" error injection
   *   buffers to zero, and write them starting at "BTMEM_EI".
   *******************************************************************/
   {
      BT_UINT    i;
      BT_U32BIT  addr;
      EI_MESSAGE error;
      memset((void*)&error, 0, sizeof(error));
      for ( i = 0; i < EI_MAX_ERROR_NUM; i++ )
      {
         addr = BTMEM_EI + i * sizeof(EI_MESSAGE);
         vbtWrite(cardnum, (LPSTR)&error, addr, sizeof(error));
      }
   }
   /*******************************************************************
   *  Initialize RT operation mode flags...  By default we enable SA31
   *  as a Mode Code, and disable RT31 as broadcast, per US Air Force
   *  default requirements.
   *******************************************************************/
   rt_bcst_enabled[cardnum]   = 0;    // RT address 31 is NOT broadcast.
   if(bt_op_mode[cardnum] == 0)
      rt_sa31_mode_code[cardnum] = 1;    // RT subaddress 31 is mode code.

   /*******************************************************************
   *  Set voltage to max, transformer coupled, PCI- and ISA-1553.V3.30.
   *******************************************************************/
   BusTools_SetVoltage(cardnum, 1980, 1);

   if(CurrentCardType[cardnum]==QPCI1553 || CurrentCardType[cardnum]==QPCX1553)
      vbtSetHWRegister(cardnum,0xd,QPCI_BIT_PASS);
   /*******************************************************************
   *  Complete misc initialization functions.
   *******************************************************************/
   TimeTagZeroModule(cardnum);        // Initialize time tag parameters
				  
   API_InterruptInit(cardnum);        // Initialize user FIFO interrupts
   /*******************************************************************
   *  Complete the reset of the 1553 functions on the board.
   *******************************************************************/

   if(bt_op_mode[cardnum] == 0x40)
	   api_writehwreg_or(cardnum, HWREG_CONTROL1, 0x40);    //Set 1553a
   else
   {
       rt_bcst_enabled[cardnum] = 1;    // RT address 31 is  broadcast.
       api_writehwreg_and(cardnum, HWREG_CONTROL1, ~0x40);  //reset 1553a
       api_writehwreg_or(cardnum, HWREG_CONTROL1, CR1_RT31BCST | CR1_SA31MC );  // Set Broadcast and SA 31 Mode Code
   } 
   return API_SUCCESS;
}


/****************************************************************************
*
*   PROCEDURE NAME - BusTools_Checksum1760()
*
*   FUNCTION
*       This function Calculates a 1760 checksum for the data in an array 
*        and stores  the result in the next location in the array.  
*
*   RETURNS
*       Calculated chechsum.
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_Checksum1760(BT_U16BIT *mbuf, BT_INT wdcnt)
{
	BT_U16BIT checksum, wd, temp, i, shiftbit;

	// Start at zero.
	checksum = 0;

	// Process each word in the data buffer.
	for (wd = 0; wd < (wdcnt-1); wd++) {
		temp = mbuf[wd];

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
    
    mbuf[wdcnt-1] = checksum;
	return checksum;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_ExtTrigIntEnable
*
*  FUNCTION
*       This procedure enables interrupt on external trigger in.
*
*  RETURNS
*       API_BUSTOOLS_BADCARDNUM
*       API_BUSTOOLS_NOTINITED
*       API_SUCCESS
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_ExtTrigIntEnable(
   BT_UINT cardnum,        // (i) card number
   BT_UINT flag)           // (i) EXT_TRIG_ENABLE
                           //     EXT_TRIG_DISABLE
{  
   BT_U16BIT reg_data;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   reg_data = vbtGetHWRegister(cardnum,HWREG_CONTROL2);
   if(flag == EXT_TRIG_ENABLE)
      reg_data |= 0x10;
   else
      reg_data &= ~0x10;
  
   vbtSetHWRegister(cardnum,HWREG_CONTROL2,reg_data);
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_ExtTriggerOut
*
*  FUNCTION
*       This procedure pulses the external output trigger.
*
*  RETURNS
*       API_BUSTOOLS_BADCARDNUM
*       API_BUSTOOLS_NOTINITED
*       API_SUCCESS
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_ExtTriggerOut(
   BT_UINT cardnum,        // (i) card number
   BT_U16BIT pwidth)       // (i) Trigger pulse width      
{  
   BT_U16BIT cdata;        //1553 control data
   BT1553_TIME ttime;
   BT_U32BIT start_micros;
   
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if((pwidth < 1) || (pwidth > 1000))
      return API_BAD_PARAM;
   
   cdata = *((BT_U16BIT *)(bt_PageAddr[cardnum][1])); //read 1553 control register
   flipw(&cdata);


   cdata |= CR1_EXT_TTL;   // set the EXTO bit
   *((BT_U16BIT *)(bt_PageAddr[cardnum][1])) = flipws(cdata);

   BusTools_TimeTagRead(cardnum,&ttime);

   start_micros = ttime.microseconds;
   do
   {
      BusTools_TimeTagRead(cardnum,&ttime);   
   }while((ttime.microseconds - start_micros) < pwidth);            

   cdata &= ~CR1_EXT_TTL;  // clear the EXTO bit
   *((BT_U16BIT *)(bt_PageAddr[cardnum][1])) = flipws(cdata);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_DMA_Setup
*
*  FUNCTION
*       This function sets up a block DMA transfer for device with PLX 9056
*   
*
*  RETURNS
*       API_SUCCESS (0)   -> success
*       API_HARDWARE_NOSUPPORT -> illegal address type specified
*       API_BUSTOOLS_BADCARDNUM -> bad card number specified
*       API_BUSTOOLS_NOTINITED  -> Channel not initialized
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_DMA_Setup(
   BT_UINT cardnum,             // (i) card number (0 - 3)
   BT_UINT dma_channel,         // (i) 0 = channel 0 1 or > = channel 1
   BT_U32BIT host_addr,         // (i) Host address for dma read/write (physical address)
   BT_U32BIT board_addr,        // (i) board address for dma read/write
   BT_U32BIT byte_count,        // (i) Number of bytes to transfer
   BT_U32BIT dma_flag)          // (i) direction of transfer 0 = board to host
                                //                           1 or > = host to board
{
   /************************************************
   *  Error checking
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_plx_dma[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(dma_channel == 0)
   {
      vbtSetPLXRegister32(cardnum,PLX_DMA_HOST_ADDR0, host_addr);
      vbtSetPLXRegister32(cardnum,PLX_DMA_BOARD_ADDR0, board_addr);
      vbtSetPLXRegister32(cardnum,PLX_DMA_BYTE_COUNT0, byte_count);
      if(dma_flag)
         vbtSetPLXRegister32(cardnum,PLX_DMA_FLAG0, 0x0);
      else
         vbtSetPLXRegister32(cardnum,PLX_DMA_FLAG0, 0x8);
   }
   else
   {
      vbtSetPLXRegister32(cardnum,PLX_DMA_HOST_ADDR1, host_addr);
      vbtSetPLXRegister32(cardnum,PLX_DMA_BOARD_ADDR1, board_addr);
      vbtSetPLXRegister32(cardnum,PLX_DMA_BYTE_COUNT1, byte_count);
      if(dma_flag)
         vbtSetPLXRegister32(cardnum,PLX_DMA_FLAG1, 0x0);
      else
         vbtSetPLXRegister32(cardnum,PLX_DMA_FLAG1, 0x8);      
   }
   vbtSetPLXRegister32(cardnum,PLX_DMA_THRESHOLD,0x0000f000);

   return API_SUCCESS;

}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_DMA_Start
*
*  FUNCTION
*       This function sets up a block DMA transfer for device with PLX 9056
*   
*
*  RETURNS
*       API_SUCCESS (0)   -> success
*       API_HARDWARE_NOSUPPORT -> illegal address type specified
*       API_BUSTOOLS_BADCARDNUM -> bad card number specified
*       API_BUSTOOLS_NOTINITED  -> Channel not initialized
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_DMA_Start(
   BT_UINT cardnum,             // (i) card number (0 - 3)
   BT_U32BIT dma_channel)       // (i) 
{
   /************************************************
   *  Error checking
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_plx_dma[cardnum])
      return API_HARDWARE_NOSUPPORT;

   vbtSetPLXRegister8(cardnum,PLX_DMA_STATUS0, 0x3);

   return API_SUCCESS;

}


/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetAddr
*
*  FUNCTION
*       This procedure returns the starting and ending memory
*       addresses for the specified block of memory.
*
*  RETURNS
*       API_SUCCESS (0)   -> success
*       API_BAD_ADDR_TYPE -> illegal address type specified
*       API_BUSTOOLS_BADCARDNUM -> bad card number specified
*       API_BUSTOOLS_NOTINITED  -> Channel not initialized
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetAddr(
   BT_UINT cardnum,         // (i) card number (0 - 3)
   BT_UINT memtype,         // (i) memory block type (e.g. API_MEM_REGISTERS)
   BT_U32BIT * start,   // (o) returned address of beginning of block
   BT_U32BIT * end)     // (o) returned address of end of block
{
   /************************************************
   *  Clear returned values
   ************************************************/
   *start = 0;
   *end   = 0;

   /************************************************
   *  Error checking
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /************************************************
   *  Lookup specified address type
   ************************************************/
   switch(memtype)
   {
      case GETADDR_HWREG:
         *start = BTMEM_HWREGS;
         *end   = BTMEM_HWREGS + HWREG_COUNT2 * 2 - 1;
         break;
      case GETADDR_RAMREG:
        *start = BTMEM_RAMREGS;
        *end   = BTMEM_RAMREGS + RAMREG_COUNT * 2 - 1;
         break;
      case GETADDR_BMTRIGGER:
         *start = BTMEM_BM_TBUF;
         *end   = BTMEM_BM_TBUF_NEXT - 1;
         break;
      case GETADDR_BMDEFCBUF:
         *start = BTMEM_BM_CBUF_DEF;
         *end   = BTMEM_BM_CBUF_NEXT - 1;
         break;
      case GETADDR_IQ:
         *start = BTMEM_IQ;
         *end   = BTMEM_IQ_NEXT - 1;
         break;
      case GETADDR_EI:
         *start = BTMEM_EI;
         *end   = BTMEM_EI_NEXT - 1;
         break;
      case GETADDR_BMFILTER:
         *start = BTMEM_BM_FBUF;
         *end   = BTMEM_BM_FBUF + sizeof(BM_FBUF) - 1;
         break;
      case GETADDR_BMCONTROL:
         *start = btmem_bm_cbuf[cardnum];
         *end   = btmem_bm_cbuf_next[cardnum] - 1;
         break;
      case GETADDR_BMMESSAGE:
         /* This is where the BM message buffers live */
         *start = btmem_bm_mbuf[cardnum];
         *end   = btmem_bm_mbuf_next[cardnum] - 1;
         break;
      case GETADDR_BCMESS:
         /* This is where the BC message and data buffers live */
         *start = btmem_bc[cardnum];
         *end   = btmem_bc_next[cardnum] - 1;
         break;
      case GETADDR_PCI_RTDATA:
         /* For the PCI-1553, get the addresses of the MBUF's which live   */
         /* from btmem_pci1553_rt_mbuf[cardnum] to the top of memory!      */
         *start = btmem_pci1553_rt_mbuf[cardnum];
         *end   = (CurrentMemKB[cardnum] * 1024L) - 1;
         break;
      case GETADDR_RTDATA:
         /* This is where the RT message buffers and control buffers live. */
         /* For the PCI-1553, only the control buffers live here!          */
         *start = btmem_tail2[cardnum];
         if(btmem_rt_top_avail[cardnum] == 0)
            *end = *start;
         else
            *end   = btmem_rt_top_avail[cardnum] - 1;
         break;
      case GETADDR_RTCBUF_BRO:
         if ( rt_bcst_enabled[cardnum] == 0 )
         {
            // Broadcast disabled.  Buffer space is NULL.
            *start = BTMEM_RT_CBUF_DEF + btmem_rt_begin[cardnum];
            *end   = *start - 1;
         }
         else
         {
            // Broadcast is enabled.  Buffer size and base address depends
            //  on SA31 mode code definition enabled or disabled.  The
            //  RT_CbufbroadAddr() function hides these details from the user.
            // The buffer starts with subaddress 0, receive.
            *start = RT_CbufbroadAddr(cardnum, 0, 0);
            if ( rt_sa31_mode_code[cardnum] )
               *end   = *start + 62*sizeof(RT_CBUFBROAD) - 1;
            else
               *end   = *start + 64*sizeof(RT_CBUFBROAD) - 1;
         }
         break;
      case GETADDR_RTCBUF_DEF:
         *start = BTMEM_RT_CBUF_DEF + btmem_rt_begin[cardnum];
         *end   = *start + 32*sizeof(RT_CBUF) - 1;
         break;
      case GETADDR_RTMBUF_DEF:
         *start = BTMEM_RT_MBUF_DEF + btmem_rt_begin[cardnum];
         *end   = *start + 32*sizeof(RT_MBUF) - 1;
         break;
      case GETADDR_RTADDRESS:
         *start = BTMEM_RT_ABUF + btmem_rt_begin[cardnum];
         *end   = *start + sizeof(RT_ABUF) - 1;
         break;
      case GETADDR_RTFILTER:
         *start = BTMEM_RT_FBUF + btmem_rt_begin[cardnum];
         *end   = *start + sizeof(RT_FBUF) - 1;
         break;

      case GETADDR_DIFF_IO:
         *start = BTMEM_DIFF_IO;
         *end   = BTMEM_DIFF_IO+0xff;
         break;

      default:
         *start = 0x00000;
         *end   = 0x0000F;
         return API_BAD_ADDR_TYPE;
   }
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetAddrName
*
*  FUNCTION
*       This procedure returns the text name of the specified block of memory.
*
*  RETURNS
*       pointer to string name -> success
*       NULL                   -> illegal address type specified
*
****************************************************************************/

NOMANGLE char * CCONV BusTools_GetAddrName(
   BT_UINT memtype)         // (i) memory block type (e.g. API_MEM_REGISTERS)
{
   /************************************************
   *  Lookup specified address name
   ************************************************/
   switch(memtype)
   {
      case GETADDR_HWREG:      return "Hardware Registers........";
      case GETADDR_RAMREG:     return "RAM Registers.............";
      case GETADDR_BMTRIGGER:  return "BM Trigger Buffer.........";
      case GETADDR_BMDEFCBUF:  return "BM Default Control Buffer.";
      case GETADDR_RTADDRESS:  return "RT Address Buffers........";
      case GETADDR_IQ:         return "Interrupt Queue...........";
      case GETADDR_EI:         return "Error Injection Buffers...";
      case GETADDR_BMFILTER:   return "BM Filter Buffer..........";
      case GETADDR_BMCONTROL:  return "BM Control Buffer.........";
      case GETADDR_BMMESSAGE:  return "BM Message Buffers........";
      case GETADDR_BCMESS:     return "BC Message Buffers........";
      case GETADDR_PCI_RTDATA: return "PCI RT Message Buffers....";
      case GETADDR_RTDATA:     return "RT Ctrl & Message Buffers.";
      case GETADDR_RTCBUF_BRO: return "RT Broadcast Ctrl Buffers.";
      case GETADDR_RTCBUF_DEF: return "RT Non-Bro Def Ctrl Bufs..";
      case GETADDR_RTMBUF_DEF: return "RT Default Message Buffers";
      case GETADDR_RTFILTER:   return "RT Filter Buffers.........";
      case GETADDR_DIFF_IO:    return "IRIG Discretes Diff I/O...";
      default:                 return "Illegal Buffer Number.....";
   }
}


/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_MemoryRead2
*
*  FUNCTION
*       This procedure reads the specified block of memory and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BAD_PARAM -> Invalid address or Register
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_MemoryRead2(
   BT_UINT cardnum,          // (i) card number
   BT_INT  region,           // (i) Regions to read HIF, HWREG, RAMREG, RAM
   BT_U32BIT start,          // (i) start address or register
   BT_U32BIT count,          // (i) bytes/registers to read
   BT_U16BIT * buff)         // (o) Pointer to user's buffer
{
   BT_UINT regnum,indx=0;

   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   switch(region)
   {
      case HIF:
         if((start >= 0x200) || (start+count > 0x200))
            return API_BAD_PARAM; 
         vbtReadHIF(cardnum, (LPSTR)buff, start, (BT_UINT)count);
         break;
      case HWREG:
         if((start > HWREG_COUNT2*2) || (start+count > HWREG_COUNT2*2))
            return API_BAD_PARAM; 

         start/=2;    //this is a word address function
         count/=2;

         for(regnum=start; regnum<(start + count); regnum++)
            buff[indx++] = vbtGetHWRegister(cardnum, regnum);
         break;
      case RAMREG:
         if((start > RAMREG_COUNT*2) || (start+count > RAMREG_COUNT*2))
            return API_BAD_PARAM;

         start/=2;    //this is a word address function
         count/=2;
 
         for(regnum=start; regnum<(start + count); regnum++)
            buff[indx++] = vbtGetFileRegister(cardnum, regnum);
         break;
      case RAM:
         if (start & 0x0001)
            return API_BUSTOOLS_EVENADDR;
         if (count & 0x0001)
             return API_BUSTOOLS_EVENBCOUNT;
         vbtReadRAM(cardnum, (LPSTR)buff, start, (BT_UINT)count);
   }

   return API_SUCCESS;
}


/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_MemoryRead
*
*  FUNCTION
*       This procedure reads the specified block of memory and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_MemoryRead(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr,          // (i) BYTE address in BT hardware to begin reading
   BT_U32BIT bcount,        // (i) EVEN number of bytes to read
   VOID * buff)         // (o) Pointer to user's buffer
{
   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Read specified memory area, anywhere on the card. 
   *******************************************************************/

   vbtRead(cardnum, (LPSTR)buff, addr, (BT_UINT)bcount);
   if (addr & 0x0001)
      return API_BUSTOOLS_EVENBCOUNT;

   if (bcount & 0x0001)
      return API_BUSTOOLS_EVENBCOUNT;

   return API_SUCCESS;
}


#ifdef PLX_DEBUG
/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_PLXWrite
*
*  FUNCTION
*       This procedure reads the specified block of memory and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_PLXWrite(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr,          // (i) BYTE address in BT hardware to begin reading
   BT_U16BIT rData)         // (o) Pointer to user's data
{
   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if((CurrentCardType[cardnum] != QPCI1553) &&
      (CurrentCardType[cardnum] != QPCX1553) &&
      (CurrentCardType[cardnum] != QCP1553) ) 
	  return API_HARDWARE_NOSUPPORT;

   /*******************************************************************
   *  Read specified memory area, anywhere on the card. 
   *******************************************************************/
   if (addr & 0x0001)
      return API_BUSTOOLS_EVENBCOUNT;

   if(addr >= 0x80)
	   return BTD_ERR_BADADDR;

   vbtSetPLXRegister(cardnum, addr,rData);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_PLXRead
*
*  FUNCTION
*       This procedure reads the specified block of memory and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_PLXRead(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr,          // (i) BYTE address in BT hardware to begin reading
   BT_U16BIT *rData)         // (o) Pointer to user's data
{
   /*******************************************************************
   *  Do error checking
   *******************************************************************/
   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if((CurrentCardType[cardnum] != QPCI1553) &&
      (CurrentCardType[cardnum] != QPCX1553) && 
      (CurrentCardType[cardnum] != QCP1553))
	  return API_HARDWARE_NOSUPPORT;

   /*******************************************************************
   *  Read specified memory area, anywhere on the card.  "buff" can
   *  only point to a 64K buffer in 16-bit land.
   *******************************************************************/
   if (addr & 0x0001)
      return API_BUSTOOLS_EVENBCOUNT;

   if(addr >= 0x80)
	   return BTD_ERR_BADADDR;

   *rData = vbtGetPLXRegister(cardnum, addr);

   return API_SUCCESS;
}

NOMANGLE BT_INT CCONV BusTools_PLX_View(cardnum)
{
   BT_U16BIT pData;
   BT_U32BIT addr;
   char cmd[2];
   char saddr[16],sdata[16];

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if((CurrentCardType[cardnum] != QPCI1553) &&
      (CurrentCardType[cardnum] != QPCX1553) &&
      (CurrentCardType[cardnum] != QCP1553))
      return API_HARDWARE_NOSUPPORT;
     
   printf("Enter w xxxx yy for write; xxxx is addr and yy is the data\n");
   printf("Enter r xxxx nn for read;  xxxx is addr and nn is the word cnt\n");

   do{
      scanf("%s",cmd);
	  if(strcmp(cmd,"q")==0)
         break;
	  scanf("%s%s",saddr,sdata);
	  addr = strtol(saddr,(char**)NULL,16);
      pData = (BT_U16BIT)strtol(sdata,(char **)NULL,16);

	  printf("cmd = %s addr = %lx data/cnt = %x\n",cmd,addr,pData);

	  if(strcmp(cmd,"w")==0)
	  {
         BusTools_PLXWrite(cardnum,addr,pData);
	  }
	  else if(strcmp(cmd,"r")==0)
	  {
		 int i, cnt;
		 cnt = pData;

         for(i=0;i<cnt;i++)
		 {
		    BusTools_PLXRead(cardnum,addr,&pData);
		    printf("%lx - %8x\n",addr,pData);
			addr+=2;
		 }
	  }
   }while(cmd != "q" || cmd != "Q");
   return API_SUCCESS;
}
#endif //PLX_DEBUG


/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_ReadVMEConfig
*
*  FUNCTION
*       This procedure reads the VME-1553 A16 space.
*       and returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_ReadVMEConfig(
   BT_INT cardnum,         // (i) card number
   BT_U16BIT *vdata)       // (o) pointer to users buffer
{
#ifdef INCLUDE_VME_VXI_1553
   int status;
   BT_U32BIT vme_config_addr;

   if((CurrentCardType[cardnum] == VME1553) ||
      (CurrentCardType[cardnum] == QVME1553)||
	  (CurrentCardType[cardnum] == VXI1553))
   {
      if(cardnum < 100)
      {
         vme_config_addr = (BT_U32BIT)bt_iobase[cardnum];
      }
      else
         vme_config_addr = (BT_U32BIT)cardnum;

      status = vbtReadVMEConfigRegs(vme_config_addr, vdata);
      return status;
   }
   else
      return API_HARDWARE_NOSUPPORT;
#else
   return API_NO_BUILD_SUPPORT;
#endif //INCLUDE_VME_VXI_1553
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_WriteVMEConfig
*
*  FUNCTION
*       This procedure writes data to a register in the VME-1553 A16 space.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_WriteVMEConfig(
   BT_INT cardnum,         // (i) card number
   BT_UINT offset,         // (i) offset to base A16 address
   BT_U16BIT vdata)        // (i) data written to offset
{
#ifdef INCLUDE_VME_VXI_1553
   int status;
   BT_U16BIT *vme_config_addr;

   if((CurrentCardType[cardnum] == VME1553) ||
      (CurrentCardType[cardnum] == QVME1553)||
      (CurrentCardType[cardnum] == VXI1553))
   {
      if(cardnum < 100)
         vme_config_addr = (BT_U16BIT *)bt_iobase[cardnum];
      else
         vme_config_addr = (BT_U16BIT *)cardnum;

      status = vbtWriteVMEConfigRegs(vme_config_addr,offset,vdata);
      return status;
   }
   else
      return API_HARDWARE_NOSUPPORT;
#else
   return API_NO_BUILD_SUPPORT;
#endif //INCLUDE_VME_VXI_1553
}


/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetChannelStatus
*
*  FUNCTION
*       This procedure returns the current channel status structure
*
*  RETURNS
*       0 -> Success
*       API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetChannelStatus(
   BT_UINT cardnum,         // (i) card number
   API_CHANNEL_STATUS * cstat)          // (o) pointer to heart beat count
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   *cstat = channel_status[cardnum];
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetChannelCount
*
*  FUNCTION
*       This procedure returns the number of channels on a board.  
*
*  RETURNS
*       API_BUSTOOLS_BADCARDNUM;
*       API_BUSTOOLS_NOTINITED
*       Number of Channels or -1 for error
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_GetChannelCount(BT_UINT cardnum)
{
   BT_U16BIT csc;
   BT_U16BIT *vme_config_addr; // word pointer to VME/VXI config registers

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(CurrentCardType[cardnum] == PCC1553)
      return 1; //PCC-1553 has a single channel.

   if(CurrentCardType[cardnum] == PCI1553  ||
      CurrentCardType[cardnum] == ISA1553  ||
      CurrentCardType[cardnum] == DT1553   ||
      CurrentCardType[cardnum] == CPCI1553 ||
      CurrentCardType[cardnum] == PMC1553 )
   {
      csc = vbtGetCSCRegister(cardnum,  0);
      csc&=0x3000;
      if(csc==0x3000)
         return 2;
      else if(csc==0x1000)
         return 1;
      else
         return -1;
   }

   if(CurrentCardType[cardnum] == QPCI1553  ||
      CurrentCardType[cardnum] == QPCX1553  ||
      CurrentCardType[cardnum] == QPMC1553  ||
      CurrentCardType[cardnum] == QPM1553   ||
      CurrentCardType[cardnum] == AMC1553   ||
      CurrentCardType[cardnum] == R15AMC    ||
      CurrentCardType[cardnum] == RPCIe1553 ||
      CurrentCardType[cardnum] == Q1041553  ||
      CurrentCardType[cardnum] == Q1041553P ||
      CurrentCardType[cardnum] == PCCD1553  ||
      CurrentCardType[cardnum] == IPD1553   ||
      CurrentCardType[cardnum] == R15EC     ||
      CurrentCardType[cardnum] == RXMC1553  ||
      CurrentCardType[cardnum] == QCP1553 )
   {
      csc = vbtGetCSCRegister(cardnum,  0);
      csc&=0x07c0;
      return csc>>6;
   }

   if(CurrentCardType[cardnum] == QVME1553 ||
      CurrentCardType[cardnum] == VME1553  ||
      CurrentCardType[cardnum] == VXI1553  )
   {
      vme_config_addr = (BT_U16BIT *)bt_iobase[cardnum];
      csc = vme_config_addr[1];
      csc&=0x000f;
      if(csc < 4)
         return csc+1;
      else
         return csc-0xb;
   }   
   return -1;    
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetCSCRegs
*
*  FUNCTION
*       This procedure returns the boards CSC and AC registers.
*
*  RETURNS
*       0 -> Success
*       API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_GetCSCRegs(BT_UINT cardnum,BT_U16BIT *csc, BT_U16BIT *acr)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if((CurrentCardType[cardnum] == VME1553) ||
      (CurrentCardType[cardnum] == QVME1553)||
	  (CurrentCardType[cardnum] == VXI1553))
      return API_HARDWARE_NOSUPPORT;
 
   *csc = vbtGetCSCRegister(cardnum,0);

   if(board_has_acr[cardnum]) 
      *acr = vbtGetCSCRegister(cardnum,  1);     
   else
      *acr = 0;

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_BoardHasIRIG
*
*  FUNCTION
*       This procedure returns the boards CSC and AC registers.
*
*  RETURNS
*       API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*       API_FEATURE_SUPPORT (board with IRIG)
*       API_SUCCESS (Boards without IRIG)
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BoardHasIRIG(BT_UINT cardnum)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;
 
   if(board_has_irig[cardnum])
      return API_FEATURE_SUPPORT;
   else
      return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_BoardIsMultiFunction
*
*  FUNCTION
*       This procedure return whether the board is single or multi-function.
*
*  RETURNS
*       API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*       API_FEAURE_SUPPORT (for IRIG)
*       API_SUCCESS (for No IRIG)
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BoardIsMultiFunction(BT_UINT cardnum)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;
 
   if(_HW_1Function[cardnum] == MULTI_FUNCTION)
      return API_MULTI_FUNCTION;
   else
   {
      if(board_is_dual_function[cardnum])
         return API_DUAL_FUNCTION;
      else
         return API_SINGLE_FUNCTION;
   }
}
/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetBoardType
*
*  FUNCTION
*       This procedure returns the type of board program for the cardnum.  
*
*  RETURNS
*       API_BUSTOOLS_BADCARDNUM;
*       API_BUSTOOLS_NOTINITED
*       board type
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_GetBoardType(BT_UINT cardnum)
{
   if (cardnum >= MAX_BTA)
     return API_BUSTOOLS_BADCARDNUM;  //203

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;  //202

   return CurrentCardType[cardnum];
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetRevision
*
*  FUNCTION
*       This procedure returns the microcode and api revision
*       number to the caller.
*
*  RETURNS
*       0 -> success
*       API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetRevision(
   BT_UINT cardnum,         // (i) card number
   BT_UINT * mrev,          // (o) pointer to microcode revision number
   BT_UINT * arev)          // (o) pointer to API revision number
{
   /*******************************************************************
   *  Error checking
   *******************************************************************/

   *arev = API_REV;                // Return API version even if error.
   *mrev = 0;                      // There is no such thing as microcode rev 0.

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Return the board's microcode version that we read and saved.
   *******************************************************************/
   *mrev = bt_ucode_rev[cardnum];
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetSerialNumber
*
*  FUNCTION
*       This procedure returns serial number from slected boards
*
*  RETURNS
*       0 -> Success
*       API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetSerialNumber(
   BT_UINT cardnum,                     // (i) card number
   BT_U32BIT * serial_number)           // (o) pointer to heart beat count
{
   int status;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(board_has_serial_number[cardnum])
   {
      status = vbtReadSerialNumber(cardnum,(BT_U16BIT *)serial_number);
      *serial_number &= 0x0000ffff;
      if(*serial_number >= 40000)
         (*serial_number)+=40000;

      return status;
   }

   return API_HARDWARE_NOSUPPORT;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetFWRevision
*
*  FUNCTION
*       This procedure returns the microcode and api revision
*       number to the caller.
*
*  RETURNS
*       0 -> success
*       API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetFWRevision(
   BT_UINT cardnum,         // (i) card number
   float * wrev,          // (o) pointer to microcode revision number
   float * lrev,
   BT_INT  * build)          // (o) pointer to lpu revision number
{
   /*******************************************************************
   *  Error checking
   *******************************************************************/
   BT_U16BIT temp;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Return the board's microcode version that we read and saved.
   *******************************************************************/
   temp = vbtGetHWRegister(cardnum,HWREG_WCS_REVISION);
   *wrev = (float)(((((temp &0x0f00)>> 8)*100) + (((temp & 0x00f0)>> 4)*10) + (temp & 0x000f)))/(float)100.0;
   temp  = vbtGetHWRegister(cardnum,HWREG_LPU_REVISION);
   *lrev = (float)( ((((temp &0x0f00)>> 8)*100) + (((temp & 0x00f0)>> 4)*10) + (temp & 0x000f)))/(float)100.0;
   *build = vbtGetHWRegister(cardnum,HWREG_LPU_BUILD_NUMBER);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_GetPulse
*
*  FUNCTION
*       This procedure returns the current contents of the Heart Beat Register
*
*  RETURNS
*       0 -> Success
*       API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetPulse(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT * beat)          // (o) pointer to heart beat count
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if((_HW_FPGARev[cardnum] & 0xfff) < 0x388)
      return API_HARDWARE_NOSUPPORT;

   *beat = vbtGetHWRegister(cardnum,HWREG_HEART_BEAT);
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_DumpHWRegister
*
*  FUNCTION
*       This procedure reads the specified hardware register and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       BTD_ERR_PARAM -> Register =value outside H/W reigster range
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_DumpHWRegisters(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT * buff)             // (i) user buffer pointer
{ 
   int i;
   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Read specified memory area, anywhere on the card. 
   *******************************************************************/
   for(i=0;i<0x20;i++)
      buff[i] = vbtGetHWRegister(cardnum, i);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_DumpRAMRegister
*
*  FUNCTION
*       This procedure reads the specified hardware register and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       BTD_ERR_PARAM -> Register =value outside H/W reigster range
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_DumpRAMRegisters(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT * buff)             // (i) user buffer pointer
{ 
   int i;
   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Read specified memory area, anywhere on the card. 
   *******************************************************************/
   for(i=0;i<0x80;i++)
      buff[i] = vbtGetFileRegister(cardnum, i);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_HWRegisterRead
*
*  FUNCTION
*       This procedure reads the specified hardware register and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       BTD_ERR_PARAM -> Register =value outside H/W reigster range
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_HWRegisterRead(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT regnum)        // (i) BYTE address of hardware register
{ 
   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(regnum > 0x20)
      return BTD_ERR_PARAM;

   /*******************************************************************
   *  Read specified memory area, anywhere on the card.
   *******************************************************************/
   return vbtGetHWRegister(cardnum, regnum);
}

/**********************************************************************
*
*  PROCEDURE NAME -   BusTools_MemoryWrite
*
*  FUNCTION
*       This procedure writes the specified block of memory.
*
*  RETURNS
*       0 -> success
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_MemoryWrite(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr,          // (i) BYTE address in BT hardware to begin reading
   BT_U32BIT bcount,        // (i) EVEN number of bytes to read
   VOID * buff)         // (i) Pointer to user's buffer
{
   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (addr & 0x0001)
      return API_BUSTOOLS_EVENBCOUNT;

   /*******************************************************************
   *  Write specified memory area, anywhere on the card.  "buff" can
   *  only point to a 64K buffer.
   *******************************************************************/
   vbtWrite(cardnum, (LPSTR)buff, addr, (BT_UINT)bcount);
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_MemoryWrite2
*
*  FUNCTION
*       This procedure writes to the specified number of bytes to the memory 
*       block specified.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BAD_PARAM -> Invalid address or Register
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_MemoryWrite2(
   BT_UINT cardnum,         // (i) card number
   BT_INT  region,          // (i) Regions to read HIF, HWREG, RAMREG, RAM
   BT_U32BIT addr,          // (i) BYTE address in BT hardware to begin writing
   BT_U32BIT bcount,        // (i) EVEN number of bytes to write
   BT_U16BIT * buff)             // (i) Data Buffer
{
   BT_UINT regnum,indx=0;

   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   switch(region)
   {
      case HIF:
         if((addr >= 0x200) || (addr+bcount > 0x200))
            return API_BAD_PARAM; 

         vbtWriteHIF(cardnum, (LPSTR)buff, addr, (BT_UINT)bcount);
         break;
      case HWREG:
         if((addr > HWREG_COUNT2*2) || (addr+bcount > HWREG_COUNT2*2))
            return API_BAD_PARAM;

         addr/=2;    //this is a word address function
         bcount/=2;
  
         for(regnum=addr; regnum<(addr + bcount); regnum++)
            vbtSetHWRegister(cardnum, regnum, buff[indx++]);
         break;
      case RAMREG:
         if((addr > RAMREG_COUNT*2) || (addr+bcount > RAMREG_COUNT*2))
            return API_BAD_PARAM;
 
         addr/=2;    //this is a word address function
         bcount/=2;

         for(regnum=addr; regnum<(addr + bcount); regnum++)
            vbtSetFileRegister(cardnum, regnum, buff[indx++] );
         break;
      case RAM:
         if (addr & 0x0001)
            return API_BUSTOOLS_EVENADDR;
         if (bcount & 0x0001)
             return API_BUSTOOLS_EVENBCOUNT;

         vbtWriteRAM(cardnum, (LPSTR)buff, addr, (BT_UINT)bcount);
   }

   return API_SUCCESS;
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_InitSeg1
*
*  FUNCTION
*     This routine allocates the required areas in the first memory segment
*     in preparation for real operations.
*     Fill in default values for Trigger Buffer
*     Allocate space for BM Filter Buffer (@ BTMEM_BM_FBUF),
*     Build interrupt queue at BTMEM_IQ to BTMEM_IQ_NEXT (IQ_SIZE entries).
*
****************************************************************************/

void BusTools_InitSeg1(
   BT_UINT cardnum)         // (i) card number
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U16BIT  i;               // Loop counter
   BT_U32BIT  first_addr;      // Word address of first interrupt queue entry
   BT_U32BIT  next_addr;       // Word address of each next int queue entry
   BM_TBUF_ENH tbuf;            // BM Trigger Buffer structure.
   IQ_MBLOCK  intqueue;        // Interrupt queue structure.

   /*******************************************************************
   *  First available address in memory is just past registers
   ********************************************************************
   *  Fill in default values for Trigger Buffer:
   *  Allocate space for BM Trigger Buffer (@ BTMEM_BM_TBUF) and set
   *   the hardware register to point to it.
   *******************************************************************/
   memset(&tbuf,0,sizeof(tbuf));

   tbuf.trigger_header = BM_TBUF_TRIG;
   vbtWrite(cardnum,(LPSTR)&tbuf,BTMEM_BM_TBUF,sizeof(tbuf));

   // The BM trigger buffer must be located in the first 64K words of memory.
   // The RAMREG_BM_TBUF_PTR register always gets a WORD address.
   vbtSetFileRegister(cardnum,RAMREG_BM_TBUF_PTR,(BT_U16BIT)(BTMEM_BM_TBUF >> 1));

   /*******************************************************************
   *  Build interrupt queue at BTMEM_IQ to BTMEM_IQ_NEXT (IQ_SIZE entries).
   *  vbtNofify needs to know the address of the beginning of the
   *   interrupt queue.  It uses BTMEM_IQ.
   *******************************************************************/
   first_addr = next_addr = BTMEM_IQ;
   vbtSetFileRegister(cardnum,RAMREG_INT_QUE_PTR,(BT_U16BIT)(BTMEM_IQ >> 1));

   memset((void*)&intqueue,0,sizeof(intqueue));
   intqueue.t.mode.iack    = 1;
   intqueue.t.mode.unused  = 32;

   for (i = 0; i < IQ_SIZE; i++)
   {
      intqueue.t.mode.iack    = 1;
      intqueue.t.mode.unused  = 32;
      if ( i == (IQ_SIZE-1) )
         intqueue.nxt_int = (BT_U16BIT)(first_addr >> 1); // Last points to first
      else
         intqueue.nxt_int  = (BT_U16BIT)((next_addr + sizeof(intqueue)) >> 1);
      vbtWrite(cardnum,(LPSTR)&intqueue,next_addr,sizeof(intqueue));
      next_addr += sizeof(intqueue);
   }

   /*******************************************************************
   *  Reserve error injection buffers from BTMEM_EI to BTMEM_EI_NEXT.
   *  They must be in the lower 64 K words.
   ********************************************************************
   *  Allocate space for BM Filter Buffer (@ BTMEM_BM_FBUF),
   *   which must be on an even 2048 word boundary.
   *******************************************************************/
   vbtSetFileRegister(cardnum,RAMREG_BM_FBUF_PTR,(BT_U16BIT)(BTMEM_BM_FBUF >> 1));

   /*******************************************************************
   *  Show memory buffer to be rest of first segment (BTMEM_BM_MBUF to
   *  btmem_bm_mbuf_next[cardnum]).
   *******************************************************************/
   btmem_bm_mbuf[cardnum]      = BTMEM_CH_SHARE_NEXT;   // for channel sharing
   btmem_bm_mbuf_next[cardnum] = BTMEM_CH_SHARE_NEXT+1; //

   btmem_bm_cbuf[cardnum]      = 0;
   btmem_bm_cbuf_next[cardnum] = 1;

   bc_mblock_count[cardnum] = 0;   // No BC message buffers allocated
   btmem_bc[cardnum]        = 0;   // No BC message buffer address defined
   btmem_bc_next[cardnum]   = 1;   // No BC message buffer end address defined

   /*******************************************************************
   *  Set up next avail location for first segment
   *******************************************************************/
   btmem_tail1[cardnum] = btmem_bm_mbuf[cardnum];

   /*******************************************************************
   *  Set up next avail and max location for PCI-1553 high memory.
   *  This memory can only be used by BC Message blocks, BC Data blocks,
   *   BM Message blocks and RT Message blocks.  Init to 64 KW and
   *   size of PCI-1553 memory.
   *******************************************************************/
   btmem_pci1553_next[cardnum]    = 0x20000L;
   btmem_pci1553_rt_mbuf[cardnum] = 1024L*CurrentMemKB[cardnum]; // V4.08.ajh
   return;
}

/****************************************************************
*
*  PROCEDURE NAME -   BusTools_MemoryAlloc
*
*  FUNCTION
*       This procedure reserves a block of memory in the specified
*       memory segment for use by the caller.  BusTools will not
*       use that block of memory.
*
*  RETURNS
*       0 -> success
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*       API_BUSTOOLS_NO_MEMORY  -> not enough memory available
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_MemoryAlloc(
   BT_UINT cardnum,         // (i) card number
   BT_UINT segnum,          // (i) Segment to allocate memory in (not used)
   BT_U32BIT bcount,        // (i) number of bytes to be allocated (even)
   BT_U32BIT * addr)    // (o) Allocated buffer address in BT hardware
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT  nextbc;    // Proposed end of first seg

   /************************************************
   *  Error checking
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bcount & 1)
      return API_BUSTOOLS_EVENBCOUNT;

   /*******************************************************************
   *  Check for memory overflow
   *******************************************************************/
   nextbc = bcount + btmem_tail1[cardnum];

   if ( nextbc > 65535*2 )   // Only allocate memory in lower 64 kwords
      return API_BUSTOOLS_NO_MEMORY;

   /*******************************************************************
   *  Save address and update next available location
   *******************************************************************/

   *addr = btmem_tail1[cardnum];

   btmem_tail1[cardnum] = nextbc;
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetBroadcast
*
*  FUNCTION
*     This routine sets or clears both the hardware and the software
*     flags for enabling broadcast.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetBroadcast(
   BT_UINT cardnum,        // (i) card number
   BT_UINT bcast)          // (i) broadcast flag (0 -> no broadcast)
{
   /************************************************
   *  Check for legal call
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_running[cardnum] )     // RT simulation running flag.
      return API_RT_RUNNING;

   /************************************************
   *  Figure out what to do with the bit.  Setup the
   *  hardware, and flay a re-init of the RT if
   *  the state of the flag is being changed.
   ************************************************/

   if ( bcast )
   {
      if ( rt_bcst_enabled[cardnum] == 0 )
         rt_inited[cardnum] = 0;       // RT must be re-initialized.
      rt_bcst_enabled[cardnum] = 1;    // RT address 31 is broadcast.
      api_writehwreg_or(cardnum,HWREG_CONTROL1,CR1_RT31BCST);
      channel_status[cardnum].broadcast=1;
   }
   else
   {
      if ( rt_bcst_enabled[cardnum] != 0 )
         rt_inited[cardnum] = 0;       // RT must be re-initialized.
      rt_bcst_enabled[cardnum] = 0;    // RT address 31 is not broadcast.
      api_writehwreg_and(cardnum,HWREG_CONTROL1,(BT_U16BIT)~CR1_RT31BCST);
      channel_status[cardnum].broadcast=0;
   }

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetExternalSync
*
*  FUNCTION
*     This routine sets the flag for external sync of the time-
*     tag register.  If the flag is set, setting the external
*     TTL input will clear the time-tag counter (both the BM
*     and the RT use this same time-tag counter).
*     This function is duplicated by the BusTools_TimeTagMode()
*     function.  For flag = 0
*     BusTools_TimeTagMode(cardnum, API_TT_DEFAULT, API_TT_DEFAULT,
*                          API_TTM_FREE, NULL, 0, 0, 0);
*     else for flag = 1
*     BusTools_TimeTagMode(cardnum, API_TT_DEFAULT, API_TT_DEFAULT,
*                          API_TTM_RESET, NULL, 0, 0, 0);
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetExternalSync(BT_UINT cardnum,
   BT_UINT flag)        // (i) 0 -> disable external sync, 1-> enable sync
{
   /************************************************
   *  Check for legal call
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /************************************************
   *  Figure out what to do with the bit
   ************************************************/
   if ( flag )
      BusTools_TimeTagMode(cardnum, API_TT_DEFAULT, API_TT_DEFAULT,
                           API_TTM_RESET, NULL, 0, 0, 0);
   else
      BusTools_TimeTagMode(cardnum, API_TT_DEFAULT, API_TT_DEFAULT,
                           API_TTM_FREE, NULL, 0, 0, 0);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetInternalBus
*
*  FUNCTION
*     This routine sets the flag for external or internal bus.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetInternalBus(
   BT_UINT cardnum,        // (i) card number
   BT_UINT busflag)        // (i) 0 -> external bus, 1-> internal bus
{
   /************************************************
   *  Check for legal call
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /************************************************
   *  Figure out what to do with the bit
   ************************************************/

   if (busflag)       // Internal bus.
   {
      api_writehwreg_or (cardnum,HWREG_CONTROL1,CR1_IN_WRP);
      channel_status[cardnum].extbus=0;
   }
   else               // External bus.
   {
      api_writehwreg_and(cardnum,HWREG_CONTROL1,(BT_U16BIT)(~CR1_IN_WRP));
      channel_status[cardnum].extbus=1;
   }

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetTestBus
*
*  FUNCTION
*     This routine sets the Test bus on card that support test bus.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetTestBus(
   BT_UINT cardnum,        // (i) card number
   BT_UINT busflag)        // (i) TEST_BUS_ENABLE enables the test bus
                           //     TEST_BUS_DISABLE disalbe the test bus
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_testbus[cardnum])
	   return API_HARDWARE_NOSUPPORT;
   
   if(busflag == TEST_BUS_ENABLE)
      api_writehwreg_or(cardnum,HWREG_CONTROL1,TEST_BUS_ENABLE);
   if(busflag == TEST_BUS_DISABLE)
      api_writehwreg_and(cardnum,HWREG_CONTROL1,TEST_BUS_DISABLE);

   return API_SUCCESS;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_SetOptions
*
*  FUNCTION
*     This function sets the various RT options.  
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetOptions(
   BT_UINT cardnum,        // (i) card number
   BT_UINT intflag,        // (i) 0x0001 -> suppress MF OFlow message
                           //     0x0002 -> Monitor Invalid Commands
                           //     0x0004 -> Dump on BM Stop
                           //     0x0008 -> BM trigger on message
                           //     0x0010 -> Ignore High Word
                           //     0x0020 -> Process BM data on Interrupt
   BT_UINT resettimer,     // (i) Reset TT on Sync Mode code
   BT_UINT trig_on_sync,   // (i) Trigger output on Sync Mode Code enable = 1
                           //     disable = 0;
   BT_UINT enable_rt)      // (i) RT start on trigger input enable = 1
                           //     Disable = 0;
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT value;
   BT_U16BIT wValue;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Output "interrupt on illegal command" flag
   *******************************************************************/
   value = vbtGetFileRegister(cardnum, RAMREG_ORPHAN);

   /*******************************************************************
   *  Output "reset timetag on RT sync mode code" flag.
   *******************************************************************/
   if (resettimer)
      value |= RAMREG_ORPHAN_RESET_TIMETAG;
   else
      value &= (BT_U16BIT)~RAMREG_ORPHAN_RESET_TIMETAG;

   if(intflag & 0x0001)
      supMF_OFlow[cardnum] = 1;
   else
      supMF_OFlow[cardnum] = 0;

   
   if((intflag & 0x0002) && (intflag & 0x0010))
      return API_PARAM_CONFLICT;
   /*******************************************************************
   *  Monitor invalid command
   *******************************************************************/
   if(intflag & 0x0002)// Monitor Invalid Commands
   {
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x418)
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_MON_INV_CMD;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }
   else //
   {
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue &= ~CR2_MON_INV_CMD;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
   }

   /*******************************************************************
   *  Monitor invalid command
   *******************************************************************/
   if(intflag & 0x0010)// Ignore high word detection
   {
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x440)
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_IGN_HGH_WRD;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }
   else //
   {
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue &= ~CR2_IGN_HGH_WRD;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
   }

   if(intflag & 0x0004)
      DumpOnBMStop[cardnum] = 1;
   else
      DumpOnBMStop[cardnum] = 0;

   if(intflag & 0x0008)
      BMTrigOutput[cardnum] = 1;
   else
      BMTrigOutput[cardnum] = 0;

   if(intflag & 0x0020)
      procBmOnInt[cardnum] = 1;
   else
      procBmOnInt[cardnum] = 0;

   /*******************************************************************
   * Output the Trigger Output on Sync Mode Code bit
   *******************************************************************/
   if (trig_on_sync)
      value |= RAMREG_ORPHAN_FIRE_EXT_ON_SYNC_MC;
   else
      value &= (BT_U16BIT)~RAMREG_ORPHAN_FIRE_EXT_ON_SYNC_MC;

   vbtSetFileRegister(cardnum, RAMREG_ORPHAN, (BT_U16BIT)value);

   /*******************************************************************
   * Output the RT Enable on External Sync bit
   *******************************************************************/
   if (enable_rt)
      api_writehwreg_or(cardnum, HWREG_CONTROL1, CR1_RTEXSYN);
   else
      api_writehwreg_and(cardnum, HWREG_CONTROL1, (BT_U16BIT)(~CR1_RTEXSYN));

   return API_SUCCESS;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_PCI_Reset
*
*  FUNCTION
*     This function enables and disables the board to respond to PCI Reset The
*     default setting is to disable PCI Reset so there is no response to a 
*     PCI reset.  If you enable the PCI reset the board will reset on a PCI
*     Reset.
*
*     THIS SETTING IS FOR THE BOARD AND ALL CHANNELS.  A CARDNUM IS REQUIRED 
*     SO WE CAN KNOW THE BOARD IN A MULTI-BOARD SYSTEM
*
**************************************************************************/
NOMANGLE BT_INT CCONV BusTools_PCI_Reset(BT_INT cardnum, BT_UINT reset_flag)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(CurrentCardType[cardnum] == PCC1553  ||
      CurrentCardType[cardnum] == PCCD1553 ||
      CurrentCardType[cardnum] == ISA1553  ||
      CurrentCardType[cardnum] == Q1041553 ||
      CurrentCardType[cardnum] == VME1553  ||
      CurrentCardType[cardnum] == QVME1553 ||
      CurrentCardType[cardnum] == IPD1553 )
      return API_HARDWARE_NOSUPPORT;

   if(_HW_FPGARev[cardnum] >= 0x420)
   {
      if(reset_flag)
         ((BT_U16BIT *)(bt_PageAddr[cardnum][3]))[9] = 0x1; //Enable PCI Reset
      else
         ((BT_U16BIT *)(bt_PageAddr[cardnum][3]))[9] = 0x0; //Disable PCI Reset

      return API_SUCCESS;
   }
   return API_HARDWARE_NOSUPPORT;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_VME_Reset
*
*  FUNCTION
*     This function enables/disables the board to respond to a VME Reset. The
*     default setting is to disable VME Reset so there is no response to 
*     SysReset.  If you enable the VME reset the board will reset on a VME
*     SysReset.
*
*     NOTE:
*     THIS SETTING IS FOR THE BOARD AND ALL CHANNELS.  A CARDNUM IS REQUIRED 
*     SO WE CAN KNOW THE BOARD IN A MULTI-BOARD SYSTEM
*
**************************************************************************/
NOMANGLE BT_INT CCONV BusTools_VME_Reset(BT_INT cardnum, BT_UINT reset_flag)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(CurrentCardType[cardnum] != QVME1553)
      return API_HARDWARE_NOSUPPORT;

   if(_HW_FPGARev[cardnum] >= 0x423)
   {
      if(reset_flag)
         ((BT_U16BIT *)(bt_PageAddr[cardnum][3]))[9] = 0x1; //Enable VME Reset
      else
         ((BT_U16BIT *)(bt_PageAddr[cardnum][3]))[9] = 0x0; //Disable VME Reset
      return API_SUCCESS;
   }
   return API_HARDWARE_NOSUPPORT;

}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_SetPolling
*
*  FUNCTION
*     This stops the timer, resets the polling interval and then restarts
*     the timer.  There is only one timer per application, no matter how 
*     many channels are running.
*
**************************************************************************/
NOMANGLE BT_INT CCONV BusTools_SetPolling(BT_UINT polling)
{
   if((polling >= 1) && (polling <= 2000))
   {
      return vbtSetPolling(polling, TIMER_RESTART);
   }
   return API_BAD_PARAM;
}

NOMANGLE BT_INT CCONV BusTools_SetSa31(
   BT_UINT cardnum,        // (i) card number
   BT_UINT sa31)           // (i) 1=SA31 is a mode code, 0=SA31 NOT a mode code
{
   /*******************************************************************
   *  Check for legal call
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (rt_running[cardnum] )     // RT running flag.
      return API_RT_RUNNING;

   /************************************************
   *  Figure out what to do with the bit
   ************************************************/

   if ( sa31 )
   {
      if ( rt_sa31_mode_code[cardnum] == 0 )
         rt_inited[cardnum] = 0;        // RT must be re-initialized.
      rt_sa31_mode_code[cardnum] = 1;   // SA 31 IS a mode code.
      api_writehwreg_or(cardnum,HWREG_CONTROL1,CR1_SA31MC);
      channel_status[cardnum].SA_31=1;
   }
   else
   {
      if ( rt_sa31_mode_code[cardnum] != 0 )
         rt_inited[cardnum] = 0;        // RT must be re-initialized.
      rt_sa31_mode_code[cardnum] = 0;   // SA 31 is NOT a mode code.
      api_writehwreg_and(cardnum,HWREG_CONTROL1,(BT_U16BIT)~CR1_SA31MC);
      channel_status[cardnum].SA_31=0;
   }
   return API_SUCCESS;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_SetVoltage
*
*  FUNCTION
*     This routine sets the voltage/coupling hardware register based
*     on the supplied parameters.
*     We need to keep track of the output voltage, and if the current
*     call is reducing that voltage, delay until the level has had a
*     chance to decay.  This will prevent us from transmitting a decaying
*     sine wave pattern, and keep the BC from crashing.
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetVoltage(
   BT_UINT cardnum,        // (i) card number
   BT_UINT  voltage,       // (i) volts peak-to-peak * 100
   BT_UINT  coupling)      // (i) 0 -> direct, 1 -> transformer coupling
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_INT  i;
   BT_INT  voltword;
   BT_INT  vword=0;
   BT_UINT delay_count=0;
   BT_U16BIT DAC_RDY;

   static BT_UINT direct_table[] =
   {
       650,  630,  610,  590,  570,  550,  520,  500,
       480,  460,  430,  410,  382,  360,  340,  320,
       300,  280,  254,  232,  212,  190,  170,  140,
       120,  100,   80,   60,   40,   20,    0,    0
   };

   static BT_UINT trans_table[] =
   {
      1980, 1910, 1880, 1760, 1690, 1640, 1560, 1500,
      1430, 1360, 1300, 1240, 1180, 1100, 1020,  950,
       880,  810,  750,  690,  620,  550,  490,  410,
       350,  276,  204,  134,   68,   19,    0,    0
   };

   /*******************************************************************
   *  Check for legal call
   *******************************************************************/

   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if ( coupling > 1 )
      return API_BUSTOOLS_BADCOUPLING;

   if ((CurrentCardType[cardnum] == PMC1553))
      return API_HARDWARE_NOSUPPORT; // Does not support variable voltage.

   /*******************************************************************
   *  Check for voltage out of range.
   *******************************************************************/
   if ( coupling == 0 )
   {
      channel_status[cardnum].coupling=0;
      if ( voltage > direct_table[0] )
         return API_BUSTOOLS_BADVOLTAGE;
   }
   else
   {
      channel_status[cardnum].coupling=1;
      if ( voltage > trans_table[0] )
         return API_BUSTOOLS_BADVOLTAGE;
   }
   /*******************************************************************
   *  Convert voltage value to scaled integer.
   *******************************************************************/
   switch(coupling)
   {
      case 0:     // direct connection
         for ( i = 1; i < 32; i++ )
         {
            if ( voltage >= direct_table[i] )
            {
               vword = 128 * i -
                  ((128 * (voltage - direct_table[i])) /
                                    (direct_table[i-1] - direct_table[i]));
               break;
            }
         }
         break;
      case 1:     // transformer connection
         for ( i = 1; i < 32; i++ )
         {
            if ( voltage >= trans_table[i] )
            {
               vword = 128 * i -
                  ((128 * (voltage - trans_table[i])) /
                                    (trans_table[i-1] - trans_table[i]));
               break;
            }
         }
         break;
   }

   /*******************************************************************
   *  Generate the voltage and coupling word and output it.
   *******************************************************************/

   /* Coupling bit is in the HW control register at location zero */
   if ( coupling )       //  1 -> transformer coupling
      api_writehwreg_or(cardnum,HWREG_CONTROL1,CR1_BUS_COUPLE);
   else
      api_writehwreg_and(cardnum,HWREG_CONTROL1,(BT_U16BIT)~CR1_BUS_COUPLE);

   /* Write the 8-bit voltage for the production (Rev 1) PCI-1553 boards */
   voltword = ((~vword >> 4) & 0x00FF) | 0x1300;

   if((_HW_FPGARev[cardnum] & 0x0fff) > 0x398 && CurrentCardType[cardnum] != (BT_UINT)AR15VPX)
   {
      if((DAC_RDY = vbtGetDiscrete(cardnum,DAC_VALID)) == 1)
      {
         api_writehwreg(cardnum, HWREG_PCI_VOLTAGE, (BT_U16BIT)voltword);
         while(vbtGetDiscrete(cardnum,DAC_VALID) == 0)
         {
            MSDELAY(1);
            if(delay_count++ > 2000)
               return API_DAC_ERROR;
         }
      }         
      else
         return API_DAC_ERROR;
   }
   else
      api_writehwreg(cardnum, HWREG_PCI_VOLTAGE, (BT_U16BIT)voltword);

   return API_SUCCESS;
}

#define FLASH_WORD_ADDR_LSB  0xa1 //142
#define FLASH_WORD_ADDR_MSB  0xa2 //144
#define FLASH_DATA_WRITE_REG 0xa3 //146
#define FLASH_DATA_READ_REG  0xa4 //148
#define READ_MODE 0xff
#define STATUS_REG 0x70
#define ERASE_SETUP 0x20
#define ERASE_CONFIRM 0xd0
#define WRITE_MODE 0x40
#define DATA_BLOCK_0 0x2000
#define DATA_BLOCK_1 0x3000
#define DATA_BLOCK_2 0x4000
#define DATA_BLOCK_3 0x5000
#define DATA_BLOCK_4 0x6000

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_1760_DataWrite
*
*  FUNCTION
*     The function writes data into the flash for 1760 start message data
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_1760_DataWrite(BT_UINT cardnum, BT_U16BIT **saData, BT_U32BIT saEnable)
{
   BT_U16BIT dbuf,rdata;
   static int pindx;
   BT_UINT rindx,sa_indx;
   BT_U16BIT edata[2];

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      //some short delay???
      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);

   //Place in erase setup mode
   dbuf = ERASE_SETUP;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   //Write erase block  address
   dbuf = DATA_BLOCK_0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);
   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);  

   //Erase the block
   dbuf = ERASE_CONFIRM;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      //some short delay???
      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);


   pindx = 0x0;

   //now write the data
   for(sa_indx=0; sa_indx<32; sa_indx++)
   {
      for(rindx=0; rindx<32; rindx++)
      { 
         //Place in write mode by writing data 0x40 to FLASH_DATA_REG
         dbuf = WRITE_MODE;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
    
         dbuf = DATA_BLOCK_0 + pindx++;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

         dbuf = 0x0;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

         dbuf = saData[sa_indx][rindx];
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

         do{
            //Place in read status mode
            dbuf = STATUS_REG;
            vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

            //some short delay???
            vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
         } while ((rdata & 0x80) == 0);
      }
   }

   edata[0] = (BT_U16BIT)(saEnable & 0xffff);
   edata[1] = (BT_U16BIT)((saEnable & 0xffff0000)>>16);

   //write out the SA enable register. 
   for(rindx = 0; rindx<2; rindx++)
   {  
      dbuf = WRITE_MODE;  //Place in write mode by writing data 0x40 to FLASH_DATA_REG
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      dbuf = DATA_BLOCK_0 + 0x400 + rindx;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);
      dbuf = 0x0;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
      dbuf = edata[rindx];
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      do{
         //Place in read status mode
         dbuf = STATUS_REG;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

         //some short delay???
         vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
      } while ((rdata & 0x80) == 0);
   }

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_1760_DataRead
*
*  FUNCTION
*     The function reads data into the flash for 1760 start message data
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_1760_DataRead(BT_UINT cardnum, BT_U16BIT **saData, BT_U32BIT *saEnable)
{
   BT_U16BIT dbuf,ebuf1,ebuf2,rdata;
   BT_UINT rindx,sa_indx,pindx=0;

   //Place in read status mode
   dbuf = STATUS_REG;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
   //some short delay???
   vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);

   //Place in read mode by writing data 0xff to FLASH_DATA_REG
   dbuf = READ_MODE;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   for(sa_indx=0; sa_indx<32; sa_indx++)
   {
      for(rindx=0; rindx<32; rindx++)
      {    
         dbuf = DATA_BLOCK_0 + pindx++;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

         dbuf = 0x0;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

         dbuf = 0x0;
         vbtReadHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_READ_REG*2,2);
         saData[sa_indx][rindx] = dbuf;
      }
   }

   //read enable LSB
   dbuf = DATA_BLOCK_0 + 0x400;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
   vbtReadHIF(cardnum,(LPSTR)&ebuf1,FLASH_DATA_READ_REG*2,2);

   //read enable MSB
   dbuf = DATA_BLOCK_0 + 0x401;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
   vbtReadHIF(cardnum,(LPSTR)&ebuf2,FLASH_DATA_READ_REG*2,2);
   
   *saEnable = (BT_U32BIT)((BT_U32BIT)ebuf1 | (BT_U32BIT)(ebuf2 << 16));

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_DataGetString
*
*  FUNCTION
*     The function convert data into engineering units
*
****************************************************************************/
NOMANGLE char * CCONV BusTools_DataGetString(DATA_CONVERT *cdat)
{

   //************************************************
   //  local variables
   //************************************************

   float         fValue;     // local copy of data
   BT_U16BIT     wValue;     // local copy of data
   BT_U32BIT     dwValue;    // local copy of data
   int           iValue;     // for signed data
   BT_U16BIT*    pwValue;    // pointer to words
   UINT          i;          // loop counter
   UINT          nOK;        // FALSE -> bad string conversion
   BT_U16BIT     wMask;      // mask for binary conversions
   char          format[64]; // format string
   BT_U16BIT     w_lsw;      // for word swapping
   BT_U16BIT     w_msw;      // for word swapping
   BT_INT		 status;	 // return value

   int      exp, temp1, temp2;
   double   usmant, sclmant;

   // Initialize return value.
   status = API_SUCCESS;

   //************************************************
   //  check for legal value
   //************************************************
   if (!(cdat->wDatatype < DATATYPE_MAX))
	   status = API_BADDATATYPE;

   //************************************************
   //  now do conversion based on wDatatype
   //************************************************
   wValue = *((unsigned*)cdat->value);

   // Initialize the return to FALSE
   nOK = 0;

   switch (cdat->wDatatype)
      {
      case DATATYPE_16_SDEC:
         dwValue = wValue;
         if (wValue & 0x8000)
            // force sign extension 
            dwValue |= 0xffff0000;
         nOK = sprintf( cdat->string, "%d", dwValue);
         break;

      case DATATYPE_16_UDEC:
         nOK = sprintf( cdat->string, "%u", wValue);
         break;
      
      case DATATYPE_16_HEX:
      case DATATYPE_16_UDB:
         nOK = sprintf( cdat->string, "%.4X", wValue);
         break;

      case DATATYPE_16_OCTAL:
         nOK = sprintf( cdat->string, "%o", wValue);
         break;

      case DATATYPE_16_BINARY:
      case DATATYPE_16_DISCRETE:
         i = 0;
         for ( wMask=0x8000; wMask; wMask = wMask >> 1 ) {
            if ( wValue & wMask )
               cdat->string[i] = '1';
            else
               cdat->string[i] = '0';
            i++;      
         }
         // Null terminate the buffer.
         cdat->string[i] = '\0';
         break;

      case DATATYPE_16_BCD:
         for ( i=0; i<4; i++ ) {
            if (( wValue & (WORD)( 0x000F << 4*i )) > (WORD)(0x0009 << 4*i) ) {
               // Invalid value...pop out of here.
			   status = API_BADBCDDATA;
               wValue = 0;
               break;
            }   
         }
         // Display the value.
         nOK = sprintf( cdat->string, "%x", wValue);
         break;

      case DATATYPE_16_BCD_2:
         for ( i=0; i<2; i++ ) {
            if (( wValue & (WORD)( 0x000F << 4*i )) > (WORD)(0x0009 << 4*i) ) {
               // Invalid value...pop out of here.
			   status = API_BADBCDDATA;
               wValue = 0;
               break;
            }   
         }

         i = 0;
         for ( wMask=0x8000; wMask != 0x0080; wMask = wMask >> 1 ) {
            if ( wValue & wMask )
               cdat->string[i] = '1';
            else
               cdat->string[i] = '0';
            i++;      
         }
         // Null terminate the buffer.
         cdat->string[i] = '\0';

         // Display the value.
         nOK = sprintf( &cdat->string[i], " %02x", wValue & 0x00ff );
         break;

      case DATATYPE_16_USCALE:
         sprintf(format,"%%.%df",cdat->wDecimals);
         switch(cdat->wFactortype) {
            case 1:     // multiply
               nOK = sprintf( cdat->string, format, (float)wValue * cdat->fFactor + cdat->fOffset);
               break;
            case 2:     // divide
               nOK = sprintf( cdat->string, format, (float)wValue / cdat->fFactor + cdat->fOffset);
               break;
			default:
			   status = API_BADFACTORTYPE;
			   break;
         }
         break;

      case DATATYPE_16_SSCALE:   // signed scaled
         iValue = *((short int*)cdat->value);
         sprintf(format,"%%.%df",cdat->wDecimals);
         switch(cdat->wFactortype) {
            case 1:     // multiply
               nOK = sprintf( cdat->string, format, (float)iValue * cdat->fFactor + cdat->fOffset);
               break;
            case 2:     // divide
               nOK = sprintf( cdat->string, format, (float)iValue / cdat->fFactor + cdat->fOffset);
               break;
			default:
			   status = API_BADFACTORTYPE;
			   break;
         }
         break;

      case DATATYPE_16_TRANSLATE:
         // Note: Table must be ordered from low to high
         // raw hex data values.
         sprintf(format,"%%.%df",cdat->wDecimals);
         if (wValue < cdat->uiTranslateRaw[0]) {
            strcpy(cdat->string, "-Undef Low-"); // Value too low
			status = API_BADTRANSLATE;
            break;
         }
         else if (wValue > cdat->uiTranslateRaw[cdat->wTranslateItems-1]) {
            strcpy(cdat->string, "+Undef High+"); // Value too high
			status = API_BADTRANSLATE;
            break;
         }

         // find the nearest lookup table entry
         for (i=0; i<cdat->wTranslateItems; i++) {
            if (wValue <= cdat->uiTranslateRaw[i]) {
               float fDisData;
               if (wValue == cdat->uiTranslateRaw[i]) {
                  fDisData = cdat->fTranslateDis[i];
               }
               else {
                  // Perform linear interpolation between points
                  WORD wDelta;
                  float fPercent, fDelta;
                  // check for divide by zero
                  wDelta = cdat->uiTranslateRaw[i] - cdat->uiTranslateRaw[i-1];
                  if (wDelta == 0)
                     wDelta = 1;
                  fPercent = (float)(wValue-cdat->uiTranslateRaw[i-1]) / (float)wDelta;
                  fDelta = cdat->fTranslateDis[i] - cdat->fTranslateDis[i-1];
                  fDisData = cdat->fTranslateDis[i-1] + fPercent * fDelta;
               }
               nOK = sprintf( cdat->string, format, fDisData);
               break;
            }
         }
         break;

      case DATATYPE_32_SDEC:
         dwValue = *((DWORD*)cdat->value);
         nOK = sprintf( cdat->string, "%d", dwValue);
         break;
      
      case DATATYPE_32_MSWF_SDEC:
         // force the most significant 16 bit word to be first
         pwValue = (WORD*)cdat->value;
         w_lsw = pwValue[0];
         w_msw = pwValue[1];
         dwValue = ((DWORD)w_lsw << 16) | (DWORD)w_msw;
         nOK = sprintf( cdat->string, "%d", dwValue);
         break;
      
      case DATATYPE_32_UDEC:
         dwValue = *((DWORD*)cdat->value);
         nOK = sprintf( cdat->string, "%u", dwValue);
         break;

      case DATATYPE_32_MSWF_UDEC:
         // force the most significant 16 bit word to be first
         pwValue = (WORD*)cdat->value;
         w_lsw = pwValue[0];
         w_msw = pwValue[1];
         dwValue = ((DWORD)w_lsw << 16) | (DWORD)w_msw;
         nOK = sprintf( cdat->string, "%u", dwValue);
         break;

      case DATATYPE_32_HEX:
      case DATATYPE_32_UDB:
         dwValue = *((DWORD*)cdat->value);
         nOK = sprintf( cdat->string, "%.8X", dwValue);
         break;

      case DATATYPE_32_1750:
         // This code courtesy of Martin Amari with Goodrich Corp
         pwValue = (WORD*)cdat->value;
         w_lsw = pwValue[1];
         w_msw = pwValue[0];
         exp = (w_lsw & 0xFF);
         if (exp > 127)
            exp -= 256;
         temp1 = w_msw;
         temp2 = w_lsw;
         usmant = (double)((temp1 * 256) + (int)(temp2 / 256));
         if (usmant > 8388607)
            usmant -= 16777216;
         sclmant = usmant / 8388608;
         if (exp > 30 )
            exp = 30;
         if (exp < -30 )
            exp = -30;

         if (exp > 0) {
            // faster...
            fValue = (float)(sclmant * (double)(0x1 << exp));
         }
         else {
            fValue = (float)ldexp( sclmant, exp );
         }
         sprintf(format,"%%.%df",cdat->wDecimals);
         nOK = sprintf( cdat->string, format, fValue );
         break;

      case DATATYPE_32_DEC_FLOAT:
         // force the most significant 16 bit word to be first
         pwValue = (WORD*)cdat->value;
         w_lsw = pwValue[0];
         w_msw = pwValue[1];
         dwValue = ((DWORD)w_lsw << 16) | (DWORD)w_msw;
         // treat the result as a float
		 /* Rich Wade - changed this:
         pdwValue = &dwValue;
         fValue = *((float*)pdwValue);
		 *** to this: */
		 fValue = (float) dwValue;

         sprintf(format,"%%.%df",cdat->wDecimals);
         // now scale the float
         switch(cdat->wFactortype) {
            case 1:     // multiply
               nOK = sprintf( cdat->string, format, fValue * cdat->fFactor + cdat->fOffset);
               break;
            case 2:     // divide
               nOK = sprintf( cdat->string, format, fValue / cdat->fFactor + cdat->fOffset);
               break;
			default:
			   status = API_BADFACTORTYPE;
			   break;
         }
         break;

      case DATATYPE_32_BCD:
         dwValue = *((DWORD*)cdat->value);
         for ( i=0; i<8; i++ ) {
            if (( dwValue & (DWORD)( 0x0000000FL << 4*i )) > (DWORD)(0x00000009L << 4*i) ) {
               // Invalid value...pop out of here.
               dwValue = 0;
               break;
            }   
         }
         nOK = sprintf( cdat->string, "%x", dwValue);
         break;

      case DATATYPE_32_IEEE:
         fValue = *((float*)cdat->value);
         sprintf(format,"%%.%df",cdat->wDecimals);
         nOK = sprintf( cdat->string, format, fValue);
         break;

   //************************************************
   //  32 bit unsigned scaled
   //************************************************
      case DATATYPE_32_USCALE:
         dwValue = *((DWORD*)cdat->value);
         sprintf(format,"%%.%df",cdat->wDecimals);
         switch(cdat->wFactortype) {
            case 1:     // multiply
               nOK = sprintf( cdat->string, format, (float)dwValue * cdat->fFactor + cdat->fOffset);
               break;
            case 2:     // divide
               nOK = sprintf( cdat->string, format, (float)dwValue / cdat->fFactor + cdat->fOffset);
               break;
			default:
			   status = API_BADFACTORTYPE;
			   break;
         }
         break;

   //************************************************
   //  32 bit signed scaled
   //************************************************
      case DATATYPE_32_SSCALE:
         iValue = *((int*)cdat->value);
         sprintf(format,"%%.%df",cdat->wDecimals);
         switch(cdat->wFactortype) {
            case 1:     // multiply
               nOK = sprintf( cdat->string, format, (float)iValue * cdat->fFactor + cdat->fOffset);
               break;
            case 2:     // divide
               nOK = sprintf( cdat->string, format, (float)iValue / cdat->fFactor + cdat->fOffset);
               break;
			default:
			   status = API_BADFACTORTYPE;
			   break;
         }
         break;

   //************************************************
   //  32 bit most significant word first unsigned scaled
   //************************************************
      case DATATYPE_32_MSWF_USCALE:
         // force the most significant 16 bit word to be first
         pwValue = (WORD*)cdat->value;
         w_lsw = pwValue[0];
         w_msw = pwValue[1];
         dwValue = ((DWORD)w_lsw << 16) | (DWORD)w_msw;
         sprintf(format,"%%.%df",cdat->wDecimals);
         switch(cdat->wFactortype) {
            case 1:     // multiply
               nOK = sprintf( cdat->string, format, (float)dwValue * cdat->fFactor + cdat->fOffset);
               break;
            case 2:     // divide
               nOK = sprintf( cdat->string, format, (float)dwValue / cdat->fFactor + cdat->fOffset);
               break;
			default:
			   status = API_BADFACTORTYPE;
			   break;
         }
         break;

   //************************************************
   //  32 bit most significant word first signed scaled
   //************************************************
      case DATATYPE_32_MSWF_SSCALE:
         // force the most significant 16 bit word to be first
         pwValue = (WORD*)cdat->value;
         w_lsw = pwValue[0];
         w_msw = pwValue[1];
         dwValue = ((DWORD)w_lsw << 16) | (DWORD)w_msw;
         iValue = (int)dwValue;
         sprintf(format,"%%.%df",cdat->wDecimals);
         switch(cdat->wFactortype) {
            case 1:     // multiply
               nOK = sprintf( cdat->string, format, (float)iValue * cdat->fFactor + cdat->fOffset);
               break;
            case 2:     // divide
               nOK = sprintf( cdat->string, format, (float)iValue / cdat->fFactor + cdat->fOffset);
               break;
			default:
			   status = API_BADFACTORTYPE;
			   break;
         }
         break;

   //************************************************
   //  32 bit table lookup
   //************************************************
      case DATATYPE_32_TRANSLATE:
         dwValue = *((DWORD*)cdat->value);
         // Note: Table must be ordered from low to high
         // raw hex data values.
         sprintf(format,"%%.%df",cdat->wDecimals);

         if (dwValue < cdat->uiTranslateRaw[0]) {
            strcpy(cdat->string, "-Undef Low-"); // Value too low
			status = API_BADTRANSLATE;
            break;
         }
         else if (dwValue > cdat->uiTranslateRaw[cdat->wTranslateItems-1]) {
            strcpy(cdat->string, "+Undef High+"); // Value too high
			status = API_BADTRANSLATE;
            break;
         }

         // find the nearest lookup table entry
         for (i=0; i<cdat->wTranslateItems; i++) {
            if (dwValue <= cdat->uiTranslateRaw[i]) {
               float fDisData;
               if (dwValue == cdat->uiTranslateRaw[i]) {
                  fDisData = cdat->fTranslateDis[i];
               }
               else {
                  // Perform linear interpolation between points
                  DWORD dwDelta;
                  float fPercent, fDelta;
                  // check for divide by zero
                  dwDelta = cdat->uiTranslateRaw[i] - cdat->uiTranslateRaw[i-1];
                  if (dwDelta == 0)
                     dwDelta = 1;
                  fPercent = (float)(wValue-cdat->uiTranslateRaw[i-1]) / (float)dwDelta;
                  fDelta = cdat->fTranslateDis[i] - cdat->fTranslateDis[i-1];
                  fDisData = cdat->fTranslateDis[i-1] + fPercent * fDelta;
               }
               nOK = sprintf( cdat->string, format, fDisData);
               break;
            }
         }
         break;

   //************************************************
   //  special lat/long datatype
   //************************************************
      case DATATYPE_48_LATLONG:
         pwValue = (WORD*)cdat->value;
         nOK = sprintf(cdat->string,"%1d %1d %x:%02x.%02x",
                           (pwValue[0] & 0x8000) >> 15,
                           (pwValue[0] & 0x1000) >> 12,
                           (pwValue[0] & 0x0fff),
                           (pwValue[1] & 0x00ff),
                           (pwValue[2] & 0x00ff));
         break;
      
      default:
         break;

      }  // End switch ( id )

   //************************************************
   //  set status and return pointer to the string.
   //************************************************
   cdat->status=status;
   return cdat->string;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_API_GetBaseAddress
*
*  FUNCTION
*     This routine returns the base address of the board.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_API_GetBaseAddr(BT_UINT cardnum, unsigned long * baseAddress) 
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   *baseAddress = (unsigned long)boardBaseAddress[cardnum];
   return BTD_OK;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_StatusGetString
*
*  FUNCTION
*     This routine returns a pointer to an ASCII
*     text string describing the specified error.
*
****************************************************************************/

NOMANGLE char * CCONV BusTools_StatusGetString(
   BT_INT status)        // (i) API Status code
{
   static char static_buf[100];

   switch(status)
   {
   case API_SUCCESS             : return "CEI-INFO -- Successful completion of routine";
   case API_FEATURE_SUPPORT     : return "CEI-INFO -- Feature supported by board";
   case API_CONTINUE            : return "CEI-INFO -- API function should continue execution normally";
   case API_RETURN_SUCCESS      : return "CEI-INFO -- API function should return immediately with API_SUCCESS";
   case API_NEVER_CALL_AGAIN    : return "CEI-INFO -- User function is never to be called again";
   case API_BUSTOOLS_INT_USED   : return "CEI-WARN -- Board Interrupt already in use";
   case API_INIT_NO_SUPPORT     : return "CEI-WARN -- Cannot Initialize board type with this function";
   case API_NO_CHANNEL_MAP      : return "CEI_WARN -- Channel mapping not support for current card type";
   case API_NULL_PTR            : return "CEI-ERROR -- NULL Pointer passed to function";
   case API_MAX_CHANNELS_INUSE  : return "CEI-WARN -- Maximum 1553 channels already in use";
   case API_CARDNUM_INUSE       : return "CEI-ERROR -- cardnum in already in use";
   case API_BAD_PRODUCT_LIST    : return "CEI-ERROR -- Unable to build the GE Intelligent Platforms Product list";
   case API_BAD_DEVICE_ID       : return "CEI-ERROR -- Bad device ID";
   case API_INSTALL_INIT_FAIL   : return "CEI-ERROR -- CEI_INSTALL init failure";
   case API_NO_POLLING          : return "CEI-WARN -- Polling is not enabled";
   case API_TIMER_ERR           : return "CEI-ERROR -- Error setting up polling timer";
   case API_DAC_ERROR           : return "CEI-ERROR -- DAC Operation Error";
   case API_TIMEOUT_ERR         : return "CEI-ERROR -- Timeout occurred during timed delay.";      
   case API_PARAM_CONFLICT      : return "CEI-INFO -- Conflicting parameters passed to function.";
   case API_BUSTOOLS_INITED     : return "CEI-WARN -- Board has already been initialized";
   case API_BUSTOOLS_NOTINITED  : return "CEI_WARN -- BusTools API has not been initialized";
   case API_BUSTOOLS_BADCARDNUM : return "CEI-WARN -- Bad card number specified";
   case API_BUSTOOLS_BADCOUPLING: return "CEI_ERROR -- Bad coupling specified in BusTools_SetVoltage";
   case API_BUSTOOLS_BADVOLTAGE : return "CEI-ERROR -- Bad voltage specified in BusTools_SetVoltage";
   case API_BUSTOOLS_EVENBCOUNT : return "CEI-ERROR -- Even byte counts always required";
   case API_BUSTOOLS_BADMEMORY  : return "CEI-ERROR -- BusTools Board Dual-Port Memory Self-Test Failed";
   case API_BUSTOOLS_TOO_MANY   : return "CEI-WARN -- Too many user interrupt functions registered";
   case API_BUSTOOLS_FIFO_BAD   : return "CEI-ERROR -- User API_INT_FIFO structure corrupted or bad entry";
   case API_BUSTOOLS_NO_OBJECT  : return "CEI-ERROR -- Error creating event object or thread";
   case API_BUSTOOLS_NO_FILE    : return "CEI-ERROR -- Could not open the specified file";
   case API_BUSTOOLS_NO_MEMORY  : return "CEI-ERROR -- BusTools_MemoryAlloc request overflows first 64 Kw";
   case API_HW_IQPTR_ERROR      : return "CEI-CRIT-ERR -- Hardware Interrupt Pointer register error";
   case API_BIT_BC_RT_FAIL_PRI  : return "CEI-ERROR -- BIT failure/data error detected BC-RT on primary bus";
   case API_BIT_BC_RT_FAIL_SEC  : return "CEI-ERROR -- BIT failure/data error detected BC-RT on secondary bus";
   case API_BIT_BM_RT_FAIL_PRI  : return "CEI-ERROR -- BIT failure/data error detected BM-RT on primary bus";
   case API_BIT_BM_RT_FAIL_SEC  : return "CEI-ERROR -- BIT failure/data error detected BM-RT on secondary bus";
   case API_STRUCT_ALIGN        : return "CEI-ERROR -- Structure Alignment Incorrect.";
   case API_BUSTOOLS_FIFO_DUP   : return "CEI-ERROR -- Specified API_INT_FIFO structure already in use";
   case API_HARDWARE_NOSUPPORT  : return "CEI-WARN -- Function not supported by current hardware";
   case API_OUTDATED_FIRMWARE   : return "CEI-WARN -- Firmware version not supported by this software, contact factory for upgrade";
   case API_NO_OS_SUPPORT       : return "CEI-WARN -- Function not supported by underlying Operating System";
   case API_NO_BUILD_SUPPORT    : return "CEI-WARN -- Function not supported by API as built";
   case API_CHANNEL_OPEN_OTHER  : return "CEI-WARN -- Board or channel already opened as another cardnum";
   case API_SINGLE_FUNCTION_ERR : return "CEI-WARN -- Attempt to run multiple functions on single function board";
   case API_DUAL_FUNCTION_ERR   : return "CEI-WARN -- Attempt to run BC and RT functions on dual function board";
   case API_CANT_LOAD_USER_DLL  : return "CEI_ERROR -- Cannot load specified user DLL";
   case API_REGISTERFUNCTION_OFF: return "CEI_WARN -- RegisterFunction operations not enabled";
   case API_BAD_PARAM			: return "CEI-WARN -- Bad parameter for the function call";
   case API_EI_BADMSGTYPE       : return "CEI-ERROR -- Bad message type specified in LoadErrors";
   case API_EI_ILLERRORNO       : return "CEI-ERROR -- Error injection buffer number > number of buffers available";
   case API_EI_ILLERRORADDR     : return "CEI-ERROR -- Illegal error buffer address";
   case API_BAD_ADDR_TYPE       : return "CEI-WARN -- Bad address type for BusTools_GetAddr()";
   case API_BAD_DISCRETE        : return "CEI-ERROR -- Attempting to use invalid discrete";
   case API_OUTPUT_DISCRETE     : return "CEI-WARN -- Attempting to read from an output";
   case API_INPUT_DISCRETE      : return "CEI-WARN -- Attempting to write to an input";
   case API_MEM_ALLOC_ERR       : return "CEI-WARN -- Error allocating memory";
   case API_BADDATATYPE	        : return "CEI-WARN -- Bad data type for EU conversion"; 
   case API_BADBCDDATA          : return "CEI-WARN -- Bad data for BCD EU conversion";
   case API_BADTRANSLATE        : return "CEI-WARN -- Bad translation table data, for translate EU conversion";
   case API_BADFACTORTYPE       : return "CEI-WARN -- Bad factor type for scaled EU conversion";
 
   case API_CHANNEL_SHARED      : return "CEI_WARN -- Channel already shared";
   case API_CHANNEL_NOTSHARED   : return "CEI_WARN -- Channel not shared";

   case API_SINGLE_FUNCTION     : return "CEI-INFO -- Board is single-function";
   case API_DUAL_FUNCTION       : return "CEI-INFO -- Board is dual-function";
   case API_MULTI_FUNCTION      : return "CEI-INFO -- Board is multi-function";

   case API_BC_NOTINITED        : return "CEI-WARN -- BC has not been initialized";
   case API_BC_INITED           : return "CEI-WARN -- BC has already been initialized";
   case API_BC_RUNNING          : return "CEI-WARN -- BC is currently running";
   case API_BC_NOTRUNNING       : return "CEI-WARN -- BC not currently running";
   case API_BC_MEMORY_OFLOW     : return "CEI-ERROR -- BC memory overflow";
   case API_BC_ILLEGAL_MBLOCK   : return "CEI-ERROR -- BC illegal memory block number specified";
   case API_BC_MBLOCK_NOMATCH   : return "CEI-ERROR -- BC specified addr is not a BC message block";
   case API_BC_MBUF_NOT_ALLOC   : return "CEI-WARN -- BC message buffers have not been allocated";
   case API_BC_MBUF_ALLOCD      : return "CEI-WARN -- BC the number of messages cannot be increased after allocation";
   case API_BC_ILLEGAL_NEXT     : return "CEI-WARN -- BC illegal next message number specified";
   case API_BC_ILLEGAL_BRANCH   : return "CEI-WARN -- BC illegal branch message number specified";
   case API_BC_MESS1_COND       : return "CEI-WARN -- BC first message in buffer is conditional";
   case API_BC_BAD_COND_ADDR    : return "CEI-WARN -- BC bad address value in conditional message";
   case API_BC_BADTIMEOUT1      : return "CEI-WARN -- BC illegal \"No Response\" timeout";
   case API_BC_BADTIMEOUT2      : return "CEI-WARN -- BC illegal \"Late Response\" timeout";
   case API_BC_BADFREQUENCY     : return "CEI-WARN -- BC illegal minor frame frequency";
   case API_BC_HALTERROR        : return "CEI-ERROR -- BC error detected during stop, bus is probably unterminated";
   case API_BC_BOTHBUFFERS      : return "CEI-WARN -- BC message cannot specify both buffers";
   case API_BC_BOTHBUSES        : return "CEI-WARN -- BC message cannot specify both buses";
   case API_BC_UPDATEMESSTYPE   : return "CEI-WARN -- BC MessageUpdate cannot operate on branch or conditional messages";
   case API_BC_ILLEGALMESSAGE   : return "CEI-ERROR -- BC message in memory is not legal";
   case API_BC_ILLEGALTARGET    : return "CEI-ERROR -- BC branch data message number not legal";
   case API_BC_NOTMESSAGE       : return "CEI-ERROR -- BC msg is not a proper 1553-type message";
   case API_BC_NOTNOOP          : return "CEI-ERROR -- BC msg is not a proper nop-type message";
   case API_BC_APERIODIC_RUNNING: return "CEI-WARN -- BC Aperiodics still running, cannot start new msg list";
   case API_BC_APERIODIC_TIMEOUT: return "CEI-ERROR -- BC Aperiodic messages did not complete in time";
   case API_BC_CANT_NOOP        : return "CEI-WARN -- BC cannot noop or un-noop a noop message";
   case API_BC_READ_TIMEOUT     : return "CEI-INFO -- BC Timeout on buffer read";
   case API_BC_READ_NODATA      : return "CEI-INFO -- No BC data in int queue";
   case API_BC_AUTOINC_INUSE    : return "CEI-WARN -- Auto-Increment in use for message";
   case API_BC_IS_RUNNING       : return "CEI-INFO -- BC is running";
   case API_BC_IS_STOPPED       : return "CEI-INFO -- BC is stopped";
   case API_BM_NOTINITED        : return "CEI-WARN -- BM Init or MessageAlloc has not been called";
   case API_BM_INITED           : return "CEI-WARN -- BM has already been initialized";
   case API_BM_RUNNING          : return "CEI-WARN -- BM is currently running";
   case API_BM_NOTRUNNING       : return "CEI-WARN -- BM not currently running";
   case API_BM_MEMORY_OFLOW     : return "CEI-ERROR -- BM memory overflow";
   case API_BM_ILLEGAL_ADDR     : return "CEI_WARN -- BM illegal RT address specified";
   case API_BM_ILLEGAL_SUBADDR  : return "CEI-WARN -- BM illegal RT sub-address specified";
   case API_BM_ILLEGAL_TRANREC  : return "CEI-WARN -- BM illegal RT transmit/receive flag specified";
   case API_BM_ILLEGAL_MBUFID   : return "CEI-WARN -- BM illegal message buffer number specified";
   case API_BM_MBUF_NOMATCH     : return "CEI-ERROR -- BM specified message number not allocated";
   case API_BM_WRAP_AROUND      : return "CEI-ERROR -- BM API message buffer has overflowed, data has been lost";
   case API_BM_MSG_ALLOC_CALLED : return "CEI-WARN -- BM_MessageAlloc has already been called";
   case API_BM_HW_WRAP_AROUND   : return "CEI-ERROR -- BM HW message buffer has overflowed, data has been lost";
   case API_BM_POINTER_REG_BAD  : return "CEI-ERROR -- BM HW pointer register contents invalid";
   case API_BM_READ_TIMEOUT     : return "CEI-INFO -- BM Timeout on buffer read";
   case API_BM_READ_NODATA      : return "CEI-INFO -- No BM data in int queue";
   case API_BM_1760_ERROR       : return "CEI-ERROR -- Checksum error on MIL-STD-1760 message.";
   case API_RT_NOTINITED        : return "CEI-WARN -- RT has not been initialized";
   case API_RT_INITED           : return "CEI-WARN -- RT has already been initialized";
   case API_RT_RUNNING          : return "CEI-WARN -- RT is currently running";
   case API_RT_NOTRUNNING       : return "CEI-WARN -- RT not currently running";
   case API_RT_MEMORY_OFLOW     : return "CEI-WARN -- RT memory overflow";
   case API_RT_CBUF_EXISTS      : return "CEI-WARN -- RT subunit MBUFs already allocated";
   case API_RT_ILLEGAL_ADDR     : return "CEI-WARN -- RT illegal RT address specified";
   case API_RT_ILLEGAL_SUBADDR  : return "CEI-WARN -- RT illegal RT sub-address specified";
   case API_RT_ILLEGAL_TRANREC  : return "CEI-WARN -- RT illegal RT transmit/receive flag specified";
   case API_RT_ILLEGAL_MBUFID   : return "CEI-WARN -- RT specified message buffer ID is larger than the number of buffers created";
   case API_RT_CBUF_BROAD       : return "CEI-WARN -- RT address 31 is broadcast only";
   case API_RT_CBUF_NOTBROAD    : return "CEI-WARN -- RT specified RT address is not broadcast";
   case API_RT_MBUF_NOMATCH     : return "CEI-ERROR -- RT message buffer not found at specified address";
   case API_RT_BROADCAST_DISABLE: return "CEI-WARN -- RT 31 Broadcast is disabled";
   case API_RT_READ_TIMEOUT     : return "CEI-INFO -- RT Timeout on buffer read";
   case API_RT_READ_NODATA      : return "CEI-INFO -- No RT data in int queue";
   case API_RT_AUTOINC_INUSE    : return "CEI-WARN -- Auto-Increment already in use for message";
   case API_NO_HARDWIRE_RT      : return "CEI-INFO -- RT hardwired address not enabled.";
   case API_LV_BADARRAY         : return "CEI-ERROR -- LabView array/structure size not correctly setup";
   case API_NO_LV_SUPPORT       : return "CEI-WARN -- Function not supported in LabView";
   case API_PLAYBACK_INIT_ERROR : return "CEI-ERROR -- Error initializing Playback";
   case API_PLAYBACK_BAD_THREAD : return "CEI-ERROR -- Attempt to create Playback thread failed";
   case API_PLAYBACK_BAD_FILE   : return "CEI-ERROR -- Playback file open failed";
   case API_PLAYBACK_BAD_EVENT  : return "CEI-ERROR -- Playback event creation error";
   case API_PLAYBACK_BUF_EMPTY  : return "CEI-ERROR -- Playback Buffer empty";
   case API_PLAYBACK_BAD_EXIT   : return "CEI-ERROR -- Unexpected Exit from Playback";
   case API_PLAYBACK_BAD_MEMORY : return "CEI-ERROR -- Unable to allocate Playback memory on Host";
   case API_PLAYBACK_DISK_READ  : return "CEI-ERROR -- Disk read Error during Playback";
   case API_PLAYBACK_RUNNING    : return "CEI-WARN -- Playback is already running";
   case API_PLAYBACK_BAD_ALLOC  : return "CEI-ERROR -- Failure to allocate enough BusTools Memory for Playback";
   case API_PLAYBACK_TIME_GAP   : return "CEI-INFO -- larger gaps in time tags in playback file.";
   case API_PLAYBACK_TIME_ORDER : return "CEI-ERROR -- Time tags in playback file out of sequence.";
 
   case API_TIMETAG_BAD_DISPLAY : return "CEI-WARN -- Unknown or unsupported Time Tag display format";
   case API_TIMETAG_BAD_INIT    : return "CEI-WARN -- Unknown Time Tag Initialization method";
   case API_TIMETAG_BAD_MODE    : return "CEI-WARN -- Unknown Time Tag Operating Mode";
   case API_TIMETAG_NO_DLL      : return "CEI-ERROR -- DLL containing BusTools_TimeTagSet() could not be loaded";
   case API_TIMETAG_NO_FUNCTION : return "CEI-ERROR -- Could not get the address of the BusTools_TimeTagSet() function";
   case API_TIMETAG_USER_ERROR  : return "CEI-ERROR -- User function BusTools_TimeTagSet() returned an error";
   case API_TIMETAG_WRITE_ERROR : return "CEI-ERROR -- Cannot write to time tag load register when in API_TM_IRIG mode";
   case API_IRIG_NO_SIGNAL      : return "CEI-INFO -- No IRIG signal present";

   case BTD_ERR_PARAM           : return "CEI-ERROR -- Driver-invalid parameter: type, slot, addr, etc.";
   case BTD_ERR_NOACCESS        : return "CEI-ERROR -- Driver-unable to map/access adapter";
   case BTD_ERR_INUSE           : return "CEI-ERROR -- Driver-adapter already in use";
   case BTD_ERR_BADADDR         : return "CEI-ERROR -- Driver-invalid address";
   case BTD_ERR_NODETECT        : return "CEI-ERROR -- Board detect failure (IO or Config register ID invalid)";
   case BTD_ERR_NOTSETUP        : return "CEI-ERROR -- Driver-adapter has not been setup";
   case BTD_ERR_FPGALOAD        : return "CEI-ERROR -- Driver-FPGA load failure";

   case BTD_ERR_NOMEMORY        : return "CEI-ERROR -- Driver-error allocating host memory for BM buffers";
   case BTD_ERR_BADADDRMAP      : return "CEI-ERROR -- Driver-bad initial mapping of address";
   case BTD_ERR_BADEXTMEM       : return "CEI-ERROR -- Driver-bad extended memory mapping";
   case BTD_ERR_BADBOARDTYPE    : return "CEI-ERROR -- Driver-unknown board type";
   case BTD_ERR_BADWCS          : return "CEI-ERROR -- Driver-read back failure of Writeable Control Store";
   case BTD_NO_PLATFORM         : return "CEI-ERROR -- Platform specified unknown or not supported";
   case BTD_BAD_MANUFACTURER    : return "CEI-ERROR -- IP ID PROM Manufacturer code not 0x79";
   case BTD_BAD_MODEL           : return "CEI-ERROR -- IP ID PROM Model number not 0x05 or 0x08";
   case BTD_BAD_SERIAL_PROM     : return "CEI-ERROR -- IP Serial PROM needs update, contact GE Intelligent Platforms";
   case BTD_CHAN_NOT_PRESENT    : return "CEI-ERROR -- Channel not present (on multi-channel board)";
   case BTD_NON_SUPPORT         : return "CEI-ERROR -- Bus/Carrier/OS combination not supported by API";
   case BTD_BAD_HW_INTERRUPT    : return "CEI-ERROR -- Hardware interrupt number bad or not defined in registry";
   case BTD_FPGA_NOT_CLEAR      : return "CEI-ERROR -- The FPGA configuration failed to clear";

   case BTD_ERR_NOWINRT         : return "CEI-ERROR -- WinRT driver not loaded/started.  Device Conflict or board not installed?";
   case BTD_ERR_BADREGISTER     : return "CEI-ERROR -- WinRT parameters don't match registry";
   case BTD_ERR_BADOPEN         : return "CEI-ERROR -- WinRT device open/map failed";
   case BTD_UNKNOWN_BUS         : return "CEI-ERROR -- Bus is not PCI, ISA or VME";

   case BTD_BAD_INT_EVENT       : return "CEI-ERROR -- Unable to create interrupt event";
   case BTD_ISR_SETUP_ERROR     : return "CEI-ERROR -- Error setting up the Interrupt Service Routine";
   case BTD_CREATE_ISR_THREAD   : return "CEI-ERROR -- Error creating the Interrupt Service Routine thread";
   case BTD_NO_REGIONS_TO_MAP   : return "CEI-ERROR -- No regions requested in call to vbtMapBoardAddresses";
   case BTD_NEW_PCCARD_FW       : return "CEI-ERROR -- PCC-1553 firmware is too new to be used with this version of the API";
   case BTD_OLD_PCCARD_FW       : return "CEI-ERROR -- PCC-1553 firmware is too old for this version of the API, use the JAM Player to update it";
   case BTD_BAD_CONF_FILE       : return "CEI-ERROR -- Unable to open ceidev.conf (UNIX Only)";
   case BTD_NO_DRV_MOD          : return "CEI-ERROR -- No Driver Module found (UNIX Only)";
   case BTD_IOCTL_DEV_ERR       : return "CEI-ERROR -- Error in ioctl get device (UNIX Only)";
   case BTD_IOCTL_SET_REG       : return "CEI-ERROR -- Error in ioctl set region (UNIX Only)";
   case BTD_IOCTL_REG_SIZE      : return "CEI-ERROR -- Error in getting ioclt region size (UNIX Only)";
   case BTD_IOCTL_GET_REG       : return "CEI-ERROR -- Error in ioctl get region (UNIX Only)";
   case BTD_BAD_SIZE            : return "CEI-ERROR -- Region size is 0 (UNIX Only)";
   case BTD_BAD_PROC_ID         : return "CEI-ERROR -- Unable to set process ID (UNIX Only)";
   case BTD_HASH_ERR            : return "CEI-ERROR -- Unable to setup hash table (UNIX Only)";
   case BTD_NO_HASH_ENTRY       : return "CEI-ERROR -- No hash table entry found (UNIX Only)";
   case BTD_WRONG_BOARD         : return "CEI-ERROR -- Wrong Board Type for this command";
   case BTD_MODE_MISMATCH       : return "CEI-ERROR -- Mode Mis-Match on IP-D1553";
   case BTD_IRIG_NO_LOW_PEAK    : return "CEI-INFO -- No lower peak on IRIG DAC calibration";
   case BTD_IRIG_NO_HIGH_PEAK   : return "CEI-INFO -- No upper peak on IRIG DAC calibration";
   case BTD_IRIG_LEVEL_ERR      : return "CEI-INFO -- IRIG-B Signal level too low";
   case BTD_IRIG_NO_SIGNAL      : return "CEI-INFO -- No IRIG Signal Detected";
   case BTD_RTADDR_PARITY       : return "CEI-INFO -- Parity Error on Hardwired RT address lines";

   case BTD_RESOURCE_ERR        : return "CEI-ERROR -- Integrity Resource Error";
   case BTD_READ_IODEV_ERR      : return "CEI-ERROR -- Integrity IO Device Read Error";
   case BTD_MEMREG_ERR          : return "CEI-ERROR -- Integrity error getting memory region";
   case BTD_MEM_MAP_ERR         : return "CEI-ERROR -- Integrity Memory Mapping error";
   case BTD_CLK_RATE_NOT_SET    : return "CEI-ERROR -- Error setting clk rate, timer not accurate";

   case BTD_VIOPEN_FAIL         : return "CEI-ERROR -- viOpen Error";
   case BTD_VIMAPADDRESS_FAIL   : return "CEI-ERROR -- viMapAddress Error";
   case BTD_VIOPENDEFAULTRM     : return "CEI-ERROR -- viOpenDefaultRM Error";
   case BTD_VIUNMAP_ERR         : return "CEI-ERROR -- viUnMapAddress Error";

   case BTD_SEM_CREATE          : return "CEI-ERROR -- Error Creating semaphore";
   case BTD_TASK_CREATE         : return "CEI-ERROR -- Error spawning task";

   case BTD_EVENT_WAIT_FAILED   : return "CEI-ERROR -- Event Wait Failure";
   case BTD_EVENT_WAIT_ABANDONED: return "CEI-ERROR -- Event Wait Abandoned";
   case BTD_EVENT_WAIT_TIMEOUT  : return "CEI-INFO -- Timeout on Event Wait";
   case BTD_EVENT_WAIT_UNKNOWN  : return "CEI-ERROR -- Unknown Event Error";
   case BTD_EVENT_SIGNAL_ERR    : return "CEI-ERROR -- Error Occurred During Event Signal";
   case BTD_SET_PRIORITY_ERR    : return "CEI-ERROR -- Error Setting Thread Priority";
   case BTD_THRD_CREATE_FAIL    : return "CEI-ERROR -- Thread Create Failure";

   case BTD_CLOSE_ERR           : return "CEI-ERROR -- Failed to close 1553 device";
   case BTD_OPEN_ERR            : return "CEI-ERROR -- Failed to open 1553 device";
   case BTD_VBT_OPEN_ERR        : return "CEI-ERROR -- Failure in vbtOpen1553Channel";
   case BTD_FIND_DEV_ERR        : return "CEI_ERROR -- Failure in BusTools_FindDevice";
   case BTD_LIST_DEV_ERR        : return "CEI_ERROR -- Failure in BusTools_ListDevices";
   case API_INSTALL_ERROR       : return install_error_string;

   }
   sprintf(static_buf,"Unknown error code: %d (0x%X).",status,status);
   return static_buf;
}
//
/****************************************************************************
*
*  PROCEDURE NAME - api_reset
*
*  FUNCTION
*     This routine resets the hardware and restores the most recent values
*     to the hardware registers.  It is called from BusTools_BM_Init().
*
****************************************************************************/

void api_reset(BT_UINT cardnum)            // (i) card number
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   int i;

   /*******************************************************************
   *  Clear any bits in the local copy of the hardware registers that
   *   we don't want to see when things are started up again,
   *   specifically the Run and BC Busy bits.
   *******************************************************************/
   hwreg_value[cardnum][HWREG_CONTROL1] &=
                   (BT_U16BIT)~(CR1_BCRUN | CR1_RTRUN | CR1_BMRUN | CR1_BCBUSY);
   
   /*******************************************************************
   *  Rewrite the registers back to the hardware.
   *******************************************************************/
   for ( i = 0; i < HWREG_COUNT; i++ )
   {
      vbtSetHWRegister(cardnum, i, hwreg_value[cardnum][i]);
   }

   return;
}

/****************************************************************************
*
*  PROCEDURE NAME -  api_writehwreg
*                    api_writehwreg_and
*                    api_writehwreg_or
*                    api_readhwreg
*
*  FUNCTION
*     These procedures write the specified word into the specified hardware
*     register.  The software's copy of the hardware registers is also updated.
*
*     The "_and" routine performs a logical AND of the supplied value with that
*     located in the software copy of the register and sends that value to the
*     hardware.
*
*     The "_or" routine performs a logical OR of the supplied value with that
*     located in the software copy of the register and sends that value to the
*     hardware.
*
*     NOTE:  For the "_and" and "_or" routines to function properly, these
*     routines must be the EXCLUSIVE access of the API to the hw registers.
*     If a direct memory write is used, the software's copy of the registers
*     will not be accurate and the "_and" and "_or" functions will write
*     the wrong values.
*
*  RETURNS
*     0 -> success
*
****************************************************************************/

void api_writehwreg_or(
   BT_UINT cardnum,        // (i) card number
   BT_UINT regnum,         // (i) Hardware register number (WORD offset)
   BT_U16BIT value)        // (i) Value to "OR" into register
{
   BT_UINT newvalue;

   switch (regnum)
   {
      case HWREG_CONTROL1:        // 0x00 control register #1    (R/W)
         newvalue = vbtGetHWRegister(cardnum, regnum) | value;
         break;
      default:
         newvalue = hwreg_value[cardnum][regnum] | value;
         break;
   }
   api_writehwreg(cardnum, regnum, (BT_U16BIT)(newvalue));
}

void api_writehwreg_and(
   BT_UINT cardnum,        // (i) card number
   BT_UINT regnum,         // (i) Hardware register number (WORD offset)
   BT_U16BIT value)        // (i) Value to "AND" into register
{
   BT_INT newvalue;

   switch (regnum)
   {
      case HWREG_CONTROL1:        // 0x00 control register #1    (R/W)
         newvalue = vbtGetHWRegister(cardnum, regnum) & value;
         break;
      default:
         newvalue = hwreg_value[cardnum][regnum] & value;
         break;
   }
   api_writehwreg(cardnum, regnum, (BT_U16BIT)newvalue);
}

void api_writehwreg(
   BT_UINT cardnum,        // (i) card number
   BT_UINT regnum,         // (i) Hardware register number (WORD offset)
   BT_U16BIT value)        // (i) Value to write into register
{
   /*******************************************************************
   *  Store value in software's copy of registers
   *******************************************************************/
   hwreg_value[cardnum][regnum] = value;

   /*******************************************************************
   *  Calculate output address and output value to that location.
   *  Note that the hardware regs are located in the first page!
   *******************************************************************/
   vbtSetHWRegister(cardnum, regnum, value);
}

BT_U16BIT api_readhwreg(
   BT_UINT cardnum,        // (i) card number
   BT_UINT regnum)         // (i) Hardware register number (WORD offset)
{
   /*******************************************************************
   *  Calculate input address and get value from that location.
   *  Note that the hardware regs are located in the first page!
   *******************************************************************/
   return (hwreg_value[cardnum][regnum] = vbtGetHWRegister(cardnum, regnum));
}


