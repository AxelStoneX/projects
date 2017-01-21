/*============================================================================*
 * FILE:                  D U M P M E M . C
 *============================================================================*
 *
 * COPYRIGHT (C) 1999 - 2010 BY
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
 *          NTELLIGENT PLATFORMS.
 *
 *===========================================================================*
 *
 * FUNCTION:   BusTools/1553-API Library:
 *             This file contains the API routines for memory dump and trace
 *             dump diagnostic functions.
 *
 * USER ENTRY POINTS: 
 *    BusTools_DumpMemory  - Function dumps all board memory to a file.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    AddTrace             - Function adds a entry into the trace buffer.
 *
 * INTERNAL ROUTINES:
 *    API_Names            - Helper; returns BusTools function name string.
 *    DumpTrace            - Helper for BusTools_DumpMemory; dumps trace buffer.
 *    PerformanceCounter   - Returns the elapsed time in microseconds.
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  --------     ---------------------------------------------------------------
  06/10/1999   Added BusTools_DumpMemory function for release.V3.11.ajh
  11/10/1999   Added RegisterFunction FIFO dump.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  03/15/2002   Add support for different O/S version V4.48.rhc
  02/24/2003   Add QPCI-1553 support
  02/18/2004   PCCard-D1553 Supoprt
 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <busapi.h>
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"

#if defined(ADD_TRACE)

#define MAX_TRACE (2*4096)
#if MAX_TRACE & (MAX_TRACE-1)
#error Trace buffer is not a power of two in length!
#endif
typedef struct trace_buffer
{
   BT_U16BIT cardnum;         // card number
   BT_U16BIT nFunction;       // function number to log
   BT_INT    nParam1;         // first parameter to log
   BT_INT    nParam2;         // second parameter to log
   BT_INT    nParam3;         // third parameter to log
   BT_INT    nParam4;         // fourth parameter to log
   BT_INT    nParam5;         // fourth parameter to log
   __int64   time;            // time trace entry was logged.
} TRACE_BUFFER;

static TRACE_BUFFER trace[MAX_TRACE];
static int trace_pointer = 0;
static int trace_wrap_around = 0;
static int time_correction;           // Time correction value


/****************************************************************************
*
*  PROCEDURE NAME -    API_Names()
*
*  FUNCTION
*       This procedure returns the string value of an API name index.
*
****************************************************************************/
static char * API_Names(int function)
{
   /**********************************************************************
   *  Return the symbolic name for the API function number
   **********************************************************************/
   switch (function)
   {
   case NBUSTOOLS_BC_MESSAGEREAD:        return "BC_MessageRead     ";
   case NBUSTOOLS_BC_START:              return "BC_Start           ";
   case NBUSTOOLS_BC_STARTSTOP:          return "BC_StartStop       ";
   case NBUSTOOLS_BM_MESSAGEREAD:        return "BM_MessageRead     ";
   case BM_MSGREADBLOCK:                 return "BM_MsgReadBlock    ";
   case NBUSTOOLS_BM_STARTSTOP:          return "BM_StartStop       ";
   case NBUSTOOLS_RT_MESSAGEGETID:       return "RT_MessageGetid    ";
   case NBUSTOOLS_RT_MESSAGEREAD:        return "RT_MessageRead     ";
   case NBUSTOOLS_RT_STARTSTOP:          return "RT_StartStop       ";
   case NBM_MESSAGECONVERT:              return "BM_MessageConvert  ";
   case NBM_TRIGGER_OCCUR:               return "BM Trigger Occured ";
   case NCALLUUSERTHREAD:                return "Calling User Thread";
   case NINTQUEUEENTRY:                  return "Interrupt Queuq Ent";
   case NSIGNALUUSERTHREAD:              return "SignalUserThread   ";
   case NTIME_TAG_CLEARFLAG:             return "Time Tag Clear Flag";
   case NTIME_TAG_INTERRUPT:             return "Time Tag Interrupt ";
   case NVBTNOTIFY:                      return "vbtNotify          ";
   case NVBTSETUP:                       return "vbtSetup           ";
   case NVBTSHUTDOWN:                    return "vbtShutDown        ";
   case NBUS_LOADING_FILTER:             return "BusLoadingFilter   ";
   default:                              return ">>Unknown function<";
   }
}

/****************************************************************************
*
*  PROCEDURE NAME -    AddTrace()
*
*  FUNCTION
*       This procedure logs the sequence of procedure calls made by a
*       user's program into the BusTools/1553-API.  This log is dumped
*       by a call to BusTools_DumpMemory to a user-specified file.
*
****************************************************************************/
void AddTrace(
   BT_UINT cardnum,         // (i) card number (0 based)
   BT_INT  nFunction,       // (i) function number to log
   BT_INT  nParam1,         // (i) first parameter to log
   BT_INT  nParam2,         // (i) second parameter to log
   BT_INT  nParam3,         // (i) third parameter to log
   BT_INT  nParam4,         // (i) fourth parameter to log
   BT_INT  nParam5)         // (i) fifth parameter to log
{

   if ( DumpTraceMask &  (1 << nFunction) )
      return;    // Logging of this function is disabled.

   trace[trace_pointer].cardnum   = (BT_U16BIT)cardnum;
   trace[trace_pointer].nFunction = (BT_U16BIT)nFunction;
   trace[trace_pointer].nParam1   = nParam1;
   trace[trace_pointer].nParam2   = nParam2;
   trace[trace_pointer].nParam3   = nParam3;
   trace[trace_pointer].nParam4   = nParam4;
   trace[trace_pointer].nParam5   = nParam5;
   trace[trace_pointer].time      = PerformanceCounter();
   trace_pointer++;
   if ( trace_pointer >= MAX_TRACE )
   {
      trace_pointer = 0;
      trace_wrap_around = 1;
   }
}


/****************************************************************************
*
*  PROCEDURE NAME -    DumpTrace()
*
*  FUNCTION
*       This procedure outputs the trace buffer to a file.  There are two
*       cases:
*       ->Trace buffer has not filled.
*         -Dump from entry zero to entry trace_pointer-1.
*       ->Trace buffer has filled up and wrapped around.
*         -Dump from entry trace_pointer to end,
*          then dump from zero to trace_pointer-1
*
****************************************************************************/
static void DumpTrace(
   FILE         *hfMemFile) // (i) file handle to write trace buffer to.
{
   int     Oldest, last;             // Trace buffer pointers
   //char    time_string[30];           // int 64 value in ASCII

   if ( trace_wrap_around )
   {
      Oldest = trace_pointer;          // Oldest entry in the trace buffer
      last    = trace_pointer-1;
   }
   else
   {
      Oldest = 0;                      // Oldest entry in the trace buffer
      last    = trace_pointer-1;
   }
   if ( last < 0 )
      last = MAX_TRACE - 1;

   // Now dump the trace entries to the debug output file.
//printf(hfMemFile, "\nTrace Buffer Output\n"
//       " Performance Counter Frequency = %dHz Time correction = %d\n\n",
//       (int)liFreq, time_correction);

   while (1)
   {
      fprintf(hfMemFile,
              "%4d-%s(%d) @ %I64d/%6d us %8.8X(%6d) %8.8X %8.8X %8.8X %8.8X\n",
              Oldest, API_Names(trace[Oldest].nFunction),
              trace[Oldest].cardnum, trace[Oldest].time,
              (int)(trace[Oldest].time - trace[(MAX_TRACE-1)&(Oldest-1)].time),
              trace[Oldest].nParam1, trace[Oldest].nParam1,
              trace[Oldest].nParam2,
              trace[Oldest].nParam3,
              trace[Oldest].nParam4,
              trace[Oldest].nParam5);
      if ( Oldest == last ) break;
      Oldest++;
      if ( Oldest >= MAX_TRACE ) Oldest = 0;
   }
}
#else
#define DumpTrace(p1)
#endif // #if defined(ADD_TRACE)

/****************************************************************************
*
*  PROCEDURE NAME -    BusTools_DumpMemory()
*
*  FUNCTION
*       This procedure outputs a dump of all requested data areas.  Regions
*       whose bit is set in region_mask are dumped to the specified 8.3 file.
*       region_mask == 1 dumps the trace buffer, successive bits dump the
*       regions mapped by BusTools_GetAddr().
*
*  RETURNS
*       API_BUSTOOLS_NO_FILE
*       API_BUSTOOLS_BADCARDNUM
*       API_SUCCESS
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_DumpMemory(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT region_mask,   // (i) mask of memory regions to dump (bit 1 = region 1)       
   char * file_name,        // (i) pointer to name of output file to create             
   char * heading)          // (i) pointer to message to display in file                      
{
#if defined(FILE_SYSTEM)
   BT_U32BIT    i;                        // Loop index
   BT_UINT      j;                        // Loop index
   BT_UINT      num_per_line;             // Number of values per line
   BT_UINT      block_id;                 // Loop index
   BT_U32BIT    first;                    // Offset to first list word
   BT_U32BIT    last;                     // Offset to last list word
   BT_U16BIT    data[0x80];               // Read 16+ words per line
   FILE         *hfMemFile;               // Error file handle is not open.
   time_t       tdate;                    // Date this test was run

   // First open the user-specified output file.
   if ( file_name[0] == 0 )
      return API_BUSTOOLS_NO_FILE;        // If file name is null forget it.V4.20

   hfMemFile = fopen(file_name, "w+t");   // Write+Text mode.
   if ( hfMemFile == NULL )
   {
      // Just return a code if the output file could not be created.V4.25.ajh
      return API_BUSTOOLS_NO_FILE;        // File could not be created.
   }

   /******************************************************************
   *  Check for legal call
   *******************************************************************/
   if (cardnum >= MAX_BTA)
   {
      fprintf(hfMemFile, "Bad card number specified: %d\n", cardnum);
      fclose(hfMemFile);         //  Close and save the debug data.
      return API_BUSTOOLS_BADCARDNUM;
   }

   // If no regions were requested, dump them all
   if ( !region_mask ) region_mask = 0xFFFFFFFF;

   // Log the user supplied message
   fprintf(hfMemFile, "%s\n\n", heading);

   // Log the board type, etc.
   if ( CurrentCardType[cardnum] == ISA1553 )
      fprintf(hfMemFile, " ISA-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == PCC1553 )
      fprintf(hfMemFile, " PCC-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == PCCD1553 )
      fprintf(hfMemFile, " PCC-D1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == PCI1553 )
      fprintf(hfMemFile, " PCI-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPCI1553 )
      fprintf(hfMemFile, " QPCI-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPCX1553 )
      fprintf(hfMemFile, " QPCX-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == Q1041553P )
      fprintf(hfMemFile, " Q104-1553P WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == Q1041553 )
      fprintf(hfMemFile, " Q104-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QCP1553 )
      fprintf(hfMemFile, " QCP-1553 WCS V%d, FPGA V%x, Channel%d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15EC )
      fprintf(hfMemFile, " R15-EC WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15AMC )
      fprintf(hfMemFile, " R15-AMC WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == RPCIe1553 )
      fprintf(hfMemFile, " RPCIe-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == AMC1553 )
      fprintf(hfMemFile, " AMC-1553 WCS V%d, FPGA V%x, Channel%d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPM1553 )
      fprintf(hfMemFile, " QPM-1553/QPMC_1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == VME1553 )
      fprintf(hfMemFile, " VME-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == RXMC1553 )
      fprintf(hfMemFile, " RXMC-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QVME1553/RQVME2 )
      fprintf(hfMemFile, " QVME-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if( CurrentCardType[cardnum] == VXI1553 )
      fprintf(hfMemFile, " VXI-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);

   // Print the API version and the firmware version.
   fprintf(hfMemFile, " API Ver = %s/%s\n", API_VER, API_TYPE);
   fprintf(hfMemFile, " Build Options = %s/%s\n", BUILD_OPTIONS, BUILD_OPTIONS_INT); // V4.20

   // Time tag the output file.
   tdate = time(NULL);
   fprintf(hfMemFile, " Memory dump at %s\n\n", ctime(&tdate));  // V4.30

   fprintf(hfMemFile, " Init:bt=%d,inuse=%d,int-enable=%d,Polling-interval=%d, cardnum=%d\n"
                      " Init:bc=%d,bm=%d,rt=%d Running bc=%d,bm=%d,rt=%d HW Interrupts=%d\n",
           bt_inited[cardnum],  bt_inuse[cardnum], bt_interrupt_enable[cardnum],
           api_polling_interval, cardnum,
           bc_inited[cardnum],  bm_inited[cardnum],  rt_inited[cardnum],
           bc_running[cardnum], bm_running[cardnum], rt_running[cardnum],
           hw_int_enable[cardnum]);

   // Memory management pointers.  Output as WORD offsets on the board...
   fprintf(hfMemFile, "\n BusTools/1553-API Memory Management Variables - Counts:\n");
   fprintf(hfMemFile, " bc_mblock_count       - %d\n",bc_mblock_count[cardnum]);
   fprintf(hfMemFile, " bm_count              - %d\n\n",bm_count[cardnum]);
   fprintf(hfMemFile, "\n BusTools/1553-API Memory Management Variables - Addresses:\n");
   fprintf(hfMemFile, "                         (Word/Byte)\n");
   fprintf(hfMemFile, " btmem_bc              - %5.5X/%5.5X\n",btmem_bc[cardnum]/2,btmem_bc[cardnum]);
   fprintf(hfMemFile, " btmem_bc_next         - %5.5X/%5.5X\n",btmem_bc_next[cardnum]/2,btmem_bc_next[cardnum]);
   fprintf(hfMemFile, " btmem_bm_cbuf         - %5.5X/%5.5X\n",btmem_bm_cbuf[cardnum]/2,btmem_bm_cbuf[cardnum]);
   fprintf(hfMemFile, " btmem_bm_cbuf_next    - %5.5X/%5.5X\n",btmem_bm_cbuf_next[cardnum]/2,btmem_bm_cbuf_next[cardnum]);
   fprintf(hfMemFile, " btmem_bm_mbuf         - %5.5X/%5.5X\n",btmem_bm_mbuf[cardnum]/2,btmem_bm_mbuf[cardnum]);
   fprintf(hfMemFile, " btmem_bm_mbuf_next    - %5.5X/%5.5X\n",btmem_bm_mbuf_next[cardnum]/2,btmem_bm_mbuf_next[cardnum]);
   fprintf(hfMemFile, " btmem_tail1           - %5.5X/%5.5X\n",btmem_tail1[cardnum]/2,btmem_tail1[cardnum]);
   fprintf(hfMemFile, " btmem_tail2           - %5.5X/%5.5X\n",btmem_tail2[cardnum]/2,btmem_tail2[cardnum]);
   fprintf(hfMemFile, " btmem_rt_begin        - %5.5X/%5.5X\n",btmem_rt_begin[cardnum]/2,btmem_rt_begin[cardnum]);
   fprintf(hfMemFile, " btmem_rt_top_avail    - %5.5X/%5.5X\n",btmem_rt_top_avail[cardnum]/2,btmem_rt_top_avail[cardnum]); 
   fprintf(hfMemFile, " btmem_pci1553_next    - %5.5X/%5.5X\n",btmem_pci1553_next[cardnum]/2,btmem_pci1553_next[cardnum]);
   fprintf(hfMemFile, " btmem_pci1553_rt_mbuf - %5.5X/%5.5X\n\n",btmem_pci1553_rt_mbuf[cardnum]/2,btmem_pci1553_rt_mbuf[cardnum]);

   // Board memory pointers.
   fprintf(hfMemFile,"                              RAM    HWREG  RAMREG HIF\n");
   fprintf(hfMemFile, " bt_PageAddr[cardnum][0..3] = %5.5X %5.5X %5.5X %5.5X\n IO-Base = %X\n",
           (BT_U32BIT)(bt_PageAddr[cardnum][0]), (BT_U32BIT)bt_PageAddr[cardnum][1],
           (BT_U32BIT)bt_PageAddr[cardnum][2], (BT_U32BIT)bt_PageAddr[cardnum][3], bt_iobase[cardnum]);

   // Dump the stuff we can, return here if the board has not been init'ed.V4.09.ajh
   if (bt_inited[cardnum] == 0)  // V4.09.ajh
   {
      fprintf(hfMemFile, "API has not been initialized!!!\n");
      fclose(hfMemFile);         //  Close and save the debug data.
      return API_BUSTOOLS_NOTINITED;
   }

   // Add the following settings to the dump...
   // BusTools_SetInternalBus   - Sets the flag for external or internal bus
   // BusTools_SetOptions       - Sets Illegal command, Reset Timetag options
   // BusTools_SetVoltage       - Sets the voltage hardware register
   fprintf(hfMemFile, "\n SA 31 is Mode Code=%d, Broadcast Enabled=%d\n\n",
           rt_sa31_mode_code[cardnum], rt_bcst_enabled[cardnum]);

   // Dump the time tag information.
   {
      BT1553_TIME ctime;
      char outbuf[80];

      BusTools_TimeTagRead(cardnum, &ctime);
      BusTools_TimeGetString(&ctime,outbuf);
      fprintf(hfMemFile, " Time Tag Register: top usec = %x  microseconds = %x\n",ctime.topuseconds,ctime.microseconds);
      fprintf(hfMemFile, " Display Time = %s\n\n",outbuf);
   }

   DumpTimeTag(cardnum, hfMemFile);

   // Dump the local copies of the hardware registers and RAM registers.

   for ( block_id = 1; block_id <= GETADDR_COUNT; block_id++ )
   {
      BusTools_GetAddr(cardnum, block_id, &first, &last);
      first /= 2;   // Convert to word offset
      last  /= 2;   // Convert to word offset
      fprintf(hfMemFile, "%s Start %6.6X End %6.6X (Word Offsets)\n",
              BusTools_GetAddrName(block_id), first, last);
   }
   
   // Dump all of the BusTools_RegisterFunction() FIFO's.
   DumpRegisterFIFO(cardnum, hfMemFile);

   // Dump the RT message block pointers
   DumpRTptrs(cardnum, hfMemFile);

   // Dump the specified memory regions to the specified file.
   for ( block_id = 1; block_id <= GETADDR_COUNT; block_id++ )
   {
      /* This is where the specified buffers live */
      BusTools_GetAddr(cardnum, block_id, &first, &last);
      first /= 2;   // Convert to word offset
      last  /= 2;   // Convert to word offset
      fprintf(hfMemFile, "\n%s.Start %6.6X End %6.6X (Word Offsets)\n",
              BusTools_GetAddrName(block_id), first, last);
      if ( block_id == GETADDR_IQ )
         num_per_line = 15;
      else if ( block_id == GETADDR_RTCBUF_DEF )
         num_per_line = 15;
      else
         num_per_line = 16;
      // If region was not requested, don't dump it.
      if ( (region_mask & (1<<block_id)) == 0 )
         continue;
      if ( block_id == GETADDR_BCMESS )
         DumpBCmsg(cardnum, hfMemFile);           // Dump the BC msg area.
      else if ( block_id == GETADDR_BMMESSAGE )
         DumpBMmsg(cardnum, hfMemFile);           // Dump the BM msg area.
      else if ( block_id == GETADDR_BMFILTER )
         DumpBMflt(cardnum, hfMemFile);           // Dump the BM filter area.
      else if (block_id == GETADDR_HWREG)
      {      
         BusTools_DumpHWRegisters(cardnum,data);
         fprintf(hfMemFile, "%4.4X:",0x0);
         for ( j = 0; j < 16; j++)
         {
            fprintf(hfMemFile, " %4.4X", data[j]);
         }
         fprintf(hfMemFile, "\n");
         fprintf(hfMemFile, "%4.4X:",0x10);
         for ( j = 16; j < 32; j++)
         {
            fprintf(hfMemFile, " %4.4X", data[j]);
         }
         fprintf(hfMemFile, "\n");
      }
      else if (block_id == GETADDR_RAMREG)
      {      
         BusTools_DumpRAMRegisters(cardnum,data);
         for ( i=0; i<0x8; i++) // i += num_per_line )
         {
            fprintf(hfMemFile, "%4.4X:", i*0x10);
            for(j = 0;j<0x10;j++)
            {
               fprintf(hfMemFile, " %4.4X", data[j+i*0x10]);
            }
               fprintf(hfMemFile, "\n");
         }
      }
      else
      {
         // Dump all other requested regions to the file.
         for ( i = first; i < last; ) // i += num_per_line )
         {
            // Read the current line of num_per_line data words from the board.
            BusTools_MemoryRead(cardnum, i*2, num_per_line*2, data);
            fprintf(hfMemFile, "%4.4X:", i);
            for ( j = 0; j < num_per_line && i <= last; j++, i++ )
               fprintf(hfMemFile, " %4.4X", data[j]);
            fprintf(hfMemFile, "\n");
         }
      }
   }
   // Lastly dump the trace buffer, if available, and if any.
   DumpTrace(hfMemFile);

   fclose(hfMemFile);         //  Close and save the debug data.
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //FILE_SYSTEM
}



