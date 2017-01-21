/*============================================================================*
 * FILE:                        B T D R V . H
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
 *          INTELLIGENT PLATFORMS
 *
 *===========================================================================*
 *
 * FUNCTION:   BusTools API low-level board access component data definitions.
 *
 * DESCRIPTION:  This module defines all of the local data structures needed
 *               to map, load and access the BusTools Boards.  All of the
 *               supported boards are defined here.
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  07/23/1999   Split data definitions from BTDRV.C into this file.V3.20.ajh
  01/18/2000   Added definitions for the ISA-1553.V4.00.ajh
  01/07/2002   Added supprt for Quad-PMC and Dual Channel IP V4.46 rhc
  02/19/2004   PCCard-D1553 Support
  01/02/2006   portibility mdofication
  11/19/2007   Add function prototype for vbtPageAccessSetupR15EC
  06/29/2009   Add support for the RXMC-1553

 */

/*---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#ifndef _BT_GLOBALS_
#define EXTERN extern
#else
#define EXTERN
#undef _BT_GLOBALS_
#endif
#include "lowlevel.h"

/* device number to cardnum translation. */
EXTERN int api_device[MAX_BTA];

/* This is the max number of Pointers needed to map the board's memory        */
/*  into the Host Address space.                                              */
#define BT_NUM_PAGES   4           /* 4 pages with 64KB in each logical page. */
/* This parameter is used in memmap.c, and must track what's there!           */
EXTERN LPSTR bt_PageAddr[MAX_BTA][BT_NUM_PAGES]; /* Page addrs for each board */
EXTERN LPSTR IPD_IDPROM_ADDR[MAX_BTA];          /* IDPROM Address for IPD1553 */
EXTERN char  bt_UserDLLName[MAX_BTA][255];       /* Name of the User DLL      */
EXTERN int   hw_int_enable[MAX_BTA];             /* Hardware Interrupt enable */

/******************************************************************************/
/* The I/O control register is used to select the Writable Control Store,     */
/*   which is accessed in an address space which parallels the card memory.   */
/*   Whenever the card is reset or powered-up, the WCS must be reloaded.      */
/* The WCS data is written, 16 bits at a time, through the 2 Kbyte window to  */
/*   the associated WCS address.  Since the WCS is 48 bits wide, every 4th    */
/*   word is skipped when writing the WCS data to the card.  The API reads    */
/*   back the WCS to verify correct initialization of the IP.                 */
/*                                                                            */
/* The PCI/ISA-1553 board maps 32 megs of memory, and uses no IO space.  This */
/*   board is flat-mapped; there are no pages.  We only support this board    */
/*   under 32-bit environments since mapping would be tough in 16-bits.       */
/******************************************************************************/

/* Define the shift counts used to convert a board offset into a board frame: */
#define BT_PCI_MEMORY          1024       /* Number of 1KB blocks of memory   */
#define BT_VME_MEMORY          1024       /* Number of 1KB blocks of memory   */
#define BT_ISA_MEMORY          1024       /* Number of 1KB blocks of memory   */
#define BT_AR15_MEMORY          256       /* Number of 1KB Blocks of memory   */

#define BT_02KBFRAMESHIFT_ISA   (11)      /* ISA-1553 2KB page frame          */
#define BT_02KBPTRSHIFT_ISA     (23)      /* ISA-1553 2KB pointer index       */
#define BT_02KBOFFSETMASK_ISA 0x000007FFL /* ISA-1553 extract 2KB offset      */
#define BT_FRAME_MASK_ISA     0xFFFF   /* Frame bits for the ISA1553 frame reg*/

/* Define the base address of the RT segment */
#define BT_RT_BASE_PC      0x20000L   /* Base address of RT-seg 2 ISA board   */
#define BT_RT_BASE_IP      0x00000L   /* Base address of RT-seg 1 IP board    */
#define BT_RT_BASE_PCI     0x00000L   /* Base address of RT-seg 1 PCI board   */
#define BT_RT_BASE_VME     0x00000L   /* Base address of RT-seg 1 PCI board   */

// Host Interface Registers for the PCI/ISA/PCC-1553 boards:
#define HIR_CSC_REG       (0/4)  /* DWORD Offset to Control/Status/Config Reg */
#define HIR_AC_REG         1     /* Additional capabilties register for QPM   */
#define HIR_CONFIG_10K    (4/4)  /* DWORD Offset to 10K Conf Load Data Reg    */
#define HIR_PAGE_1        (8/2)  /* WORD  Offset to Page Register Channel 1   */
#define HIR_JUMPER_REV    (10/2) /* WORD  Offset to Jumpers/Revision ID Reg   */
#define HIR_IRQ_ENABLE    (12/2) /* WORD  Offset to Interrupt Enable Register */
#define HIR_PAGE_2        (14/2) /* WORD  Offset to Page Register Channel 2   */

#define HIR_QPAGE_1        (16/2)  /* WORD  Offset to Page Register Channel 1  */
#define HIR_QPAGE_2        (18/2)  /* WORD  Offset to Page Register Channel 2  */
#define HIR_QPAGE_3        (20/2)  /* WORD  Offset to Page Register Channel 3  */
#define HIR_QPAGE_4        (22/2)  /* WORD  Offset to Page Register Channel 4  */

// Definitions for the PCI-1553 board...byte offsets from the base address:
#define CHAN1_PCI        0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_PCI        0x01000000   /* Offset to channel 2 specific fncts  */
#define DATA_RAM_PCI     0x00200000   /* Offset to 1553 data RAM area        */
#define HW_REG_PCI       0x00400000   /* Offset to 1553 Control Register     */
#define REG_FILE_PCI     0x00500000   /* Offset to Register File             */
#define WCS_RAM_PCI      0x00600000   /* Offset to WCS RAM area              */

// Definitions for the ISA/PCC-1553 board...byte offsets from the base address:
#define CHAN1_ISA        0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_ISA        0x00002000   /* Offset to channel 2 specific fncts  */
#define DATA_RAM_ISA     0x00000800   /* Offset to 1553 data RAM area        */
#define HW_REG_ISA       0x00001000   /* Offset to 1553 Control Register     */
#define REG_FILE_ISA     0x00001400   /* Offset to Register File             */
#define WCS_RAM_ISA      0x00001800   /* Offset to WCS RAM area              */

// Definitions for the PCC-D1553 board...byte offsets from the base address:
#define CHAN1_PCCD        0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_PCCD        0x00001000   /* Offset to channel 2 specific fncts  */
#define DATA_RAM_PCCD     0x00000800   /* Offset to 1553 data RAM area        */
#define HW_REG_PCCD       0x00000200   /* Offset to 1553 Control Register     */
#define REG_FILE_PCCD     0x00000400   /* Offset to Register File             */

// Definitions for the VME-1553 board...byte offsets from the base address:
#define CHAN1_VME        0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_VME        0x00200000   /* Offset to channel 2 specific fncts  */
#define CHAN3_VME        0x00400000   /* Offset to channel 3 specific fncts  */
#define CHAN4_VME        0x00600000   /* Offset to channel 4 specific fncts  */
#define DATA_RAM_VME     0x00100000   /* Offset to 1553 data RAM area        */
#define HW_REG_VME       0x00000800    /* Offset to 1553 Control Register    */
#define REG_FILE_VME     0x00001000   /* Offset to Register File             */

// Definitions for the QPMC-1553 board...byte offsets from the base address:
#define CHAN1_QPMC       0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_QPMC       0x00200000   /* Offset to channel 2 specific fncts  */
#define CHAN3_QPMC       0x00400000   /* Offset to channel 3 specific fncts  */
#define CHAN4_QPMC       0x00600000   /* Offset to channel 4 specific fncts  */
#define DATA_RAM_QPMC    0x00100000   /* Offset to 1553 data RAM area        */
#define HW_REG_QPMC      0x00000800   /* Offset to 1553 Control Register     */
#define REG_FILE_QPMC    0x00001000   /* Offset to Register File             */

// Definitions for the QPCI-1553 board...byte offsets from the base address:
#define CHAN1_QPCI       0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_QPCI       0x00200000   /* Offset to channel 2 specific fncts  */
#define CHAN3_QPCI       0x00400000   /* Offset to channel 3 specific fncts  */
#define CHAN4_QPCI       0x00600000   /* Offset to channel 4 specific fncts  */
#define DATA_RAM_QPCI    0x00100000   /* Offset to 1553 data RAM area        */
#define HW_REG_QPCI      0x00000800   /* Offset to 1553 Control Register     */
#define REG_FILE_QPCI    0x00001000   /* Offset to Register File             */

// Definitions for the QCP-1553 board...byte offsets from the base address:
#define CHAN1_QCP       0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_QCP       0x00200000   /* Offset to channel 2 specific fncts  */
#define CHAN3_QCP       0x00400000   /* Offset to channel 3 specific fncts  */
#define CHAN4_QCP       0x00600000   /* Offset to channel 4 specific fncts  */
#define DATA_RAM_QCP    0x00100000   /* Offset to 1553 data RAM area        */
#define HW_REG_QCP      0x00000800   /* Offset to 1553 Control Register     */
#define REG_FILE_QCP    0x00001000   /* Offset to Register File             */

// Definitions for the IP-D1553 ...byte offsets from the base address:
#define CHAN1_IPD        0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_IPD        0x00200000   /* Offset to channel 2 specific fncts  */
#define DATA_RAM_IPD     0x00100000   /* Offset to 1553 data RAM area        */
#define HW_REG_IPD       0x00000800   /* Offset to 1553 Control Register     */
#define REG_FILE_IPD     0x00001000   /* Offset to Register File             */
#define IPD_IDPROM       0x00002000   /* Offset to Mirrored IP IDPROM        */

// Definitions for the Q104-ISA-1553 board...byte offsets from the base address:
#define CHAN1_Q104    0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_Q104    0x00001000   /* Offset to channel 2 specific fncts  */
#define CHAN3_Q104    0x00002000   /* Offset to channel 3 specific fncts  */
#define CHAN4_Q104    0x00003000   /* Offset to channel 4 specific fncts  */
#define DATA_RAM_Q104 0x00000800   /* Offset to 1553 data RAM area        */
#define HW_REG_Q104   0x00000200   /* Offset to 1553 Control Register     */
#define REG_FILE_Q104 0x00000400   /* Offset to Register File             */

// Definitions for the Q104-PCI-1553 board...byte offsets from the base address:
#define CHAN1_Q104P    0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_Q104P    0x00200000   /* Offset to channel 2 specific fncts  */
#define CHAN3_Q104P    0x00400000   /* Offset to channel 3 specific fncts  */
#define CHAN4_Q104P    0x00600000   /* Offset to channel 4 specific fncts  */
#define DATA_RAM_Q104P 0x00100000   /* Offset to 1553 data RAM area        */
#define HW_REG_Q104P   0x00000800   /* Offset to 1553 Control Register     */
#define REG_FILE_Q104P 0x00001000   /* Offset to Register File             */

/******************************************************************************/
/* Module Private (Static) Data Base.  Shared with HWSETUP.C and IPSETUP.C    */
/******************************************************************************/

EXTERN BT_U32BIT bt_OffsetMask[MAX_BTA];  // Extracts page offset from addr.
EXTERN BT_U16BIT bt_FrameShift[MAX_BTA];  // Shift to get Frame Register value.
EXTERN BT_U16BIT bt_PtrShift[MAX_BTA];    // Shift FR for index to addr pointer.
EXTERN BT_U32BIT bt_FrameMask[MAX_BTA];   // Frame register mask.
EXTERN BT_U16BIT bt_FrameReg[MAX_BTA];    // Current frame register setting.

#ifdef _64_BIT_
EXTERN CEI_UINT64 bt_iobase[MAX_BTA];      // I/O addresses, per board.
#else
EXTERN BT_U32BIT bt_iobase[MAX_BTA];      // I/O addresses, per board.
#endif //_64_BIT_
EXTERN BT_U32BIT MemoryMapSize[MAX_BTA];  // Size of region mapped by carrier.
EXTERN DEVMAP_T bt_devmap[MAX_BTA];       // Struture for vbtMapBoardAddresses

#if defined(__WIN32__)
void vbtAcquireFrameRegister(
   BT_UINT   cardnum,           // (i) card number.
   BT_UINT   flag);             // (i) 1=Acquire Frame Critical Section,
                                //     0=Release Critical Section.
#else
#define vbtAcquireFrameRegister(p1,p2)
#endif

/******************************************************************************/
/* Module Private Function Definitions, shared with HWSETUP.C                 */
/******************************************************************************/
EXTERN BT_INT vbtBoardAccessSetup(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT phys_addr);

LPSTR vbtMapPage(
   BT_UINT    cardnum,       // (i) card number
   BT_U32BIT  offset,        // (i) byte address within adapter memory
   BT_U32BIT *pagebytes,     // (o) number of bytes remaining in page
   BT_U16BIT *framereg);     // (o) value of frame register which maps page

void vbtSetFrame(BT_UINT cardnum, BT_U16BIT frame);

BT_INT vbtMapUserBoardAddress(BT_UINT cardnum, BT_INT wOpenFlag,
                              BT_U32BIT phys_addr, BT_U32BIT io_addr,
                              char * UserDLLName);

BT_INT vbtPageAccessSetupPC(BT_UINT cardnum, char *lpbase);

BT_INT vbtPageAccessSetupISA_IP(BT_UINT cardnum, char *lpbase);

BT_INT vbtPageAccessSetupPCI_IP(BT_UINT cardnum, char *lpbase, BT_U32BIT phys_addr);

BT_INT vbtPageAccessSetupVME_IP(BT_UINT cardnum);

BT_INT vbtPageAccessSetupVXW_IP(BT_UINT cardnum, BT_U32BIT phys_addr);

BT_INT vbtPageAccessSetupPCI(BT_UINT cardnum, char *lpbase);

BT_INT vbtPageAccessSetupISA(BT_UINT cardnum, char *lpbase);

BT_INT vbtPageAccessSetupPCC(BT_UINT cardnum, char *lpbase);

#ifdef BM_EXPRESS
BT_INT vbtPageAccessSetupAR15(BT_UINT cardnum, char *lpbase);
#endif //BM_EXPRESS

#ifdef INCLUDE_PCCD
BT_INT vbtPageAccessSetupPCCD(BT_UINT cardnum, char *lpbase);
#endif //INCLUDE_VME_VXI_1553

#ifdef INCLUDE_VME_VXI_1553
BT_INT vbtPageAccessSetupVME(BT_UINT cardnum, unsigned mem_addr, char *lpbase);
#endif

BT_INT vbtPageAccessSetupQPMC(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupQPCI(BT_UINT, char *);
BT_INT vbtPageAccessSetupQCP(BT_UINT, char *);
BT_INT vbtPageAccessSetupQ104(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupIPD(BT_UINT cardnum, BT_U32BIT phys_addr, char *lpbase);
BT_INT vbtPageAccessSetupR15EC(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupRXMC(BT_UINT cardnum, char *lpbase);
void vbtDmaInit(BT_INT);


#if defined(DEMO_CODE)
BT_INT vbtDemoSetup(BT_UINT cardnum);
BT_INT vbtDemoShutdown(BT_UINT cardnum);
#else
#define vbtDemoSetup(p1)
#define vbtDemoShutdown(p1)
#endif

void get_cardID(CEI_UINT *cndx);
void get_dev_chan(CEI_UINT *dchan);
void get_dev_id(CEI_UINT *devid);


