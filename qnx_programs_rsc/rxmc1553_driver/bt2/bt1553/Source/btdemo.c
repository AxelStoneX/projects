/*============================================================================*
 * FILE:                        B T D E M O . C
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
 *             Module which supports the simulation or demo mode,
 *             which simulates the operation of a board without having the
 *             hardware actually installed.
 *
 * API ENTRY POINTS:
 *    none
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    vbtHardwareSimulation   Simulates the PCI-1553 board in DEMO mode.
 *
 * INTERNAL ROUTINES:
 *    vbtBMMessage            Helper routine for vbtHardwareSimulation.
 *
 *===========================================================================*/

/* $Revision:  5.26 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  05/13/1998   Split the demo-mode functions into this file from BTDEMO.C
               to help isolate the system-specific functions to that module.
               V2.47.ajh
  01/05/2000   Replaced the API-specific copy with generic memcpy.V3.30.ajh
  02/01/2000   Wrap the time tag counter around just like a PC-1553.V4.01.ajh
  05/18/2000   Added support for broadcast, fixed BC interrupt cause of
               "high word error" and made it "end of message".V4.02.ajh
  12/08/2000   Fixed problem in demo init, needed to set simulation flag before
               calling register write function.V4.27.ajh
  01/04/2000   Moved setting simulation flag into vbtSetup.V4.30.ajh
  12/01/2004   Change BTDEMO to run with a PCI-1553
 */

/*---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <busapi.h>
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"


#if defined(DEMO_CODE)
/*---------------------------------------------------------------------------*
 *                            Local Data
 *---------------------------------------------------------------------------*/
static long previous_frame_time[4];  // Used to keep track of minor frames


//
/****************************************************************************
*
*   Name:       vbtBMMessage
*
*   Abstract:   This routine is only used in the simulation version
*               of the driver.
*
*               This routine updates the contents of the BM message
*               at the specified address.  The current value of the
*               "bc_msg_pointer_save" register "RAMREG_BC_MSG_PTRSV(0x41)"
*               is read.  This pointer is used to fetch the current Bus
*               Controller message from the RAM area into a local
*               BC_MESSAGE structure.
*
*               Conditional and branch messages are processed, with the
*               RAMREG_BC_MSG_PTRSV RAM register getting updated to point to
*               the next message.
*               For 1553 messages the RAMREG_BC_MSG_PTRSV RAM register
*               is updated to point to the next BC message, via the fetched
*               "addr_next" field of the BC_MESSAGE structure.  The fetched
*               message is decoded into one of the following 1553 messages:
*               BC -> RT
*               RT -> BC
*               RT -> RT
*               Mode Code
*               Broadcast
*
*               The interrupt queue is updated with a packet for
*               1.  The BM entry.
*               2.  The BC entry.
*               3.  The RT entry, or two RT entries if RT->RT message.
*
*
*  PARAMETERS
*   BT_UINT   cardnum,        // (i) card number (0 - 3)
*   BT_U32BIT bm_addr         // (i) address of the BM message to fill in.
****************************************************************************/

static int vbtBMMessage(BT_UINT cardnum, BT_U16BIT bm_addr)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_U16BIT       mode;         // Used to decode message type.
   BT_U16BIT       count;        // Word count of current message.
   BT_U16BIT       saddr;        // Current subaddress.
   BT_U16BIT      *data;         // Pointer to the data word list.
   BT_U32BIT       dwBCAddr;     // Address of current BC message.
   BT_U32BIT       dwBMAddr;     // Address of current BM entry.
   BM_MBUF         bm_mbuf;      // Current BM entry.
   BC_MESSAGE      bcbuf;        // Current BC entry.
   BC_CBUF        *bccbuf;       // Alias if BC entry is a branch instr.
   BC_DBLOCK       bcdata;       // Data block from either the BC or RT.
   BT1553_COMMAND  command1;     // Transmiter/first command word.
   BT1553_COMMAND  command2;     // Second command word (only for RT - RT msg).
   unsigned int    i,j;          // Loop counters.
   BT_U16BIT       wSW, wSW2;    // 1553 status words.

   IQ_MBLOCK       int_queue;    // Interrupt queue entry.
   BT_U16BIT       wIQAddr;      // WORD address of interrupt queue entry.
   BT_U32BIT       dwIQAddr;     // DWORD address of interrupt queue entry.

   BT_U16BIT       waddr;        // WORD address of board parameter
   RT_CBUF         cbuf;         // Copy of the RT Control buffer.
   RT_CBUFBROAD    cbuf_bro;     // Copy of the RT Control buffer for broadcast.
   BT_U32BIT       fbuf_addr;    // Address of the RT Filter Buffer entry that points to the cbuf.
   BT_U32BIT       cbuf_addr;    // Address of the RT Control Buffer.
   BT_U32BIT       mbuf_addr;    // Address of the RT Message Buffer.
   RT_MBUF         rt_mbuf;      // Copy of the RT Message buffer.
   RT_MBUF         rt_mbuf_rtrt; // Copy of the Receive RT Message buffer for RT->RT msg
   BT_U32BIT       rt_rcv_mbuf_addr; // Address of the receive RT Message buffer, RT->RT msg
   BT_U16BIT       data_to_compare; // Data word to compare, conditional msg.

   /*******************************************************************
   *  Read current BM message from the caller-specified location
   *  (so we can have all the right link information).
   *******************************************************************/

   dwBMAddr = ((BT_U32BIT)bm_addr) << (4+0);
   vbtRead(cardnum, (LPSTR)&bm_mbuf, dwBMAddr, sizeof(bm_mbuf));

   /*******************************************************************
   *  Get current BC message from the location specified by the RAM
   *  register "RAMREG_BC_MSG_PTRSV(0x41)".  Update the RAM register
   *  to point to the next message in the BC list.
   *******************************************************************/

   dwBCAddr = vbtGetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV);
   dwBCAddr = ((BT_U32BIT)dwBCAddr) << (4+0);

   vbtRead(cardnum, (LPSTR)&bcbuf, dwBCAddr, sizeof(bcbuf));
   if ( bcbuf.control_word & BC_HWCONTROL_INTERRUPT ) // ints enabled this msg
   {
      bcbuf.mstatus[1] |= BT1553_INT_END_OF_MESS>>16;  // end of message
      vbtWrite(cardnum, (LPSTR)&bcbuf, dwBCAddr, sizeof(bcbuf));
   }

   /*******************************************************************
   *  Determine if this is a Bus Controller flow control message.
   *  If it is, perform the jump and return...V2.44
   *******************************************************************/
   if ( (bcbuf.control_word & BC_HWCONTROL_MESSAGE) == 0 )
   {
      bccbuf = (BC_CBUF *)&bcbuf; // Hard way to get a type cast.
      // If the message is a no op, take the next message...
      if ( bccbuf->control_word & BC_HWCONTROL_NOP || bccbuf->control_word == 0 )
      {
         // No op message, take the address of the next message.
         vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV, bccbuf->addr_next);
      }
      else if ( bccbuf->control_word & BC_HWCONTROL_CONDITION )
      {
         // Branch on condition message.  Test the condition and take
         //  the specified branch.  First fetch the data word we need
         //  to test.
         vbtRead(cardnum, (LPSTR)&data_to_compare, (BT_U32BIT)bccbuf->tst_wrd_addr1 << 4, 2);
         if ( (data_to_compare & bccbuf->bit_mask) == (bccbuf->data_pattern & bccbuf->bit_mask) )
         {
            // Condition is TRUE, take the branch message address.
            vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV, bccbuf->branch_msg_ptr);
         }
         else
         {
            // Condition is FALSE, take the next message address.
            vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV, bccbuf->addr_next);
         }
      }
      else if ( bccbuf->control_word & BC_HWCONTROL_MFRAMEEND)
      {
         vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV, bcbuf.addr_next);
         dwBCAddr = vbtGetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV);
         dwBCAddr = ((BT_U32BIT)dwBCAddr) << 4;
         vbtRead(cardnum, (LPSTR)&bcbuf, dwBCAddr, sizeof(bcbuf));
         if ( bcbuf.control_word & BC_HWCONTROL_MFRAMEBEG ) // beginning of minor frame
            return 1;
         else
           return 0;
      }
      else if ( bccbuf->control_word & BC_HWCONTROL_LASTMESS )
      {
         // Must be the last message in the block.  Branch to it and turn
         //  off the Bus Controller.
         vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV, bccbuf->addr_next);
         BusTools_BC_StartStop(cardnum, 0);
      }
      return bm_addr;      // We ignor conditionals and other control msgs.
   }

   // Update the BC pointer to point to the next message.
   vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV, bcbuf.addr_next);

   // Fill in message command parameters.
   bm_mbuf.time_tag.microseconds  = bmrec_timetag[cardnum].microseconds *20;        // Time tag.
   bm_mbuf.command1  = command1 = bcbuf.mess_command1;   // BC command word.
   command2 = bcbuf.mess_command2;                   // Second command word.
   bm_mbuf.status_c1 = 0;                      // No errors in command word.

   /*******************************************************************
   *  Determine message type -- mode word has following bits:
   *     1 -> RT->BC (0 -> BC->RT)
   *     2 -> BROADCAST
   *     4 -> MODECODE
   *     8 -> RT->RT
   ******************************************************************/

   if ( (count = command1.wcount) == 0 )
      count = 32;

   mode = command1.tran_rec;      // Set LSB if TRANSMIT, else clear LSB.

   if ( (command1.rtaddr == 31) && rt_bcst_enabled[cardnum] )
      mode |= 2;                  // Message is broadcast.

   saddr = command1.subaddr;
   if ( saddr == 0 )
      mode |= 4;                  // Message is mode code.
   else if ( (saddr == 31) && rt_sa31_mode_code[cardnum] )
      mode |= 4;                  // Message is mode code.

   if ( bcbuf.control_word & BC_HWCONTROL_RTRTFORMAT )
   {
      mode |= 8;                  // Message is RT-RT message
      /////////////////////////////////////////////////////////////////////
      // Message is RT - RT format, read the data from the source(transmit)
      //  RT.  Broadcast RT-RT is valid, but the transmitter cannot be RT 31.
      // This is the RT specified in the second command word.
      /*******************************************************************
      *  Compute the location in fbuf on the card which points to the
      *  transmit cbuf, then read the cbuf from the hardware.
      *******************************************************************/
      fbuf_addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
           (command2.rtaddr << 7) + (command2.tran_rec << 6) +
           (command2.subaddr << 1 ); // Byte offset.
      vbtRead(cardnum, (LPSTR)&waddr, fbuf_addr, sizeof(BT_U16BIT));

      cbuf_addr = (waddr << 1) + btmem_rt_begin[cardnum];
      vbtRead(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));
      mbuf_addr = (((BT_U32BIT)(cbuf.message_pointer)) << 4) + btmem_rt_begin[cardnum];

      // Now read the transmit RT_MESSAGE_BUFFER "RT_MBUF"
      vbtRead(cardnum, (LPSTR)&rt_mbuf, mbuf_addr, sizeof(rt_mbuf));
      cbuf.message_pointer = rt_mbuf.hw.nxt_msg_ptr;
      vbtWrite(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));

      // Update the transmit RT message buffer with the new TA/TR/SA/WC data,
      //  then write the buffer back to simulated HW RAM:
      rt_mbuf.hw.mess_command = command2;
      vbtWrite(cardnum, (LPSTR)&rt_mbuf, mbuf_addr, sizeof(rt_mbuf));

      // Now we have the data in rt_mbuf.  Convert it to bcdata and
      // pretend it is a BC -> RT message.
      memcpy(bcdata.word, rt_mbuf.hw.mess_data, 64);
      vbtWrite(cardnum, (LPSTR)&bcdata, ((BT_U32BIT)bcbuf.addr_data1) << 4,
                  sizeof(bcdata));
      rt_mbuf.hw.time_tag.microseconds  = bmrec_timetag[cardnum].microseconds * 20;
      vbtWrite(cardnum, (LPSTR)&rt_mbuf, mbuf_addr, sizeof(rt_mbuf));

      /////////////////////////////////////////////////////////////////////
      // RT -> RT format, read the Cbuf from the destination (Receive) RT.
      // This is the RT specified in the first command word.
      // Broadcast RT-RT is valid so the receiver can be RT 31 (Broadcast).
      /*******************************************************************
      *  Compute the location in fbuf on the card which points to the
      *  desired cbuf, then read the cbuf from the hardware.
      *******************************************************************/
      cbuf_addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
           (command1.rtaddr << 7) + (command1.tran_rec << 6) +
           (command1.subaddr << 1 ); // Byte offset.
      vbtRead(cardnum, (LPSTR)&waddr, cbuf_addr, sizeof(BT_U16BIT));

      cbuf_addr = (waddr << 1) + btmem_rt_begin[cardnum];

      // Handle broadcast.  Read the cbuf as either normal or broadcast...V4.02
      if ( (command1.rtaddr == 31) && rt_bcst_enabled[cardnum] )  // V4.02
      {
         vbtRead(cardnum, (LPSTR)&cbuf_bro, cbuf_addr, sizeof(cbuf_bro));
         rt_rcv_mbuf_addr = (((BT_U32BIT)(cbuf_bro.message_pointer)) << 4) + btmem_rt_begin[cardnum];
      }
      else
      {
         vbtRead(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));
         rt_rcv_mbuf_addr = (((BT_U32BIT)(cbuf.message_pointer)) << 4) + btmem_rt_begin[cardnum];
      }

      // Now read the RT_MESSAGE_BUFFER "RT_MBUF"
      vbtRead(cardnum, (LPSTR)&rt_mbuf_rtrt, rt_rcv_mbuf_addr, sizeof(rt_mbuf));

      // Update the cbuf to point to the next MBUF in the chain.
      if ( (command1.rtaddr == 31) && rt_bcst_enabled[cardnum] )  // V4.02
      {
         cbuf_bro.message_pointer = rt_mbuf.hw.nxt_msg_ptr;
         vbtWrite(cardnum, (LPSTR)&cbuf_bro, cbuf_addr, sizeof(cbuf_bro));
      }
      else
      {
         cbuf.message_pointer = rt_mbuf_rtrt.hw.nxt_msg_ptr;
         vbtWrite(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));
      }

      // Update the RT message buffer with the new TA/TR/SA/WC data,
      //  the BC message data, the current time tag, and
      //  then write the buffer back to simulated HW RAM:
      rt_mbuf_rtrt.hw.mess_command = command1;
      memcpy(rt_mbuf_rtrt.hw.mess_data, bcdata.word, 64);
      rt_mbuf_rtrt.hw.time_tag.microseconds  = bmrec_timetag[cardnum].microseconds*20;
      vbtWrite(cardnum, (LPSTR)&rt_mbuf_rtrt, rt_rcv_mbuf_addr, sizeof(rt_mbuf));
      /////////////////////////////////////////////////////////////////////
   }
   else
   {
      if ( mode & 1 )  // RT Transmit format
      {  // RT -> BC format, read the data from the source RT.
         // This is the RT specified in the first command word.
         /*******************************************************************
         *  A broadcast RT -> BC is illegal...
         *  Compute the location in fbuf on the card which points to the
         *  desired cbuf, then read the cbuf from the hardware.
         *******************************************************************/
         cbuf_addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
           (command1.rtaddr << 7) + (command1.tran_rec << 6) +
           (command1.subaddr << 1 ); // Byte offset.
         vbtRead(cardnum, (LPSTR)&waddr, cbuf_addr, sizeof(BT_U16BIT));

         cbuf_addr = (waddr << 1) + btmem_rt_begin[cardnum];
         if ( (command1.rtaddr == 31) && rt_bcst_enabled[cardnum] ) // V4.02
         {
            vbtRead(cardnum, (LPSTR)&cbuf_bro, cbuf_addr, sizeof(cbuf_bro));
            mbuf_addr = (((BT_U32BIT)(cbuf_bro.message_pointer)) << 4) + btmem_rt_begin[cardnum];
         }
         else
         {
            vbtRead(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));
            mbuf_addr = (((BT_U32BIT)(cbuf.message_pointer)) << 4) + btmem_rt_begin[cardnum];
         }

         // Now read the RT_MESSAGE_BUFFER "RT_MBUF"
         vbtRead(cardnum, (LPSTR)&rt_mbuf, mbuf_addr, sizeof(rt_mbuf));

         // Update the cbuf to point to the next MBUF in the chain.
         if ( (command1.rtaddr == 31) && rt_bcst_enabled[cardnum] )   // V4.02
         {
            cbuf_bro.message_pointer = rt_mbuf.hw.nxt_msg_ptr;
            vbtWrite(cardnum, (LPSTR)&cbuf_bro, cbuf_addr, sizeof(cbuf_bro));
         }
         else
         {
            cbuf.message_pointer = rt_mbuf.hw.nxt_msg_ptr;
            vbtWrite(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));
         }

         // Update the RT message buffer with the new TA/TR/SA/WC data,
         //  then write the buffer back to simulated HW RAM:
         rt_mbuf.hw.mess_command.rtaddr   = (BT_U16BIT)command1.rtaddr;
         rt_mbuf.hw.mess_command.tran_rec = (BT_U16BIT)command1.tran_rec;
         rt_mbuf.hw.mess_command.subaddr  = (BT_U16BIT)command1.subaddr;
         rt_mbuf.hw.mess_command.wcount   = (BT_U16BIT)command1.wcount;
         vbtWrite(cardnum, (LPSTR)&rt_mbuf, mbuf_addr, sizeof(rt_mbuf));

         // Now we have the data in rt_mbuf.  Convert it to bcdata and
         // pretend it is a BC -> RT message.
         memcpy(bcdata.word, rt_mbuf.hw.mess_data, 64);
         vbtWrite(cardnum, (LPSTR)&bcdata, ((BT_U32BIT)bcbuf.addr_data1) << 4,
                  sizeof(bcdata));
         rt_mbuf.hw.time_tag.microseconds  = bmrec_timetag[cardnum].microseconds*20;
         vbtWrite(cardnum, (LPSTR)&rt_mbuf, mbuf_addr, sizeof(rt_mbuf));
      }
      else  // RT Receive format
      {  // BC -> RT format, Data buffer is from the BC, so go read it.
         vbtRead(cardnum, (LPSTR)&bcdata, ((BT_U32BIT)bcbuf.addr_data1) << 4,
                 sizeof(bcdata));

         // BC -> RT format, read the Cbuf from the destination RT.
         // This is the RT specified in the first command word.
         /*******************************************************************
         *  A broadcast BC -> RT is legal, just read the correct cbuf format.
         *  Compute the location in fbuf on the card which points to the
         *  desired cbuf, then read the cbuf from the hardware.
         *******************************************************************/
         cbuf_addr = BTMEM_RT_FBUF + btmem_rt_begin[cardnum] +
           (command1.rtaddr << 7) + (command1.tran_rec << 6) +
           (command1.subaddr << 1 ); // Byte offset.
         vbtRead(cardnum, (LPSTR)&waddr, cbuf_addr, sizeof(BT_U16BIT));

         cbuf_addr = (waddr << 1) + btmem_rt_begin[cardnum];

         // Handle broadcast.  Read the cbuf as either normal or broadcast...
         if ( (command1.rtaddr == 31) && rt_bcst_enabled[cardnum] )  // V4.02
         {
            vbtRead(cardnum, (LPSTR)&cbuf_bro, cbuf_addr, sizeof(cbuf_bro));
            mbuf_addr = (((BT_U32BIT)(cbuf_bro.message_pointer)) << 4) + btmem_rt_begin[cardnum];
         }
         else
         {
            vbtRead(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));
            mbuf_addr = (((BT_U32BIT)(cbuf.message_pointer)) << 4) + btmem_rt_begin[cardnum];
         }

         // Now read the RT_MESSAGE_BUFFER "RT_MBUF"
         vbtRead(cardnum, (LPSTR)&rt_mbuf, mbuf_addr, sizeof(rt_mbuf));

         // Update the cbuf to point to the next MBUF in the chain.
         if ( (command1.rtaddr == 31) && rt_bcst_enabled[cardnum] )  // V4.02
         {
            cbuf_bro.message_pointer = rt_mbuf.hw.nxt_msg_ptr;
            vbtWrite(cardnum, (LPSTR)&cbuf_bro, cbuf_addr, sizeof(cbuf_bro));
         }
         else
         {
            cbuf.message_pointer = rt_mbuf.hw.nxt_msg_ptr;
            vbtWrite(cardnum, (LPSTR)&cbuf, cbuf_addr, sizeof(cbuf));
         }

         // Update the RT message buffer with the new TA/TR/SA/WC data,
         //  the BC message data, the current time tag, and
         //  then write the buffer back to simulated HW RAM:
         rt_mbuf.hw.mess_command.rtaddr   = (BT_U16BIT)command1.rtaddr;
         rt_mbuf.hw.mess_command.tran_rec = (BT_U16BIT)command1.tran_rec;
         rt_mbuf.hw.mess_command.subaddr  = (BT_U16BIT)command1.subaddr;
         rt_mbuf.hw.mess_command.wcount   = (BT_U16BIT)command1.wcount;
         memcpy(rt_mbuf.hw.mess_data, bcdata.word, 64);
         rt_mbuf.hw.time_tag.microseconds  = bmrec_timetag[cardnum].microseconds*20;
         vbtWrite(cardnum, (LPSTR)&rt_mbuf, mbuf_addr, sizeof(rt_mbuf));

         // Now we have updated the BC_MBUF, write it back to the HW.
         vbtWrite(cardnum, (LPSTR)&bcdata, ((BT_U32BIT)bcbuf.addr_data1) << 4,
                  sizeof(bcdata));
      }
   }

   /*******************************************************************
   *  Setup the BM message interrupt status.
   *******************************************************************/
   bm_mbuf.int_status = BT1553_INT_END_OF_MESS;

   /*******************************************************************
   *  Convert message to Bus Monitor format according to the message
   *  type decoded above.
   *******************************************************************/
   wSW  = (BT_U16BIT)(bcbuf.mess_command1.rtaddr<<11); // Status word, first cmd wrd
   wSW2 = (BT_U16BIT)(bcbuf.mess_command2.rtaddr<<11); // Status word, second cmd wrd

   data = (BT_U16BIT *)bm_mbuf.data;
   memset(bm_mbuf.data, 0, 74*2);        // Initialize the data to be zero.
   i = 0;
   switch(mode)
   {
      case 0:     // BC->RT message
         bmrec_timetag[cardnum].microseconds += (count+3)*20;    // Estimated bus time.
         for ( j = 0; j < count; j++ )
         {
            data[i++] = bcdata.word[j];
            data[i++] = 0;                    // No Errors...
         }
         // i = 2*count;
         data[i++] = 9;     // response1 time
         data[i++] = wSW;   // 1553 status word 1
         data[i]   = 0;    // no status word errors
         break;

      case 1:     // RT->BC message
         bmrec_timetag[cardnum].microseconds += (count+3)*20;    // Estimated bus time.
         data[i++]  = 9;    // response1 time
         data[i++]  = wSW;  // 1553 status word 1
         data[i++]  = 0;    // no status word errors
         for ( j = 0; j < count; j++ )
         {
            data[i++] = bcdata.word[j];    // data word
            data[i++] = 0;                      // No errors...
         }
         break;

      case 2:     // BROADCAST BC->RT message
         bmrec_timetag[cardnum].microseconds += (count+2)*20;    // Estimated bus time.
         for ( j = 0; j < count; j++ )
         {
            data[i++] = bcdata.word[j];    // data word
            data[i++] = 0;                      // No errors...
         }
         break;

      case 4:     // Modecode (receive -- always has one data word)
         bmrec_timetag[cardnum].microseconds += 65;    // Estimated bus time.V4.01
         data[0] = bcdata.word[0];
         data[1] = 0;
         data[2] = 11;    // response1 time
         data[3] = wSW;   // 1553 status word 1
         data[4]  = 0;    // no status word errors
         break;

      case 5:     // Modecode (transmit -- either with or without data)
         bmrec_timetag[cardnum].microseconds += 65;    // Estimated bus time.V4.01
         data[0] = 11;   // response1 time
         data[1] = wSW;  // 1553 status word 1
         data[2] = 0;    // no status word errors
         data[3] = bcdata.word[0];      // Move data anyway
         data[4] = 0;                        // No errors.
         break;

      case 6:     // BROADCAST Modecode (receive -- always has one data word)
         bmrec_timetag[cardnum].microseconds += 65;    // Estimated bus time.V4.01
         data[0] = bcdata.word[0]  = data[0];
         data[1] = 0;                         // No errors.
         break;

      case 8:     // RT->RT command
         bmrec_timetag[cardnum].microseconds += (count+5)*20;    // Estimated bus time.
         bm_mbuf.int_status |= BT1553_INT_RT_RT_FORMAT;
         data[0] = *(BT_U16BIT *)&bcbuf.mess_command2; // mbuf_user->command2
         data[1] = 0;                     // mbuf_user->status_c2
         data[2] = 11;                    // response1 time
         data[3] = wSW;                   // 1553 status word 1
         data[4] = 0;                     // no status word errors
         i = 5;
         for ( j = 0; j < count; j++ )
         {
            data[i++] = bcdata.word[j];
            data[i++] = 0;                   // No errors.
         }
         //  i = 2*count + 5;
         data[i++] = 9;      // response2 time,
         data[i++] = wSW2;   // second 1553 status word
         data[i]  = 0;       // No status word error.
         break;

      case 10:    // BROADCAST RT->RT command
         bmrec_timetag[cardnum].microseconds += (count+4)*20;    // Estimated bus time.
         bm_mbuf.int_status |= BT1553_INT_RT_RT_FORMAT;
         data[0] = *(BT_U16BIT *)&bcbuf.mess_command2; // mbuf_user->command2
         data[1] = 0;                     // mbuf_user->status_c2
         data[2] = 11;                    // mbuf_user->response1
         data[3] = wSW;                   // mbuf_user->status1
         data[4] = 0;                     // mbuf_user->status_s1
         i = 5;
         for ( j = 0; j < count; j++ )
         {
            data[i++] = bcdata.word[j];
            data[i++] = 0;  // No errors
         }
         break;

      case 7:     // BROADCAST Modecode (transmit -- only without data)
         bmrec_timetag[cardnum].microseconds += 30;    // Estimated bus time.
         // RT's with broadcast enabled shall set the broadcast received
         //  bit in the status word and not transmit the status word.
         break;

      case 3:     // ILLEGAL (BROADCAST RT->BC message)
      case 9:     // ILLEGAL (RT->RT with first command being transmit)
      case 11:    // ILLEGAL (BROADCAST RT->RT, first command being transmit)
      case 12:    // ILLEGAL (RT->RT and Modecode)
      case 13:    // ILLEGAL (RT->RT and Modecode)
      case 14:    // ILLEGAL (RT->RT and Broadcast and Modecode)
      case 15:    // ILLEGAL (RT->RT and Broadcast and Modecode)
         break;
   }

   /*******************************************************************
   *  Store new Bus Monitor message in simulated board RAM
   *******************************************************************/
   vbtWrite(cardnum, (LPSTR)&bm_mbuf, dwBMAddr, sizeof(bm_mbuf));

   /*******************************************************************
   *  Generate the interrupt queue entries for this message.
   *******************************************************************/
   // Generate a Bus Monitor interrupt if enabled.
   if ( bm_mbuf.int_enable )
   {
      wIQAddr = vbtGetFileRegister(cardnum, RAMREG_INT_QUE_PTR);
      dwIQAddr = ((BT_U32BIT)wIQAddr) << 1;
      vbtRead(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));

      int_queue.t.modeword = 0x0008;     // BM interrupt.

      int_queue.msg_ptr = bm_addr;
      vbtWrite(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));
      wIQAddr = int_queue.nxt_int;
      vbtSetFileRegister(cardnum, RAMREG_INT_QUE_PTR, wIQAddr);
   }

   // Generate a Bus Controller interrupt if enabled.
   if ( bcbuf.mstatus[0] | bcbuf.mstatus[1] )
   {
      wIQAddr = vbtGetFileRegister(cardnum, RAMREG_INT_QUE_PTR);
      dwIQAddr = ((BT_U32BIT)wIQAddr) << 1;
      vbtRead(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));

      int_queue.t.modeword = 0x0010;  // BC interrupt.

      int_queue.msg_ptr = (BT_U16BIT)(dwBCAddr >> 4);
      vbtWrite(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));
      wIQAddr = int_queue.nxt_int;
      vbtSetFileRegister(cardnum, RAMREG_INT_QUE_PTR, wIQAddr);
   }

   // Generate a Remote Terminal interrupt if enabled.
   if ( rt_mbuf.hw.enable )
   {
      wIQAddr = vbtGetFileRegister(cardnum, RAMREG_INT_QUE_PTR);
      dwIQAddr = ((BT_U32BIT)wIQAddr) << 1;
      vbtRead(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));

      int_queue.t.modeword = 0x0004;  // RT interrupt.

      int_queue.msg_ptr = (BT_U16BIT)(mbuf_addr>>4); // RT msg which caused interrupt.
      vbtWrite(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));
      wIQAddr = int_queue.nxt_int;
      vbtSetFileRegister(cardnum, RAMREG_INT_QUE_PTR, wIQAddr);
      // If this is an RT->RT message, we need to generate the interrupt for
      //  the receive side of the message, e.g., the receive RT.
      if ( mode == 8 )   // Is message RT->RT??
      {
         if ( rt_mbuf_rtrt.hw.enable )   // Is interrupt enabled?
         {
         wIQAddr = vbtGetFileRegister(cardnum, RAMREG_INT_QUE_PTR);
         dwIQAddr = ((BT_U32BIT)wIQAddr) << 1;
         vbtRead(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));

         int_queue.t.modeword = 0x0004;  // RT interrupt.

         int_queue.msg_ptr = (BT_U16BIT)(rt_rcv_mbuf_addr>>4); // RT msg which caused interrupt.
         vbtWrite(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));
         wIQAddr = int_queue.nxt_int;
         vbtSetFileRegister(cardnum, RAMREG_INT_QUE_PTR, wIQAddr);
         }
      }
   }

   /*******************************************************************
   *  Update to the address of the next BM message queue entry.
   *******************************************************************/
   vbtSetFileRegister(cardnum, RAMREG_BM_PTR_SAVE, bm_mbuf.next_mbuf);

   /*******************************************************************
   *  Add in the inter-message gap time.
   *******************************************************************/
   bmrec_timetag[cardnum].microseconds += bcbuf.gap_time;    // Estimated bus time.

   /*******************************************************************
   *  See if the simulated time tag (for a PCI-1553) has overflowed,
   *   and if so, put the entry in the interrupt queue and wrap the
   *   value just like the hardware does...V4.01.ajh
   *******************************************************************/
   if ( bmrec_timetag[cardnum].microseconds > 214748365L )
   {
      bmrec_timetag[cardnum].microseconds -= 214748365L;  // Wrap the counter around
      previous_frame_time[cardnum] -= 214748365L;
      // Add a time tag counter overflow message to the interrupt queue
      wIQAddr = vbtGetFileRegister(cardnum, RAMREG_INT_QUE_PTR);
      dwIQAddr = ((BT_U32BIT)wIQAddr) << 1;
      vbtRead(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));

      int_queue.t.modeword = 0x0002;  // Time Tag interrupt.

      int_queue.msg_ptr = 0;          // Msg address is null.
      vbtWrite(cardnum, (LPSTR)&int_queue, dwIQAddr, sizeof(IQ_MBLOCK));
      wIQAddr = int_queue.nxt_int;
      vbtSetFileRegister(cardnum, RAMREG_INT_QUE_PTR, wIQAddr);
   }

   /*******************************************************************
   *  Read next BC message, tell caller if it is beginning of minor frame.
   *******************************************************************/
   dwBCAddr = vbtGetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV);
   dwBCAddr = ((BT_U32BIT)dwBCAddr) << 4;
   vbtRead(cardnum, (LPSTR)&bcbuf, dwBCAddr, sizeof(bcbuf));
   if ( bcbuf.control_word & BC_HWCONTROL_MFRAMEBEG ) // beginning of minor frame
      return 1;
   else
      return 0;
}

//***************************************************************************
//
//  Name:       vbtHardwareSimulation
//
//  Abstract:   This function is called by the Windows multimedia timer
//              callback procedure vbtTimerCallback() to simulate the
//              BusTools board by generating simulated interrupts which
//              mimic the operation of the real board.
//
//***************************************************************************
void vbtHardwareSimulation(BT_UINT cardnum)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_U16BIT      wBMAddr;
   int            end_of_frame;
   static long    waittime[4] = {0, 0, 0, 0};

   if ( waittime[cardnum] )
      waittime[cardnum]--;         // Decrement the wait time

   /*******************************************************************
   *  Generate a Message every now and again.  Read the Bus Controller
   *  bus list, copy the message into the Bus Monitor buffer, and
   *  generate a BM, BC and/or RT interrupt queue entry, depending on
   *  which or any of these interrupt functions is enabled.
   *  frametime[cardnum] is in units of 25 microseconds.
   *******************************************************************/
   if ( waittime[cardnum] )
      return;
   if ( bc_running[cardnum] == 0 )
      return;

   if ( bmrec_timetag[cardnum].microseconds == 0 )
      previous_frame_time[cardnum] = 0;

   wBMAddr = vbtGetFileRegister(cardnum, RAMREG_BM_PTR_SAVE);
   end_of_frame = vbtBMMessage(cardnum, wBMAddr);

   /*******************************************************************
   *  Update stuff used to fill in message.
   *******************************************************************/
   if ( end_of_frame )
   {
      waittime[cardnum] = ((long)frametime[cardnum]*25L)/10000; // Simulated time 10 ms ticks.
      bmrec_timetag[cardnum].microseconds = previous_frame_time[cardnum] +
                           (long)frametime[cardnum]*25L;
      previous_frame_time[cardnum] = bmrec_timetag[cardnum].microseconds;
   }
   else
      waittime[cardnum] = 1;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:  v b t D e m o S e t u p
 *===========================================================================*
 *
 * FUNCTION:    Setup Demo Hardware Emulation Mode.
 *
 * DESCRIPTION: The software demo mode is initialized.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/

BT_INT vbtDemoSetup(BT_UINT cardnum)             // (i) card number
{
   int    i;

   /****************************************************************
   *  Simulation Version, allocate 4*1024 Kbytes of simulated RAM,
   *   in 4 blocks of 1024 Kbytes each.
   ****************************************************************/
   CurrentCardType[cardnum] = QPCI1553;        // the only basic card left.
   CurrentMemKB[cardnum]    = 1024;            // Number of 1KB memory blocks
   btmem_rt_begin[cardnum]  = BT_RT_BASE_PCI;  // RT is in segment 2 

   board_has_testbus[cardnum] = 1;
   board_has_discretes[cardnum] = 1;
   board_has_differential[cardnum] = 1;
   numDiscretes[cardnum] = 10;
   bt_dismask[cardnum] = 0x3ff;

   for ( i = 0; i < 4; i++ )
   {
      bt_PageAddr[cardnum][i] =
        (char *)CEI_MALLOC(0x100000L);  // 1024KB buffers.
      if (bt_PageAddr[cardnum][i] == NULL)
         return BTD_ERR_NOMEMORY;
   }

   /****************************************************************
   *  Write zeros to all simulated H/W registers, set ucode version.
   ****************************************************************/
   // The software version of the driver must be setup before calling
   //  this function (e.g. set bt_inuse[cardnum] = -1;)     V4.30.ajh
   for (i = 0; i < HWREG_COUNT2; i++)
      api_writehwreg(cardnum, i, 0);

   return BTD_OK;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:  v b t D e m o S h u t d o w n
 *===========================================================================*
 *
 * FUNCTION:    Shutdown Demo Hardware Emulation Mode.
 *
 * DESCRIPTION: The software demo mode is closed.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/

BT_INT vbtDemoShutdown(BT_UINT cardnum)             // (i) card number
{
   int pagenum;

   for (pagenum = 0; pagenum < BT_NUM_PAGES; pagenum++)
   {
      if (bt_PageAddr[cardnum][pagenum] != NULL)
         CEI_FREE(bt_PageAddr[cardnum][pagenum]);
      bt_PageAddr[cardnum][pagenum] = NULL;
   }
   return BTD_OK;
}
#endif // #if defined(DEMO_CODE)
