/*============================================================================*
 * FILE:                      A P I I N T . H
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
 * FUNCTION:    BusTools API library:
 *              This file contains definitions used internally by the API code.
 *              Most of the definitions describe hardware related structures,
 *              word offsets and bit positions.
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/11/1996   Modified RAMREG_RESERVED1 (Register 78) to be RAMREG_FLAGS.
               Added definition for the "new_minor_flag" bit in the RAM
               Register File word 78hex.  This should fix the problem
               in OneShot mode which caused the messages to be delayed by up
               to a full minor frame time.  Bug Report A-000012.ajh
  09/02/1996   Modified to support 32-bit operation under Win95 and WinNT.ajh
  03/17/1997   Merged 16- and 32-bit versions.V2.21.ajh
  04/04/1997   Modified to support IP module.  Removed unused space in the
               BM_TBUF and the BC_MESSAGE and the BC_CBUF.V2.22.ajh
  07/13/1997   Added support for the 64 Kw IP module, RT mode.V2.30.ajh
  01/05/1998   Removed WinRT function definitions into LOWLEVEL.H.V2.43.ajh
  05/13/1998   Split INIT.C and BTDRV.C into new files to help isolate the
               system-specific functions to individual files.V2.47.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  08/03/2000   Added RAMREG_RT_DISA and RAMREG_RT_DISB to support LPU V3.05 and
               WCS V3.07.  Added RAMREG_RT_ITF 8/9/00.v4.09.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  12/12/2000   Changed time tag register definitions for V3.08/3.06 FW.V4.28.ajh
  01/15/2001   Added enhanced BM trigger buffer definition, reduced the size of
               the EI buffer by one entry to fit the BM trigger buffer.V4.31.ajh
  04/18/2001   Changed the file register definitions for WCS V3.20 for
               non-IP-1553 products.V4.38.ajh
  02/07/2002   Added support for modular design v4.46
  03/15/2002   Added IRIG Support. V4.48
  11/19/2007   Added prototype for vbtSetPLXRegister8 and vbtSetPLXRegister32 to 
               support DMA for PLX 9056 boards.
  11/19/2007   Added PLX DMA Defines.
  11/19/2007   Add defines for Fixed gap monitor invalid commands and undefined 
               mode code illegal. 
 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/
#ifndef APIINT_H
#define APIINT_H
#if defined(ADD_TRACE)
#include "apinames.h"  /* Only used by trace function */
#endif

/**********************************************************************
*  Swap the two words in a dword, two bytes in a word
**********************************************************************/

#ifdef NON_INTEL_WORD_ORDER
#define flip(longword) *(longword)= \
    ((*(longword) >> 16) & 0x0000ffff) | \
    ((*(longword) << 16) & 0xffff0000)
#else
#define flip(longword)
#endif

#ifndef NON_INTEL_WORD_ORDER
#define flip32(longword) *(longword)= \
    ((*(longword) >> 16) & 0x0000ffff) | \
    ((*(longword) << 16) & 0xffff0000)
#else
#define flip32(longword)
#endif

#ifdef NON_INTEL_WORD_ORDER
#define flips(longword) (((longword>>16)&0x0000ffff) | ((longword<<16)&0xffff0000))
#else
#define flips(longword) longword
#endif

#ifdef NON_INTEL_WORD_ORDER
#define little_endian_32(longword) (((longword & 0x000000ff) << 24) | \
                            ((longword & 0x0000ff00) << 8)  | \
                            ((longword & 0xff0000)   >> 8)  | \
                            ((longword & 0xff000000) >> 24))
#else
#define little_endian_32(longword) longword
#endif

#ifdef WORD_SWAP
#define flipw(word) *(word)= \
    (short)(((*(word) >> 8) & 0x00ff) | \
            ((*(word) << 8) & 0xff00))
#else
#define flipw(word)
#endif

#ifdef WORD_SWAP
#define flipws(word) (short)(((word >> 8) & 0x00ff) | ((word << 8) & 0xff00))
#else
#define flipws(word) word
#endif

#ifdef NON_INTEL_WORD_ORDER
#define fliplong(longword)  *(longword)= (((*longword & 0x000000ff) << 24) | \
                            ((*longword & 0x0000ff00) << 8)  | \
                            ((*longword & 0xff0000)   >> 8)  | \
                            ((*longword & 0xff000000) >> 24))
#else
#define fliplong(longword)
#endif

/**********************************************************************
*  Eliminate the Windows-specific MessageBox
**********************************************************************/
#if !defined(_Windows) && !defined(_LVRT_)
 #define MessageBox(p1, p2, p3, p4)
#endif

/**********************************************************************
*  Bus Controller Definitions
**********************************************************************/

#define BC_HWCONTROL_NOP        0x0000  //
#define BC_HWCONTROL_OP         0x0001  //
#define BC_HWCONTROL_SET_NOP    ~BC_HWCONTROL_OP // 0xFFFE
#define BC_HWCONTROL_SET_TNOP   0x4000
#define BC_HWCONTROL_CLEAR_TNOP 0xbfff// 1011 1111 1111 1111
 
// These bits defined only if BC Control word bit 0 = 0...
#define BC_HWCONTROL_MESSAGE    0x0002  // 1 -> BC Message
#define BC_HWCONTROL_LASTMESS   0x0004  // 1 -> last message in major frame
#define BC_HWCONTROL_CONDITION  0x0006  // 1 -> branch on condition

// These bits defined only if BC Control word bit 0 = 1....
#define BC_HWCONTROL_MFRAMEBEG  0x0008  // 1 -> beg of minor frame
#define BC_HWCONTROL_MFRAMEEND  0x0080  // 1 -> end of minor frame
#define BC_HWCONTROL_RTRTFORMAT 0x0020  // 1 -> rt-to-rt format
#define BC_HWCONTROL_RETRY      0x0400  // 1 -> retry enabled 

#define BC_HWCONTROL_INTERRUPT  0x0010  // 1 -> interrupts enabled for this msg
#define BC_HWCONTROL_INTQ_ONLY  0x1000  // 1 -> No H/W interrupt)

#define BC_HWCONTROL_BUFFERB    0x0000  // 0 -> use buffer b 
#define BC_HWCONTROL_BUFFERA    0x0040  // 1 -> use buffer a

#define BC_HWCONTROL_CHANNELA   0x0000  // 0 -> use channel a 
#define BC_HWCONTROL_CHANNELB   0x0100  // 1 -> use channel b 

/**************************************************
*  IRIG Register Definitions
**************************************************/
#define IRIG_CTL_BASE           0x0100
#define IRIG_DAC_REG            0x0000
#define IRIG_CNTL_REG           0x0002
#define IRIG_TOY_REG_LSB        0x0004
#define IRIG_TOY_REG_MSB        0x0006
#define DAC_MIN                 0x0000
#define DAC_MAX                 0x00ff
#define MIN_DAC_LEVEL           0x10
#define IRIG_DEFAULT_DAC        155

/**************************************************
*  QPMC Discrete Register Definitions
***************************************************/
#define DISCRETE_OUT_LSB        0x0008
#define DISCRETE_OUT_MSB        0x000a

/**************************************************
*  QPCI BIT LEDS
***************************************************/
#define QPCI_BIT_FAIL 0x2
#define QPCI_BIT_PASS 0x1
#define QPCI_BIT_OFF  0x0

/**************************************************
*  PLX 9056 DMA DEFINES
***************************************************/
#define PLX_DMA_MODE0        0x80
#define PLX_DMA_HOST_ADDR0	 0x84
#define PLX_DMA_BOARD_ADDR0  0x88
#define PLX_DMA_BYTE_COUNT0  0x8c
#define PLX_DMA_FLAG0        0x90
#define PLX_DMA_STATUS0      0xa8

#define PLX_DMA_MODE1        0x94
#define PLX_DMA_HOST_ADDR1	 0x98
#define PLX_DMA_BOARD_ADDR1  0x9c
#define PLX_DMA_BYTE_COUNT1  0xa0
#define PLX_DMA_FLAG1        0xa4
#define PLX_DMA_STATUS1      0xa9

#define PLX_DMA_THRESHOLD    0xb0

/**************************************************
*  BC Message Block
**************************************************/

typedef struct bc_message
   {
   BT_U16BIT      control_word;    // Control and function definition
   BT1553_COMMAND mess_command1;   // First 1553 control word
   BT_U16BIT      addr_error_inj;  // Address of error injection buffer
   BT_U16BIT      gap_time;        // Intermessage gap time in microseconds
   BT_U16BIT      addr_data1;      // Word address data buffer A (PCI-1553: 8-word address)
   BT_U16BIT      addr_data2;      // Word address data buffer B (PCI-1553: 8-word address) or rep-rate w/Msg Sched option
   BT1553_COMMAND mess_command2;   // Second 1553 control word (RT->RT transmit) 
   BT1553_STATUS  mess_status1;    // First 1553 status word (RT->RT transmit)
   BT1553_STATUS  mess_status2;    // Second 1553 status word (RT->RT receive)
   BT_U16BIT      mstatus[2];      // Message status: protocol errors, et.al.  Avoid mis-aligned long.
   BT_U16BIT      addr_next;       // Address of next message to be processed (PCI-1553: 8-word address)
   BT1553_TIME    timetag PACKED;  // Tag Time for F/W v3.97 and greater
   BT_U16BIT      start_frame;     // Used with word 5 (addr_data2) when using message scheduling other un-used 
   BT_U16BIT      gap_time2;
   BT_U16BIT      reserve[7];
   }
BC_MESSAGE;

/**************************************************
* BC Control word bits, FOR IP Ver 2.x and above!
**************************************************/

/********************************************************
*  BC Conditional/Control/Noop Message
********************************************************/
typedef struct bc_cbuf
   {
   BT_U16BIT  control_word;    // Control and function definition
   BT_U16BIT  tst_wrd_addr1;   // Most significant 16 bits, or entire addr
   BT_U16BIT  tst_wrd_addr2;   // Least significant 3 bits of word addr (PCI-1553)
   BT_U16BIT  data_pattern;    // Fixed value to compare with test word
   BT_U16BIT  bit_mask;        // Mask for compare
   BT_U16BIT  cond_count_val;  // Conditional count value (Not used, set to zero)
   BT_U16BIT  cond_counter;    // Hardware cond counter   (Not used, set to zero)
   BT_U16BIT  res1;
   BT_U16BIT  res2;
   BT_U16BIT  res3;
   BT_U16BIT  branch_msg_ptr;  // Address of message to execute if condition TRUE (8-word if PCI-1553)
   BT_U16BIT  addr_next;       // Address of next message or msg if test FALSE (8-word if PCI-1553)
   BT1553_TIME timetag PACKED; // Tag Time for F/W v3.97 and greater only on a stop block
   BT_U16BIT  tbd1;
   BT_U16BIT  reserved[8];
   }
BC_CBUF;

/************************************************
*  BC Data Block
************************************************/

typedef struct bc_dblock
   {
   BT_U16BIT word[34];
   }
BC_DBLOCK;

/************************************************
*  BC Data Block
************************************************/
typedef struct branch
{
   int messno;
   int next_messno;
   int branch_messno;
}BRANCH;

typedef struct bc_frame
{
   int frame_number;
   int message_count;        // number of total message
   int mc_count;
   int bc_rt_count;
   int rt_bc_count;
   int rt_rt_count;
   int brdcst_count;
   int total_word_count;
   int total_data_words;
   unsigned cum_gap_time;
   unsigned cum_resp_time;
   unsigned max_frame_time;
   unsigned nom_frame_time;
   unsigned min_frame_time;
   int nop_count;
   int frame_start;
   int frame_end;
   int next_frame;
   int branch_count;
   BRANCH condition[50];
}BC_FRAME, *pBC_FRAME;


typedef struct channel_share
{
   BT_UINT header;                                                    // - 0
   BT_INT  share_count;       //                                      // - 4
   BT_U8BIT  bc_inited;       // Non-zero indicates BC initialized    // - 8
   BT_U8BIT  bc_running;      // Non-zero indicates BC is running     // - 9
   BT_U8BIT  bm_inited;       // BM has been initialized              // - 10
   BT_U8BIT  bm_running;      // BM is running flag                   // - 11
   BT_U8BIT  rt_inited;       // RT initialized flag.                 // - 12
   BT_U8BIT  rt_running;      // RT running flag.                     // - 13

   BT_UINT CurrentPlatform;                                           // - 14 (e)
   BT_UINT CurrentCardSlot;                                           // - 18 (12) 
   BT_UINT CurrentCarrier;                                            // - 22 (16)
   BT_UINT CurrentCardType;                                           // - 26 (1a)
   int hw_int_enable;                                                 // - 30 (1e)
   BT_UINT bt_ucode_rev;                                              // - 34 (22)

   BT_INT board_has_irig;                                             // - 38 (26) 
   BT_INT board_has_testbus;                                          // - 42 (2a) 
   BT_INT board_has_discretes;                                        // - 46 (2e)
   BT_INT board_has_differential;                                     // - 50 (32)
   BT_INT board_access_32;                                            // - 54 (36)
   BT_INT board_has_bc_timetag;                                       // - 58 (3a)
   BT_INT board_has_485_discretes;                                    // - 62 (3e)
   BT_INT board_has_serial_number;                                    // - 66 (42)
   BT_INT board_has_hwrtaddr;                                         // - 70 (46)

   BT_INT hwRTAddr;                                                   // - 74 (4a)

   API_CHANNEL_STATUS channel_status;                                 // - 78 (4e)

   BT_INT bt_op_mode;                                                 // - 82 (52)
                                              
   BT_INT api_device;                                                 // - 86 (56)                                                 
   BT_INT bt_interrupt_enable;                                        // - 90 (5a)

   char *lpAPI_BM_Buffers;                                            // - 94 (5e)

   BT_INT numDiscretes;                                               // - 98 (62)
   BT_U32BIT bt_dismask;                                              // - 102 (66)

   BT_INT _HW_1Function;                                              // - 106 (6a)
   BT_INT _HW_FPGARev;                                                // - 110 (6e) 
   BT_INT _HW_PROMRev;                                                // - 114 (72)
   BT_INT _HW_WCSRev;                                                 // - 118 (76)

   BT_U32BIT btmem_bm_mbuf;                                           // - 122 (7a)
   BT_U32BIT btmem_bm_mbuf_next;                                      // - 126 (7e)
   BT_U32BIT btmem_bm_cbuf;                                           // - 130 (82)
   BT_U32BIT btmem_bm_cbuf_next;                                      // - 134 (86)
   BT_U32BIT btmem_bc;                                                // - 138 (8a)
   BT_U32BIT btmem_bc_next;                                           // - 142 (8e)
   BT_U32BIT btmem_tail1;                                             // - 146 (92)
   BT_U32BIT btmem_pci1553_next;                                      // - 150 (96)
   BT_U32BIT btmem_pci1553_rt_mbuf;                                   // - 154 (9a)
}CHANNEL_SHARE;

#define CHANNEL_SHARED 0xca5eba7e //unique
#define CHANNEL_NOT_SHARED 0

#define SHARE_HEADER                     0
#define SHARE_COUNT                      4
#define SHARE_BC_INITED                  8
#define SHARE_BC_RUNNING                 9
#define SHARE_BM_INITED                 10
#define SHARE_BM_RUNNING                11
#define SHARE_RT_INITED                 12
#define SHARE_RT_RUNNING                13
#define SHARE_PLATFORM                  14 // (e)
#define SHARE_CARDSLOT                  18 // (12) 
#define SHARE_CARRIER                   22 // (16)
#define SHARE_CardType                  26 // (1a)
#define SHARE_INT_ENABLE                30 // (1e)
#define SHARE_UCODE_REV                 34 // (22)
#define SHARE_IRIG                      38 // (26) 
#define SHARE_TESTBUS                   42 // (2a) 
#define SHARE_DISCRETES                 46 // (2e)
#define SHARE_DIFFERENTIAl              50 // (32)
#define SHARE_ACCESS_32                 54 // (36)
#define SHARE_BC_TIMETAG                58 // (3a)
#define SHARE_485_DISCRETES             62 // (3e)
#define SHARE_SERIAL_NUMBER             66 // (42)
#define SHARE_HAS_HWRTADDR              70 // (46)
#define SHARE_HWRTADDR                  74 // (4a)
#define SHARE_CHANNEL_STATUS            78 // (4e)
#define SHARE_BT_OP_MODE                82 // (52)
#define SHARE_DEVICE                    86 // (56)                                                 
#define SHARE_INTERRUPT_ENABLE          90 // (5a)
#define SHARE_LPAPI_BM_BUFFERS          94 // (5e)
#define SHARE_NUMDISCRETES              98 // (62)
#define SHARE_BT_DISMASK                102 // (66)
#define SHARE_HW_1FUNCTION              106 // (6a)
#define SHARE_HW_FPGAREV                110 // (6e) 
#define SHARE_HW_PROMREV                114 // (72)
#define SHARE_HW_WCSREV                 118 // (76)
#define SHARE_BTMEM_BM_MBUF             122 // (7A)
#define SHARE_BTMEM_BM_MBUF_NEXT        126 // (7E)
#define SHARE_BTMEM_BM_CBUF             130 // (82)
#define SHARE_BTMEM_BM_CBUF_NEXT        134 // (86)
#define SHARE_BTMEM_BC                  138 // (8A)
#define SHARE_BTMEM_BC_NEXT             142 // (8E)
#define SHARE_BTMEM_TAIL1               146 // (92)
#define SHARE_BTMEM_PCI1553_NEXT        150 // (96)
#define SHARE_BTMEM_PCI1553_RT_MBUF     154 // (9A)

#define RT_QUIT 1
#define BM_QUIT 2
#define BC_QUIT 4

/**********************************************************************
*  Bus Monitor Definitions
**********************************************************************/

/************************************************
*  BM Control Buffer -- 4 words in structure, one
*  will be defined for each active subaddress
************************************************/

typedef struct bm_cbuf
   {
   BT_U32BIT wcount;            // enabled word counts or mode codes
   BT_U16BIT pass_count2;       // number of msgs before interrupt (original)
   BT_U16BIT pass_count;        // number of msgs before interrupt (counter)
   //BT_U16BIT bm_control;      // API control word // Deleted V2.22.ajh
   }
BM_CBUF;


/**********************************************************************
*  BM Message Buffer
*  The first 10 words are independent of the BM message type --
*   after that, the contents of the buffer varies according
*   to the message type and the number of data words.
*  When using the PCI/PMC/ISA-1553, be sure to add 4 words/8 bytes
*   to the end of this structure to properly align the following
*   buffer on an 8-word boundry
* This structure contains mis-aligned long's!  (84 words long)
**********************************************************************/

typedef struct bm_mbuf
   {
   BT_U16BIT       next_mbuf PACKED;  // Address of next message in chain
   BT_U32BIT       int_enable PACKED; // Interrupt enable bits
   BT_U32BIT       int_status PACKED; // Status bits for this mbuf
   BT1553_TIME     time_tag PACKED;   // 45 bit time tag, 1 MHz clock
   BT1553_COMMAND  command1 PACKED;   // Command word #1
   BT_U16BIT       status_c1 PACKED;  // Command word #1 status
   BT_U16BIT       data[74] PACKED;   // The rest of the data
#ifdef _BM_USER_DATA_
   BT_U16BIT       reserved1 PACKED;  // spare word.
   BT_U16BIT       user_data1 PACKED; // User Data
   BT_U16BIT       user_data2 PACKED; // User Data
#endif //_BM_USER_DATA_
   }
BM_MBUF;

/**********************************************************************
*  BM Filter Buffer
**********************************************************************/

typedef struct bm_fbuf
    {             //    32         *        2        *      32 = 2048 words
    BT_U16BIT addr[BM_ADDRESS_COUNT][BM_TRANREC_COUNT][BM_SUBADDR_COUNT];
    }
BM_FBUF;

/************************************************
*  Bit definitions for BM_TBUF header word
************************************************/

#define BM_TBUF_TRIG    0x0001  // trigger occurred
#define BM_TBUF_START   0x0002  // enables start trigger
#define BM_TBUF_STOP    0x0004  // enables stop trigger
#define BM_TBUF_EXT     0x0008  // enables external trigger
#define BM_TBUF_INT     0x0010  // (internal use)
#define BM_TBUF_ERROR   0x0020  // trigger on errors
//                      0x0040  // reserved
#define BM_TBUF_OUTPUT  0x0080  // external output on trigger
#define BM_TBUF_EVERY   0x0100  // external output on every BM interrupt

/************************************************
*  Field definitions for BM_TBUF word_type
************************************************/

#define BM_TBUF_TYPE_NULL   0x0000  // no word type
#define BM_TBUF_TYPE_CMD    0x0020  // command
#define BM_TBUF_TYPE_STAT   0x0040  // status
#define BM_TBUF_TYPE_DATA   0x0080  // data
#define BM_TBUF_TYPE_TIME   0x00A0  // time

/************************************************
*  Bit definitions for BM_TBUF control word
************************************************/

#define BM_TBUF_CONT_EVENT_MASK 0x001F  // event occurred indicator
#define BM_TBUF_CONT_OR         0x0000  // 'OR'
#define BM_TBUF_CONT_AND        0x0020  // 0 -> 'OR', 1 -> 'AND'
#define BM_TBUF_CONT_ARM        0x0040  // if set, causes event to ARM next event
#define BM_TBUF_CONT_END        0x0080  // specified last event to be processed


/**********************************************************************
*  Enhanced BM Trigger Buffer (53 words)
*  The format of this buffer applies to:
*    ISA/PCI/cPCI-1553 (FW versions >= 3.20)
*    PCC-1553 (FW versions > 3.20)
**********************************************************************/
typedef struct bm_tbuf_enh
   {
   BT_U16BIT trigger_header;            // Trigger header word (See below)

   // Four words determine logic functions for triggering, and record the events
   //  as they occur for the FW.  The arming words are used anytime triggering
   //  is enabled; the armed words are used only if arming is selected.
   BT_U16BIT start_trigger_event;      // Start arming logic function
   BT_U16BIT stop_trigger_event;       // Stop  arming logic function

   struct
      {
      BT_U16BIT trigger_offset;         // Offset in BM msg buffer to trigger word
      BT_U16BIT trigger_event_value;    // Compare masked word to this value
      BT_U16BIT trigger_event_bit_mask; // Masks the selected word
      BT_U16BIT trigger_initial_count;  // Initial count of events
      BT_U16BIT trigger_event_count;    // Current count of events
      }
   start_event[4];              // Start trigger events
   struct
      {
      BT_U16BIT trigger_offset;         // Offset in BM msg buffer to trigger word
      BT_U16BIT trigger_event_value;    // Compare masked word to this value
      BT_U16BIT trigger_event_bit_mask; // Masks the selected word
      BT_U16BIT trigger_initial_count;  // Initial count of events
      BT_U16BIT trigger_event_count;    // Current count of events
      }
   stop_event[4];               // Stop trigger events 

   BT_U16BIT trig_save[10];  // padding to keep the same size as before
   }
BM_TBUF_ENH;

/**********************************************************
*  Bit definitions for BM_TBUF_ENH trigger_header word
**********************************************************/

#define BM_ETBUF_INTE   0x0001  // Generate host interrupt & IQ entry on BM message.
                                // Set by API if no triggering, set by the FW when
                                //  all trigger conditions have been met.
#define BM_ETBUF_START  0x0002  // Enables start trigger.
#define BM_ETBUF_STOP   0x0004  // Enables stop trigger.
#define BM_ETBUF_ARMED  0x0008  // Used by FW to detect if the first sequence
//                              //  of events has occurred when arming selected.
#define BM_ETBUF_EXT    0x0010  // Enables external trigger input.
//                              //  Start and stop trigger ignored if set.
#define BM_ETBUF_ERROR  0x0020  // Trigger on 1553 words with errors.
//                      0x0040  // Reserved
#define BM_ETBUF_OUTPUT 0x0080  // Enable external TTL output on trigger.
#define BM_ETBUF_EVERY  0x0100  // Enable external TTL output on every BM interrupt.
//                      0xFE00  // Reserved

#define EI_MAX_ERROR_NUM    64 //Number of error injection buffers supported

/************************************************
*  Error injection buffer definition
************************************************/

typedef struct ei_message
    {
    BT_U16BIT data[EI_COUNT];  // 33 words per buffer.
    }
EI_MESSAGE;

/************************************************
*  Error injection hardware definitions
************************************************/

#define EI_HW_NONE          0x0000  // no error to inject
#define EI_HW_BITCOUNT      0x0040  // bit count error
#define EI_HW_SYNC          0x0080  // inverted sync error
#define EI_HW_PARITY        0x0100  // parity error
#define EI_HW_DATAWORDGAP   0x0200  // gap between words
#define EI_HW_WORDCOUNT     0x0400  // word count error
#define EI_HW_LATERESPONSE  0x0800  // programmable response time
#define EI_HW_NOINTERMSGGAP 0x1000  // no intermessage gap (BC)
#define EI_HW_BADADDR       0x1000  // response to wrong address
#define EI_HW_MIDSYNC       0x2000  // mid sync error
#define EI_HW_MIDBIT        0x4000  // mid bit error
#define EI_HW_RESPWRONGBUS  0x8000  // response on wrong bus
#define EI_HW_BIPHASE       0xC000  // Bi-phase Error
#define EI_HW_MIDPARITY     0xE000  // Parity zero crossing

/**********************************************************************
*  IQ (Interrupt Queue) definitions
*  Number of hardware interrupt blocks; 3 word per block.
**********************************************************************/

#define IQ_SIZE      296    // Number of interrupt blocks

/**********************************************************************
*  RT Definitions
**********************************************************************/

/************************************************
*  RT Address Buffer -- 128 words
*  (4 words for each of 32 RT addresses)
************************************************/

typedef struct rt_abuf_entry
    {
    BT_U16BIT enables;         //  RT specific condition enables
    BT_U16BIT status;          //  RT status word
    BT_U16BIT last_command;    //  Used for transmit last cmd mode code
    BT_U16BIT bit_word;        //  Built-In-Test (BIT) results
    }
RT_ABUF_ENTRY;

typedef struct rt_abuf
    {
    RT_ABUF_ENTRY aentry[RT_ADDRESS_COUNT];
    }
RT_ABUF;

/************************************************
*  Address buffer 'enables' bits
************************************************/

#define RT_MONITOR_MODE    0x0001  // Enable RT Monitor Mode
#define RT_ABUF_DISA       0x0001  // disable channel 'A'
#define RT_ABUF_DISB       0x0002  // disable channel 'B' or enable DBA (WCS >= 308)
#define RT_ABUF_ITF        0x0004  // inhibit terminal flag (both old/new firmware)
#define RT_ABUF_DBC_ENA    0x4000  // Set this bit to enable Dynamic Bus Acceptance
#define RT_ABUF_DBC_RT_OFF 0x8000  // Set this bit to shut down RT on valid DBA

/**********************************************************************
*  RT Control Buffer Block -- 3 words.
*  Pointed to by the RT Filter Buffer.  Changed to 16-bit words.V4.01.ajh
**********************************************************************/

typedef struct rt_cbuf
    {
    BT_U16BIT legal_wordcount0 PACKED; // legal word counts/mode codes (bit field 15-1,0)
    BT_U16BIT legal_wordcount1 PACKED; // legal word counts/mode codes (bit field 31-16)
    BT_U16BIT message_pointer PACKED;  // addr of RT message buffer (8-word addr if ISA-/PCI-1553)
    }
RT_CBUF;

/************************************************
*  RT Control Buffer Block (broadcast) --
*  63 words for each broadcast subunit
* This structure contains mis-aligned long's!
************************************************/

typedef struct rt_cbufbroad
    {
    BT_U16BIT message_pointer PACKED;     // address of RT message buffer (8-word address if PCI-1553)
    BT_U32BIT legal_wordcount[31] PACKED; // legal word count (bit field)
    }
RT_CBUFBROAD;

/************************************************
*  RT Filter Buffer, located on 2048 word boundry
*   in BusTools board memory.
************************************************/

typedef struct rt_fbuf
    {         //         32        *        2        *      32 = 2048 Words
    BT_U16BIT addr[RT_ADDRESS_COUNT][RT_TRANREC_COUNT][RT_SUBADDR_COUNT];
    }
RT_FBUF;

/**********************************************************************
*  RT Message Buffer is composed of a hardware portion which is used
*   by the microcode, and an SW/API portion used only by the API.
*  Note that the hardware portion cannot be shared by both receive
*   and transmit, since the RT Status word gets stored at
*   different offsets in the buffer.  But the data is always in the
*   same relative locations in either type of buffer.
***********************************************************************/

/**********************************************************************
*  RT Message Buffer (hardware) for the IP/PCI/ISA-1553:
**********************************************************************/
typedef struct rt_mbuf_hw
    {
    // The following words are defined by the microcode:
    BT_U16BIT   nxt_msg_ptr PACKED;  // Address of next message (8-word address for PCI-1553)
    BT_U16BIT   ei_ptr PACKED;       // error injection message address (lower 64Kw only)
    BT_U32BIT   enable PACKED;       // RT interrupt enable bits
    BT_U32BIT   status PACKED;       // bits indicating what caused the interrupt
    BT1553_TIME time_tag PACKED;     // 45 bit time tag, IP/PCI/ISA-1553 (1 MHz Clock)

    BT1553_COMMAND mess_command PACKED;  // 1553 command word
    BT1553_STATUS  mess_status1 PACKED;  // 1553 status word (Transmit) (reserved Receive)
    BT_U16BIT      mess_data[32] PACKED; // 1553 data words
    BT1553_STATUS  mess_status2 PACKED;  // 1553 status word (Receive)
    }
RT_MBUF_HW;     // 46 words

typedef struct rt_mbuf_api
    {
    // The following words are used ONLY by the API, NOT the microcode:
    BT1553_COMMAND mess_id;         // API RT#, T/R, SA#, Status word code.
    BT_U16BIT      mess_bufnum;     // API buffer number.
    BT_U16BIT      mess_statuswd;   // API 1553 Status Word.
    BT_U16BIT      mess_verify;     // API verification word (RT_VERIFICATION).
    }
RT_MBUF_API;    // 4 words-->48 words total

#define RT_SET        0x8000   /* Used in mess_statuswd to set bits into 1553 */
#define RT_NOCHANGE   0x0000   /*  status word when msg is read or written.   */
#define RT_SET_RESERVE        0x8001   /* Used in mess_statuswd to set bits into 1553 */
#define RT_NOCHANGE_RESERVE   0x0001   /*  status word when msg is read or written.   */
#define RT_STATUSMASK     0x071F  /* Defines bits copied into RT status word. Added BCR bit 10/28/98 */
#define RT_STATUSMASK_RES 0x07FF  /* Bits settable in RT status word in ABUF. Added BCR bit 10/28/98 */
#define RT_VERIFICATION   0xBABE  /* Value of mess_verify indicates valid MBUF*/
#define VERIFICATION_RT   0xBEBA  /* Value of mess_verify indicates valid MBUF*/

typedef struct rt_mbuf
    {
    RT_MBUF_HW     hw;      // 45 words defined by the firmware.
    RT_MBUF_API    api;     //  4 words defined by the API.
    }
RT_MBUF;                    // 49 words

//#define HWREG_BD_DETECT       0x03  // board detect pattern (PC)            (RO)
//#define HWREG_REGISTERA       0x05  // protocol sequencer register a

/************************************************************************
*  Hardware register WORD offsets;
************************************************************************/
#define HWREG_CONTROL1         0x00  // control register #1                 (R/W)
#define HWREG_CONTROL_T_TAG    0x01  // Control the PCI-1553 time tag cntr   (RO).V4.28
#define HWREG_CLRTAG           0x02  // Clear IP/PCI-1553 time tag counter   (WO)
#define HWREG_PB_CONTROL       0x03  // Playback Control Register (PCI-1553) (RW)
#define HWREG_RESPONSE         0x04  // response time out/late response (IP, PCI, PC WO)
#define HWREG_BC_EXT_SYNC      0x06  // Enable external TTL BC sync          (WO)
#define HWREG_PB_CLEAR         0x07  // Playback Status Bit Clear (PCI-1553) (WO)
#define HWREG_PCI_VOLTAGE      0x08  // Set PCI-1553 output voltage          (WO)
#define HWREG_LPU_REVISION     0x09  // Read LPU Revision information        (RO).V4.31
#define HWREG_INTERRUPT_VEC    0x0A  // Set IP-D1553 interrupt vector        (WO)
#define HWREG_WRITE_INTERRUPT  0x0B  // Set/Clr bit 9 to set/clear interrupt (RW)
#define HWREG_READ_T_TAG       0x0C  // Read the current time tag counter val(RW).V4.28
#define HWREG_READ_T_TAG386    0x15  // Read the current time tag counter LPU 386 or greater
#define HWREG_MINOR_FRAME      0x0E  // Minor Frame interval, 25 us LSB      (WO)
#define HWREG_MINOR_FRAME_LSB  0x1c  // Minor Frame interval, 1 us LSB       (WO)
#define HWREG_MINOR_FRAME_MSB  0x1d  // Minor Frame interval, 1 us LSB       (WO)
#define HWREG_CONTROL2         0x0F  // 2nd HWcontrol Register               (RW)
#define HWREG_WCS_REVISION     0x18  // WCS Revision regiseter               (RO)
#define HWREG_HEART_BEAT       0x19  // Heart Beat Register                  (RO)
#define HWREG_LPU_BUILD_NUMBER 0x1a  // LPU Build number                     (RO)

#define HWREG_COUNT           0x10  // size of hw register area in WORDS.V3.30.ajh
#define HWREG_COUNT2          0x20  // Real size

/******************************************************
*  Hardware 1553_control_reg1 BIT definitions
******************************************************/
#define CR1_SMODE        0x0001   // Singal Mode Warning
#define CR1_BCRUN        0x0002   // start bc running
#define CR1_RTRUN        0x0004   // start rt running
#define CR1_BMRUN        0x0008   // start bm running
#define CR1_EXT_TTL      0x0010   // Set the external TTL Output
#define CR1_BCBUSY       0x0020   // bc is busy
#define CR1_1553A        0x0040   // 1553 A Mode
#define CR1_BC_EXT_SYNC  0x0080   // external BC sync enabled (RO)
#define CR1_IN_WRP       0x0100   // internal wrap (disable external 1553 bus)
#define CR1_HOST_INTERP  0x0200   // host interrupt is asserted  
#define CR1_RT31BCST     0x0400   // rt 31 broadcast
#define CR1_SA31MC       0x0800   // subaddress 31 mode code
#define CR1_TESTBUS      0x1000   // Test Bus Setting
#define CR1_RTEXSYN      0x2000   // rt external sync enable
#define CR1_INT_ENABLE   0x4000   // Set bit to enable HW interrupts 
#define CR1_BUS_COUPLE   0x8000   // Enable transformer coupling
#define CR2_MSG_SCHD     0x0020   // Enable Message 
#define CR2_FIXED_GAP    0x0040   // Fixed Gap message timing
#define CR2_NO_PREDICT   0x0400   // No frame end predition
#define CR2_FR_STRT_TIME 0x0800   // Use Frame Start Timing option
#define CR2_EXTD_TIME    0x1000   // Use BC extended gap and frame timing 
#define CR2_MON_INV_CMD  0x0080   // Monitor Invalid Commands
#define CR2_IGN_HGH_WRD  0x0200   // Ignore High word detection
#define CR2_UND_MC_ILL   0x0100   // Undefined Mode Codes Illegal

/******************************************************
*  Paged Hardware I/O Register bit definitions
******************************************************/
#define IO_CONFIG_WCS  0x2000   // Load the WCS into the board
#define IO_RUN_STATE   0x0000   // Transition board to runable state

/**********************************************************************
*  RAM Register WORD offsets, from the beginning of the board.  Note
*  that the RAM Registers start at location BTMEM_RAMREGS (WORD 0x10).
**********************************************************************/
#define RAMREG_USER1            0x0a       // BM user data 1 register
#define RAMREG_USER2            0x0b       // BM user data 2 register


/*****************************************************************************
*  BM-Only File Registers (only used with AR15-VPX
*  WORD offsets from the beginning file register offset.
******************************************************************************/
#define BM_BUFFER_START1        0x02       // Start Address of BM Buffer 1
#define BM_BUFFER_START2        0x03       // Start Address of BM Buffer 2
#define BM_ONLY_DEBUG           0x04       // Debug only

/*****************************************************************************
*  Playback File Registers
*  WORD offsets from the beginning file register offset.
******************************************************************************/
#define RAMREG_DQ_RETRY         0x07       // mask for the Interrupt Status Word 1 bits
#define PB_START_POINTER        0x10       // Address of Playback Data Buffer
#define PB_END_POINTER          0x11       // End of Playback Data Buffer
#define PB_TAIL_POINTER         0x12       // Current FW loc in PB Data Buffer
#define PB_HEAD_POINTER         0x13       // Addr where host is adding data
#define PB_INT_THRSHLD          0x14       // Number of words between FW interrupts
#define PB_INT_STATUS           0x15       // Number of FW interrupts generated
#define PB_THRSHLD_CNT          0x16       // Interrupt counter used by FW
#define PB_CURRENT_WORD         0x17       // Reserved for the Playback firmware

#define RAMREG_HIGHAPTR         0x2A       // Addr of high-priority aperiodic msg list (RW)
#define RAMREG_LOWAPTR          0x2B       // Addr of low-priority aperiodic msg list  (RW)
#define RAMREG_HIGHAFLG         0x2E       // Flags used for high-priority aperiodics
#define RAMREG_LOW_AFLG         0x2F       // Flags used for low-priority aperiodics
#define RAMREG_RT_DISA          0x34       // RT Chan A Disables, registers 34-35
#define RAMREG_RT_DISB          0x36       // RT Chan B Disables, registers 36-37
#define RAMREG_CLRWORD          0x38       // Microcode constant register 38
#define RAMREG_SETWORD          0x39       // Microcode constant register 39
#define RAMREG_RETRYPTR         0x3A       // Microcode constant register 3A
#define RAMREG_INTBUSTRG        0x3B       // Microcode constant register 3B
#define RAMREG_RTMONITR         0x3C       // Microcode constant register 3C
#define RAMREG_MASK001F         0x3D       // Microcode constant register 3D
#define RAMREG_MASK003F         0x3E       // Microcode constant register 3E
#define RAMREG_RTMONITT         0x3F       // Microcode constant register 3F = 0x12 LPU V3.05+(V4.06)
#define RAMREG_BC_MSG_PTR      (0x40+0x00) // BC message pointer
#define RAMREG_BC_MSG_PTRSV    (0x40+0x01) // BC message pointer save
#define RAMREG_BC_DATA_PTR     (0x40+0x02) // BC data pointer
#define RAMREG_BC_INT_ENB1     (0x40+0x03) // BC interrupt enables 1
#define RAMREG_BC_INT_ENB2     (0x40+0x04) // BC interrupt enables 2
#define RAMREG_BC_RETRY        (0x40+0x05) // BC retry enable information
#define RAMREG_BCMSGPTR1       (0x40+0x06) // minor frame time register
#define RAMREG_BCMSGPTR2       (0x40+0x07) // minor frame counter
#define RAMREG_RT_CONT_PTR     (0x40+0x08) // RT control pointer
#define RAMREG_RT_MSG_PTR      (0x40+0x09) // RT message pointer
#define RAMREG_RT_MSG_PTRSV    (0x40+0x0A) // RT message pointer save
#define RAMREG_RT_MSG_PTR2     (0x40+0x0B) // RT message pointer 2 (RT to RT)
#define RAMREG_BM_PTR_SAVE     (0x40+0x0C) // BM pointer save (initialize by host)
#define RAMREG_BM_MBUF_PTR     (0x40+0x0D) // BM message pointer (used by firmware)
#define RAMREG_BC_OUT_WC       (0x40+0x0E) // output word count
#define RAMREG_BC_DAT_WC       (0x40+0x0F) // input word count
#define RAMREG_RT_ERRINJ1      (0x40+0x10) // RT error injection pointer 1
#define RAMREG_RT_ERRINJ2      (0x40+0x11) // RT error injection pointer 2
#define RAMREG_MPROC_ADDR      (0x40+0x12) // 
#define RAMREG_INT_QUE_PTR     (0x40+0x13) // HW interrupt queue pointer
#define RAMREG_RT_ABUF_PTR     (0x40+0x14) // RT address buffer base address
#define RAMREG_RT_FBUF_PTR     (0x40+0x15) // RT filter base address
#define RAMREG_BM_FBUF_PTR     (0x40+0x16) // BM filter base address
#define RAMREG_BM_TBUF_PTR     (0x40+0x17) // BM trigger event pointer
#define RAMREG_BC_RETRYPTR     (0x40+0x18) // BC retry buffer pointer
#define RAMREG_MESS_ERROR1     (0x40+0x19) // message error 1
#define RAMREG_MESS_ERROR2     (0x40+0x1A) // message error 2
#define RAMREG_ORPHAN          (0x40+0x1B) // orphaned hardware bits
#define RAMREG_MODECODE1       (0x40+0x1C) // Undefined mode code #1
#define RAMREG_MODECODE2       (0x40+0x1D) // Undefined mode code #2
#define RAMREG_MODECODE3       (0x40+0x1E) // Undefined mode code #3 
#define RAMREG_MODECODE4       (0x40+0x1F) // Undefined mode code #4

#define RAMREG_REG60           (0x40+0x20) // Microcode constant register 60
#define RAMREG_REG61           (0x40+0x21) // Microcode constant register 61
#define RAMREG_REG62           (0x40+0x22) // Microcode constant register 62
#define RAMREG_REG63           (0x40+0x23) // Microcode constant register 63
#define RAMREG_REG64           (0x40+0x24) // Microcode constant register 64
#define RAMREG_REG56           (0x40+0x25) // Microcode constant register 65
#define RAMREG_REG66           (0x40+0x26) // Microcode constant register 66
#define RAMREG_REG67           (0x40+0x27) // Microcode constant register 67
#define RAMREG_REG68           (0x40+0x28) // Microcode constant register 68
#define RAMREG_REG69           (0x40+0x29) // Microcode constant register 69
#define RAMREG_REG6A           (0x40+0x2A) // Microcode constant register 6A
#define RAMREG_REG6B           (0x40+0x2B) // Microcode constant register 6B
#define RAMREG_REG6C           (0x40+0x2C) // Microcode constant register 6C
#define RAMREG_REG6D           (0x40+0x2D) // Microcode constant register 6D
#define RAMREG_REG6E           (0x40+0x2E) // Microcode constant register 6E
#define RAMREG_REG6F           (0x40+0x2F) // Microcode constant register 6F
#define RAMREG_REG70           (0x40+0x30) // Microcode constant register 70
#define RAMREG_REG71           (0x40+0x31) // Microcode constant register 71
#define RAMREG_REG72           (0x40+0x32) // Microcode constant register 72
#define RAMREG_REG73           (0x40+0x33) // Microcode constant register 73
#define RAMREG_REG74           (0x40+0x34) // Microcode constant register 74
#define RAMREG_REG75           (0x40+0x35) // Microcode constant register 75
#define RAMREG_REG76           (0x40+0x36) // Microcode constant register 76
#define RAMREG_REG77           (0x40+0x37) // Microcode constant register 77
#define RAMREG_FLAGS           (0x40+0x38) // Microcode flags
#define RAMREG_ENDFLAGS        (0x40+0x39) // Microcode end flags
#define RAMREG_BC_CONTROL_SAVE (0x40+0x3A) // Microcode saved BM control word
#define RAMREG_CURRENT_WORD    (0x40+0x3B) // Microcode temp
#define RAMREG_DECODER_WORD    (0x40+0x3C) // Microcode temp
#define RAMREG_DECODER_MSG_FMT (0x40+0x3D) // Microcode temp
#define RAMREG_TEMP            (0x40+0x3E) // Microcode temp
#define RAMREG_RESERVED8       (0x40+0x3F) // Reserved

#define RAMREG_COUNT            128        // Number of RAM Registers

/*******************************************************************
*  Discrete register defines
********************************************************************/
#define DISREG_DIS_OUT1         0x84
#define DISREG_DIS_OUT2         0x85
#define DISREG_DIS_OUTEN1       0x86
#define DISREG_DIS_OUTEN2       0x87
#define DISREG_HW_RTADDR        0x88
#define DISREG_TRIG_CLK         0x89
#define DISREG_DIFF_IO          0x8a
#define DISREG_DIS_IN1          0x8c
#define DISREG_DIS_IN2          0x8d 
#define DISREG_RTADDR_RD1       0x8e
#define DISREG_RTADDR_RD2       0x8f
#define DISREG_RTADDR_OV1       0x90
#define DISREG_RTADDR_OV2       0x91
#define RS485_TX_DATA           0x92
#define RS845_TX_ENABLE         0x93
#define RS485_RX_DATA           0x94
#define DAC_VALID               0x97
#define RXMC_EXT_TRIG_OUT_CH1   0x98
#define RXMC_EXT_TRIG_OUT_CH2   0x99

#define NO_OUTPUT		0
#define PIO_OPN_GRN		1
#define PIO_28V_OPN		2
#define DIS_OPN_GRN		3
#define DIS_28V_OPN		4
#define EIA485_OPN_GRN	5
#define EIA485_28V_OPN	6

/************************************************
*  Misc Bit definitions for RAM registers
************************************************/
#define BIT00 0x0001 /* Generic LSB definition */
#define BIT01 0x0002
#define BIT02 0x0004
#define BIT03 0x0008
#define BIT04 0x0010
#define BIT05 0x0020
#define BIT06 0x0040
#define BIT07 0x0080
#define BIT08 0x0100
#define BIT09 0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000 /* Generic MSB definition */

/************************************************
*  Bit definitions for RAMREG_ORPHAN
************************************************/
#define RAMREG_ORPHAN_BM_DISABLE_A        0x0004
#define RAMREG_ORPHAN_BM_DISABLE_B        0x0008
#define RAMREG_ORPHAN_RESET_TIMETAG       0x0010
#define RAMREG_ORPHAN_FIRE_EXT_ON_SYNC_MC 0x0040
#define RAMREG_ORPHAN_MINORFRAME_OFLOW    0x0400

/**********************************************************************
*  Internal API procedure definitions (various files).
**********************************************************************/

BT_U16BIT api_readhwreg     (BT_UINT cardnum,BT_UINT regnum);
void      api_writehwreg    (BT_UINT cardnum,BT_UINT regnum,BT_U16BIT value);
void      api_writehwreg_or (BT_UINT cardnum,BT_UINT regnum,BT_U16BIT value);
void      api_writehwreg_and(BT_UINT cardnum,BT_UINT regnum,BT_U16BIT value);

void      api_reset(BT_UINT cardnum);

void BusTools_EI_Init(BT_UINT cardnum);
void BusTools_InitSeg1 (BT_UINT cardnum);

void TimeTagClearFlag(BT_UINT cardnum);
void TimeTagConvert(  BT_UINT cardnum,
                      BT1553_TIME *time_tag,
                      BT1553_TIME *time_actual);

BT_U32BIT TimeTagInterrupt(BT_UINT cardnum);
void TimeTagZeroModule(BT_UINT   cardnum);

void BM_MsgReadBlock(BT_UINT cardnum);
BT_U32BIT RT_CbufbroadAddr(BT_UINT cardnum, BT_UINT sa, BT_UINT tr);

/**********************************************************************
*  OS-specific memory mapping functions (MEM*.C).
**********************************************************************/

#ifdef INCLUDE_VMIC
BT_INT vbtMapVMICAddress(BT_UINT cardnum, BT_U32BIT baseaddr,
                         BT_INT  ioaddress);
DWORD vbtFreeVMICAddress(BT_UINT cardnum);
#endif //_VMIC_

/**********************************************************************
*  Interrupt timing and BM recording time-out intervals.  The default
*   polling interval is 10 milliseconds.
**********************************************************************/
// Not all processors support less than a 10 ms polling interval(?).
#define MIN_POLLING          1    /* Min polling resolution/interval, in ms */
#define MAX_POLLING        100    /* Max polling resolution/interval, in ms */
#define BM_RECORD_TIME     500    /* BM recording interval in milliseconds  */

/**********************************************************************
*  Internal low-level API procedure definitions.
**********************************************************************/
// Memory test error reporting function
void API_ReportMemErrors(BT_UINT cardnum, int type,
                         BT_U32BIT base_address, BT_U32BIT length,
                         BT_U16BIT * bufout, BT_U16BIT * bufin,
                         BT_U16BIT * buf2, int * first_time);

BT_INT  vbtSetup(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT baseaddr,            // (i) 32 bit base address
   BT_U32BIT ioaddr,              // (i) io address
   BT_UINT   flag);               // (i) hw/sw flag: 0 -> sw, 1 -> hw

LPSTR vbtGetPagePtr(
   BT_UINT   cardnum,             // (i) card number.
   BT_U32BIT byteOffset,          // (i) offset within adapter memory.
   BT_UINT   bytesneeded,         // (i) number of bytes needed in page.
   LPSTR     local_board_buffer);    // (io) scratch buffer V4.01

BT_U16BIT vbtGetRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U16BIT vbtGetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);        // (i) register number, WORD offset

BT_U16BIT vbtGetFileRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U16BIT vbtGetHWRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U16BIT vbtGetDiscrete(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U16BIT vbtGetPLXRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_INT vbtReadSerialNumber(
   BT_UINT cardnum, 
   BT_U16BIT *serial_number);

void vbtSetRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U16BIT regval);             // (i) new value

void vbtSetFileRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U16BIT regval);

void vbtSetHWRegister(
   BT_UINT   cardnum,            // (i) card number (0 based)
   BT_U32BIT regnum,             // (i) register number, WORD offset
   BT_U16BIT regval);            // (i) new value

void vbtSetDiscrete(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U16BIT regval);             // (i) new value

void vbtSetPLXRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U16BIT regval);             // (i) new value

void vbtSetPLXRegister8(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) host buffer (source)
   BT_U8BIT regval);              // (o) byte offset within adapter (destination))

BT_U8BIT vbtGetPLXRegister8(BT_UINT   cardnum,    // (i) card number
                            BT_U32BIT regnum);     // (i) PLX register address

void vbtSetPLXRegister32(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U32BIT regval);             // (i) new value

void vbtReadTimeTag(
   BT_UINT   cardnum,             // (i) card number
   BT_U16BIT * timetag);      // (o) resulting 48-bit time value from HW

void vbtWriteTimeTag(
   BT_UINT   cardnum,         // (i) card number
   BT1553_TIME * timetag);    // (i) 48-bit time value to load into register

void vbtWriteTimeTagIncr(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT incr);          // (i) 32-bit time increment
   
void vbtReadBlock(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numblocks);     // (i) number of bytes to copy

void vbtRead(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void vbtReadHIF(
   BT_UINT   cardnum,        // (i) card number
   LPSTR     lpbuffer,       // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);   // (i) number of bytes to copy

void vbtWriteHIF(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void vbtReadRAM8(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);   // (i) number of bytes to copy

void vbtReadRAM(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);   // (i) number of bytes to copy

void vbtWriteRAM8(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void vbtWriteRAM(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void vbtRead_iq(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void vbtRead32(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

BT_U16BIT vbtReadModifyWrite(
   BT_UINT   cardnum,       // (i) card number
   BT_UINT   region,        // (i) HWREG, FILEREG, or  RAM
   BT_U32BIT byteOffset,    // (i) memory offset within adapter memory (source)
   BT_U16BIT wNewWord,      // (i) new value to be written under mask
   BT_U16BIT wNewWordMask); // (i) mask for new value

void vbtWrite(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void vbtWrite32(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numbytes);     // (i) number of bytes to copy

BT_INT vbtIRIGCal(BT_UINT cardnum, BT_INT flag);

void vbtIRIGConfig(BT_UINT cardnum, BT_U16BIT value);

void vbtIRIGValid(BT_UINT cardnum, BT_U16BIT *valid);

void vbtIRIGSetTime(BT_UINT cardnum, BT_U16BIT time_lsb, BT_U16BIT time_msb);

void vbtIRIGWriteDAC(BT_UINT cardnum, BT_U16BIT value);

void vbtShutdown(BT_UINT cardnum);   // (i) card number

void vbtNotify(BT_UINT cardnum, BT_U32BIT *bmflag); // Interrupt processing routine

void vbtMapUserDLL(BT_UINT cardnum);  // Setup user interface dlls

#ifdef _INTEGRITY_
DWORD timeGetTime(void); 
#endif

void API_InterruptInit(BT_UINT cardnum);   // User interrupt initialization function.

#ifdef DEMO_CODE
void vbtHardwareSimulation(BT_UINT cardnum);  // Simulation/Demo mode function.
#endif

void bt_memcpy(void * dest, void * source, int numbytes); // Assembly WORD copy function.

BT_INT API_MemTest(BT_UINT cardnum);
void get_48BitHostTimeTag(BT_INT mode, BT1553_TIME *host_time);
void RegisterFunctionOpen(BT_UINT cardnum);
void RegisterFunctionClose(BT_UINT cardnum);
void SignalUserThread(BT_UINT cardnum, BT_UINT nType, BT_U32BIT brdAddr, BT_UINT IQAddr);
BT_INT  vbtInterruptSetup(BT_UINT cardnum, BT_INT hw_int_enable, BT_INT hw_int_num);
void vbtInterruptClose(BT_UINT cardnum);
unsigned long CEI_GET_TIME(void);
BT_INT vbtSetPolling(BT_UINT polling,  // (i) polling interval
                     BT_UINT tflag);   // (i) timer option 0 - start; 1 - restart 

//void vbtIDpromRead(BT_INT cardnum, BT_INT numChars, unsigned char * lpsProm);  // Read IP ID PROM

BT_INT vbtOpen1553Channel( BT_UINT   *chnd,       // (o) card number (0 - 12) (device number in 32-bit land)
                           BT_UINT   mode,        // (i) operational mode
                           BT_INT    devid,       // (i) linear base address of board/carrier memory area or WinRT device number
                           BT_UINT   channel);

#ifdef INCLUDE_VME_VXI_1553
BT_INT vbtReadVMEConfigRegs(
   BT_U32BIT config_addr,       // A16 config address
   BT_U16BIT *config_data);     // cofiguration data storage

BT_INT vbtWriteVMEConfigRegs(
   BT_U16BIT  *vme_config_addr,  // A16 Address
   BT_UINT   offset,             // Byte offset of register  
   BT_U16BIT config_data);       // (i) data to write
#endif

// Debug only functions.  Convertable to other environments
//#if defined(FILE_SYSTEM)
void DumpBCmsg(             // Dump the BC message list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void DumpBMmsg(             // Dump the BM message list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void DumpBMflt(             // Dump the BM filter list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void DumpRTptrs(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile);      // (i) handle of output file
void DumpTimeTag(
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file

  void DumpRegisterFIFO(
     BT_UINT cardnum,         // (i) card number
     FILE  * hfMemFile);      // (i) handle of output file
//#endif //FILE_SYSTEM

#ifdef ADD_TRACE
void AddTrace(
   BT_UINT cardnum,         // (i) card number (0 - max)
   BT_INT  nFunction,       // (i) function number to log
   BT_INT  nParam1,         // (i) first parameter to log
   BT_INT  nParam2,         // (i) second parameter to log
   BT_INT  nParam3,         // (i) third parameter to log
   BT_INT  nParam4,         // (i) fourth parameter to log
   BT_INT  nParam5);        // (i) fifth parameter to log
#else
#define AddTrace(a,b,c,d,e,f,g)
#endif

#if defined(DO_BUS_LOADING)
void BM_BusLoadingFilter(BT_UINT cardnum);
#else
#define BM_BusLoadingFilter(p1)
#endif

#endif //APIINT_H


