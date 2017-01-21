/*============================================================================*
 * FILE:                 P L A Y B A C K . H
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
 *          INTELLIGENT PLATFORMS.
 *
 *===========================================================================*
 *
 * FUNCTION:    Header file for Playback.  This file is only used to build
 *              the PlayBack section of the API.
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  11/05/1999   Initial version for Playback.V3.23.rc
  02/15/2002   Added moduler API support. v4.48
 */

#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

/**********************************************************************
*  Playback Function Structure Definition and constants.
*  Playback is only supported in 32-bit Windows (95/98/NT/2000)
**********************************************************************/
typedef struct bt1553_XmitMessage
   {
#ifdef NON_INTEL_BIT_FIELDS     //                                 (MSB)
   BT_U16BIT channel:1;         // Channel 0 = 1553 channel A used; 1 = 1553 channel 2 used
   BT_U16BIT xmit_code:1;       // Set indicates transmit message
   BT_U16BIT gap_code:1;        // Set indciates gap message
   BT_U16BIT eop:1;             // End of Playback = 1
   BT_U16BIT notUsed:3;         // Not used
   BT_U16BIT parityError:1;     // Set indicates a parity error
   BT_U16BIT syncPolarity:1;    // Set indicates Command/Status sync, Reset = Data sync
   BT_U16BIT bitCount:1;        // Set indicates bit count error
   BT_U16BIT wcount:6;          // word count or mode code field   (LSB)
#else  /* INTEL-Compatable bit field ordering */
   BT_U16BIT wcount:6;          // word count or mode code field   (LSB)
   BT_U16BIT bitCount:1;        // Set indicates bit count error
   BT_U16BIT syncPolarity:1;    // Set indicates Command/Status sync, Reset = Data sync
   BT_U16BIT parityError:1;     // Set indicates a parity error
   BT_U16BIT notUsed:3;         // Not used
   BT_U16BIT eop:1;             // End of Playback = 1
   BT_U16BIT gap_code:1;        // Set indciates gap message
   BT_U16BIT xmit_code:1;       // Set indicates transmit message
   BT_U16BIT channel:1;         // Channel 0 = 1553 channel A used; 1 = 1553 channel 2 used
#endif                          //                                 (MSB)
   }
XMIT_MESSAGE;

/**********************************************************************
*  Playback Function Constant Definitions.
*  Playback is only supported in 32-bit Windows (95/98/NT/2000)
**********************************************************************/
#define gapMessage              0x02000
#define bytesPerWord            2
#define shiftBits               (1+3)   /* Byte offset to 8-word offset */
#define numberOfRT              32
#define timePerWord             20      /* 20 uSec per word */
#define commandWords            3
#define bufferEmpty             0x0010
#define messageNumberFilter     2
#define stackSize               8196
#define errorBit                0x0008
#define runBit                  0x0004
#define intEnable               0x0002
#define startBit                0x0001
#define broadcast               31
#define modeCode1               0
#define sizeOfLookAheadBuffer   0x30180
#define numberOfMessages        1760    /* was 1728 */
#define blockSize               57
#define pSync                   0
#define nSync                   1
#define commandCount            1
#define statusCount             1
#define readSize                512
#define numberOfBlocks          4
#define sizeOfDiskBuffer        0x40000
#define startLatency            20
#define includeRecord           1
#define excludeRecord           0

/**********************************************************************
*  Playback Function Local Variable Definitions.
*  Playback is only supported in 32-bit Windows (95/98/NT/2000)
**********************************************************************/

typedef struct playback_local_data
{
#ifdef _UNIX_
   pthread_cond_t hExitEvent;
   pthread_cond_t blockReadEvent;
   pthread_cond_t readCompleteEvent;

   pthread_mutex_t hExitMutex;
   pthread_mutex_t blockReadMutex;
   pthread_mutex_t readCompleteMutex;
#else
   HANDLE hExitEvent;
   HANDLE blockReadEvent;
   HANDLE readCompleteEvent;
#endif

   BT_INT modeCode2;
   BT_U16BIT eopMessage;

   API_INT_FIFO intFifo;

   BT_INT moreDataInFile;

   // lookAheadBuffer stuff
   BT_U16BIT *lookAheadBuffer;
   BT_U16BIT *lookAheadHeadPTR;
   BT_U16BIT *lookAheadTailPTR;

   // playAheadBuffer stuff
   BT_U32BIT playbackBuffer32;      // Beginning of board buffer, must be a multiple of 16-bytes
   BT_U32BIT sizeOfPlaybackBuffer;  // Must be a multiple of 16-bytes
   BT_U32BIT endOfPlaybackBuffer32; // End of board buffer, must be a multiple of 16-bytes
   BT_U32BIT playbackHeadPTR32;     // Current board buffer head pointer in bytes

   BT_U32BIT blockCount;

   API_BM_MBUF *diskBuffer;
   API_BM_MBUF *diskBufferPTR;
   API_BM_MBUF *diskReadPTR;
   API_BM_MBUF *endOfDiskBuffer;
   API_BM_MBUF endRecord;

   BT_INT bufferCount;

   BT_U16BIT *messagePacket;
   BT_U16BIT *endOfBuffer;

   API_BM_MBUF BMRecord;
   BT_INT noFilter;

   int fileID;
   FILE *filePTR;

   BT_U32BIT lastTagTime;
   BT_U32BIT lastRespTime;
   unsigned long messageNumberStartPoint;
   unsigned long messageNumberStopPoint;
   BT1553_TIME startTagTime;
   BT1553_TIME stopTagTime;

   BT_U32BIT hwTagTime;
   int numberOfDataWords;

   BT_U32BIT timeOffset;
   API_PLAYBACK_STATUS *playbackStatus;

   BT_INT filterFlag;
   BT_INT endOfFilterData;
   BT_INT diskFileNotEmpty;

   BT_U32BIT startTimeDelay;

   BT_INT RTarray[numberOfRT];
} PB_DATA;

static PB_DATA pb[MAX_BTA];

static BT_INT modeCodeData[] = {0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,
                                1,1,1,1,1,1,1,1,
                                1,1,1,1,1,1,1,1};

#endif  // #ifndef _PLAYBACK_H_
