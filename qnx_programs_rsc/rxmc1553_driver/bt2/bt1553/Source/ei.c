/*============================================================================*
 * FILE:                          E I . C
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
 *             Error Injection routines.
 *             These functions assume that the BusTools_API_Init-() function
 *             has been successfully called.
 *
 * USER ENTRY POINTS:
 *    BusTools_EI_EbufRead  - Error Injection buffer read.
 *    BusTools_EI_EbufWrite - Error Injection buffer write.
 *    BusTools_EI_Getaddr   - Returns the address of the specified entry
 *                            in the EI injection table.
 *    BusTools_EI_Getid     - Returns the error id associated with the
 *                            specified error injection address.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    BusTools_EI_Init      - Initialize EI operations.
 *
 * INTERNAL ROUTINES:
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  03/17/1997   Merged 16- and 32-bit versions together.V2.21.ajh
  09/28/1997   Added additional LabView interface definitions. V2.32.ajh
  02/15/2002   Added spport for modular API. v4.46
  03/15/2002   Added Bi-Phase error V4.48
  03/16/2008   Expanded error injection (data gap and mid parity)
 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_EI_EbufRead
*
*  FUNCTION
*     This function reads an error injection buffer from the
*     BusTools memory.  The error type is saved by the busapi
*     software.
*
*  RETURNS
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_EI_EbufRead(
   BT_UINT  cardnum,        // (i) card number (0 - based)
   BT_UINT  errorid,
   API_EIBUF * ebuf)
{
#ifdef ERROR_INJECTION 
  /************************************************
   *  Check initial conditions
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   /*******************************************************************
   *  Check for bigger than the largest allowed
   *  error message number
   *******************************************************************/

   if (errorid > EI_MAX_ERROR_NUM)
      return(API_EI_ILLERRORNO);

   /*******************************************************************
   *  Determine how many words to process based on message type
   *******************************************************************/

   switch ( ebuf->buftype )
   {
        case EI_BC_REC:
            break;
        case EI_BC_TRANS:
            break;
        case EI_BC_RTTORT:
            break;
        case EI_RT_REC:
            break;
        case EI_RT_TRANS:
            break;
   }
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_EI_EbufWrite
*
*  FUNCTION
*     This function writes an error injection buffer to the
*     BusTools memory.  The error type is saved by the BusAPI
*     software.
*
*  PARAMETERS
*
*  RETURNS
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_EI_EbufWrite(
   BT_UINT  cardnum,        // (i) card number (0 - based)
   BT_UINT  errorid,
   API_EIBUF * ebuf)
{
#ifdef ERROR_INJECTION
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_INT       i;
   BT_INT       errortype;
   BT_INT       nWords=0;
   BT_U32BIT    addr;
   EI_MESSAGE   error;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   /*******************************************************************
   *  Check for bigger than the largest allowed error message number.
   *******************************************************************/

   if (errorid > EI_MAX_ERROR_NUM)
      return API_EI_ILLERRORNO;

   /*******************************************************************
   *  Process buffer based on error buffer type.
   *******************************************************************/
   errortype = ebuf->buftype;

   /*******************************************************************
   * First figure out how many words go into the error buffer.  Each
   *  error buffer is the same fixed length, but the number of words
   *  used in the buffer depends on the message type associated with
   *  the specific buffer.
   * Error buffers are specified by the 1553 message type, and are
   *  only used for that message type.  This is a "feature" of BusTools,
   *  and is not required by the hardware or by this API.  The only
   *  thing which limits the usage of an error injection buffer is
   *  the fact that the hardware does not support all error types
   *  for all messages types, command, status and data words.
   *******************************************************************/
   switch ( errortype )
   {
      case EI_BC_REC:
      case EI_RT_TRANS:
         nWords = EI_COUNT;
         break;
      case EI_RT_REC:
      case EI_BC_TRANS:
         nWords = 1;
         break;
      case EI_BC_RTTORT:
         nWords = 2;
         break;
   }
   // End of code to setup the number of words in the error injection buffer.

   // Now copy the specified number of words into the HW error injection buf.
   memset((void*)&error,0,sizeof(error));  // Initialize the result.
   for ( i = 0; i < nWords; i++ )
   {
       switch ( ebuf->error[i].etype )
       {
        case EI_NONE:          error.data[i] = EI_HW_NONE;          break;
        case EI_BITCOUNT:      error.data[i] = EI_HW_BITCOUNT;      break;
        case EI_SYNC:          error.data[i] = EI_HW_SYNC;          break;
        case EI_DATAWORDGAP:   error.data[i] = EI_HW_DATAWORDGAP;   break;
        case EI_PARITY:        error.data[i] = EI_HW_PARITY;        break;
        case EI_WORDCOUNT:     error.data[i] = EI_HW_WORDCOUNT;     break;
        case EI_LATERESPONSE:  error.data[i] = EI_HW_LATERESPONSE;  break;
        case EI_NOINTERMSGGAP: error.data[i] = EI_HW_NOINTERMSGGAP; break;
        case EI_BADADDR:       error.data[i] = EI_HW_BADADDR;       break;
        case EI_MIDSYNC:       error.data[i] = EI_HW_MIDSYNC;       break;
        case EI_MIDBIT:        error.data[i] = EI_HW_MIDBIT;        break;
        case EI_RESPWRONGBUS:  error.data[i] = EI_HW_RESPWRONGBUS;  break;
        case EI_MIDPARITY:     error.data[i] = EI_HW_MIDPARITY;     break;
		case EI_BIPHASE:       error.data[i] = EI_HW_BIPHASE;       break;
        default:
            return(API_EI_BADMSGTYPE);
      }
      switch ( ebuf->error[i].etype )
      {
        case EI_BITCOUNT:
        case EI_DATAWORDGAP:
        case EI_WORDCOUNT:
		case EI_MIDBIT:
		case EI_BIPHASE: 
        case EI_LATERESPONSE:
        case EI_BADADDR:
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F);
      }
   }

   /*******************************************************************
   *  Write the error buffer to hardware
   *******************************************************************/

   addr = BTMEM_EI + errorid * sizeof(EI_MESSAGE);
   vbtWrite(cardnum,(LPSTR)&error,addr,sizeof(error));
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //ERROR_INJECTION
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_EI_Getaddr
*
*  FUNCTION
*     This function returns the address of the specified entry
*     in the EI injection table.
*
*  PARAMETERS
*       BT_UINT cardnum    BTA Card Number
*       WORD errorid    Error ID
*
*  RETURNS
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_EI_Getaddr(
   BT_UINT cardnum,              // (i) card number
   BT_UINT errorid,              // (i) Error Injection buffer number
   BT_U32BIT * addr)         // (o) address of error injection buffer
{
#ifdef ERROR_INJECTION
   /************************************************
   *  local variables
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   /************************************************
   *  check for bigger than the largest allowed
   *  error message number
   ************************************************/

   if (errorid > EI_MAX_ERROR_NUM)
      return(API_EI_ILLERRORNO);

   /************************************************
   *  Calculate address from base address & index.
   ************************************************/

   *addr = BTMEM_EI + errorid * sizeof(EI_MESSAGE);
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //ERROR_INJECTION
}

/**********************************************************************
*
*  PROCEDURE NAME - BusTools_EI_Getid
*
*  FUNCTION
*     This function returns the error id associated with the
*     specified error injection address.
*
*  PARAMETERS
*       BT_UINT cardnum    BTA Card Number
*       BT_U16BIT errorid    Error ID
*
*  RETURNS
*       0 -> success
*       API_EI_BADMSGTYPE   -> Specified message type is not defined
*       API_EI_ILLERRORADDR -> Error address is not defined
*
**********************************************************************/

NOMANGLE BT_INT CCONV BusTools_EI_Getid(
   BT_UINT cardnum,              // (i) card number
   BT_U32BIT     addr,           // (i) address of buffer to locate
   BT_UINT * errorid)        // (o) buffer number
{
#ifdef ERROR_INJECTION
   /************************************************
   *  local variables
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   /************************************************
   *  If address is out of range, return an error
   ************************************************/

   if (addr < BTMEM_EI)
      return API_EI_ILLERRORADDR;

   if (addr > BTMEM_EI_NEXT)
      return API_EI_ILLERRORADDR;

   /************************************************
   *  Calculate error id number
   ************************************************/

   *errorid = (BT_U16BIT)((addr - BTMEM_EI) / sizeof(EI_MESSAGE));
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //ERROR_INJECTION
}

/**********************************************************************
*
*  PROCEDURE NAME - BusTools_EI_Init
*   
*  FUNCTION
*       Initialize EI operations.  Each of the "EI_MAX_ERROR_NUM"
*       error injection buffers between "BTMEM_EI" and "BTMEM_EI_NEXT"
*       are initialized to zero.  BusTools and this API supports a
*       fixed number of error injection buffers, which is fixed-allocated
*       by the API in the lower 64 K of memory.
*
*  Returns
*     Nothing
*
**********************************************************************/

NOMANGLE void BusTools_EI_Init(BT_UINT cardnum)
{
#ifdef ERROR_INJECTION
   /************************************************
   *  Local variables
   ************************************************/

   BT_UINT    i;
   BT_U32BIT  addr;
   EI_MESSAGE error;

   /************************************************
   *  Check for legal call
   ************************************************/

   if (cardnum >= MAX_BTA)
      return;

   if (bt_inited[cardnum] == 0)
      return;

   /*******************************************************************
   *  Clear all of the buffers and output them to the hardware.
   *******************************************************************/

   memset((void*)&error, 0, sizeof(error));
   for ( i = 0; i < EI_MAX_ERROR_NUM; i++ )
   {
      addr = BTMEM_EI + i * sizeof(EI_MESSAGE);
      vbtWrite(cardnum, (LPSTR)&error, addr, sizeof(error));
   }
   return;
#else
   return;
#endif //ERROR_INJECTION
}

