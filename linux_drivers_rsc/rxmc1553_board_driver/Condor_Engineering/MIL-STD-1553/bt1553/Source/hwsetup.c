 /*============================================================================*
 * FILE:                        H W S E T U P . C
 *============================================================================*
 *
 * COPYRIGHT (C) 1994 - 2012 BY
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
 *             Low-level board setup component.
 *
 *             This module performs the setup of the address pointers and the
 *             loading of the WCS and FPGA configurations.
 *
 * DESCRIPTION:  See vbtSetup() for a description of the functions performed
 *               by this module.
 *
 * BTDRV ENTRY POINTS: (only used by btdrv.c):
 *    vbtPageAccessSetupVME   Setup adapter page access data pointers for the
 *                            VME-1553 on VME bus.
 *    vbtPageAccessSetupQPMC  Setup adapter page access data pointers for the
 *                            QPMC-1553 and QPM-1553 on the PCI bus, and the
 *                            R15-AMC and RPCIe-1553 on the PCI express bus.
 *    vbtPageAccessSetupQPCX  Setup adapter page access data pointers for the
 *                            QPCX-1553 on the PCI bus.
 *    vbtPageAccessSetupQCP   Setup adapter page access data pointers for the
 *                            QCP-1553 on the PCI bus.
 *    vbtPageAccessSetupQ104  Setup adapter page access data pointers for the
 *                            Q104-1553 on the ISA bus and Q104-1553P on the
 *                            PCI bus.
 *    vbtPageAccessSetupPCCD  Setup adapter page access data pointers for the
 *                            PCC-D1553 on the PCMCIA bus.
 *    vbtPageAccessSetupR15EC Setup adapter page access data pointers for the
 *                            R15-EC on the PCI express bus.
 *    vbtPageAccessSetupRXMC  Setup adapter page access data pointers for the
 *                            RXMC-1553 on the PCI express bus.
 *    vbtPageAccessSetupRXMC2 Setup adapter page access data pointers for the
 *                            RXMC2-1553 on the PCI express bus.
 *    vbtPageAccessSetupLPCIE Setup adapter page access data pointers for the
 *                            LPCIe-1553 on the PCI express bus.
 *    vbtPageAccessSetupRAR15 Setup adapter page access data pointers for the
 *                            RAR15-XMC on the PCI express bus.
 *    vbtPageAccessSetupRAR15XT Setup adapter page access data pointers for the
 *                            RAR15-XMC on the PCI express bus.
 *    vbtPageAccessSetupUSB   Setup device page access data pointers for the
 *                            USB-1553 on the USB bus.
 *
 * LOCAL FUNCTIONS:
 *    vbtLoad_vmefpga      Loads the FPGA/WCS configuration data into the VME-1553 board
 *    vbtIRIGCal           Calibrates external IRIG signal.
 *
 *===========================================================================*/

/* $Revision:  8.06 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  07/23/1999   Split functions from btdrv.c.V3.20.ajh
  10/12/1999   Changed WCS load sequence for the PCI-1553.V3.22.ajh
  11/15/1999   Changed WCS and FPGA load sequence for the PCI-1553.V3.30.ajh
  12/05/1999   Incorporated V3.01 WCS and FPGA for the PCI-1553.V3.30.ajh
  12/12/1999   Incorporated V3.02 WCS and FPGA for the PCI-1553.V3.30.ajh
  01/14/2000   Merged IPSETUP.C into this file to support common WCS load for
               the IP/PCI/PMC/ISA-1553 boards.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  02/21/2000   Changed code to load WCS V3.03.V4.01.ajh
  03/16/2000   Changed to load LPU Version 3.03.V4.01.ajh
  04/05/2000   Changed to load LPY V3.04 and WCS V3.06, changed vbtLoad_1553fpga()
               to write 16-bit values to the FPGA data port.V4.01.ajh
  02/05/2000   Changed timing for fpga loading.V4.02.ajh
  06/05/2000   Changed IP-1553 to load 2.50B WCS.V4.05.ajh
  07/18/2000   Detect and report FPGA configuration failed to clear.V4.06.ajh
  08/09/2000   Support the new WCS/LPU firmware for the ISA-/PCI-1553.V4.09.ajh
  08/19/2000   Modify vbtLoadPagedWCS to correctly handle WCS loads that exceed
               32Kbytes in length for 16-bit programs.V4.11.ajh
  09/05/2000   Modify ISA board detection logic to ID the ISA-1553.V4.13.ajh
  11/03/2000   Load WCS version 3.06 to IP-1553 V5 PROM, even if newer FW is
               included.V4.20.ajh
  12/04/2000   Mask off the high bits of the IP ID PROM values.V4.25.ajh
  12/11/2000   Modify vbtLoadPCIWCS() to correctly the address in WCS where a
               miscompare is detected.V4.27.ajh
  12/12/2000   Moved PC1553 version initialization code here from INIT.V4.28.ajh
  03/27/2001   Added V3.11 WCS to support IP-1553 V6 PROMs.V4.31.ajh
  04/11/2001   Updated to latest V3.11 WCS for the IP-1553 V6 PROM.V4.37.ajh
  04/18/2001   Loaded the new WCS/LPU V3.20 firmware for non-IP-1553 products.
               This supports the trigger on data BM function.V4.38.ajh
  04/26/2001   Loaded the new WCS/LPU V3.21 firmware for non-IP-1553 products.
               This load supports counted conditional and fixed the BM enable
               problem.V4.39.ajh
  07/26/2001   Corrected problem reading firmware version from PCCARD.V4.39.ajh
  01/07/2002   Added supprt for Quad-PMC and Dual Channel IP V4.46 rhc 
  03/15/2002   Added IRIG Support. v4.48.rhc
  04/22/2002   Remove support for IP-1553 PROM 1-4
  06/05/2002   Add the latest v3.40 firmware. v4.52
  07/10/2002   Add suppot for IP-D1553 for all carriers v4.54
  09/12/2002   Add new ceWCS350.h and lpu_350.h files beta v4.55
  02/26/2002   Add QPCI-1553 support.
  10/22/2003   Add ceWCS384 and LPU384.
  10/22/2003   Add LPU version read from register.
  02/19/2004   PCCard-D1553 Support
  03/01/2004   Update to the lpu 3.86 ofr PCI-1553
  08/02/2004   Add support for the QCP-1553.
  03/01/2005   Add new VME and PCCD firmware
  08/30/2006   Add AMC-1553 Support
  10/04/2006   fix error in IRIG logic for QPM/QPMC-1553.
  11/19/2007   Set board_has_plx_dma[cardnum] for QPCX-1553, QCP-1553.
  11/19/2007   Add function vbtPageAccessSetupR15EC.
  05/12/2008   Add support for the R15-AMC
  11/21/2008   Add IO_SYNC in vbtPageAccessSetupVME
  06/29/2009   Add support for the RXMC-1553
  12/03/2010   Move vbtSetDicrete and vbtGetDiscrete from hwsetup.c to btdrv.c
  04/11/2011   Add support for LPCIe-1553 and RXMC2-1553
  05/11/2012   Major change to combine V6 F/W and V5 F/W into single API  
*/

/* ---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"

#define IRIG_FLAG 0x1000
#define HIF_MASK 0x0fff
#define BOARD_MASK 0x3e
#define ADDITIONAL_CAPABILITY 0x8000 

#ifdef INCLUDE_PCCD
static unsigned char PCCD_FpgaData[] = {  /* Define the array and its */
#include "lpu_pccd_440.h"                 /*  contents which initializes  */
                                           }; /*   the PCI/ISA FPGA firmware. */
#endif //INCLUDE_PCCD

/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t G e t R T a d d r 
 *===========================================================================*
 *
 * FUNCTION:    Get the Hardwired RT address for the channel.
 *
 * DESCRIPTION: Special routine for reading the Writable Control Store on
 *              boards with a paged WCS.	 
 *
 *      It will return:  Hardwired RT address or -1 if not programmed
 *
 *===========================================================================*/
BT_UINT vbtGetRTaddr(BT_UINT cardnum, BT_UINT channel)
{

   if((_HW_FPGARev[cardnum] & 0xfff) >= 0x499 && (_HW_FPGARev[cardnum] & 0xfff) < 0x600)
   {
      BT_U16BIT rdata,adata;
  
      BT_INT rt_valid_mask[] = {0x3,0x18, 0xc0,0x600};
      BT_INT rt_addr_addr[] = {DISREG_RTADDR_RD1,DISREG_RTADDR_RD1,DISREG_RTADDR_RD2,DISREG_RTADDR_RD2};
      BT_INT rt_addr_mask[] = {0x1f,0x1f00,0x1f,0x1f00};
      BT_INT rt_addr_shft[] = {0,8,0,8};

      board_has_hwrtaddr[cardnum] =0x1;   

      rdata = (BT_U16BIT)vbtGetDiscrete[cardnum](cardnum,DISREG_HW_RTADDR); 
      if((rdata & rt_valid_mask[channel]) == 0x0)
         return((BT_UINT)-1);

      if((rdata>>(3*channel) & 0x3) == 0x1)
      {
         adata = (BT_U16BIT)vbtGetDiscrete[cardnum](cardnum,rt_addr_addr[channel]);
         adata = (adata & rt_addr_mask[channel])>>rt_addr_shft[channel];
      }
      else
      {  
         adata = 0xffff;
      }
      return adata;
   }
   else if((_HW_FPGARev[cardnum] & 0xfff) >= 0x600)
   {
      BT_U32BIT rtaddr;
      rtaddr = vbtGetRegister[cardnum](cardnum,HWREG_RT_HW_ADDR);

      if(rtaddr & V6_HW_RT_ADDR_VALID)
         hwRTAddr[cardnum] = rtaddr & V6_RT_ADDR_MASK;
      else
         hwRTAddr[cardnum] = -1;

      board_has_hwrtaddr[cardnum] = 0x1;
      return API_SUCCESS;   
   }
   else
   {
      BT_U32BIT rdata;

      switch(CurrentCardType[cardnum])
      {
         case QPM1553:
         case QCP1553:
         case RPCIe1553:
         case RXMC1553:
            if(CurrentCardSlot[cardnum] >= CHANNEL_2)
            {
               board_has_hwrtaddr[cardnum] = 0x0;
               hwRTAddr[cardnum] = -1;
               return API_SUCCESS;
            }
            break;
         case QPCI1553:
         case QPCX1553:
         case Q1041553:
         case Q1041553P:
               board_has_hwrtaddr[cardnum] = 0x0;
               hwRTAddr[cardnum] = -1;
               return API_SUCCESS;
      }
              
      board_has_hwrtaddr[cardnum] = 0x1;

      rdata = vbtGetDiscrete[cardnum](cardnum,DISREG_HW_RTADDR); 

      if(CurrentCardSlot[cardnum] == 0)
      {
         if((rdata & 0x3) == 0x0)
            hwRTAddr[cardnum] = -1;
         else if((rdata & 0x3) == 0x1)
         {
            hwRTAddr[cardnum] = (vbtGetDiscrete[cardnum](cardnum,DISREG_RTADDR_RD1))&0x1f;
         }
         else
         {  
            hwRTAddr[cardnum] = -1;
         }
      }
      else if(CurrentCardSlot[cardnum] == 1)
      {   
         if((rdata & 0x18) == 0x0)
            hwRTAddr[cardnum] = -1;
         else if((rdata & 0x18) == 0x8)
            hwRTAddr[cardnum] = ((vbtGetDiscrete[cardnum](cardnum,DISREG_RTADDR_RD1)) & 0x1f00)>>8;
         else
         {
            hwRTAddr[cardnum] = -1;
         }
      }
      else if(CurrentCardSlot[cardnum] == 2)
      {   
         if((rdata & 0xc0) == 0x0)
            hwRTAddr[cardnum] = -2;
         else if((rdata & 0xc0) == 0x40)
            hwRTAddr[cardnum] = ((vbtGetDiscrete[cardnum](cardnum,DISREG_RTADDR_RD2)) & 0x1f);
         else
         {
            return (BT_UINT)-1;
         }
      }
      else if(CurrentCardSlot[cardnum] == 3)
      {   
         if((rdata & 0x600) == 0x0)
            hwRTAddr[cardnum] = -1;
         else if((rdata & 0x600) == 0x200)
            hwRTAddr[cardnum] = ((vbtGetDiscrete[cardnum](cardnum,DISREG_RTADDR_RD2)) & 0x1f00)>>8;
         else
         {
            hwRTAddr[cardnum] = -1;
         }
      }
   }
   return API_SUCCESS;
}

#ifdef INCLUDE_PCCD
/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t L o a d _ p c c d _ f p g a
 *===========================================================================*
 *
 * FUNCTION:     This function loads the Altera Flex 10K part on the VME-1553,
 *               with the FPGA config data.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
static int vbtLoad_pccd_fpga(
   volatile short *addr,   // (i) address of host interface registers
   unsigned char *PCCD_FpgaData,
   unsigned nbytes) // (i) pointer to FPGA data array
{
   unsigned char *laddr;         // load address;
   unsigned     i;               // Loop counters.
   int status;

   laddr = (char *)addr;
   /***********************************************************************
   *  Power up the 1.5V supply for FPGA.
   ***********************************************************************/
   addr[0] |= 0x8000;  // Power up 1.5 V
   addr[1] = 0x0;      // Config Mode low for 2 usec
   MSDELAY(1);                 // This seems like overkill
   addr[1] = 0x1;      // Now ready to configure
   MSDELAY(1);                 // This seems like overkill

   for ( i = 0; i < nbytes; i++ )    // Load the first FPGA, then the second
   {
      laddr[4] = PCCD_FpgaData[i];
   }
 
   MSDELAY(5);
   // Check CSC Register for successful 10K configuration load.
   if ( (addr[1] & 0x4) != 0x4 )
      status = BTD_ERR_FPGALOAD;    // Driver-FPGA load failure
   else
	  status = BTD_OK;

   return status;
}
#endif //INCLUDE_PCCD


/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p V M E
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the specified VME-1553
 *              native VME bus board.  This board is flat-mapped (no frames)
 *              and supports up to four MIL-STD-1553 interfaces on a single 
 *              board.  Load the FPGA if possible, then load the WCS for the
 *              specified current channel.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
#ifdef __BORLANDC__
#pragma argsused
#endif
BT_INT vbtPageAccessSetupVME(
   BT_UINT   cardnum,          // (i) card number (0 based)
   unsigned mem_addr,          // memory base address
   char     *lpbase)           // (i) board base address in user space
{

#define BOARD_RESET 0xd
#define BOARD_SET 0x800c

   int mode[] = {0x8150,  //0000 s
                 0x815c,  //1100 m
                 0x8151,  //0001 ss
                 0x815d,  //1101 mm
                 0x8153,  //0011 ssss
                 0x815f}; //1111 mmmm

   BT_INT       i;
   BT_INT       found = 0;
   unsigned short *vme_config_addr; // word pointer to VME/VXI config registers
   unsigned short vme_config; //


   /**********************************************************************
   *  Setup the VME address of the boards dual-port, Hardware Registers
   *  and RAM Registers.  
   **********************************************************************/
   vme_config_addr = (unsigned short *)bt_iobase[cardnum];
   vme_config = vme_config_addr[1];

   if(vme_config & 0x200)
      board_has_irig[cardnum] = 0x1;
   else
      board_has_irig[cardnum] = 0x0;

   vme_config &= ~0x200;

   for( i=0; i<6; i++)
   {
      if((vme_config &0xffff) == mode[i])
      {
         if(vme_config & 0x000c)
            _HW_1Function[cardnum] = MULTI_FUNCTION;
         else
            _HW_1Function[cardnum] = SINGLE_FUNCTION;
         found = 1;
      }
   }
   if(found == 0)
   {
      return BTD_ERR_BADBOARDTYPE;
   }

   if(CurrentCardSlot[cardnum] > MAX_CHANNEL_COUNT)
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_VME + DATA_RAM_VME; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_VME + HW_REG_VME;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_VME + REG_FILE_VME; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_VME;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_VME + DATA_RAM_VME; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_VME + HW_REG_VME;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_VME + REG_FILE_VME; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_VME;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN3_VME + DATA_RAM_VME; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN3_VME + HW_REG_VME;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN3_VME + REG_FILE_VME; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_VME;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_4 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN4_VME + DATA_RAM_VME; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN4_VME + HW_REG_VME;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN4_VME + REG_FILE_VME; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_VME;     // Host Interface Registers
   }

   /****************************************************************
   *  Load the FPGA configuration from VME_FpgaData.
   ****************************************************************/
   //set the A24/A32 config bit when defined.
   if(CurrentCarrier[cardnum] == NATIVE_32     || 
      CurrentCarrier[cardnum] == NI_A32_MAP_16 ||
      CurrentCarrier[cardnum] == NI_A32_MAP_32 )
   {
      vme_config_addr[4] |= 0x88; // Select A32 0x88
      MSDELAY(1);
      if(mem_addr & 0x7fffff)
         return BTD_ERR_BADADDRMAP;
      vme_config_addr[3] = (short)((mem_addr & 0xff800000)>>16);
   }
   else
   {
      vme_config_addr[4] &= ~0x8; // Clear A32 Addr Bit
      vme_config_addr[4] |= 0x80; // Select A24  0x80
      if((mem_addr == 0x0) || (mem_addr == 0x800000))
         vme_config_addr[3] = (short)((mem_addr & 0xff800000)>>16);
      else
         return BTD_ERR_BADADDRMAP;    
   }
   MSDELAY(1);

   vme_config_addr[2] = (BT_U16BIT)BOARD_SET; //un-reset the board and enable A24/A32 addressing

#if defined (PPC_SYNC)
   IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
   eieio
   sync
#pragma endasm
#endif

   board_is_v5_uca[cardnum]         = 0x1;
   hw_addr_shift[cardnum]           = 4;
   board_has_discretes[cardnum]     = 0x1;
   board_has_hwrtaddr[cardnum]      = 0x1;
   board_has_serial_number[cardnum] = 0x1;
   numDiscretes[cardnum]            = 4;
   bt_dismask[cardnum]              = 0xf;
   board_has_testbus[cardnum]       = 0x1;
   board_access_32[cardnum]         = 0x1;

   setReadWrite(cardnum);

   hwRTAddr[cardnum] = vbtGetRTaddr(cardnum,CurrentCardSlot[cardnum]);
   if(hwRTAddr[cardnum]== BTD_RTADDR_PARITY)
      hwRTAddr[cardnum] = -1;

   if(board_is_v5_uca[cardnum]) 
      _HW_FPGARev[cardnum] = vbtGetRegister16(cardnum,HWREG_LPU_REVISION);      
   else
      _HW_FPGARev[cardnum] = vbtGetRegister[cardnum](cardnum,HWREG_LPU_V6REVISION);

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t R E A D V M E C O N F I G
 *===========================================================================*
 *
 * FUNCTION:    The function returns the information in the A16 configuration
 *              space.  Use this function for diagnostics.
 *
 * DESCRIPTION: Return an array of shorts containing the config information:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
#ifdef __BORLANDC__
#pragma argsused
#endif
BT_INT vbtReadVMEConfigRegs(
   BT_U32BIT config_addr,        // A16 config address
   BT_U16BIT *config_data)     // cofiguration data storage
{
   volatile  BT_U16BIT *vme_config_addr; // word pointer to VME/VXI config registers
   int i;

   vme_config_addr = (BT_U16BIT *)config_addr;

   for(i=0;i<NUM_VME_CONFIG_REG;i++)
   {
      config_data[i] = vme_config_addr[i];
   }
   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t W R I T E V M E C O N F I G
 *===========================================================================*
 *
 * FUNCTION:    The function write to a register the A16 configuration
 *              space.  Use this function for setting up VME interrupts and 
 *              debugging.  Write with caution since cahnging certain addresses
 *              can affect board operation.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
#ifdef __BORLANDC__
#pragma argsused
#endif
BT_INT vbtWriteVMEConfigRegs(
   BT_U16BIT  *vme_config_addr,  // A16 Address
   BT_UINT   offset,             // Byte offset of register  
   BT_U16BIT config_data)        // (i) data to write
{

   if(offset > 24)
      return BTD_ERR_PARAM;
   if((offset % 2) != 0)
      return BTD_ERR_PARAM;
   
   vme_config_addr[offset/2] = config_data;

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:                   channel_setup
 *===========================================================================*
 *
 * FUNCTION:    Common setup steps for each board
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT channel_setup(BT_UINT cardnum, char *lpbase, BT_U16BIT *board_config)
{
   BT_INT i=0;
   BT_U16BIT host_interface=0, board_type=0, config;
   BT_UINT number_of_channels;

   /**********************************************************************
   * Get the CSC information
   **********************************************************************/
   bt_PageAddr[cardnum][CSC_REG_PAGE]  = lpbase;  // Common Registers
   bt_PageAddr[cardnum][V5_CSC_PAGE]   = lpbase;  // Common Registers

#ifdef INCLUDE_USB_SUPPORT
   if(CurrentCardType[cardnum]==R15USB)
   {
      BT_U32BIT temp;
      temp = usbGetCSCRegister(cardnum,0x0);
      host_interface = (BT_U16BIT)(temp & 0xffff);
   }
   else
#endif //INCLUDE_USB_SUPPORT
      host_interface = flipws(*(BT_U16BIT *)lpbase);

   config = host_interface & HIF_MASK;
   board_type = (host_interface & BOARD_MASK)>>1;

   if(host_interface & ADDITIONAL_CAPABILITY)
      board_has_acr[cardnum] = 0x1;
   else
      board_has_acr[cardnum] = 0x0;
    
   do
   {   
      if(board_config[i] == config)
         break;
      i++;
   }while(board_config[i] != 0xffff);

   if (board_config[i] == 0xffff)
      return BTD_ERR_BADBOARDTYPE;
   
   number_of_channels = (host_interface & CHANNEL_COUNT_MASK) >> CHANNEL_COUNT_SHIFT; 

   if(CurrentCardSlot[cardnum] > number_of_channels-1)
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   if(host_interface & UCA32 && board_type != 5)
   {
      bt_PageAddr[cardnum][SMP_LOCK_PAGE] = lpbase + SMP_LOCKS_OFFSET;    // SMP Lock Registers
      bt_PageAddr[cardnum][SHR_MEM_PAGE]  = lpbase + SHARED_MEM_OFFESET;  // Shared Memory
      bt_PageAddr[cardnum][HWREG_PAGE]    = lpbase + REG_OFFSET + (REG_SIZE * CurrentCardSlot[cardnum]) + HWREG_OFFSET;
      bt_PageAddr[cardnum][TTREG_PAGE]    = lpbase + REG_OFFSET + (REG_SIZE * CurrentCardSlot[cardnum]) + TTREG_OFFSET;
      bt_PageAddr[cardnum][TRIG_PAGE]     = lpbase + REG_OFFSET + (REG_SIZE * CurrentCardSlot[cardnum]) + TRIG_OFFSET;
      RAM_SIZE[cardnum]=0x100000;
      board_is_v5_uca[cardnum] = 0;
#ifdef INCLUDE_USB_SUPPORT
      if(CurrentCardType[cardnum]==R15USB)
         RAM_OFFSET[cardnum] = usbGetCSCRegister(cardnum,GLBREG_RAMSTART);
      else
#endif //INCLUDE_USB_SUPPORT
      RAM_OFFSET[cardnum] = ((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE]))[GLBREG_RAMSTART];

      fliplong(&RAM_OFFSET[cardnum]);
      bt_PageAddr[cardnum][RAM_PAGE]      = lpbase + RAM_OFFSET[cardnum] + (RAM_SIZE[cardnum] * CurrentCardSlot[cardnum]);
   }
   else
   {
      if(board_is_paged[cardnum])
      {
         bt_PageAddr[cardnum][V5_RAM_PAGE]    = lpbase + (0x1000 * CurrentCardSlot[cardnum]) + DATA_RAM_PCCD;        // V5 RAM
         bt_PageAddr[cardnum][V5_HWREG_PAGE]  = lpbase + (0x1000 * CurrentCardSlot[cardnum]) + HW_REG_PCCD;
         bt_PageAddr[cardnum][V5_RAMREG_PAGE] = lpbase + (0x1000 * CurrentCardSlot[cardnum]) + REG_FILE_PCCD;   
      }
      else
      {     
         bt_PageAddr[cardnum][V5_RAM_PAGE]    = lpbase + (0x200000 * CurrentCardSlot[cardnum]) + DATA_RAM_QPMC;      // V5 RAM
         bt_PageAddr[cardnum][V5_HWREG_PAGE]  = lpbase + (0x200000 * CurrentCardSlot[cardnum]) + HW_REG_QPMC;
         bt_PageAddr[cardnum][V5_RAMREG_PAGE] = lpbase + (0x200000 * CurrentCardSlot[cardnum]) + REG_FILE_QPMC;
      }

      hw_addr_shift[cardnum] = 4;
      board_is_v5_uca[cardnum] = 1;
   }

   if(CurrentCardType[cardnum] == PCCD1553  ||
      CurrentCardType[cardnum] == Q1041553  ||
      CurrentCardType[cardnum] == QPCI1553  ||
      CurrentCardType[cardnum] == Q1041553P ||
      board_has_acr[cardnum]   == 0x0)
      board_has_serial_number[cardnum] = 0x0;
   else
      board_has_serial_number[cardnum] = 0x1;

   //Set up the read/write function based on F/W version 
   setReadWrite(cardnum);

   if ((host_interface & MODE_MASK) >> MODE_SHIFT)
      _HW_1Function[cardnum] = MULTI_FUNCTION;
   else
      _HW_1Function[cardnum] = SINGLE_FUNCTION;

   board_has_irig[cardnum] = host_interface & IRIG_FLAG;
 
   if(board_is_v5_uca[cardnum]) 
      _HW_FPGARev[cardnum] = vbtGetRegister16(cardnum,HWREG_LPU_REVISION);      
   else
      _HW_FPGARev[cardnum] = vbtGetRegister[cardnum](cardnum,HWREG_LPU_V6REVISION);

   return API_SUCCESS;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p Q P M C
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the QPMC-1553
 *              native PCI bus board.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupQPMC(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   BT_INT status;
   BT_U16BIT board_config[] = {
	                       0x0042,   //QPM     sf 1ch
                           0x0082,   //QPM     sf 2ch
                           0x0102,   //QPM     sf 4ch
                           0x0842,   //QPM     mf 1ch
                           0x0882,   //QPM     mf 2ch
                           0x0902,   //QPM     mf 4ch
                           0x0054,   //R15-AMC sf 1ch
                           0x0094,   //R15-AMC sf 2ch
                           0x0114,   //R15-AMC sf 4ch
                           0x0854,   //R15-AMC mf 1ch
                           0x0894,   //R15-AMC mf 2ch
                           0x0914,   //R15-AMC mf 4ch
                           0x0058,   //RPCIe   sf 1ch
                           0x0098,   //RPCIe   sf 2ch
                           0x0118,   //RPCIe   sf 4ch
                           0x0858,   //RPCIe   mf 1ch
                           0x0898,   //RPCIe   mf 2ch
                           0x0918,   //RPCIe   mf 4ch
                           0xffff};  


   if((status = channel_setup(cardnum, lpbase, board_config)))
      return status;
        
   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1;

   // Set up discretes and hardwaired RT addressing

   numDiscretes[cardnum] = 18;
   bt_dismask[cardnum] = 0x3ffff;

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p L P C I E
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the LPCIE-1553
 *              native PCI bus board.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupLPCIE(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   BT_INT status;

  unsigned short board_config[] = {
                           0x005e,    //LPCIe   sf 1ch
                           0x009e,    //LPCIe   sf 2ch 
                           0x085e,    //LPCIe   mf 1ch
                           0x089e,    //LPCIe   mf 2ch  
                           0xffff};

   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;
        
   /********************************************************************
   * Check to See if the Selected Channel is present and board is active
   ********************************************************************/  

   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1; 
   boardHasMultipleTriggers[cardnum] = 0x1;
   board_has_temp_sensor[cardnum] = 0x1;

   // Set up discretes and hardwaired RT addressing

   numDiscretes[cardnum] = 14;
   bt_dismask[cardnum] = 0x3fff;

   hwRTAddr[cardnum] = vbtGetRTaddr(cardnum,CurrentCardSlot[cardnum]);

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p Q P C X
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the QPCX
 *              -1553 native PCI bus board.  This board is flat-mapped
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupQPCX(
   BT_UINT   cardnum,           // (i) card number (0 based)
   char      *lpbase)           // (i) board base address in user space
{
   BT_INT status;

   unsigned short board_config[] = {
	                       0x0046,   // sf 1ch
                           0x0086,   // sf 2ch
                           0x0106,   // sf 4ch
                           0x0846,   // mf 1ch
                           0x0886,   // mf 2ch
                           0x0906,   // mf 4ch
                           0xffff};

   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;

   board_has_discretes[cardnum] = 0x1;
   board_has_testbus[cardnum] = 0x1;
   board_has_hwrtaddr[cardnum] = 0x1;
   board_has_differential[cardnum] = 0x1;

   numDiscretes[cardnum] = 10;
   bt_dismask[cardnum] = 0x3ff;
 
   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p Q C P
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the QCP-1553
 *              native PCI bus board.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupQCP(
   BT_UINT   cardnum,           // (i) card number (0 based)
   char      *lpbase)           // (i) board base address in user space
{
   BT_INT status;


   unsigned short board_config[] = {
	                       0x004e,   // sf 1ch
                           0x008e,   // sf 2ch
                           0x010e,   // sf 4ch
                           0x084e,   // mf 1ch
                           0x088e,   // mf 2ch
                           0x090e,   // mf 4ch
                           0xffff};

   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;

   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1;
   board_has_plx_dma[cardnum] = 0x1;
   
   numDiscretes[cardnum] = 18;
   bt_dismask[cardnum] = 0x3ffff;

   hwRTAddr[cardnum] = vbtGetRTaddr(cardnum,CurrentCardSlot[cardnum]);

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p Q 1 0 4
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the QPMC-1553
 *              native PCI bus board.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupQ104(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   BT_INT status;
   unsigned short board_config[] = {
                                     // These are for the Q104-1553P 
	                       0x004a,   // sf 1ch
                           0x008a,   // sf 2ch
                           0x010a,   // sf 4ch
                           0x084a,   // mf 1ch
                           0x088a,   // mf 2ch
                           0x090a,   // mf 4ch
                           0xffff};

   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;

   if(CurrentCardType[cardnum] == Q1041553)
   { 
      bt_OffsetMask[cardnum] = BT_02KBOFFSETMASK_ISA;
      bt_FrameShift[cardnum] = BT_02KBFRAMESHIFT_ISA;
      bt_PtrShift[cardnum]   = BT_02KBPTRSHIFT_ISA;
      bt_FrameMask[cardnum]  = BT_FRAME_MASK_ISA;
   }

   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1; 
   numDiscretes[cardnum] = 10;
   bt_dismask[cardnum] = 0x3ff;

   hwRTAddr[cardnum] = vbtGetRTaddr(cardnum,CurrentCardSlot[cardnum]);

   return BTD_OK;
}


/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p R X M C 2
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the RXMC2-1553
 *              native PCI bus board.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupRXMC2(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   BT_INT status,i;
   unsigned short board_config[] = {
                           0x005c,   //   sf 1ch
                           0x009c,   //   sf 2ch
                           0x011c,   //   sf 4ch
                           0x085c,   //   mf 1ch
                           0x089c,   //   mf 2ch
                           0x091c,   //   mf 4ch
                           0xffff};  


   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;
   
   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1;
   board_is_dual_function[cardnum] = 0x1;
   boardHasMultipleTriggers[cardnum] = 0x1;

   // Set up discretes and hardwaired RT addressing


   if((_HW_FPGARev[cardnum] & 0xfff) < 0x601)
   {
       numDiscretes[cardnum] = 12;
       bt_dismask[cardnum] = 0xfff;
   }
   else
   {
       bt_dismask[cardnum]= vbtGetCSCRegister[cardnum](cardnum,GLBREG_DISCR_OPT);

       for(i=0;i<32;i++)
          if(bt_dismask[cardnum] & (1<<i))
             numDiscretes[cardnum]++;
   }

   hwRTAddr[cardnum] = vbtGetRTaddr(cardnum,CurrentCardSlot[cardnum]);

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p R A R 1 5
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the RAR15-XMC
 *              native PCI bus board.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupRAR15(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   BT_INT status;
   unsigned short board_config[] = {
                           0x0062,   //   sf 1ch
                           0x00a2,   //   sf 2ch
                           0x0122,   //   sf 4ch
                           0x0862,   //   mf 1ch
                           0x08a2,   //   mf 2ch
                           0x0922,   //   mf 4ch
                           0xffff};  


   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;
   
   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1;
   board_is_dual_function[cardnum] = 0x1;
   boardHasMultipleTriggers[cardnum] = 0x0;

   // Set up discretes and hardwaired RT addressing

   numDiscretes[cardnum] = 12;
   bt_dismask[cardnum] = 0xfff;

   hwRTAddr[cardnum] = vbtGetRTaddr(cardnum,CurrentCardSlot[cardnum]);

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p R A R 1 5 X T
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the RAR15-XMC-XT
 *              native PCI bus board.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupRAR15XT(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   BT_INT status;
   unsigned short board_config[] = {
                           0x0064,   //   sf 1ch
                           0x00a4,   //   sf 2ch
                           0x0124,   //   sf 4ch
                           0x0864,   //   mf 1ch
                           0x08a4,   //   mf 2ch
                           0x0924,   //   mf 4ch
                           0xffff};  


   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;
    
   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x0;
   board_is_dual_function[cardnum] = 0x1;
   boardHasMultipleTriggers[cardnum] = 0x0;
   board_has_temp_sensor[cardnum] = 0x1;
   board_using_shared_memory[cardnum] = 0x1;

   // Set up discretes and hardwaired RT addressing

   numDiscretes[cardnum] = 12;
   bt_dismask[cardnum] = 0xfff;

   hwRTAddr[cardnum] = vbtGetRTaddr(cardnum,CurrentCardSlot[cardnum]);

   return BTD_OK;
}


/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p R 1 5 E C
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the R15-EC
 *              native Express Card.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupR15EC(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   BT_INT status;

   unsigned short board_config[] = {
	                       0x0052,   //R15-EC sf 1ch
                           0x0092,   //R15-EC sf 2ch
                           0x0852,   //R15-EC mf 1ch
                           0x0892,   //R15-EC mf 2ch
                           0xffff};

   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;

   board_has_discretes[cardnum]=0x1;
   numDiscretes[cardnum] = 2;
   bt_dismask[cardnum] = 0xc0;
   board_has_hwrtaddr[cardnum] = 0; 
   board_has_differential[cardnum] = 0x1;
   board_has_485_discretes[cardnum]=0x1;

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:    v b t P a g e A c c e s s S e t u p R X M C 1 5 5 3
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the RXMC-1553
 *              native Express Card.  This board is flat-mapped (no frames)
 *              and supports four MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupRXMC(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   BT_INT status;
   BT_U16BIT capabilities = 0;
   BT_U16BIT fdata[32];

#define RXMC_OUTPUT_CONFIG_MASK 0x1f

   unsigned short board_config[] = {
	                       0x0056,   //RXMC-1553 sf 1ch
                           0x0096,   //RXMC-1553 sf 2ch
                           0x0856,   //RXMC-1553 mf 1ch
                           0x0896,   //RXMC-1553 mf 2ch
                           0xffff};

   if((status = channel_setup(cardnum,lpbase, board_config)))
      return status;

   board_has_rtaddr_latch[cardnum] = 0x1;

   if((_HW_FPGARev[cardnum] & 0xfff) <= 0x499)  // firmware revision V4.XX
   {
       capabilities = *(BT_U16BIT *)(bt_PageAddr[cardnum][V5_CSC_PAGE] + 1);
       rxmc_output_config[cardnum] = (capabilities & RXMC_OUTPUT_CONFIG_MASK)>>4;
   }
   else if((_HW_FPGARev[cardnum] & 0xfff) < 0x600)  // firmware revision V5.XX
   {
       vbtReadFlashData(cardnum, fdata);
       rxmc_output_config[cardnum] = fdata[2] & RXMC_OUTPUT_CONFIG_MASK;
   }
   else // firmware revision V6.XX
       rxmc_output_config[cardnum] = vbtGetCSCRegister[cardnum](cardnum,GLBREG_BD_SPECIFIC);

   switch(rxmc_output_config[cardnum])
   {
      default:
      case NO_OUTPUT:
         numDiscretes[cardnum] = 0;
         board_has_discretes[cardnum]=0x0;
         break;
      case PIO_OPN_GRN:
         board_has_pio[cardnum] = 0x1;
         board_has_discretes[cardnum]=0x1;
         numDiscretes[cardnum] = 4;
         numPIO[cardnum] = 8;
         bt_dismask[cardnum] = 0xf;
         bt_piomask[cardnum] = 0xff0;
         break;
      case PIO_28V_OPN:
         board_has_pio[cardnum] = 0x1;
         board_has_discretes[cardnum]=0x1;
         numDiscretes[cardnum] = 4;
         numPIO[cardnum] = 8;
         bt_dismask[cardnum] = 0xf;
         bt_piomask[cardnum] = 0xff0;
         break;
      case DIS_OPN_GRN:
         board_has_discretes[cardnum]=0x1;
         numPIO[cardnum] = 0;
         numDiscretes[cardnum] = 12;
         bt_dismask[cardnum] = 0xfff;
         break;
      case DIS_28V_OPN:
         board_has_discretes[cardnum]=0x1;
         numPIO[cardnum] = 0;
         numDiscretes[cardnum] = 12;
         bt_dismask[cardnum] = 0xfff;
         break;
      case EIA485_OPN_GRN:
         board_has_discretes[cardnum]=0x1;
         numDiscretes[cardnum] = 4;
         board_has_485_discretes[cardnum] = 0x1;
         board_has_differential[cardnum] = 0x1;
         bt_dismask[cardnum] = 0xf;
         break;
      case EIA485_28V_OPN:
         board_has_discretes[cardnum]=0x1;
         numDiscretes[cardnum] = 4;
         board_has_485_discretes[cardnum] = 0x1;
         board_has_differential[cardnum] = 0x1;
         bt_dismask[cardnum] = 0xf;
         break;
   }

   //Clear the external output triggers.
   if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
      vbtSetDiscrete[cardnum](cardnum,EXT_TRIG_OUT_CH1,0x0);
   else
      vbtSetDiscrete[cardnum](cardnum,EXT_TRIG_OUT_CH2,0x0);

   board_is_dual_function[cardnum] = 0x1;
   boardHasMultipleTriggers[cardnum] = 0x1;

   hwRTAddr[cardnum] = vbtGetRTaddr(cardnum,CurrentCardSlot[cardnum]);

   return BTD_OK;
}

#ifdef INCLUDE_USB_SUPPORT
/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p U S B 
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the USB-1553 USB
 *              device. This board is flat-mapped (no frames) and supports
 *              two MIL-STD-1553 interfaces on a single board.
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtPageAccessSetupUSB(BT_UINT cardnum)
{
   BT_INT status;
   BT_INT num_config = 4;
   BT_U16BIT capabilities=0;

   unsigned short board_config[] = {
	                       0x0060,   //USB-1553 df 1ch
                           0x00A0,   //USB-1553 df 2ch
                           0x0860,   //USB-1553 mf 1ch
                           0x08A0,   //USB-1553 mf 2ch
                           0xffff};    

   usb_host_hwreg[cardnum]  = (BT_U32BIT *)malloc(sizeof(BT_U32BIT) * 0x100);
   if(usb_host_hwreg[cardnum] == NULL)
      return 667;

   memset(usb_host_hwreg[cardnum],0,0x400);

   usb_int_queue[cardnum]   = (IQ_MBLOCK_V6 *)malloc(sizeof(IQ_MBLOCK_V6) * 512);
   if(usb_int_queue[cardnum] == NULL)
      return 668;

   status = usb_open(cardnum);
   if(status)
      return status;

   status = channel_setup(cardnum, NULL, board_config);
   if(status)
      return status;

   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x0;
   board_has_485_discretes[cardnum]=0x0;
   board_has_hwrtaddr[cardnum] = 0x0;

   numDiscretes[cardnum] = 8;
   if((_HW_FPGARev[cardnum] & 0xfff) <= 0x600)
       bt_dismask[cardnum] = 0xff;
   else
       bt_dismask[cardnum]= vbtGetCSCRegister[cardnum](cardnum,GLBREG_DISCR_OPT);

   return BTD_OK;
}
#endif //#ifdef INCLUDE_USB_SUPPORT

#ifdef INCLUDE_PCCD
/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p P C C D
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access for the PCC-D1553
 *              PCMCIA bus board.  This board is pagged in memory space
 *              (no I/O ports) and supports two MIL-STD-1553 interfaces on
 *              a single board.
 *
 *              Note that this function is not complete!
 *
 * DESCRIPTION: Setup the following access pointer:.
 *
 *              bt_PageAddr[cardnum][0] maps the board dual-port memory, and
 *              bt_PageAddr[cardnum][1] points to the Hardware Registers.
 *              bt_PageAddr[cardnum][2] points to the RAM Registers.
 *              bt_PageAddr[cardnum][3] points to the Host Interface Registers.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
#ifdef __BORLANDC__
#pragma argsused
#endif
BT_INT vbtPageAccessSetupPCCD(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   volatile BT_U16BIT *board_short_addr;  // DWORD pointer to board CSC Register. 
   BT_INT num_config = 4;
   BT_INT status;
   
   unsigned short board_config[] = {
	                       0x004c,   // sf 1ch
                           0x008c,   // sf 2ch
                           0x084c,   // mf 1ch
                           0x088c};  // mf 2ch

   bt_OffsetMask[cardnum] = BT_02KBOFFSETMASK_ISA;
   bt_FrameShift[cardnum] = BT_02KBFRAMESHIFT_ISA;
   bt_PtrShift[cardnum]   = BT_02KBPTRSHIFT_ISA;
   bt_FrameMask[cardnum]  = BT_FRAME_MASK_ISA;

   board_has_discretes[cardnum]= 0x1;
   board_has_hwrtaddr[cardnum] = 0x0;
   board_has_differential[cardnum] = 0x1; //This so the PCCD trigger can get set. No Differtial I/O suppoprt
   board_is_paged[cardnum] = 1;
   board_access_32[cardnum] = 0;

   board_short_addr = (volatile BT_U16BIT *)lpbase;
   if ((board_short_addr[1] & 0x4) == 0) // FPGA not configured so load the FPGA
   {
       status = vbtLoad_pccd_fpga(board_short_addr,PCCD_FpgaData,sizeof(PCCD_FpgaData));
       if ( status != BTD_OK )
          return status;
   }

   status = channel_setup(cardnum, lpbase, board_config);
   if(status)
      return status;
 
   numDiscretes[cardnum] = 2;
   bt_dismask[cardnum] = 0xc0;

   return API_SUCCESS;
}
#endif //INCLUDE_PCCD

/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G C a l
 *===========================================================================*
 *
 * FUNCTION:    Calibrates the IRIG DAC for an external input IRIG signal
 *
 * DESCRIPTION: This routine sets the DAC to Vmin + .825(Vmax -Vmin).  Vmin is
 *              the lower peak value and Vmax is the upper peak value.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtIRIGCal(BT_UINT cardnum,BT_INT flag)
{
   BT_U16BIT *irig_dac_reg;
   BT_U16BIT *irig_cal_reg;

   BT_U16BIT dac_min=0,dac_max=0xff;

   BT_U16BIT vmin, vmax, cdata;
   BT_U16BIT dac_start_value=0, dac_start_increment=0;
   BT_U16BIT margin=0;
   int dac_value, dac_increment;
   int bad_signal=0;

   int off;
   int found;

#define dprint(a,b) if(flag){printf(a,b);}

   irig_dac_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_DAC_REG);
   irig_cal_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_CNTL_REG);

   // set the DAC to 0x80 and increase until the upper peak is found
   // the signal range.

   found = FALSE;
   dac_value = 0x80;

   dprint("Initial DAC Setting = %x\n",dac_value);
   *irig_dac_reg = flipws(dac_value);  // Set the DAC value
   MSDELAY(5);                   // Wait 5 Msec for signal
   cdata = irig_cal_reg[0];    // Read the cal register to clear it
   MSDELAY(30);                  // Wait 30 Msec for signal
   cdata = irig_cal_reg[0];    // read the cal register again
   flipw(&cdata);
   if((cdata & 0x1) == 0x1)
   {
      //make sure we are not at a ragged edge
      *irig_dac_reg = flipws((dac_value+3));  // Set the DAC value
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = irig_cal_reg[0];    // Read the cal register to clear it
      MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = irig_cal_reg[0];    // read the cal register again
      flipw(&cdata);
      if((cdata & 0x1)==0x0)
        bad_signal+=1;
         
      *irig_dac_reg = flipws((dac_value-3));  // Set the DAC value
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = irig_cal_reg[0];    // Read the cal register to clear it
      MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = irig_cal_reg[0];    // read the cal register again
      flipw(&cdata);
      if((cdata & 0x1)==0x0)
         bad_signal+=1;
   }
   else
      bad_signal+=1;

   if(!bad_signal)    // if the cal bit is set we inside the IRIG-B signal 
   {
	  dac_start_value = 0x80;
      dac_start_increment = 0x40;
      dac_min = 0;
	  dac_max =0xff;
      dprint("IRIG-B Signal found at %x initial setting\n", dac_value);
      dprint("dac_start_value     = %x\n",dac_start_value);
      dprint("dac_start_increment = %x\n",dac_start_increment);
      dprint("dac_min             = %x\n",dac_min);
      dprint("dac_max             = %x\n",dac_max);
   }
   else  // search for a point within the IRIG-B singal to start the search.
   {
	  for (dac_value = 0x88;dac_value<0x100;dac_value+=0x8)
	  {
	     *irig_dac_reg = flipws(dac_value);  // Set the DAC value
         dprint("Finding the signal above the mid point -- DAC Setting = %x\n",dac_value);
         MSDELAY(5);                   // Wait 5 Msec for signal
         cdata = irig_cal_reg[0];    // Read the cal register to clear it
         MSDELAY(30);                  // Wait 30 Msec for signal
         cdata = irig_cal_reg[0];    // read the cal register again
         flipw(&cdata);
         if((cdata & 0x1) == 0x1)    // if the cal register is set make sure its not just some noise
		 {
            dac_min = 0x7f;
			dac_max = 0xff;
			dac_start_value = dac_value+3;//(dac_value+dac_max)/2;
			dac_start_increment = 0x20;
			found = TRUE;
            dprint("IRIG-B Signal found at %x above the mid point\n", dac_value);
            dprint("dac_start_value     = %x\n",dac_start_value);
            dprint("dac_start_increment = %x\n",dac_start_increment);
            dprint("dac_min             = %x\n",dac_min);
            dprint("dac_max             = %x\n",dac_max);
			break;
         }
      }
      if(found==FALSE)
	  {
         for (dac_value = 0x78;dac_value>0x0;dac_value-=0x8)
         {
	        *irig_dac_reg = flipws(dac_value);  // Set the DAC value
	        dprint("Finding the signal below the mid point -- DAC Setting = %x\n",dac_value);
            MSDELAY(5);                   // Wait 5 Msec for signal
            cdata = irig_cal_reg[0];    // Read the cal register to clear it
            MSDELAY(30);                  // Wait 30 Msec for signal
            cdata = irig_cal_reg[0];    // read the cal register again
            flipw(&cdata);
            if((cdata & 0x1) == 0x1)    // 
			{
			    dac_min = 0;
			    dac_max = 0x81;
			    dac_start_increment = 0x20;
				dac_start_value = dac_value-3;//(dac_value-dac_min)/2;
                dprint("IRIG-B Signal found at %x below the mid point\n", dac_value);
                dprint("dac_start_value     = %x\n",dac_start_value);
                dprint("dac_start_increment = %x\n",dac_start_increment);
                dprint("dac_min             = %x\n",dac_min);
                dprint("dac_max             = %x\n",dac_max);
			    found = TRUE;
				break;
            }
         }
		 if(found==FALSE)
            return BTD_IRIG_NO_SIGNAL;
      }
   }

   dprint("Signal found at %x.  Now calibrating\n",dac_value);
   found = FALSE;
   dac_increment = dac_start_increment;
   dac_value = dac_start_value;
   while(1)
   {  
      *irig_dac_reg = flipws(dac_value);  // Set the DAC value
	  dprint("Finding the lower transition -- DAC Setting = %x\n",dac_value);
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = irig_cal_reg[0];    // Read the cal register to clear it
	  MSDELAY(30);                  // Wait 30 Msec for signal
	  cdata = irig_cal_reg[0];    // read the cal register again
      flipw(&cdata);	
      if((cdata & 0x1) == 0x1)    // 
      {
         dac_value = dac_value - dac_increment;
		 if(dac_increment <= 4)
		 {
            off = -1;
            //fine_tune
            dac_increment = 1;
            do
			{
			   *irig_dac_reg = flipws(dac_value);
	           dprint("Fine tuning lower transition -- DAC Setting = %x\n",dac_value);
			   cdata = irig_cal_reg[0];
			   MSDELAY(20);
			   cdata = irig_cal_reg[0];
               flipw(&cdata);
			   if((cdata & 0x1) == 0x1)
			   {
                  if(off == 1)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
			         off = 0; //false
                     dac_value -= dac_increment;
				  }
			   }
			   else
			   {
                  if(off == 0)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
                     off = 1; //true
                     dac_value += dac_increment;
				  }
			   }
			}while(found==FALSE);
			break;
		 }
	  }
      else
	  {
		  dac_value+=dac_increment;
		  if (dac_value >= dac_max)
		    return BTD_IRIG_NO_LOW_PEAK;
	  }
	  dac_increment=dac_increment/2;
	  if(dac_increment == 0)
		  dac_increment = 2;
   }
   vmin = dac_value+margin;
   dprint("Lower transition value = %x\n",vmin);   

   found = FALSE;
 
   dac_value = (vmin+dac_max)/2; // start at the mid point between vmin and the dac_max
   dprint("Start the upper transition search at %x\n",dac_start_value);
   dac_increment = (0xff - vmin)/2;
   while(1)
   {  
      *irig_dac_reg = flipws(dac_value);
	  dprint("Finding upper transition -- DAC Setting = %x\n",dac_value);
      MSDELAY(30);                  // Wait 20 Msec for signal
      cdata = irig_cal_reg[0];    // Read the cal register to clear it
	  MSDELAY(30);                  // Wait 20 Msec for signal
	  cdata = irig_cal_reg[0];    // read the cal register again
      flipw(&cdata);
	  if((cdata & 0x1) == 0x1)    // if the cal register is set let make sure it not just some noise
      {
         dac_value += dac_increment;
         if(dac_increment <= 4)
		 {
			off = -1;
            //fine_tune
            dac_increment = 1;
            do
			{
			   *irig_dac_reg = flipws(dac_value);
	           dprint("Fine tuning upper transition -- DAC Setting = %x\n",dac_value);
			   cdata = irig_cal_reg[0];
			   MSDELAY(20);
			   cdata = irig_cal_reg[0];
               flipw(&cdata);
			   if((cdata & 0x1) == 0x1 )
			   {
                  if(off == 1)
				  {
            		  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
			         off = 0; //false
                     dac_value += dac_increment;
				  }
			   }
			   else
			   {
                  if(off == 0)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
                     off = 1; //true
                     if(dac_value>vmin)
                        dac_value -= dac_increment;
                     else
                        dac_value += dac_increment;
				  }
			   }
			}while(found==FALSE);
			break;
         }
	  }
      else
	  {
          if(dac_value > vmin)
          {
             dac_value-=dac_increment;
             if(dac_value < 0)
               dac_value = 0;
          }
          else
          {
             dac_value+=dac_increment;
             if(dac_value > 0xff)
                dac_value = 0xff;
          }
	  }
	  dac_increment = dac_increment/2;
      if(dac_increment == 0)
         dac_increment = 2;
   }
   vmax = dac_value-margin;
   dprint("Upper transition value = %x\n",vmax);

   dac_value = vmin + (BT_U16BIT)((825*(vmax-vmin))/1000);
   *irig_dac_reg = flipws(dac_value);
   dprint("vmin = %d\n",vmin);
   dprint("vmax = %d\n",vmax);
   dprint("vpp  = %d\n",vmax-vmin);
   dprint("final DAC setting = %d\n",dac_value);
   *irig_dac_reg = flipws(dac_value);

   if((vmax-vmin)<MIN_DAC_LEVEL)
	   return BTD_IRIG_LEVEL_ERR;

   return API_SUCCESS;
}


/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G C a l V6
 *===========================================================================*
 *
 * FUNCTION:    Calibrates the IRIG DAC for an external input IRIG signal
 *
 * DESCRIPTION: This routine sets the DAC to Vmin + .825(Vmax -Vmin).  Vmin is
 *              the lower peak value and Vmax is the upper peak value.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtIRIGCalV6(BT_UINT cardnum,BT_INT flag)
{
   BT_U16BIT dac_min=0,dac_max=0xff;

   BT_U32BIT vmin, vmax, cdata;
   BT_U16BIT dac_start_value=0, dac_start_increment=0;
   BT_U16BIT margin=0;
   BT_UINT dac_value, dac_increment;
   int bad_signal=0;

   int off;
   int found;

#define dprint(a,b) if(flag){printf(a,b);}

   // set the DAC to 0x80 and increase until the upper peak is found
   // the signal range.

   found = FALSE;
   dac_value = 0x80;

   dprint("Initial DAC Setting = %x\n",dac_value);

   vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,dac_value + V6_IRIG_DAC_SEL);
   MSDELAY(5);                   // Wait 5 Msec for signal

   cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
   MSDELAY(30);                  // Wait 30 Msec for signal

   cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
   flipw(&cdata);
   if((cdata & 0x1) == 0x1)
   {
      //make sure we are not at a ragged edge
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + 3 + V6_IRIG_DAC_SEL)));
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      flipw(&cdata);
      if((cdata & 0x1)==0x0)
        bad_signal+=1;
         
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + 3 + V6_IRIG_DAC_SEL)));
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      flipw(&cdata);
      if((cdata & 0x1)==0x0)
         bad_signal+=1;
   }
   else
      bad_signal+=1;

   if(!bad_signal)    // if the cal bit is set we inside the IRIG-B signal 
   {
	  dac_start_value = 0x80;
      dac_start_increment = 0x40;
      dac_min = 0;
	  dac_max =0xff;
      dprint("IRIG-B Signal found at %x initial setting\n", dac_value);
      dprint("dac_start_value     = %x\n",dac_start_value);
      dprint("dac_start_increment = %x\n",dac_start_increment);
      dprint("dac_min             = %x\n",dac_min);
      dprint("dac_max             = %x\n",dac_max);
   }
   else  // search for a point within the IRIG-B singal to start the search.
   {
	  for (dac_value = 0x88;dac_value<0x100;dac_value+=0x8)
	  {
         vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + V6_IRIG_DAC_SEL)));
         dprint("Finding the signal above the mid point -- DAC Setting = %x\n",dac_value);
         MSDELAY(5);                   // Wait 5 Msec for signal
         cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
         MSDELAY(30);                  // Wait 30 Msec for signal
         cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
         flipw(&cdata);
         if((cdata & 0x1) == 0x1)    // if the cal register is set make sure its not just some noise
		 {
            dac_min = 0x7f;
			dac_max = 0xff;
			dac_start_value = dac_value+3;//(dac_value+dac_max)/2;
			dac_start_increment = 0x20;
			found = TRUE;
            dprint("IRIG-B Signal found at %x above the mid point\n", dac_value);
            dprint("dac_start_value     = %x\n",dac_start_value);
            dprint("dac_start_increment = %x\n",dac_start_increment);
            dprint("dac_min             = %x\n",dac_min);
            dprint("dac_max             = %x\n",dac_max);
			break;
         }
      }
      if(found==FALSE)
	  {
         for (dac_value = 0x78;dac_value>0x0;dac_value-=0x8)
         {
            vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + V6_IRIG_DAC_SEL)));
	        dprint("Finding the signal below the mid point -- DAC Setting = %x\n",dac_value);
            MSDELAY(5);                   // Wait 5 Msec for signal
            cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
            MSDELAY(30);                  // Wait 30 Msec for signal
            cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
            flipw(&cdata);
            if((cdata & 0x1) == 0x1)    // 
			{
			    dac_min = 0;
			    dac_max = 0x81;
			    dac_start_increment = 0x20;
				dac_start_value = dac_value-3;//(dac_value-dac_min)/2;
                dprint("IRIG-B Signal found at %x below the mid point\n", dac_value);
                dprint("dac_start_value     = %x\n",dac_start_value);
                dprint("dac_start_increment = %x\n",dac_start_increment);
                dprint("dac_min             = %x\n",dac_min);
                dprint("dac_max             = %x\n",dac_max);
			    found = TRUE;
				break;
            }
         }
		 if(found==FALSE)
            return BTD_IRIG_NO_SIGNAL;
      }
   }

   dprint("Signal found at %x.  Now calibrating\n",dac_value);
   found = FALSE;
   dac_increment = dac_start_increment;
   dac_value = dac_start_value;
   while(1)
   {  
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + V6_IRIG_DAC_SEL)));
	  dprint("Finding the lower transition -- DAC Setting = %x\n",dac_value);
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
	  MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      flipw(&cdata);	
      if((cdata & 0x1) == 0x1)    // 
      {
         dac_value = dac_value - dac_increment;
		 if(dac_increment <= 4)
		 {
            off = -1;
            //fine_tune
            dac_increment = 1;
            do
			{
               vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + V6_IRIG_DAC_SEL)));
	           dprint("Fine tuning lower transition -- DAC Setting = %x\n",dac_value);
               cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
			   MSDELAY(20);
               cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
               flipw(&cdata);
			   if((cdata & 0x1) == 0x1)
			   {
                  if(off == 1)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
			         off = 0; //false
                     dac_value -= dac_increment;
				  }
			   }
			   else
			   {
                  if(off == 0)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
                     off = 1; //true
                     dac_value += dac_increment;
				  }
			   }
			}while(found==FALSE);
			break;
		 }
	  }
      else
	  {
		  dac_value+=dac_increment;
		  if (dac_value >= dac_max)
		    return BTD_IRIG_NO_LOW_PEAK;
	  }
	  dac_increment=dac_increment/2;
	  if(dac_increment == 0)
		  dac_increment = 2;
   }
   vmin = dac_value+margin;
   dprint("Lower transition value = %x\n",vmin);   

   found = FALSE;
 
   dac_value = (vmin+dac_max)/2; // start at the mid point between vmin and the dac_max
   dprint("Start the upper transition search at %x\n",dac_start_value);
   dac_increment = (0xff - vmin)/2;
   while(1)
   {  
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + V6_IRIG_DAC_SEL)));
	  dprint("Finding upper transition -- DAC Setting = %x\n",dac_value);
      MSDELAY(30);                  // Wait 20 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
	  MSDELAY(30);                  // Wait 20 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      flipw(&cdata);
	  if((cdata & 0x1) == 0x1)    // if the cal register is set let make sure it not just some noise
      {
         dac_value += dac_increment;
         if(dac_increment <= 4)
		 {
			off = -1;
            //fine_tune
            dac_increment = 1;
            do
			{
               vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + V6_IRIG_DAC_SEL)));
	           dprint("Fine tuning upper transition -- DAC Setting = %x\n",dac_value);
               cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
			   MSDELAY(20);
               cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
               flipw(&cdata);
			   if((cdata & 0x1) == 0x1 )
			   {
                  if(off == 1)
				  {
            		  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
			         off = 0; //false
                     dac_value += dac_increment;
				  }
			   }
			   else
			   {
                  if(off == 0)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
                     off = 1; //true
                     if(dac_value>vmin)
                        dac_value -= dac_increment;
                     else
                        dac_value += dac_increment;
				  }
			   }
			}while(found==FALSE);
			break;
         }
	  }
      else
	  {
          if(dac_value > vmin)
          {
             dac_value-=dac_increment;
          }
          else
          {
             dac_value+=dac_increment;
             if(dac_value > 0xff)
                dac_value = 0xff;
          }
	  }
	  dac_increment = dac_increment/2;
      if(dac_increment == 0)
         dac_increment = 2;
   }
   vmax = dac_value-margin;
   dprint("Upper transition value = %x\n",vmax);

   dac_value = vmin + (BT_U16BIT)((825*(vmax-vmin))/1000);
   vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + V6_IRIG_DAC_SEL)));
   dprint("vmin = %d\n",vmin);
   dprint("vmax = %d\n",vmax);
   dprint("vpp  = %d\n",vmax-vmin);
   dprint("final DAC setting = %d\n",dac_value);
   vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,flipws((dac_value + V6_IRIG_DAC_SEL)));

   if((vmax-vmin)<MIN_DAC_LEVEL)
	   return BTD_IRIG_LEVEL_ERR;

   return API_SUCCESS;
}
/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G C o n f i g
 *===========================================================================*
 *
 * FUNCTION:    configure the IRIG contrl register.
 *
 * DESCRIPTION: The function write the value to the IRIG control register
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/
void vbtIRIGConfig(BT_UINT cardnum, BT_U16BIT value)
{
   BT_U16BIT * irig_cntl_reg;

   irig_cntl_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_CNTL_REG);
   *irig_cntl_reg = flipws(value);
}

/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G V a l i d
 *===========================================================================*
 *
 * FUNCTION:    configure the IRIG contrl register.
 *
 * DESCRIPTION: The function returns the valid bit setting in the config register
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/
void vbtIRIGValid(BT_UINT cardnum, BT_U16BIT * valid)
{
   BT_U16BIT * irig_cntl_reg;

   irig_cntl_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_CNTL_REG);
   *valid = (BT_U16BIT)(flipws(*irig_cntl_reg) & 0x8);
}

/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G W r i t e D A C
 *===========================================================================*
 *
 * FUNCTION:    Set IRIG DAC register.
 *
 * DESCRIPTION: The function write the value to the IRIG DAC register
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/
void vbtIRIGWriteDAC(BT_UINT cardnum, BT_U16BIT value)
{
   BT_U16BIT * irig_dac_reg;

   irig_dac_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_DAC_REG);
   *irig_dac_reg = flipws(value); 
}

/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G S E T T I M E
 *===========================================================================*
 *
 * FUNCTION:    Set the IRIG time value.
 *
 * DESCRIPTION: The function write the value to the IRIG TOY register
 *
 *      It will return:
 *              nothing
 *===========================================================================*/
void vbtIRIGSetTime(BT_UINT cardnum, BT_U16BIT time_lsb, BT_U16BIT time_msb)
{
   BT_U16BIT *irig_toy_reg;

   irig_toy_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_TOY_REG_LSB);

   irig_toy_reg[0] = flipws(time_lsb);
   irig_toy_reg[1] = flipws(time_msb);
}



