 /*============================================================================*
 * FILE:                        H W S E T U P . C
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
 *             Low-level board setup component.
 *
 *             This module performs the setup of the address pointers and the
 *             loading of the WCS and FPGA configurations.
 *
 * DESCRIPTION:  See vbtSetup() for a description of the functions performed
 *               by this module.
 *
 * BTDRV ENTRY POINTS: (only used by btdrv.c):
 *    vbtPageAccessSetupPCI   Setup adapter page access data pointers for the
 *                            PCI-1553 on PCI bus.
 *    vbtPageAccessSetupISA   Setup adapter page access data pointers for the
 *                            ISA-1553 on ISA bus.
 *    vbtPageAccessSetupPCC   Setup adapter page access data pointers for the
 *                            PCCard-1553 on ISA bus.
 *    vbtPageAccessSetupPCCD  Setup adapter page access data pointers for the
 *                            PCCard-D1553 on ISA bus.
 *    vbtPageAccessSetupVME   Setup adapter page access data pointers for the
 *                            VME-1553 on VME bus.
 *    vbtPageAccessSetupQPMC  Setup adapter page access data pointers for the
 *                            QPMC-1553, QPM-1553 and AMC-1553 on PCI bus.
 *    vbtPageAccessSetupQPCI  Setup adapter page access data pointers for the
 *                            QPCI-1553 and QPCX-1553 on PCI bus.
 *    vbtPageAccessSetupQCP   Setup adapter page access data pointers for the
 *                            QCP-1553 on PCI bus.
 *    vbtPageAccessSetupIPD   Setup adapter page access data pointers for the
 *                            IP-D1553 on PCI40, VIPC616/618 and AVME9660.
 *    vbtPageAccessSetupQ104  Setup adapter page access data pointers for the
 *                            Q104-1553 on ISA bus oe Q104-1553P on PCI bus.
 *    vbtPageAccessSetupR15EC Setup adapter page access data pointers for the
 *                            R15-EC the express bus.
 *    vbtPageAccessSetupRXMC Setup adapter page access data pointers for the
 *                            R15-EC the express bus.
 *
 * LOCAL FUNCTIONS:
 *    vbtReadWCSWordPaged  Read specified word from paged WCS (IP/ISA-1553).
 *    vbtWriteWCSWordPaged Load specified word into paged WCS (IP/ISA-1553).
 *    vbtLoadPagedWCS      Load IP/ISA-1553 WCS with specified version.
 *    vbtLoadPCIWCS        Loads the WCS into the PCI/PMC-1553 boards
 *    veryShortDelay       Two microsecond FPGA load delay.
 *    vbtLoad_1553fpga     Loads the FPGA configuration data into the ISA-1553
 *                         or PCI/PMC/cPCI-1553 board.
 *    vbtLoad_vmefpga      Loads the FPGA/WCS configuration data into the VME-1553 board
 *    vbtVerifyIPD_IDPROM  Verifies the ID PROM of an IPD module.
 *    vbtIRIGCal           Calibrates external IRIG signal
 *    vbtIRIGConfig        Configures the IRIG setting.
 *    vbtIRIGValid         Reads and returns the IRIG valid bit.
 *    vbtIRIGWriteDAC      Write to the IRIG DAC register.
 *    vbtIRIGSetTime       Sets the internal IRIG time.
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
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

// Define the WCS load

#ifdef INCLUDE_LEGACY_PCI
static BT_U32BIT PCI_FpgaData[] = {  /* Define the array and its     */
#include "lpu_388.h"                          /*  contents which initializes  */
                                           }; /*   the PCI/ISA FPGA firmware. */

static unsigned short WCS_LoadData[] = {  /* Define the WCS data for the  */
#include "ceWCS388.h"                         /*  PCI-1553, ISA-1553 and      */
                                           };
#endif //#ifdef INCLUDE_LEGACY_PCI

#ifdef INCLUDE_PCCD
static unsigned char PCCD_FpgaData[] = {  /* Define the array and its */
#include "lpu_pccd_440.h"                 /*  contents which initializes  */
                                           }; /*   the PCI/ISA FPGA firmware. */
#endif //INCLUDE_PCCD

#ifdef INCLUDE_VME_VXI_1553
static unsigned char VME_FpgaData[] = {  /* Define the array and its     */
#include "lpu_vme_393.h"                 /*  contents which initializes  */
                                      }; /*   the PCI/ISA FPGA firmware. */
#endif //INCLUDE_VME_VXI_1553

#define MAX_ID_LEN  0x14
static char IP_ID_PROM[MAX_BTA][MAX_ID_LEN];

/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t V e r i f y I P D _ I D P R O M
 *===========================================================================*
 *
 * FUNCTION:    Verify that this is a real IP-D1553.
 *              As an additional test, read and verify the ID bits in the
 *              IO page register.
 *
 * DESCRIPTION: Setup the page management variables for an IP-D1553.
 *              Read and verify the Manufacturer ID, the Single/Multi-function
 *              Model Number, and the Revision.
 *              The Revision number indicates which firmware version gets loaded
 *              into the WCS, and also determines the features of the IP.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *              BTD_BAD_MANUFACTURER
 *              BTD_BAD_MODEL
 *              BTD_MODE_MISMATCH;
 *===========================================================================*/

int vbtVerifyIPD_IDPROM(BT_UINT cardnum)
{
   BT_U16BIT  temp;     // Temp value read from IP ID PROM space.

   int        i;        // Loop counter.
   BT_U16BIT  numChars; // Number of characters in the ID PROM space.

   numChars = *(BT_U16BIT *)(IPD_IDPROM_ADDR[cardnum] + 0x14);
   if ( numChars > MAX_ID_LEN )
      numChars = MAX_ID_LEN;
   for ( i = 0; i < numChars; i++ )        // ID PROM always readable.V3.30.
   {  // Try to make sure that the compiler generates WORD accesses!
      temp = *(BT_U16BIT *)(IPD_IDPROM_ADDR[cardnum]+2*i);
      IP_ID_PROM[cardnum][i] = (char) temp;
   }

   /****************************************************************
   *  Read the ID PROM and verify the Manufacturer ID.  Always read
   *  WORDS from the ID PROM to avoid big- and little-endin problems.
   ****************************************************************/
   temp = *(BT_U16BIT *)(IPD_IDPROM_ADDR[cardnum]+8);   // Manufacturer ID
   temp &= 0x00FF;      // V4.25
   if ( temp != 0x079 )
      return BTD_BAD_MANUFACTURER;

   /****************************************************************
   *  Read the ID PROM & verify Single/Multi-function Model Number.
   ****************************************************************/
   temp = *(BT_U16BIT *)(IPD_IDPROM_ADDR[cardnum]+10);   // Model ID
   temp &= 0x00FF;      // V4.25
   //     Multi-Function / Single-Function check
   if ( temp == 0xe || temp == 0xc )
   {
      if(_HW_1Function[cardnum] != 0x0)
         return BTD_MODE_MISMATCH;
   }
   else if ( temp == 0xd || temp == 0xf )
   {
      if(_HW_1Function[cardnum] != 0x1)
         return BTD_MODE_MISMATCH;
   }
   else
         return BTD_BAD_MODEL;

   /****************************************************************
   *  Now read the serial PROM revision level.
   ****************************************************************/

   _HW_PROMRev[cardnum] = *(BT_U16BIT *)(IPD_IDPROM_ADDR[cardnum]+12);

   // Report the complete FPGA version ID.
   _HW_FPGARev[cardnum] = *(BT_U16BIT *)(IPD_IDPROM_ADDR[cardnum]+0x20)*100 +
                          *(BT_U16BIT *)(IPD_IDPROM_ADDR[cardnum]+0x22);

   /*******************************************************************
   *  Setup the firmware load version for this version of the
   *   IP-D1553 hardware serial prom.
   *******************************************************************/

   if ((_HW_PROMRev[cardnum] == 1) ||  (_HW_PROMRev[cardnum] == 8))
   {
      return BTD_OK;
   }
   else
   {
      return BTD_BAD_SERIAL_PROM;              // Old version not supported
   }   
}
#ifdef INCLUDE_LEGACY_PCI
/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t R e a d W C S W o r d P a g e d
 *===========================================================================*
 *
 * FUNCTION:    Read a single word from the WCS.
 *
 * DESCRIPTION: Special routine for reading the Writable Control Store on
 *              boards with a paged WCS.	 
 *
 *      It will return:
 *              Nothing.
 *===========================================================================*/
static void vbtReadWCSWordPaged(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *buffer,       // (o) buffer to receive 2 bytes of data from WCS
   BT_U32BIT byteOffset)    // (i) offset within adapter WCS memory to write
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT   *lpword;
   BT_U32BIT    pagebytes;  // Can be up to the size of a frame, 2Kb
   BT_U16BIT    framenum;

   /*******************************************************************
   * Given a byte offset within adapter memory, setup the page and
   *  the pointer within the page.
   *******************************************************************/
   lpword = (LPWORD)vbtMapPage(cardnum,byteOffset,&pagebytes,&framenum);

   // Set frame to WCS frame number returned by GetPagePtr, plus WCS access.
   vbtSetFrame(cardnum, (BT_U16BIT)(framenum | IO_CONFIG_WCS));

   *buffer = *lpword;   // Read the specified WCS word.
   
   return;
}

/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t W r i t e W C S W o r d P a g e d
 *===========================================================================*
 *
 * FUNCTION:    Write a single word to the WCS.
 *
 * DESCRIPTION: Special routine for loading the Writable Control Store on
 *              boards with a paged WCS.
 *
 *      It will return:
 *              Nothing.
 *===========================================================================*/

static void vbtWriteWCSWordPaged(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT buffer,        // (i) buffer to copy 2 bytes of data from
   BT_U32BIT byteOffset)    // (i) offset within adapter WCS memory to write
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   LPWORD       lpword;
   BT_U32BIT    pagebytes;
   BT_U16BIT    framenum;

   /*******************************************************************
   * Given a byte offset within adapter memory, setup the page and
   *  the pointer within the page.
   *******************************************************************/
   lpword = (LPWORD)vbtMapPage(cardnum,byteOffset,&pagebytes,&framenum);

   // Set frame to WCS frame number returned by GetPagePtr, plus WCS access.
   vbtSetFrame(cardnum, (BT_U16BIT)(framenum | IO_CONFIG_WCS));

   *lpword = buffer;  // Write the specified WCS word.
   return;
}

/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t L o a d P a g e d W C S
 *===========================================================================*
 *
 * FUNCTION:    Load the Paged WCS and read back the data to verify correct 
 *              loading.
 *
 * DESCRIPTION: Load the WCS from the caller-specified array, using the
 *              specified number of words.
 *              Only the amount of WCS being used is loaded or tested.
 *              Uses helper functions to actually read or write the WCS.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *              BTD_ERR_BADWCS if error reading back the WCS
 *===========================================================================*/
static BT_INT vbtLoadPagedWCS(
   BT_UINT         cardnum,       // (i) card number (0 - based)
   unsigned short *WCS_LoadData,  // (i) pointer to WCS data to load
   BT_UINT         nLoadData_Len) // (i) number of WCS bytes to load.V4.11.ajh
{
   BT_UINT   i;              // Index through "IP_LoadData".
   int       j;              // Index the three words then skip one.
   BT_U32BIT WCS_Offset;     // Address within the WCS.
   BT_U16BIT buffer, buf2;   // Data read back from the WCS.
   BT_INT    status;         // Return status.
   int       first_time = 0; // Output file has not been created.

   // Load the microcode from the array "WCS_LoadData" into the specified board.
   WCS_Offset = 0;           // Start at the beginning of the WCS.
   for ( i = 0; i < nLoadData_Len/2; i += 3 )
   {
      for ( j = 0; j < 3; j++ )
         vbtWriteWCSWordPaged(cardnum, WCS_LoadData[i+j], WCS_Offset+j+j);
      WCS_Offset += 8;       // WCS 48-bit words are located on 64-bit boundries.
   }

   // Read back and verify the microcode from the board.
   WCS_Offset = 0;           // Start at the beginning again.
   status = BTD_OK;

   for ( i = 0; i < nLoadData_Len/2; i += 3 )
   {
      for ( j = 0; j < 3; j++ )
      {
         vbtReadWCSWordPaged(cardnum, &buffer, WCS_Offset+j+j);
         if ( buffer != WCS_LoadData[i+j] )
         {  // Report the offset in WCS memory, rather than the offset in the
            //  WCS data array, as the memory error address.V4.27.ajh
            vbtReadWCSWordPaged(cardnum, &buf2, WCS_Offset+j+j);
            API_ReportMemErrors(cardnum, 0, WCS_Offset+j+j, 1,
                                &WCS_LoadData[i+j], &buffer, &buf2, &first_time);
            status = BTD_ERR_BADWCS;
         }
      }
      WCS_Offset += 8;      // WCS 48-bit words are located on 64-bit boundries.
   }
   first_time = -1;
   API_ReportMemErrors(cardnum, 0, 0, 0, WCS_LoadData, &buffer, &buf2, &first_time);
   vbtSetFrame(cardnum, IO_RUN_STATE);  // Switch access from WCS to data space.
   return status;
}

/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t L o a d P C I W C S
 *===========================================================================*
 *
 * FUNCTION:    This function loads the PCI-1553 WCS with the firmware, and
 *              reads it back to verify it.  Do not perform a very good test,
 *              since the hardware will fail.  So if it is close, it's good
 *              enough... :-(
 *              If it fails somewhere else, it will be a real trick to
 *              debug.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/

static int vbtLoadPCIWCS(
   BT_UINT         cardnum,   // (i) card number (0 based)
   char           *lpbase,    // (i) pointer to board WCS area
   unsigned short *load_data, // (i) pointer to WCS data array
   BT_UINT         nLength)   // (i) length of WCS data in bytes.V4.11.ajh
{
   BT_U16BIT *wcs_ptr;   // Pointer to the wcs area.
   BT_U16BIT  pattern;   // First read of wcs
   BT_UINT         i;         // Loop counter and WCS array index.
   int             j;         // WCS board word index.
   int        first_time = 0; // Output file has not been created.
   BT_INT          status;    // Return ststus from this function.

   /*******************************************************************
   * Convert the WCS address to a short word address.
   *******************************************************************/
   wcs_ptr = (unsigned short *)(lpbase);

   /*******************************************************************
   *  Now copy the WCS from the array to the board.
   *******************************************************************/
   for ( i = 0, j = 0; i <  nLength/2; i++, j++ )
   {
      wcs_ptr[j] = flipws(load_data[i]);
      if ( (j & 0x0003) == 2 ) j++;  // Write three words then skip a word.
   }
   /*******************************************************************
   *  Now read back the WCS and see if it matches what we wrote.
   *  Give it two chances to read back correctly.Per Corey.V3.30.ajh
   *******************************************************************/
   status = BTD_OK;
   for ( i = 0, j = 0; i < nLength/2; i++, j++ )
   {
      pattern = flipws(wcs_ptr[j]);
      if ( pattern != load_data[i] )
      {  // Report the offset in WCS memory, rather than the offset in the
         //  WCS data array, as the memory error address.V4.27.ajh
         API_ReportMemErrors(cardnum, 0, j*2, 1, &load_data[i], &pattern,
                             &wcs_ptr[j], &first_time);
         status = BTD_ERR_BADWCS; // Read back failure
      }
      if ( (j & 0x0003) == 2 ) j++;      // Read three words then skip a word.
   }
   first_time = -1;  // Close the output file, if one was created.
   API_ReportMemErrors(cardnum, 0, 0, 0, load_data, &pattern, wcs_ptr, &first_time);
   return status;
}

/*===========================================================================*
 * LOCAL ENTRY POINT:      v e r y S h o r t D e l a y
 *===========================================================================*
 *
 * FUNCTION:     This function does a Bus read to generate a very short delay.
 *
 *===========================================================================*/
static BT_U32BIT veryShortDelay(volatile BT_U32BIT *board_long_addr)
{
   // Use the PCI/ISA bus access timing to generate a short delay
   //  which is relatively independent of the processor speed.
   return board_long_addr[HIR_CSC_REG];
}

/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t L o a d _ 1 5 5 3 f p g a
 *===========================================================================*
 *
 * FUNCTION:     This function loads the Altera Flex 10K part on the PCI-1553,
 *               cPCI-1553, PMC-1553 or the ISA-1553 with the FPGA config data.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
static int vbtLoad_1553fpga(
   volatile BT_U32BIT *board_long_addr, // (i) address of host interface registers
   BT_U32BIT *PCI_FpgaData)             // (i) pointer to FPGA data array
{
   BT_U32BIT input_long;        // Dword of data to write to 10K device.
   int     i, j, k;                 // Loop counters.
//   unsigned char * ConfigDataReg;   // WORD address used to write FPGA config data

#ifdef WORD_SWAP
#define ASSERT_RESET 0x07000000
#define ENABLE_10K   0x00000100
#define RESET_10K    0x00000400
#else
#define ASSERT_RESET 0x00000007
#define ENABLE_10K   0x00010000
#define RESET_10K    0x00040000
#endif

   /***********************************************************************
   *  Reset the FPGA by writing to the Control/Status/Config register.
   ***********************************************************************/
   board_long_addr[HIR_CSC_REG] = ASSERT_RESET;   // Assert reset of 10K device:
                                                  // Reset 7K and LPUs

   // Delay for 2 microseconds.  Use the PCI bus access timing to generate
   // a short delay which is relatively independent of the processor speed.
   veryShortDelay(board_long_addr);
   veryShortDelay(board_long_addr);             // Added 3/5/00.V4.02.ajh

   board_long_addr[HIR_CSC_REG] = ENABLE_10K;   // Enable 10K configuration.

   // Delay for 3 microseconds before loading the config
   veryShortDelay(board_long_addr);             // Added 3/5/00.V4.02.ajh
   veryShortDelay(board_long_addr);             // Added 3/5/00.V4.02.ajh
   veryShortDelay(board_long_addr);             // Added 3/5/00.V4.02.ajh

   // Check CSC Register for successful 10K configuration reset.  If it
   //  did not reset we have a problem.

    if ( (board_long_addr[HIR_CSC_REG] & RESET_10K) != 0)
      return BTD_FPGA_NOT_CLEAR;         // Driver-FPGA failure to clear
   /***********************************************************************
   * Load the FPGA Configuration from our array one bit at a time.  Note
   *   that the array has the data for the first FPGA, the second one gets
   *   the same load by us loading it a second time...
   * Each FPGA load is exactly 77655 bytes long.  We load this data from
   *   exactly 19414 longs.  We must load exactly 24 bits of the final long
   *   into the first FPGA, no more, for this stick to work.
   * We could probably make this stuff parametric, but the hard code is
   *   easier to read and test.  And the hardware is not likely to change.
   ***********************************************************************/
   //ConfigDataReg = (unsigned char *)&board_long_addr[HIR_CONFIG_10K];

   for ( k = 0; k < 2; k++ )        // Load two FPGA's from one file.
   {
      for ( i = 0; i < 19413; i++ )    // Load the first FPGA, then the second
      {
         input_long = PCI_FpgaData[i];   // Get the next long to write from array
         for ( j = 0; j < 32; j++ )
         {
            board_long_addr[HIR_CONFIG_10K] = input_long; // Write LSB of dword.
           // ConfigDataReg[0] = (unsigned char)input_long;   // Write LSB of dword.
            input_long >>= 1;                               // Shift right one bit.
         }
      }

      input_long = PCI_FpgaData[19413];   // Get the last long from array.
      for ( j = 0; j < 24; j++ )          // Write only the last 24 bits.
      {
         board_long_addr[HIR_CONFIG_10K] = input_long; // Write LSB of dword.
        // ConfigDataReg[0] = (unsigned char)input_long;   // Write LSB of dword.
         input_long >>= 1;                               // Shift right one bit.
      }
   }
   for ( j = 0; j < 10; j++ )          // Write 10 more bits for 10K, 40 for APEX.
   {
       board_long_addr[HIR_CONFIG_10K]=0;
     //ConfigDataReg[0] = 0;            // Data is don't care.
   }

    // Check CSC Register for successful 10K configuration load.
   if ( (board_long_addr[HIR_CSC_REG] & RESET_10K) == 1 )
       return BTD_ERR_FPGALOAD;         // Driver-FPGA load failure

   return BTD_OK;
}
#endif //INCLUDE_LEGACY_PCI

#ifdef INCLUDE_VME_VXI_1553
/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t L o a d _ v m e f p g a
 *===========================================================================*
 *
 * FUNCTION:     This function loads the Altera Flex 10K part on the VME-1553,
 *               with the FPGA config data.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
static int vbtLoad_vmefpga(
   volatile short *board_addr,   // (i) address of host interface registers
   unsigned char *FpgaData) // (i) pointer to FPGA data array
{
   unsigned char input_char;        // Dword of data to write to 10K device.
   int     i, j;                 // Loop counters.
   int status;

   //int mask[8] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
   int data[2] = {0x0,0xf};


   /***********************************************************************
   *  Reset the FPGA by writing to the Control/Status/Config register.
   ***********************************************************************/

   board_addr[5] = 0x0;   // enable APEX write
   board_addr[5] = 0x3;

   // Check CSC Register for successful 10K configuration reset.  If it
   //  did not reset we have a problem.
   if ( (board_addr[5] & 0x4) != 0 )
      return BTD_FPGA_NOT_CLEAR;         // Driver-FPGA failure to clear

   for ( i = 0; i < sizeof(VME_FpgaData); i++ )    // Load the first FPGA, then the second
   {
      input_char = FpgaData[i];   // Get the next long to write from array.
      for ( j = 0; j < 8; j++ )
      {
         board_addr[6] = (short)(data[(input_char & 0x0001)]);
		 input_char>>=1;
      }
   }

   for ( i = 0; i < 40; i++ )          // Write 40 more bits for fun.
      board_addr[6] = 0;               // Data is don't care.

   // Check CSC Register for successful 10K configuration load.
   if ( (board_addr[5] & 0x4) == 0 )
      status = BTD_ERR_FPGALOAD;    // Driver-FPGA load failure
   else
	  status = BTD_OK;

   return status;
}
#endif

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

#ifdef INCLUDE_LEGACY_PCI
/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p P C I
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the specified PCI-1553
 *              native PCI bus board.  This board is flat-mapped (no frames)
 *              and supports two MIL-STD-1553 interfaces on a single board.
 *              Load the FPGA if possible, then load the WCS for the
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
BT_INT vbtPageAccessSetupPCI(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   int       status;                 // Function error return code.
   volatile  BT_U32BIT *board_long_addr;  // DWORD pointer to board CSC Register.

#ifdef WORD_SWAP
#define POWER_UP 0xC0000000
#define CHANNEL_A 0x00100000
#define CHANNEL_A_ON 0x40000000
#define CHANNEL_B 0x00200000
#define CHANNEL_B_ON 0x80000000
#define CHANNEL_A_SF 0x00010000
#define CHANNEL_B_SF 0x00020000
#define CHANNEL_A_WCS 0x50000000
#define CHANNEL_B_WCS 0xA0000000
#else
#define POWER_UP 0x000000C0
#define CHANNEL_A 0x00001000
#define CHANNEL_A_ON 0x00000040
#define CHANNEL_B 0x00002000
#define CHANNEL_B_ON 0x00000080
#define CHANNEL_A_SF 0x00000100
#define CHANNEL_B_SF 0x00000200
#define CHANNEL_A_WCS 0x00000050
#define CHANNEL_B_WCS 0x000000A0
#endif

   /**********************************************************************
   *  Read the PCI Revision ID from the PCI Configuration space.
   **********************************************************************/

   status = vbtGetPCIConfigRegister(api_device[cardnum], 0x08, 1, &_HW_PROMRev[cardnum]);
   if ( status )
      return status;
   	   
   /**********************************************************************
   *  Setup the PCI address of the boards dual-port, Hardware Registers
   *    and RAM Registers.  The first interface is SLOT_A, and the
   *    second interface is SLOT_B.  The Host Interface Registers
   *    pointer is also used to map the WCS (@+0x600000)
   **********************************************************************/
   CurrentMemKB[cardnum]   = BT_PCI_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   if ( CurrentCardSlot[cardnum] == SLOT_A )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_PCI + DATA_RAM_PCI; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_PCI + HW_REG_PCI;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_PCI + REG_FILE_PCI; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_PCI;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == SLOT_B )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_PCI + DATA_RAM_PCI; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_PCI + HW_REG_PCI;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_PCI + REG_FILE_PCI; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_PCI;     // Host Interface Registers
   }
   else
      return BTD_ERR_PARAM;     // Invalid slot specified	   
    
   /*******************************************************************
   *  Setup the board (reset, configure lpu's, enable power, load wcs).
   *  If both sides are powered DOWN than neither side is in use,
   *   reload the FPGA and WCS for both sides.                // V3.30
   *******************************************************************/
   board_long_addr = (BT_U32BIT *)bt_PageAddr[cardnum][3];	   

   if ( (board_long_addr[HIR_CSC_REG] & POWER_UP ) == 0 )
   {
      /****************************************************************
      *  Load the FPGA configuration from PCI_FpgaData.
      ****************************************************************/
      bt_PageAddr[cardnum][3][HIR_CSC_REG] = 0x0f; // Reset and power down.   

      status = vbtLoad_1553fpga(board_long_addr, PCI_FpgaData);
      if ( status != BTD_OK )
         return status;
      /****************************************************************
      * Load WCS into Bus 1 if Bus 1 interface is present.
      ****************************************************************/

      if ( board_long_addr[HIR_CSC_REG] & CHANNEL_A )  //If Chan A is present...
      {
         // Clear the Bus 1 WCS Setup Complete bit.
         bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0xe0;
         // Power Bus 1 on:
         board_long_addr[HIR_CSC_REG] |= CHANNEL_A_ON;

         if ( vbtLoadPCIWCS(cardnum,
                      bt_PageAddr[cardnum][3] + CHAN1_PCI + WCS_RAM_PCI,
                      WCS_LoadData, sizeof(WCS_LoadData)) )
         {
           // Power Bus 1 off and clear WCS Setup Complete, return error.
           bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0xaf;
           return BTD_ERR_BADWCS;
         }
         // Power Bus 1 off and clear WCS Setup Complete
         bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0xaf;
      }

      /****************************************************************
      * Load WCS into Bus 2 if Bus 2 interface is present.
      ****************************************************************/
      if ( board_long_addr[HIR_CSC_REG] & CHANNEL_B )  // If Chan B is present...
      {
         // Clear the Bus 2 WCS Setup Complete bit.
         bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0xd0;
         // Power Bus 2 on:
         board_long_addr[HIR_CSC_REG] |= CHANNEL_B_ON;
         if ( vbtLoadPCIWCS(cardnum,
                       bt_PageAddr[cardnum][3] + CHAN2_PCI + WCS_RAM_PCI,
                       WCS_LoadData, sizeof(WCS_LoadData)) )
         {
           // Power Bus 2 off and clear WCS Setup Complete, return error:
           bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0x5F;
           return BTD_ERR_BADWCS;
         }
         // Power Bus 2 off and clear WCS Setup Complete
         bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0x5F;
      }
   }

   /*******************************************************************
   * PCI-1553 power on, set WCS Setup Complete on the specified channel.
   *******************************************************************/
   if ( CurrentCardSlot[cardnum] == SLOT_A )
   {
      // Set the Single Function flag if this is a single function board.
      _HW_1Function[cardnum] = board_long_addr[HIR_CSC_REG] & CHANNEL_A_SF ? 0 : 1;  //00000100
      if ( board_long_addr[HIR_CSC_REG] & CHANNEL_A )  // If Chan A is present... //00001000
         board_long_addr[HIR_CSC_REG] |=  CHANNEL_A_WCS; // Bus 1 Power On, WCS Complete //00000050
      else
         return BTD_CHAN_NOT_PRESENT;   // Channel not present to load!!!!
   }
   else if ( CurrentCardSlot[cardnum] == SLOT_B )
   {
      // Set the Single Function flag if this is a single function board.
      _HW_1Function[cardnum] = board_long_addr[HIR_CSC_REG] & CHANNEL_B_SF ? 0 : 1; //00000200
      if ( board_long_addr[HIR_CSC_REG] & CHANNEL_B )  // If Chan B is present...//00002000
         board_long_addr[HIR_CSC_REG] |= CHANNEL_B_WCS; // Bus 2 Power On, WCS Complete //000000A0
      else
         return BTD_CHAN_NOT_PRESENT;   // Channel not present to load!!!!
   }
   else
      return BTD_CHAN_NOT_PRESENT;      // Unknown Channel to load!!!!

   MSDELAY(1);
   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] + 0x12);

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p I S A
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the specified ISA-1553
 *              native ISA bus board.  This board is pagged in memory space
 *              (no I/O ports) and supports two MIL-STD-1553 interfaces on
 *              a single board.  Load the FPGA if the board has not been
 *              initialized, then the WCS for the current channel.
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
BT_INT vbtPageAccessSetupISA(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   int       status;           // Function error return code.
   volatile  BT_U32BIT *board_long_addr;  // DWORD pointer to board CSC Register.
   short    *board_ID_reg;     // Pointer to the jumper/revision_ID Register
   BT_INT    current_channel;  // Saved channel number we are opening.

   /**********************************************************************
   *  Read the ISA-1553 Revision ID from the jumpers/revision_id
   *   register.  Return with error if not a supported version.
   **********************************************************************/
   board_ID_reg = (short *)(lpbase + 0x0A);
   _HW_PROMRev[cardnum] = board_ID_reg[0] & 0x00ff;
   if ( _HW_PROMRev[cardnum] > 0x06 )
      return BTD_ERR_NODETECT;       //vbtPageAccessSetupISA Invalid revision ID indicates no board.V4.13.ajh

   if ( (*(lpbase + 1) & 0xC0) != 0x40 )
      return BTD_ERR_NODETECT;       //vbtPageAccessSetupISA Bad ISA bus ID indicates no board.V4.13.ajh

   /*********************************************************************
   *  Setup the ISA-1553 frame access parameters.  Since the ISA-1553
   *   dual port memory always mapped in 2K byte pages starting at a
   *   single address, these values are constants.
   *********************************************************************/
   bt_OffsetMask[cardnum] = BT_02KBOFFSETMASK_ISA;
   bt_FrameShift[cardnum] = BT_02KBFRAMESHIFT_ISA;
   bt_PtrShift[cardnum]   = BT_02KBPTRSHIFT_ISA;
   bt_FrameMask[cardnum]  = BT_FRAME_MASK_ISA;

   /**********************************************************************
   *  Setup the ISA address of the boards dual-port, Hardware
   *    Registers and RAM Registers.  The first interface is SLOT_A,
   *    and the second interface is SLOT_B.  The Host Interface
   *    Registers pointer is also used to map the page register.
   **********************************************************************/
   CurrentMemKB[cardnum]   = BT_ISA_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   if ( CurrentCardSlot[cardnum] == SLOT_A )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_ISA + DATA_RAM_ISA; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_ISA + HW_REG_ISA;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_ISA + REG_FILE_ISA; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_ISA;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == SLOT_B )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_ISA + DATA_RAM_ISA; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_ISA + HW_REG_ISA;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_ISA + REG_FILE_ISA; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_ISA;     // Host Interface Registers
   }
   else
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   /*******************************************************************
   *  Setup the board (reset, configure lpu's, enable power, load wcs).
   *  If both sides are powered DOWN than neither side is in use,
   *   reload the FPGA and WCS for both sides.                // V3.30
   *******************************************************************/
   board_long_addr = (BT_U32BIT *)lpbase;
   if ( (board_long_addr[HIR_CSC_REG] & 0x000000C0) == 0 )
   {
      /****************************************************************
      *  Load the FPGA from PCI_FpgaData into the ISA-1553.
      ****************************************************************/
      bt_PageAddr[cardnum][3][HIR_CSC_REG] = 0x0f; // Reset and power down.
      status = vbtLoad_1553fpga(board_long_addr, PCI_FpgaData);
      if ( status != BTD_OK )
         return status;
      /****************************************************************
      * Load PCI/ISA-1553 WCS into Bus 1 if Bus 1 interface is present.
      ****************************************************************/
      current_channel = CurrentCardSlot[cardnum];
      if ( board_long_addr[HIR_CSC_REG] & 0x00001000 )  // If Chan A is present...
      {
         // Clear the Bus 1 WCS Setup Complete bit.
         bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0xE0;
         // Power Bus 1 on:
         board_long_addr[HIR_CSC_REG] |= 0x00000040;
         // Set the Dual Port Memory address pointer to point at the WCS.
         bt_PageAddr[cardnum][0] = lpbase + CHAN1_ISA + WCS_RAM_ISA;
         // Load WCS and verify function to load the WCS:
         CurrentCardSlot[cardnum] = SLOT_A;
         status = vbtLoadPagedWCS(cardnum, WCS_LoadData, sizeof(WCS_LoadData));
         CurrentCardSlot[cardnum] = current_channel;
         // Power Bus 1 off and clear WCS Setup Complete
         bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0xAF;
         if ( status )
            return status;
      }

      /****************************************************************
      * Load PCI/ISA-1553 WCS into Bus 2 if Bus 2 interface is present.
      ****************************************************************/
      if ( board_long_addr[HIR_CSC_REG] & 0x00002000 )  // If Chan B is present...
      {
         // Clear the Bus 2 WCS Setup Complete bit.
         bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0xD0;
         // Power Bus 2 on:
         board_long_addr[HIR_CSC_REG] |= 0x00000080;
         // Set the Dual Port Memory address pointer to point at the WCS.
         bt_PageAddr[cardnum][0] = lpbase + CHAN2_ISA + WCS_RAM_ISA;
         // Load WCS and verify function to load the WCS:
         CurrentCardSlot[cardnum] = SLOT_B;
         status = vbtLoadPagedWCS(cardnum, WCS_LoadData, sizeof(WCS_LoadData));
         CurrentCardSlot[cardnum] = current_channel;
         // Power Bus 2 off and clear WCS Setup Complete, return error:
         bt_PageAddr[cardnum][3][HIR_CSC_REG] &= 0x5F;
         if ( status )
            return status;
      }
   }

   /*******************************************************************
   * ISA-1553 power on, set WCS Setup Complete on the specified channel.
   *******************************************************************/
   if ( CurrentCardSlot[cardnum] == SLOT_A )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_ISA + DATA_RAM_ISA; // Dual-Port Memory space
      // Set the Single Function flag if this is a single function board.
      _HW_1Function[cardnum] = board_long_addr[HIR_CSC_REG] & 0x00000100 ? 0 : 1;
      if ( board_long_addr[HIR_CSC_REG] & 0x00001000 )  // If Chan A is present...
         board_long_addr[HIR_CSC_REG] |= 0x00000050; // Bus 1 Power On, WCS Complete
      else
         return BTD_CHAN_NOT_PRESENT;   // Channel not present to load!!!!
   }
   else if ( CurrentCardSlot[cardnum] == SLOT_B )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_ISA + DATA_RAM_ISA; // Dual-Port Memory space
      // Set the Single Function flag if this is a single function board.
      _HW_1Function[cardnum] = board_long_addr[HIR_CSC_REG] & 0x00000200 ? 0 : 1;
      if ( board_long_addr[HIR_CSC_REG] & 0x00002000 )  // If Chan B is present...
         board_long_addr[HIR_CSC_REG] |= 0x000000A0; // Bus 2 Power On, WCS Complete
      else
         return BTD_CHAN_NOT_PRESENT;   // Channel not present to load!!!!
   }
   else
      return BTD_CHAN_NOT_PRESENT;      // Unknown Channel to load!!!!
  
   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);
   board_is_paged[cardnum] = 1;

   return BTD_OK;
}
#endif //#ifdef INCLUDE_LEGACY_PCI

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p P C C
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the specified PCC-1553
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
BT_INT vbtPageAccessSetupPCC(
   BT_UINT   cardnum,          // (i) card number (0 based)
   char     *lpbase)           // (i) board base address in user space
{
   volatile BT_U32BIT *board_long_addr;  // DWORD pointer to board CSC Register.
   short    *board_ID_reg;     // Pointer to the jumper/revision_ID Register
//   int       version;          // Figure out the version ID for a PCCARD.7/26/2001.ajh

   /**********************************************************************
   *  Read the PCC-1553 Revision ID from the jumpers/revision_id
   *   register.  Return with error if not a supported version.
   **********************************************************************/
   board_long_addr = (BT_U32BIT *)lpbase;
   board_ID_reg = (short *)(lpbase + 0x0A);
   _HW_PROMRev[cardnum] = board_ID_reg[0] & 0x00ff;
   if ( _HW_PROMRev[cardnum] > 0x06 )
      return BTD_ERR_NODETECT;       //vbtPageAccessSetupPCC Invalid revision ID indicates no board.V4.13.ajh

   if ( (*(lpbase + 1) & 0xC0) != 0x80 )
   {
      return BTD_ERR_NODETECT;       //vbtPageAccessSetupPCC Bad PCC bus ID indicates no board.V4.30.ajh
   }
    /**********************************************************************
    * Read the WCS/LPU version ID from the LPU Revision Register.  The
    *  version is stored in HEX, but we want decimal...
    *********************************************************************
    version = *(BT_U16BIT *)(lpbase + 0x1012);
    version = (version & 0x000F) + 10  * ((version >> 4) & 0x000F)
                                 + 100 * ((version >> 8) & 0x000F);
   _HW_WCSRev[cardnum]  = version; */

   /*********************************************************************
   *  Setup the PCC-1553 frame access parameters.  Since the ISA-1553
   *   dual port memory always mapped in 2K byte pages starting at a
   *   single address, these values are constants.
   *********************************************************************/
   bt_OffsetMask[cardnum] = BT_02KBOFFSETMASK_ISA;
   bt_FrameShift[cardnum] = BT_02KBFRAMESHIFT_ISA;
   bt_PtrShift[cardnum]   = BT_02KBPTRSHIFT_ISA;
   bt_FrameMask[cardnum]  = BT_FRAME_MASK_ISA;

   /**********************************************************************
   *  Setup the PCC-1553 address of the boards dual-port, Hardware
   *    Registers and RAM Registers.  The Host Interface Register
   *    pointer is also used to map the page register.
   **********************************************************************/
   CurrentMemKB[cardnum]   = BT_ISA_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   bt_PageAddr[cardnum][0] = lpbase + CHAN1_ISA + DATA_RAM_ISA; // DP Memory
   bt_PageAddr[cardnum][1] = lpbase + CHAN1_ISA + HW_REG_ISA;   // HW Registers
   bt_PageAddr[cardnum][2] = lpbase + CHAN1_ISA + REG_FILE_ISA; // RAM Registers
   bt_PageAddr[cardnum][3] = lpbase + CHAN1_ISA;     // Host Interface Registers

   /*******************************************************************
   * PCC-1553 power on.
   *******************************************************************/
   // Set the Single Function flag if this is a single function board.
   _HW_1Function[cardnum] = board_long_addr[HIR_CSC_REG] & 0x00000100 ? 0 : 1;
   if( board_long_addr[HIR_CSC_REG] & 0x00001000 )  // If Chan A is present...
      board_long_addr[HIR_CSC_REG] |= 0x00000050; // Bus 1 Power On.
   else
      return BTD_CHAN_NOT_PRESENT;   // Channel not present to load!!!!
   
   board_has_irig[cardnum] = 0x1000;
   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);
   board_is_paged[cardnum] = 1;

   return BTD_OK;
}

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
   BT_INT i, status;
   
   unsigned short board_config[] = {
	                       0x004c,   // sf 1ch
                           0x008c,   // sf 2ch
                           0x084c,   // mf 1ch
                           0x088c};  // mf 2ch
#define IRIG_FLAG 0x1000
#define HIF_MASK 0x0fff

   BT_U16BIT host_interface;
   BT_UINT num_chans[] = {1,2,1,2};
   BT_INT mode[] = {1,1,0,0};

   /*********************************************************************
   *  Setup the PCC-D1553 frame access parameters.  Since the ISA-1553
   *   dual port memory always mapped in 2K byte pages starting at a
   *   single address, these values are constants.
   *********************************************************************/
   bt_OffsetMask[cardnum] = BT_02KBOFFSETMASK_ISA;
   bt_FrameShift[cardnum] = BT_02KBFRAMESHIFT_ISA;
   bt_PtrShift[cardnum]   = BT_02KBPTRSHIFT_ISA;
   bt_FrameMask[cardnum]  = BT_FRAME_MASK_ISA;

   /**********************************************************************
   *  Setup the PCC-1553 address of the boards dual-port, Hardware
   *    Registers and RAM Registers.  The Host Interface Register
   *    pointer is also used to map the page register.
   **********************************************************************/
   
   CurrentMemKB[cardnum]   = BT_ISA_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   
   if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_PCCD + DATA_RAM_PCCD; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_PCCD + HW_REG_PCCD;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_PCCD + REG_FILE_PCCD; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_PCCD;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_PCCD + DATA_RAM_PCCD; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_PCCD + HW_REG_PCCD;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_PCCD + REG_FILE_PCCD; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_PCCD;     // Host Interface Registers
   }
   else
      return BTD_CHAN_NOT_PRESENT;
 
   board_short_addr = (volatile BT_U16BIT *)bt_PageAddr[cardnum][3];
   /*******************************************************************
   * PCCD-1553 power on and FPGA load.
   *******************************************************************/
   host_interface = board_short_addr[HIR_CSC_REG];
   flipw(&host_interface);  

   board_has_irig[cardnum] = host_interface & IRIG_FLAG;
   host_interface &= HIF_MASK;
 
   board_has_discretes[cardnum]= 0x1;
   board_has_hwrtaddr[cardnum] = 0x0;
   board_has_differential[cardnum] = 0x1; //This so the PCCD trigger can get set. No Differtial I/O suppoprt
   board_is_paged[cardnum] = 1;

   for(i=0;i<num_config;i++)
   {
      if(board_config[i] == host_interface)
         break;
   }

   if (i == num_config)
	   return BTD_ERR_BADBOARDTYPE;

   _HW_1Function[cardnum] = mode[i];

   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT;
 
   numDiscretes[cardnum] = 2;
   bt_dismask[cardnum] = 0xc0;

   if ((board_short_addr[1] & 0x4) == 0) // FPGA not configured so load the FPGA
   {
       status = vbtLoad_pccd_fpga(board_short_addr,PCCD_FpgaData,sizeof(PCCD_FpgaData));
       if ( status != BTD_OK )
          return status;
   }

   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);

   return API_SUCCESS;
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

   int       i,number_of_channels=0;
   int       found = 0;
   unsigned short *vme_config_addr; // word pointer to VME/VXI config registers
   unsigned short vme_config; //
   BT_U16BIT rdata;

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

         number_of_channels = (vme_config & 0x0003)+1;
         found = 1;
      }
   }
   if(found == 0)
   {
	   return BTD_ERR_BADBOARDTYPE;
   }

   CurrentMemKB[cardnum]   = BT_VME_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_VME;    // RT in first seg with BC&BM

   if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_VME + DATA_RAM_VME; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_VME + HW_REG_VME;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_VME + REG_FILE_VME; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_VME;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
   {
	  if(number_of_channels < 2)
		  return BTD_CHAN_NOT_PRESENT;
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_VME + DATA_RAM_VME; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_VME + HW_REG_VME;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_VME + REG_FILE_VME; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_VME;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
   {
      if(number_of_channels < 4)
         return BTD_CHAN_NOT_PRESENT;
      bt_PageAddr[cardnum][0] = lpbase + CHAN3_VME + DATA_RAM_VME; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN3_VME + HW_REG_VME;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN3_VME + REG_FILE_VME; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_VME;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_4 )
   {
      if(number_of_channels < 4)
         return BTD_CHAN_NOT_PRESENT;
      bt_PageAddr[cardnum][0] = lpbase + CHAN4_VME + DATA_RAM_VME; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN4_VME + HW_REG_VME;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN4_VME + REG_FILE_VME; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_VME;     // Host Interface Registers
   }
   else
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   /****************************************************************
   *  Load the FPGA configuration from VME_FpgaData.
   ****************************************************************/
   //set the A24/A32 config bit when defined.
   if(CurrentCarrier[cardnum] == NATIVE_32     || 
      CurrentCarrier[cardnum] == NI_A32_MAP_16 ||
      CurrentCarrier[cardnum] == NI_A32_MAP_32 )
   {
      vme_config_addr[4] |= 0x88; // Select A32 0x88
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
   
   if(CurrentCardType[cardnum] == QVME1553)
      vme_config_addr[2] = (short)BOARD_SET; //un-reset the board and enable A24/A32 addressing

#ifdef INCLUDE_VME_VXI_1553
   else
   {
      if ( (vme_config_addr[5] & 0x4) == 0 ) // FPGA not configured so load the FPGA to all 4 channels
      {
          int status;
          vme_config_addr[2] = (short)BOARD_RESET; //reset the board
          MSDELAY(1);
          vme_config_addr[2] = (short)BOARD_SET; //un-reset the board and enable A24/A32 addressing

          status = vbtLoad_vmefpga(vme_config_addr, VME_FpgaData);
          if ( status != BTD_OK )
             return status;
      }
      vme_config_addr[2] = (short)BOARD_SET; //un-reset the board and enable A24/A32 addressing
   }
#endif

   board_has_testbus[cardnum]=0x1;

#if defined (PPC_SYNC)
   IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
   eieio
   sync
#pragma endasm
#endif

   if(CurrentCardType[cardnum] == QVME1553)
   {
      board_has_discretes[cardnum]= 0x1;
      board_has_hwrtaddr[cardnum] = 0x1;
      numDiscretes[cardnum] = 4;
      bt_dismask[cardnum] = 0xf;

      rdata = vbtGetDiscrete(cardnum,DISREG_HW_RTADDR); 

      if(CurrentCardSlot[cardnum] == 0)
      {
         if((rdata & 0x3) == 0x0)
            hwRTAddr[cardnum] = -1;
         else if((rdata & 0x3) == 0x1)
         {
            hwRTAddr[cardnum] = (vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1))&0x1f;
         }
         else
         {  
	        hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
         }
      }
      else if(CurrentCardSlot[cardnum] == 1)
      {   
         if((rdata & 0x18) == 0x0)
            hwRTAddr[cardnum] = -2;
         else if((rdata & 0x18) == 0x8)
            hwRTAddr[cardnum] = ((vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1)) & 0x1f00)>>8;
         else
         {
            hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
         }
      }
      else if(CurrentCardSlot[cardnum] == 2)
      {   
         if((rdata & 0xc0) == 0x0)
            hwRTAddr[cardnum] = -2;
         else if((rdata & 0xc0) == 0x40)
            hwRTAddr[cardnum] = ((vbtGetDiscrete(cardnum,DISREG_RTADDR_RD2)) & 0x1f);
         else
         {
            return BTD_RTADDR_PARITY;
         }
      }
      else if(CurrentCardSlot[cardnum] == 3)
      {   
         if((rdata & 0x600) == 0x0)
            hwRTAddr[cardnum] = -1;
         else if((rdata & 0x600) == 0x200)
            hwRTAddr[cardnum] = ((vbtGetDiscrete(cardnum,DISREG_RTADDR_RD2)) & 0x1f00)>>8;
         else
         {
            hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
         }
      }
   }
   board_access_32[cardnum] = 0x1;
   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);

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

   if(offset < 0 || offset > 24)
      return BTD_ERR_PARAM;
   if((offset % 2) != 0)
      return BTD_ERR_PARAM;
   
   vme_config_addr[offset/2] = config_data;

   return BTD_OK;
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
   BT_INT       status;                 // Function error return code.
   volatile  BT_U16BIT *board_short_addr;  // DWORD pointer to board CSC Register.
   BT_U16BIT host_interface;
   BT_INT i;
   BT_INT num_config;
   BT_U16BIT rdata,capabilities=0;

   unsigned short board_config[] = {
	                       0x0042,   //QPM     sf 1ch
                           0x0082,   //QPM     sf 2ch
                           0x0102,   //QPM     sf 4ch
                           0x0842,   //QPM     mf 1ch
                           0x0882,   //QPM     mf 2ch
                           0x0902,   //QPM     mf 4ch
                           0x0050,   //AMC     sf 1ch
                           0x0090,   //AMC     sf 2ch
                           0x0110,   //AMC     sf 4ch
                           0x0850,   //AMC     mf 1ch
                           0x0890,   //AMC     mf 2ch
                           0x0910,   //AMC     mf 4ch
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
                           0x0918};  //RPCIe   mf 4ch};  


   #define IRIG_FLAG 0x1000
   #define HIF_MASK 0x0fff
   #define ADDITIONAL_CAPABILITY 0x8000 

   BT_UINT num_chans[] = {1,2,4,1,2,4,1,2,4,1,2,4,1,2,4,1,2,4,1,2,4,1,2,4};
   BT_INT mode[] =       {1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0};

   num_config = sizeof(board_config)/2;
   /**********************************************************************
   *  Read the PCI Revision ID from the PCI Configuration space.
   **********************************************************************/  
   status = vbtGetPCIConfigRegister(api_device[cardnum], 0x08, 1, &_HW_PROMRev[cardnum]);
   if ( status )
     return status;

   /**********************************************************************
   *  Setup the PCI address of the boards dual-port, Hardware Registers
   *    and RAM Registers.  The first interface is SLOT_A, and the
   *    second interface is SLOT_B.  The Host Interface Registers
   *    pointer is also used to map the WCS (@+0x600000)
   **********************************************************************/
   CurrentMemKB[cardnum]   = BT_PCI_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   if(board_is_channel_mapped[cardnum])
   {  
      bt_PageAddr[cardnum][0] = lpbase +  DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase +  HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase +  REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase;     // Host Interface Registers
   } 
   else if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_QPMC + DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_QPMC + HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_QPMC + REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPMC;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_QPMC + DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_QPMC + HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_QPMC + REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPMC;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN3_QPMC + DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN3_QPMC + HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN3_QPMC + REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPMC;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_4)
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN4_QPMC + DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN4_QPMC + HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN4_QPMC + REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPMC;     // Host Interface Registers
   }
   else
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   board_short_addr = (volatile BT_U16BIT *)bt_PageAddr[cardnum][3];

   /**********************************************************************
   * Find out if this is a QPM-1553 or a QPMC-1553
   **********************************************************************/
   host_interface = board_short_addr[HIR_CSC_REG];
   flipw(&host_interface);
  
   if(host_interface & ADDITIONAL_CAPABILITY)
   {
      board_has_acr[cardnum] = 0x1;
      board_has_serial_number[cardnum] = 0x1;
      capabilities = board_short_addr[HIR_AC_REG];
      flipw(&capabilities);
   }
        
   /********************************************************************
   * Check to See if the Selected Channel is present and board is active
   ********************************************************************/  
   board_has_irig[cardnum] = host_interface & IRIG_FLAG;
   host_interface &= HIF_MASK;
   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1;
   board_access_32[cardnum] = 0x1;

   if((capabilities & 0x3) == 2)
   {
      board_has_485_discretes[cardnum]=0x1;
   }

   for(i=0;i<num_config;i++)
   {
      if(board_config[i] == host_interface)
         break;
   }

   if (i == num_config)
	   return BTD_ERR_BADBOARDTYPE;

   if(i > 11)
      ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));

   _HW_1Function[cardnum] = mode[i];

   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT; 

   // Set up discretes and hardwaired RT addressing

   numDiscretes[cardnum] = 18;
   bt_dismask[cardnum] = 0x3ffff;

   rdata = vbtGetDiscrete(cardnum,DISREG_HW_RTADDR); 

   if(CurrentCardSlot[cardnum] == 0)
   {
      board_has_hwrtaddr[cardnum] =0x1;
      if((rdata & 0x3) == 0x0)
         hwRTAddr[cardnum] = -1;
      else if((rdata & 0x3) == 0x1)
      {
         hwRTAddr[cardnum] = (vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1))&0x1f;
      }
      else
      {  
         hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
      }
   }
   else if(CurrentCardSlot[cardnum] == 1)
   {
      board_has_hwrtaddr[cardnum] =0x1;   
      if((rdata & 0x18) == 0x0)
         hwRTAddr[cardnum] = -1;
      else if((rdata & 0x18) == 0x8)
         hwRTAddr[cardnum] = ((vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1)) & 0x1f00)>>8;
      else
      {
         hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
      }
   }
   else
      hwRTAddr[cardnum] = -1;

   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);
   flipw(&_HW_FPGARev[cardnum]);

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p Q P C I
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the QPCI-1553 and QPCX
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
BT_INT vbtPageAccessSetupQPCI(
   BT_UINT   cardnum,           // (i) card number (0 based)
   char      *lpbase)           // (i) board base address in user space
{
   BT_INT       status;                 // Function error return code.
   volatile  BT_U16BIT *board_short_addr;  // DWORD pointer to board CSC Register.
   BT_U16BIT host_interface;
   BT_INT i;
   BT_INT num_config = 6;
   BT_U16BIT rdata;

   unsigned short board_config[] = {
	                       0x0046,   // sf 1ch
                           0x0086,   // sf 2ch
                           0x0106,   // sf 4ch
                           0x0846,   // mf 1ch
                           0x0886,   // mf 2ch
                           0x0906};  // mf 4ch
   #define IRIG_FLAG 0x1000 
   #define HIF_MASK 0x0fff

   BT_UINT num_chans[] = {1,2,4,1,2,4};
   BT_INT mode[] = {1,1,1,0,0,0};

   /**********************************************************************
   *  Read the PCI Revision ID from the PCI Configuration space.
   **********************************************************************/
   status = vbtGetPCIConfigRegister(api_device[cardnum], 0x08, 1, &_HW_PROMRev[cardnum]);
   if ( status )
      return status;

   /**********************************************************************
   *  Setup the PCI address of the boards dual-port, Hardware Registers
   *    and RAM Registers.  The first interface is SLOT_A, and the
   *    second interface is SLOT_B.  The Host Interface Registers
   *    pointer is also used to map the WCS (@+0x600000)
   **********************************************************************/
   CurrentMemKB[cardnum]   = BT_PCI_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   if(board_is_channel_mapped[cardnum])
   {  
      bt_PageAddr[cardnum][0] = lpbase +  DATA_RAM_QPCI; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase +  HW_REG_QPCI;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase +  REG_FILE_QPCI; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase;     // Host Interface Registers
   } 
   else if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_QPCI + DATA_RAM_QPCI; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_QPCI + HW_REG_QPCI;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_QPCI + REG_FILE_QPCI; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPCI;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_QPCI + DATA_RAM_QPCI; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_QPCI + HW_REG_QPCI;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_QPCI + REG_FILE_QPCI; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPCI;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN3_QPCI + DATA_RAM_QPCI; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN3_QPCI + HW_REG_QPCI;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN3_QPCI + REG_FILE_QPCI; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPCI;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_4)
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN4_QPCI + DATA_RAM_QPCI; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN4_QPCI + HW_REG_QPCI;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN4_QPCI + REG_FILE_QPCI; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPCI;     // Host Interface Registers
   }
   else
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   board_short_addr = (volatile BT_U16BIT *)bt_PageAddr[cardnum][3];
   /********************************************************************
   * Check to See if the Selected Channel is present and board is active
   ********************************************************************/
   host_interface = board_short_addr[HIR_CSC_REG];
   flipw(&host_interface);

   if(host_interface & ADDITIONAL_CAPABILITY)
   {
      board_has_acr[cardnum] = 0x1;
      board_has_serial_number[cardnum] = 0x1;
      board_has_plx_dma[cardnum] = 0x1;
      if(CurrentCardType[cardnum] == (unsigned)QPCI1553)
         CurrentCardType[cardnum] = (unsigned)QPCX1553;  
   }
    
   board_has_irig[cardnum] = host_interface & IRIG_FLAG;
   host_interface &= HIF_MASK;

   board_has_discretes[cardnum] = 0x1;
   board_has_testbus[cardnum] = 0x1;
   board_has_hwrtaddr[cardnum] = 0x1;
   board_has_differential[cardnum] = 0x1;
   board_access_32[cardnum] = 0x1;

   for(i=0;i<num_config;i++)
   {
      if(board_config[i] == host_interface)
         break;
   }

   if (i == num_config)
   {
      return BTD_ERR_BADBOARDTYPE;
   }

   _HW_1Function[cardnum] = mode[i];
   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT;
   
   numDiscretes[cardnum] = 10;
   bt_dismask[cardnum] = 0x3ff;

   if(CurrentCardSlot[cardnum] == 0)
   {
      board_has_hwrtaddr[cardnum] = 0x1;
      rdata = vbtGetDiscrete(cardnum,DISREG_HW_RTADDR);
	  if((rdata & 0x3) == 0x0)
      {
         hwRTAddr[cardnum] = -1;
      }
      else if((rdata & 0x3) == 0x1)
      {
         hwRTAddr[cardnum] = (vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1))&0x1f;
      }
      else if((rdata & 0x2f) == 0x3)
      {
         hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
      }
   }
   else
      hwRTAddr[cardnum] = -1;

   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);

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
   BT_INT       status;                 // Function error return code.
   volatile  BT_U16BIT *board_short_addr;  // DWORD pointer to board CSC Register.
   BT_U16BIT host_interface;
   BT_INT i;
   BT_INT num_config = 6;
   BT_U16BIT rdata;


   unsigned short board_config[] = {
	                       0x004e,   // sf 1ch
                           0x008e,   // sf 2ch
                           0x010e,   // sf 4ch
                           0x084e,   // mf 1ch
                           0x088e,   // mf 2ch
                           0x090e};  // mf 4ch
   #define IRIG_FLAG 0x1000
   #define HIF_MASK 0x0fff

   BT_UINT num_chans[] = {1,2,4,1,2,4};
   BT_INT mode[] = {1,1,1,0,0,0};

   /**********************************************************************
   *  Read the PCI Revision ID from the PCI Configuration space.
   **********************************************************************/
   status = vbtGetPCIConfigRegister(api_device[cardnum], 0x08, 1, &_HW_PROMRev[cardnum]);
   if ( status )
      return status;

   /**********************************************************************
   *  Setup the PCI address of the boards dual-port, Hardware Registers
   *    and RAM Registers.  The first interface is SLOT_A, and the
   *    second interface is SLOT_B.  The Host Interface Registers
   *    pointer is also used to map the WCS (@+0x600000)
   **********************************************************************/
   CurrentMemKB[cardnum]   = BT_PCI_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   if(board_is_channel_mapped[cardnum])
   {  
      bt_PageAddr[cardnum][0] = lpbase +  DATA_RAM_QCP; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase +  HW_REG_QCP;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase +  REG_FILE_QCP; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase;     // Host Interface Registers
   } 
   else    if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_QCP + DATA_RAM_QCP; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_QCP + HW_REG_QCP;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_QCP + REG_FILE_QCP; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QCP;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_QCP + DATA_RAM_QCP; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_QCP + HW_REG_QCP;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_QCP + REG_FILE_QCP; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QCP;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN3_QCP + DATA_RAM_QCP; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN3_QCP + HW_REG_QCP;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN3_QCP + REG_FILE_QCP; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QCP;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_4)
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN4_QCP + DATA_RAM_QCP; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN4_QCP + HW_REG_QCP;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN4_QCP + REG_FILE_QCP; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QCP;     // Host Interface Registers
   }
   else
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   board_short_addr = (volatile BT_U16BIT *)bt_PageAddr[cardnum][3];
   /********************************************************************
   * Check to See if the Selected Channel is present and board is active
   ********************************************************************/
   host_interface = board_short_addr[HIR_CSC_REG];
   flipw(&host_interface);

   board_has_irig[cardnum] = host_interface & IRIG_FLAG;
   host_interface &= HIF_MASK;

   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1;
   board_access_32[cardnum] = 0x1;
   board_has_plx_dma[cardnum] = 0x1;

   for(i=0;i<num_config;i++)
   {
      if(board_config[i] == host_interface)
         break;
   }

   if (i == num_config)
	   return BTD_ERR_BADBOARDTYPE;

   _HW_1Function[cardnum] = mode[i];
   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT;
   
   numDiscretes[cardnum] = 18;
   bt_dismask[cardnum] = 0x3ffff;
   
   rdata = vbtGetDiscrete(cardnum,DISREG_HW_RTADDR); 
   if(CurrentCardSlot[cardnum] == 0)
   {
      board_has_hwrtaddr[cardnum] =0x1;
      if((rdata & 0x3) == 0x0)
         hwRTAddr[cardnum] = -1;
      else if((rdata & 0x3) == 0x1)
      {
         hwRTAddr[cardnum] = (vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1))&0x1f;
      }
      else
      {  
	     hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
      }
   }
   else if(CurrentCardSlot[cardnum] == 1)
   {
      board_has_hwrtaddr[cardnum] =0x1;   
      if((rdata & 0x18) == 0x0)
         hwRTAddr[cardnum] = -1;
      else if((rdata & 0x18) == 0x8)
         hwRTAddr[cardnum] = ((vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1)) & 0x1f00)>>8;
      else
      {
	     hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
      }
   }
   else
      hwRTAddr[cardnum] = -1;

   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);
   flipw(&_HW_FPGARev[cardnum]);

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
   BT_INT       status;                 // Function error return code.
   volatile  BT_U16BIT *board_short_addr;  // DWORD pointer to board CSC Register.
   BT_U16BIT host_interface;
   BT_INT i;
   BT_INT num_config;
   BT_U16BIT rdata;

   unsigned short board_config[] = { // These are for the Q104-1553 
	                       0x0048,   // sf 1ch
                           0x0088,   // sf 2ch
                           0x0108,   // sf 4ch
                           0x0848,   // mf 1ch
                           0x0888,   // mf 2ch
                           0x0908,   // mf 4ch
                                     // These are for the Q104-1553P 
	                       0x004a,   // sf 1ch
                           0x008a,   // sf 2ch
                           0x010a,   // sf 4ch
                           0x084a,   // mf 1ch
                           0x088a,   // mf 2ch
                           0x090a};  // mf 4ch

   #define IRIG_FLAG 0x1000
   #define HIF_MASK 0x0fff
   #define board_id_mask 0x000f

   BT_UINT num_chans[] = {1,2,4,1,2,4,1,2,4,1,2,4};
   BT_INT mode[] = {1,1,1,0,0,0,1,1,1,0,0,0};

   num_config = sizeof(board_config)/2;
   /**********************************************************************
   *  Read the PCI Revision ID from the PCI Configuration space.
   **********************************************************************/
   if(CurrentCardType[cardnum] == Q1041553P)
   {
      status = vbtGetPCIConfigRegister(api_device[cardnum], 0x08, 1, &_HW_PROMRev[cardnum]);
      if ( status )
         return status;
     
      /**********************************************************************
      *  Setup the PCI address of the boards dual-port, Hardware Registers
      *    and RAM Registers.  The first interface is SLOT_A, and the
      *    second interface is SLOT_B.  The Host Interface Registers
      *    pointer is also used to map the WCS (@+0x600000)
      **********************************************************************/

      CurrentMemKB[cardnum]   = BT_PCI_MEMORY;     // Number of 1KB memory blocks
      btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
      board_access_32[cardnum] = 0x1;
   }
   else
   {
      /*********************************************************************
      *  Setup the ISA-1553 frame access parameters.  Since the ISA-1553
      *   dual port memory always mapped in 2K byte pages starting at a
      *   single address, these values are constants.
      *********************************************************************/
      bt_OffsetMask[cardnum] = BT_02KBOFFSETMASK_ISA;
      bt_FrameShift[cardnum] = BT_02KBFRAMESHIFT_ISA;
      bt_PtrShift[cardnum]   = BT_02KBPTRSHIFT_ISA;
      bt_FrameMask[cardnum]  = BT_FRAME_MASK_ISA;

      /**********************************************************************
      *  Setup the ISA address of the boards dual-port, Hardware
      *    Registers and RAM Registers.  The first interface is SLOT_A,
      *    and the second interface is SLOT_B.  The Host Interface
      *    Registers pointer is also used to map the page register.
      **********************************************************************/

      CurrentMemKB[cardnum]   = BT_ISA_MEMORY;     // Number of 1KB memory blocks
      btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   }
   
   if(CurrentCardType[cardnum] == Q1041553P)
   {
      if(board_is_channel_mapped[cardnum])
      {  
         bt_PageAddr[cardnum][0] = lpbase +  DATA_RAM_Q104P; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase +  HW_REG_Q104P;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase +  REG_FILE_Q104P; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase;     // Host Interface Registers
      } 
      else if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
      {
         bt_PageAddr[cardnum][0] = lpbase + CHAN1_Q104P + DATA_RAM_Q104P; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase + CHAN1_Q104P + HW_REG_Q104P;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase + CHAN1_Q104P + REG_FILE_Q104P; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase + CHAN1_Q104P;     // Host Interface Registers
      }
      else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
      {
         bt_PageAddr[cardnum][0] = lpbase + CHAN2_Q104P + DATA_RAM_Q104P; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase + CHAN2_Q104P + HW_REG_Q104P;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase + CHAN2_Q104P + REG_FILE_Q104P; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase + CHAN1_Q104P;     // Host Interface Registers
      }
      else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
      {
         bt_PageAddr[cardnum][0] = lpbase + CHAN3_Q104P + DATA_RAM_Q104P; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase + CHAN3_Q104P + HW_REG_Q104P;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase + CHAN3_Q104P + REG_FILE_Q104P; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase + CHAN1_Q104P;     // Host Interface Registers
      }
      else if ( CurrentCardSlot[cardnum] == CHANNEL_4)
      {
         bt_PageAddr[cardnum][0] = lpbase + CHAN4_Q104P + DATA_RAM_Q104P; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase + CHAN4_Q104P + HW_REG_Q104P;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase + CHAN4_Q104P + REG_FILE_Q104P; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase + CHAN1_Q104P;     // Host Interface Registers
      }
      else
         return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified
   }
   else
   {
      if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
      {
         bt_PageAddr[cardnum][0] = lpbase + CHAN1_Q104 + DATA_RAM_Q104; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase + CHAN1_Q104 + HW_REG_Q104;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase + CHAN1_Q104 + REG_FILE_Q104; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase + CHAN1_Q104;     // Host Interface Registers
      }
      else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
      {
         bt_PageAddr[cardnum][0] = lpbase + CHAN2_Q104 + DATA_RAM_Q104; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase + CHAN2_Q104 + HW_REG_Q104;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase + CHAN2_Q104 + REG_FILE_Q104; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase + CHAN1_Q104;     // Host Interface Registers
      }
      else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
      {
         bt_PageAddr[cardnum][0] = lpbase + CHAN3_Q104 + DATA_RAM_Q104; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase + CHAN3_Q104 + HW_REG_Q104;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase + CHAN3_Q104 + REG_FILE_Q104; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase + CHAN1_Q104;     // Host Interface Registers
      }
      else if ( CurrentCardSlot[cardnum] == CHANNEL_4)
      {
         bt_PageAddr[cardnum][0] = lpbase + CHAN4_Q104 + DATA_RAM_Q104; // DP Memory
         bt_PageAddr[cardnum][1] = lpbase + CHAN4_Q104 + HW_REG_Q104;   // HW Registers
         bt_PageAddr[cardnum][2] = lpbase + CHAN4_Q104 + REG_FILE_Q104; // RAM Registers
         bt_PageAddr[cardnum][3] = lpbase + CHAN1_Q104;     // Host Interface Registers
      }
      else
         return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified
   }
   board_short_addr = (volatile BT_U16BIT *)bt_PageAddr[cardnum][3];

   /********************************************************************
   * Check to See if the Selected Channel is present and board is active
   ********************************************************************/
   host_interface = board_short_addr[HIR_CSC_REG];
   flipw(&host_interface);

   if(CurrentCardType[cardnum] == Q1041553P)
   {
      if((host_interface & board_id_mask) != 0xa)
         return BTD_ERR_BADBOARDTYPE;
   }
   else
   {
      board_is_paged[cardnum] = 1;
      if((host_interface & board_id_mask) != 0x8)
         return BTD_ERR_BADBOARDTYPE;
   }   

   board_has_irig[cardnum] = host_interface & IRIG_FLAG;
   host_interface &= HIF_MASK;
 
   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1;

   for(i=0;i<num_config;i++)
   {
      if(board_config[i] == host_interface)
         break;
   }

   if (i == num_config)
	   return BTD_ERR_BADBOARDTYPE;

   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT;

   _HW_1Function[cardnum] = mode[i]; 
   numDiscretes[cardnum] = 10;
   bt_dismask[cardnum] = 0x3ff;

   rdata = vbtGetDiscrete(cardnum,DISREG_HW_RTADDR);
   if(CurrentCardSlot[cardnum] == 0)
   {
      board_has_hwrtaddr[cardnum] = 0x1;
      if((rdata & 0x3) == 0x0)
         hwRTAddr[cardnum] = -1;
      else if((rdata & 0x3) == 0x1)
      {
         hwRTAddr[cardnum] = (vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1))&0x1f;
      }
      else
      {
         vbtSetHWRegister(cardnum,0xd,QPCI_BIT_FAIL);
		 hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
      }
   }
   else
      hwRTAddr[cardnum] = -1;

   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);

   return BTD_OK;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t P a g e A c c e s s S e t u p I P D
 *===========================================================================*
 *
 * FUNCTION:    Setup the page access data elements for the IP-D1553 dual
 *              channel single wide IP board.  This board is flat-mapped (no
 *              frames )and supports 2 MIL-STD-1553 interfaces on a board.
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
BT_INT vbtPageAccessSetupIPD( BT_UINT   cardnum,          // (i) card number (0 based)
                              BT_U32BIT phys_addr,        // physical address
                              char *lpbase)           // (i) board base address in user space
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

#define NUM_IPD_CONFIG 4
   unsigned num_chans[] = {1,2,1,2};
   unsigned nchan;
   int mode[] = {1,1,0,0};

   int status;
   int i = 0;
   BT_U32BIT base_addr;
   short *ipMemoryEnableRegister;  // Control register pointer to AVME9660
   BT_U16BIT ipd_board_config;
   unsigned slot_a_base=0, slot_b_base=0, slot_c_base=0, slot_d_base=0;

   BT_U16BIT ipd_config[] = {0x0044,  //single channel single function
	                     0x0084,  //dual channel   single function 
                             0x0844,  //single channel multi function
                             0x0884}; //dual channel   multi function

   unsigned channel_offset[] = {CHAN1_IPD, //0 - 1
	                            CHAN2_IPD, //1 - 2
				                CHAN1_IPD, //2 - 3
				                CHAN2_IPD, //3 - 4
				                CHAN1_IPD, //4 - 5
				                CHAN2_IPD, //5 - 6
				                CHAN1_IPD, //6 - 7
				                CHAN2_IPD};//7 - 8

   if(CurrentCarrier[cardnum] == AVME9660)
   {
      slot_a_base = 0x0;
      slot_b_base = 0x0;
      slot_c_base = 0x0;
      slot_d_base = 0x0;
	  
      /*********************************************************************
      *  Enable A24 memory addressing on the AVME9660 for selected slot,
      *  4M memory sizes, slot address programmable to any 4M boundary.
      *********************************************************************/
      /* Enable slected Slot in A24 Memory Spaces */

      if(phys_addr % 0x400000)
         return BTD_ERR_BADADDRMAP;
      if(phys_addr > 0xC00000)
         return BTD_ERR_BADADDRMAP;
         
      ipMemoryEnableRegister = (short *)((char *)bt_iobase[cardnum] + 0x00c6);
      base_addr = phys_addr >> 16;   // Use memory address, not IO!

      if ( (CurrentCardSlot[cardnum] == SLOT_A_CH1) || (CurrentCardSlot[cardnum] == SLOT_A_CH2)  )
      {
         *ipMemoryEnableRegister |= 0x0001; //Enable Slots A
	     ipMemoryEnableRegister = (short *)((char *)bt_iobase[cardnum] + 0x00d0);
         *ipMemoryEnableRegister = (short)(base_addr + 0x02); // base + 0x0
      }
      else if ( (CurrentCardSlot[cardnum] == SLOT_B_CH1) || (CurrentCardSlot[cardnum] == SLOT_B_CH2) )
      {
         *ipMemoryEnableRegister |= 0x0002; //Enable Slots B
	     ipMemoryEnableRegister = (short *)((char *)bt_iobase[cardnum] + 0x00d2);
         *ipMemoryEnableRegister = (short)(base_addr + 0x02); // base + 0x0
      }
      else if ( (CurrentCardSlot[cardnum] == SLOT_C_CH1) || (CurrentCardSlot[cardnum] == SLOT_C_CH2) )
      {
         *ipMemoryEnableRegister |= 0x0004; //Enable Slots C
	     ipMemoryEnableRegister = (short *)((char *)bt_iobase[cardnum] + 0x00d4);
         *ipMemoryEnableRegister = (short)(base_addr + 0x02); // base + 0x0
      }
      else if ( (CurrentCardSlot[cardnum] == SLOT_D_CH1) || (CurrentCardSlot[cardnum] == SLOT_D_CH2) )
      {
         *ipMemoryEnableRegister |= 0x0008; //Enable Slots D
	     ipMemoryEnableRegister = (short *)((char *)bt_iobase[cardnum] + 0x00d6);
         *ipMemoryEnableRegister = (short)(base_addr + 0x02); // base + 0x0
      }
      else
         return BTD_ERR_PARAM;

   }
   else if(CurrentCarrier[cardnum] == VIPC616_618)
   {
      slot_a_base = 0x0;
      slot_b_base = 0x0800000;
      slot_c_base = 0x1000000;
      slot_d_base = 0x1800000;
   }
   else if(CurrentCarrier[cardnum] == IPD1553_PCI)
   {
      slot_a_base = 0x01000000;
      slot_b_base = 0x01800000;
      slot_c_base = 0x02000000;
      slot_d_base = 0x02800000;

      /*******************************************************************
      *  Setup the PCI40-100 or PCI-40A CNTL0 register to enable
      *   interrupts and to enable the bus error timer.
      *******************************************************************/
      *(BT_U8BIT *)(lpbase+0x0500) |= 0x020;   // Enable bus error timer

      /*******************************************************************
      *  Setup the PCI40-100 or PCI-40A CNTL0 register to 8 MHz
      *   for this slot, since all IP-D1553's are 8 MHz only.
      *******************************************************************/  
      if ( CurrentCardSlot[cardnum] == SLOT_A_CH1 || CurrentCardSlot[cardnum] == SLOT_A_CH2  )
         *(BT_U8BIT *)(lpbase+0x0500) &= ~0x01;   // Set 8 Mhz
      else if ( CurrentCardSlot[cardnum] == SLOT_B_CH1 || CurrentCardSlot[cardnum] == SLOT_B_CH2 )
         *(BT_U8BIT *)(lpbase+0x0500) &= ~0x02;   // Set 8 Mhz
      else if ( CurrentCardSlot[cardnum] == SLOT_C_CH1 || CurrentCardSlot[cardnum] == SLOT_C_CH2 )
         *(BT_U8BIT *)(lpbase+0x0500) &= ~0x04;   // Set 8 Mhz
      else if ( CurrentCardSlot[cardnum] == SLOT_D_CH1 || CurrentCardSlot[cardnum] == SLOT_D_CH2 )
         *(BT_U8BIT *)(lpbase+0x0500) &= ~0x08;   // Set 8 Mhz
      else
         return BTD_ERR_PARAM;

   }
   else
      return BTD_ERR_PARAM;

   if ( (CurrentCardSlot[cardnum] == SLOT_A_CH1) || (CurrentCardSlot[cardnum] == SLOT_A_CH2) )
      bt_PageAddr[cardnum][3] = lpbase + slot_a_base;     // Host Interface Registers
   else if ( (CurrentCardSlot[cardnum] == SLOT_B_CH1) || (CurrentCardSlot[cardnum] == SLOT_B_CH2) )
      bt_PageAddr[cardnum][3] = lpbase + slot_b_base;     // Host Interface Registers
   else if ( (CurrentCardSlot[cardnum] == SLOT_C_CH1) || (CurrentCardSlot[cardnum] == SLOT_C_CH2) )
      bt_PageAddr[cardnum][3] = lpbase + slot_c_base;     // Host Interface Registers
   else if ( (CurrentCardSlot[cardnum] == SLOT_D_CH1) || (CurrentCardSlot[cardnum] == SLOT_D_CH2) )
      bt_PageAddr[cardnum][3] = lpbase + slot_d_base;     // Host Interface Registers
   else
      return BTD_ERR_PARAM;

   CurrentMemKB[cardnum] = BT_PCI_MEMORY;     // Number of 1KB memory blocks

   bt_PageAddr[cardnum][0] = bt_PageAddr[cardnum][3] + channel_offset[CurrentCardSlot[cardnum]] + DATA_RAM_IPD; // DP Memory
   bt_PageAddr[cardnum][1] = bt_PageAddr[cardnum][3] + channel_offset[CurrentCardSlot[cardnum]] + HW_REG_IPD;   // HW Registers
   bt_PageAddr[cardnum][2] = bt_PageAddr[cardnum][3] + channel_offset[CurrentCardSlot[cardnum]] + REG_FILE_IPD; // RAM Registers   
   IPD_IDPROM_ADDR[cardnum]= bt_PageAddr[cardnum][3] + channel_offset[CurrentCardSlot[cardnum]] + IPD_IDPROM;   // ID PROM Shadow RAM
   nchan = (CurrentCardSlot[cardnum]%2)+1;                            // Max Channels per IP
   do {
      ipd_board_config = *(BT_U16BIT *)bt_PageAddr[cardnum][3];          // Read the Host Interface

      if(++i==4)
         break;

   } while(ipd_board_config == 0xffff);

   for (i=0;i<NUM_IPD_CONFIG;i++)
   {
      if(ipd_board_config == ipd_config[i])                           // Find the Board Configuration
         break;
   }

   if(i==NUM_IPD_CONFIG)
   {
      return BTD_CHAN_NOT_PRESENT;
   }
   
   board_has_irig[cardnum] = 0x1000; //All IPDs have IRIG_B sorta
   _HW_1Function[cardnum] = mode[i];
   if ( num_chans[i] < nchan )
   {
      return BTD_CHAN_NOT_PRESENT;
   }

   status = vbtVerifyIPD_IDPROM(cardnum);
   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);
   return status;
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
   BT_INT       status;                 // Function error return code.
   volatile  BT_U16BIT *board_short_addr;  // DWORD pointer to board CSC Register.
   BT_U16BIT host_interface;
   BT_INT i;
   BT_INT num_config = 4;
   BT_U16BIT capabilities=0;

   unsigned short board_config[] = {
	                       0x0052,   //R15-EC sf 1ch
                           0x0092,   //R15-EC sf 2ch
                           0x0852,   //R15-EC mf 1ch
                           0x0892};  //R15-EC mf 2ch

   #define IRIG_FLAG 0x1000
   #define HIF_MASK 0x0fff
   #define ADDITIONAL_CAPABILITY 0x8000 

   BT_UINT num_chans[] = {1,2,1,2};
   BT_INT mode[] = {1,1,0,0};

   /**********************************************************************
   *  Read the PCI Revision ID from the PCI Configuration space.
   **********************************************************************/
   status = vbtGetPCIConfigRegister(api_device[cardnum], 0x08, 1, &_HW_PROMRev[cardnum]);
   if ( status )
      return status;

   /**********************************************************************
   *  Setup the PCI address of the boards dual-port, Hardware Registers
   *    and RAM Registers.  The first interface is CHANNEL_1 or SLOT_A, 
   *    and the second interface is CHANNEL_@ or SLOT_B
   *    This board uses the same mapping offset as the QPMC-1553.
   **********************************************************************/
   CurrentMemKB[cardnum]   = BT_PCI_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_QPMC + DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_QPMC + HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_QPMC + REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPMC;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_QPMC + DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_QPMC + HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_QPMC + REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPMC;     // Host Interface Registers
   }
   else
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   board_short_addr = (volatile BT_U16BIT *)bt_PageAddr[cardnum][3];
   host_interface = board_short_addr[HIR_CSC_REG];
   flipw(&host_interface);
  
   if(host_interface & ADDITIONAL_CAPABILITY)
   {
      board_has_acr[cardnum] = 0x1;
      board_has_serial_number[cardnum] = 0x0;
      capabilities = board_short_addr[HIR_AC_REG];
      flipw(&capabilities);
   }
        
   /********************************************************************
   * Check to See if the Selected Channel is present and board is active
   ********************************************************************/  
   board_has_irig[cardnum] = host_interface & IRIG_FLAG;
   host_interface &= HIF_MASK;
   board_has_discretes[cardnum]=0x1;
   board_has_differential[cardnum] = 0x1;
   board_access_32[cardnum] = 0x1;
   board_has_485_discretes[cardnum]=0x1;
   board_has_serial_number[cardnum] = 0x1;

   for(i=0;i<num_config;i++)
   {
      if(board_config[i] == host_interface)
         break;
   }

   if (i == num_config)
	   return BTD_ERR_BADBOARDTYPE;

   _HW_1Function[cardnum] = mode[i];
   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT; 

   // Set up discretes and hardwired RT addressing

   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT;
 
   numDiscretes[cardnum] = 2;
   bt_dismask[cardnum] = 0xc0;
   hwRTAddr[cardnum] = -1;  

   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);
   flipw(&_HW_FPGARev[cardnum]);

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
   BT_INT       status;                 // Function error return code.
   volatile  BT_U16BIT *board_short_addr;  // DWORD pointer to board CSC Register.
   BT_U16BIT host_interface;
   BT_INT i;
   BT_INT num_config = 4;
   BT_U16BIT capabilities=0;
   BT_U16BIT rdata=0;

   unsigned short board_config[] = {
	                       0x0056,   //RXMC-1553 sf 1ch
                           0x0096,   //RXMC-1553 sf 2ch
                           0x0856,   //RXMC-1553 mf 1ch
                           0x0896};  //RXMC-1553 mf 2ch

   #define IRIG_FLAG 0x1000
   #define HIF_MASK 0x0fff
   #define ADDITIONAL_CAPABILITY 0x8000
   #define RXMC_OUTPUT_CONFIG_MASK 0x70

   BT_UINT num_chans[] = {1,2,1,2};
   BT_INT mode[] = {1,1,0,0};

   /**********************************************************************
   *  Read the PCI Revision ID from the PCI Configuration space.
   **********************************************************************/
   status = vbtGetPCIConfigRegister(api_device[cardnum], 0x08, 1, &_HW_PROMRev[cardnum]);
   if ( status )
      return status;

   /**********************************************************************
   *  Setup the PCI address of the boards dual-port, Hardware Registers
   *    and RAM Registers.  The first interface is CHANNEL_1 or SLOT_A, 
   *    and the second interface is CHANNEL_@ or SLOT_B
   *    This board uses the same mapping offset as the QPMC-1553.
   **********************************************************************/
   CurrentMemKB[cardnum]   = BT_PCI_MEMORY;     // Number of 1KB memory blocks
   btmem_rt_begin[cardnum] = BT_RT_BASE_PCI;    // RT in first seg with BC&BM
   if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN1_QPMC + DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN1_QPMC + HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN1_QPMC + REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPMC;     // Host Interface Registers
   }
   else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
   {
      bt_PageAddr[cardnum][0] = lpbase + CHAN2_QPMC + DATA_RAM_QPMC; // DP Memory
      bt_PageAddr[cardnum][1] = lpbase + CHAN2_QPMC + HW_REG_QPMC;   // HW Registers
      bt_PageAddr[cardnum][2] = lpbase + CHAN2_QPMC + REG_FILE_QPMC; // RAM Registers
      bt_PageAddr[cardnum][3] = lpbase + CHAN1_QPMC;     // Host Interface Registers
   }
   else
      return BTD_CHAN_NOT_PRESENT;     // Invalid slot specified

   board_short_addr = (volatile BT_U16BIT *)bt_PageAddr[cardnum][3];
   host_interface = board_short_addr[HIR_CSC_REG];
   flipw(&host_interface);
  
   if(host_interface & ADDITIONAL_CAPABILITY)
   {
      board_has_acr[cardnum] = 0x1;
      board_has_serial_number[cardnum] = 0x1;
      capabilities = board_short_addr[HIR_AC_REG];
      flipw(&capabilities);
   }
        
   /********************************************************************
   * Check to See if the Selected Channel is present and board is active
   ********************************************************************/  
   board_has_irig[cardnum] = host_interface & IRIG_FLAG;
   host_interface &= HIF_MASK; 
   board_access_32[cardnum] = 0x1;
   board_has_serial_number[cardnum] = 0x1;
   board_has_rtaddr_latch[cardnum] = 0x1;

   for(i=0;i<num_config;i++)
   {
      if(board_config[i] == host_interface)
         break;
   }

   if (i == num_config)
	   return BTD_ERR_BADBOARDTYPE;

   _HW_1Function[cardnum] = mode[i];
   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT; 

   // Set up discretes and hardwired RT addressing

   if ( num_chans[i] < CurrentCardSlot[cardnum]+1)
      return BTD_CHAN_NOT_PRESENT;

   rxmc_output_config[cardnum] = (capabilities & RXMC_OUTPUT_CONFIG_MASK)>>4;
   switch(rxmc_output_config[cardnum])
   {
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
      default:
         return BTD_ERR_BADBOARDTYPE;
   }

   //Clear the external output triggers.
   if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
      vbtSetDiscrete(cardnum,RXMC_EXT_TRIG_OUT_CH1,0x0);
   else
      vbtSetDiscrete(cardnum,RXMC_EXT_TRIG_OUT_CH2,0x0);

   rdata = vbtGetDiscrete(cardnum,DISREG_HW_RTADDR); 
   if(CurrentCardSlot[cardnum] == 0)
   {
      board_has_hwrtaddr[cardnum] = 0x1;
      if((rdata & 0x3) == 0x0)
         hwRTAddr[cardnum] = -1;
      else if((rdata & 0x3) == 0x1)
      {
         hwRTAddr[cardnum] = (vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1))&0x1f;
      }
      else
      {  
	     hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
      }
   }
   else if(CurrentCardSlot[cardnum] == 1)
   {
      board_has_hwrtaddr[cardnum] = 0x1;   
      if((rdata & 0x18) == 0x0)
         hwRTAddr[cardnum] = -1;
      else if((rdata & 0x18) == 0x8)
         hwRTAddr[cardnum] = ((vbtGetDiscrete(cardnum,DISREG_RTADDR_RD1)) & 0x1f00)>>8;
      else
      {
	     hwRTAddr[cardnum] = BTD_RTADDR_PARITY;
      }
   }
   else
      hwRTAddr[cardnum] = -1;
   
   _HW_FPGARev[cardnum]  = *(BT_U16BIT *)(bt_PageAddr[cardnum][1] +0x12);
   flipw(&_HW_FPGARev[cardnum]);
 
   if((_HW_FPGARev[cardnum]&0x0fff) >= 0x451)
   {
      board_is_dual_function[cardnum] = 0x1;
         boardHasMultipleTriggers[cardnum] = 0x1;
   }

   return BTD_OK;
}


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

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t S e t D i s c r e t e
 *===========================================================================*
 *
 * FUNCTION:    Sets the discrete register.
 *
 * DESCRIPTION: Set the specified discrete register to the specify value.
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void vbtSetDiscrete(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U16BIT regval)        // (i) new value
{
   *((BT_U16BIT *)(bt_PageAddr[cardnum][3])+regnum) = flipws(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t D i s c r e t e
 *===========================================================================*
 *
 * FUNCTION:    Read discrete reigsters.
 *
 * DESCRIPTION: The routine read the specified discrete registers.
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U16BIT vbtGetDiscrete(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][3])+regnum);
   flipw(&regval); // for PMC on PowerPC
   return regval;
}

