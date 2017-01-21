 /*============================================================================*
 * FILE:                        D I S C R E T E . C
 *============================================================================*
 *
 * COPYRIGHT (C) 2007 - 2010 BY
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
 * FUNCTION:   This file contains all function related to discrete and trigger
 *             functions.
 *
 * USER ENTRY POINTS: 
 *     BusTools_DiscreteGetIO      - Reads the discrete input signals setting
 *     BusTools_PIO_GetIO          - Reads the PIO input setting
 *     BusTools_DiscreteRead       - Reads the discrete input signals values
 *     BusTools_PIO_Read           - Reads the PIO input signals values
 *     BusTools_DiscreteReadRegister - Read the discrete registers
 *     BusTools_DiscreteSetIO      - Configures the discrete signals as input/outputs
 *     BusTools_PIO_SetIO          - Configures the PIO signals as input/outputs
 *     BusTools_DiscreteWrite      - Writes the discrete output signals
 *     BusTools_PIO_Write          - Writes the PIO output signals
 *     BusTools_DiscreteTriggerOut - Sets which channels use discrete 7 and 8
 *     BusTools_DiscreteTriggerIn  - Sets which channels use discrete 7 and 8
 *     BusTools_DiffTriggerOut     - Sets which channels uses the differential output
 *     BusTools_ExtTrigIntEnable   - Enables interrupt on External Trigger.
 *     BusTools_RS485_TX_Enable    - Enables the 8 485-transmit signals as outputs
 *     BusTools_RS485_Set_TX_Data  - Sets the RS-485 transmit data
 *     BusTools_RS485_ReadRegs     - Read the 485 reigsters
 *     BusTools_SetMultipleExtTrig - Enables multiple out put triggers (RXMC Only)
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *===========================================================================*/

/* $Revision:  6.32 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  11/19/2007   Added support for the R15-EC in functions BusTools_DiscreteGetIO, 
               BusTools_DiscreteRead, BusTools_DiscreteSetIO, and BusTools_DiscreteWrite
  12/28/2009   Add PIO functions for RXMC-1553
*/

#include <stdio.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"


/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteGetIO
*
*  FUNCTION
*     This routine read the discrete input signals setting
*     on the QPCI-1553, QPMC-1553 and other.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteGetIO(
   BT_UINT cardnum,        // (i) card number
   BT_U32BIT *disDirValue)    // (o) discrete values
{
   BT_U16BIT temp1,temp2;
   BT_U32BIT temp3;
	
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   temp1 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN1);
   temp2 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN2);
   temp3 = (BT_U32BIT)((temp1 & 0xffff) | (temp2 << 16));
   temp3 &= bt_dismask[cardnum];
 
   if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
      temp3>>=6;

//   if(CurrentCardType[cardnum]==RXMC1553)
//      temp3>>=4;

   *disDirValue = temp3;
 
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_PIO_GetIO
*
*  FUNCTION
*     This routine read the PIO input signals setting
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_PIO_GetIO(
   BT_UINT cardnum,        // (i) card number
   BT_U32BIT *pioDirValue)    // (o) discrete values
{
   BT_U16BIT temp1,temp2;
   BT_U32BIT temp3;
	
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_pio[cardnum])
      return API_HARDWARE_NOSUPPORT;

   temp1 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN1);
   temp2 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN2);
   temp3 = (BT_U32BIT)((temp1 & 0xffff) | (temp2 << 16));
   temp3 &= bt_piomask[cardnum];
 
   temp3>>=4;

   *pioDirValue = temp3;
 
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteRead
*
*  FUNCTION
*     This routine read the discrete input signals
*     on the QPCI-1553 and the QPMC-1553.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteRead(
   BT_UINT cardnum,        // (i) card number
   BT_INT disSel,          // (i) Discrete select = 0 all
   BT_U32BIT *disValue)    // (o) discrete values
{
   BT_U16BIT  temp1,temp2;
   BT_U32BIT  testbit = 1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;
      
   if(disSel > numDiscretes[cardnum])
      return API_BAD_DISCRETE;

   if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
   {
      if(disSel != 0)
      {
         disSel+=6;
      }
   }

      
   temp1 = vbtGetDiscrete(cardnum,DISREG_DIS_IN1);
   temp2 = vbtGetDiscrete(cardnum,DISREG_DIS_IN2);
   
   *disValue = (BT_U32BIT)(temp1 | (temp2<<16));
   *disValue &= bt_dismask[cardnum];

   if(disSel == 0)
   {
	   return API_SUCCESS;
   }
   else
   {
       *disValue &= (testbit << (disSel-1));
	   if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
	   {
		   *disValue>>=6;
	   }
   }
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_PIO_Read
*
*  FUNCTION
*     This routine read the discrete input signals
*     on the QPCI-1553 and the QPMC-1553.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_PIO_Read(
   BT_UINT cardnum,        // (i) card number
   BT_INT pioSel,          // (i) Discrete select = 0 all
   BT_U32BIT *pioValue)    // (o) discrete values
{
   BT_U16BIT  temp1,temp2;
   BT_U32BIT  testbit = 1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;
      
   if(pioSel > numPIO[cardnum])
      return API_BAD_DISCRETE;

   if(pioSel != 0)
   {
      pioSel+=4;
   }

   temp1 = vbtGetDiscrete(cardnum,DISREG_DIS_IN1);
   temp2 = vbtGetDiscrete(cardnum,DISREG_DIS_IN2);
   
   *pioValue = (BT_U32BIT)(temp1 | (temp2<<16));
   *pioValue &= bt_piomask[cardnum];

   if(pioSel == 0)
   {
	   return API_SUCCESS;
   }
   else
   {
       *pioValue &= (testbit << (pioSel-1));
	   *pioValue>>=4;
   }
   return API_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteReadRegister
*
*  FUNCTION
*     This routine reads the discrete/PIO/485 input signals
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteReadRegister(
   BT_UINT cardnum,        // (i) card number
   BT_INT regnum,          // (i) Discrete select = 0 all
   BT_U32BIT *disValue)    // (o) discrete values
{
   BT_U16BIT  temp1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   temp1 = vbtGetDiscrete(cardnum,regnum);

   *disValue = temp1;

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteSetIO
*
*  FUNCTION
*     This routine configures the discrete signals as input/outputs
*     on the QPCI-1553 and the QPMC-1553.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteSetIO(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT disSet,        // (i) discrete flags
   BT_U32BIT mask)          // (i) mask
{
   BT_U32BIT  disval;
   BT_U16BIT  temp1,temp2;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   
   if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
   {
      mask &=0x3; //only 2 discretes
      mask<<=6;   //only discrete 7 and 8 active so shift over
	  disSet<<=6;
   }
   else //for QPCI-1553 and Q104-1553 and Q104-1553P
      mask &= bt_dismask[cardnum];    //

   temp1 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN1);
   temp2 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN2);
   disval = (BT_U32BIT)(temp1 | (temp2<<16));
   
   disval = (BT_U32BIT)((disval & ~mask) | (disSet & mask));
   
   temp1 = (BT_U16BIT)(disval & 0xffff);
   temp2 = (BT_U16BIT)((disval & 0xffff0000)>>16);

   vbtSetDiscrete(cardnum,DISREG_DIS_OUTEN1,temp1);
   vbtSetDiscrete(cardnum,DISREG_DIS_OUTEN2,temp2);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_PIO_SetIO
*
*  FUNCTION
*     This routine configures the PIO as input/outputs
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_PIO_SetIO(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT disSet,        // (i) discrete flags
   BT_U32BIT mask)          // (i) mask
{
   BT_U32BIT  disval;
   BT_U16BIT  temp1,temp2;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   mask &= bt_piomask[cardnum];    //
   disSet = disSet << 4;

   temp1 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN1);
   temp2 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN2);
   disval = (BT_U32BIT)(temp1 | (temp2<<16));
   
   disval = (BT_U32BIT)((disval & ~mask) | (disSet & mask));
   
   temp1 = (BT_U16BIT)(disval & 0xffff);
   temp2 = (BT_U16BIT)((disval & 0xffff0000)>>16);

   vbtSetDiscrete(cardnum,DISREG_DIS_OUTEN1,temp1);
   vbtSetDiscrete(cardnum,DISREG_DIS_OUTEN2,temp2);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteWrite
*
*  FUNCTION
*     This routine writes the discrete output signals
*     on the QPCI-1553 and the QPMC-1553.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteWrite(
   BT_UINT cardnum,        // (i) card number
   BT_INT  disSel,         // (i) Discrete select 0 = all
   BT_UINT disFlag)        // (i) 0 reset the discrete 1 set the discrete
{
   BT_U16BIT  temp1,temp2,temp3,temp4;
   BT_U32BIT  testbit = 1;
   BT_U32BIT  disValue;
   BT_U32BIT  disdir;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(disSel > numDiscretes[cardnum])
         return API_BAD_DISCRETE;

   temp1 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN1);
   temp2 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN2);

   temp3 = vbtGetDiscrete(cardnum,DISREG_DIS_OUT1);
   temp4 = vbtGetDiscrete(cardnum,DISREG_DIS_OUT2);

   disdir = (BT_U32BIT)(temp1 | (temp2<<16));
   disValue = (BT_U32BIT)(temp3 | (temp4<<16));

   if(disSel == 0)  //Set all output discrete to the value of disFlag
   {
	  if(disFlag)
         disValue = disdir;
	  else
         disValue = 0;
   }
   else
   {
      if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
         disSel+=6;

//      if(CurrentCardType[cardnum]==RXMC1553)
//         disSel+=4;

      if(disFlag)
         disValue |= (testbit << (disSel-1));
      else
         disValue &= ~(testbit <<(disSel-1));

   }

   disValue &= bt_dismask[cardnum];
   temp1 = (BT_U16BIT)(disValue & 0xffff);
   temp2 = (BT_U16BIT)((disValue & 0xffff0000) >> 16);
   vbtSetDiscrete(cardnum,DISREG_DIS_OUT1,temp1);
   vbtSetDiscrete(cardnum,DISREG_DIS_OUT2,temp2);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_PIO_Write
*
*  FUNCTION
*     This routine writes the PIO output signals
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_PIO_Write(
   BT_UINT cardnum,        // (i) card number
   BT_INT  pioSel,         // (i) Discrete select 0 = all
   BT_UINT pioFlag)        // (i) 0 reset the discrete 1 set the discrete
{
   BT_U16BIT  temp1,temp2,temp3,temp4;
   BT_U32BIT  testbit = 1;
   BT_U32BIT  disValue;
   BT_U32BIT  disdir;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_pio[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(pioSel > numPIO[cardnum])
         return API_BAD_DISCRETE;

   temp1 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN1);
   temp2 = vbtGetDiscrete(cardnum,DISREG_DIS_OUTEN2);

   temp3 = vbtGetDiscrete(cardnum,DISREG_DIS_OUT1);
   temp4 = vbtGetDiscrete(cardnum,DISREG_DIS_OUT2);

   disdir = (BT_U32BIT)(temp1 | (temp2<<16));
   disValue = (BT_U32BIT)(temp3 | (temp4<<16));

   if(pioSel == 0)  //Set all output discrete to the value of disFlag
   {
	  if(pioFlag)
         disValue = disdir;
	  else
         disValue = 0;
   }
   else
   {
      pioSel+=4;

      if(pioFlag)
         disValue |= (testbit << (pioSel-1));
      else
         disValue &= ~(testbit <<(pioSel-1));

   }

   disValue &= bt_piomask[cardnum];
   temp1 = (BT_U16BIT)(disValue & 0xffff);
   temp2 = (BT_U16BIT)((disValue & 0xffff0000) >> 16);
   vbtSetDiscrete(cardnum,DISREG_DIS_OUT1,temp1);
   vbtSetDiscrete(cardnum,DISREG_DIS_OUT2,temp2);

   return API_SUCCESS;
}



/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteTriggerOut
*
*  FUNCTION
*     This routine configures which channels use discrete 7 and 8
*     as trigger outputs.  You must program discrete 7 and 8 as 
*     outputs when you call BusTools_DiscreteSetIO.
*
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_DiscreteTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT disflag)          // (i) TRIGGER_OUT_DIS_7 - use Discrete 7 as trigger out
                            //     TRIGGER_OUT_DIS_8 - use Discrete 8 as trigger out
                            //     TRIGGER_OUT_DIS_NONE - use neither
{
   BT_U16BIT temp;
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   temp = vbtGetDiscrete(cardnum,DISREG_TRIG_CLK);

   switch(CurrentCardSlot[cardnum])
   {
   case 0:
      temp &= ~0x11;   // clear the disrete setting 
      if(disflag == TRIGGER_OUT_DIS_7)
         temp |= 0x1;  // set discrete 7 as trigger
	  else if(disflag == TRIGGER_OUT_DIS_8)
         temp |= 0x10; // set discrete 8 as trigger
      else if(disflag == TRIGGER_OUT_NONE)
         break;        // already done
      else
         return  API_BAD_PARAM; //this is an error
	  break;
   case 1:
      temp &= ~0x22;   // clear the disrete setting 
      if(disflag == TRIGGER_OUT_DIS_7)
         temp |= 0x2;  // set discrete 7 as trigger
	  else if(disflag == TRIGGER_OUT_DIS_8)
         temp |= 0x20; // set discrete 8 as trigger
      else if(disflag == TRIGGER_OUT_NONE)
         break;        // already done
      else
         return  API_BAD_PARAM; //this is an error
	  break;
   case 2:
      temp &= ~0x44;  // clear the disrete setting 
      if(disflag == TRIGGER_OUT_DIS_7)
         temp |= 0x4;  // set discrete 7 as trigger
	  else if(disflag == TRIGGER_OUT_DIS_8)
         temp |= 0x40; // set discrete 8 as trigger
      else if(disflag == TRIGGER_OUT_NONE)
         break;        // already done
      else
         return  API_BAD_PARAM; //this is an error
	  break;
   case 3:
      temp &= ~0x88;   // clear the disrete setting 
      if(disflag == TRIGGER_OUT_DIS_7)
         temp |= 0x8;  // set discrete 7 as trigger
	  else if(disflag == TRIGGER_OUT_DIS_8)
         temp |= 0x80; // set discrete 8 as trigger
      else if(disflag == TRIGGER_OUT_NONE)
         break;        // already done
      else
         return  API_BAD_PARAM; //this is an error
	  break;
   }

   vbtSetDiscrete(cardnum,DISREG_TRIG_CLK,temp);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteTriggerIn
*
*  FUNCTION
*     This routine configures which channels use discrete 7 and 8
*     as trigger input.  You must program discrete 7 and 8 as 
*     inputs
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteTriggerIn(
   BT_UINT cardnum,         // (i) card number
   BT_UINT trigflag)         // (i) 0=none, 1=485, 2=dis7 3=dis8 
{
	BT_U16BIT tempVal[4][4] = {
	{0x0,0x100,0x200,0x300},
	{0x0,0x400,0x800,0xc00},
	{0x0,0x1000,0x2000,0x3000},
	{0x0,0x4000,0x8000,0xc000}
	};

   BT_U16BIT temp;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(trigflag>3)
      return API_BAD_PARAM;

   temp = vbtGetDiscrete(cardnum,DISREG_TRIG_CLK);      // Read the Setting
   temp &= ~tempVal[CurrentCardSlot[cardnum]][3];       // Clear the Channel setting
   temp |= tempVal[CurrentCardSlot[cardnum]][trigflag]; // Set the Channel
 
   vbtSetDiscrete(cardnum,DISREG_TRIG_CLK,temp);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiffTriggerOut
*
*  FUNCTION
*     This routine configures which channels use the differential
*     outputs.  You must program discrete 7 and 8 as 
*     outputs
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiffTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT  chflag,          // (i) 0 = disconnect 1 = connect channel
   BT_INT  diffen)          // (i) 0 = disable 1 = enable differential output 
{
   BT_U16BIT temp;

   BT_U16BIT tval[4]={1,2,4,8};
   BT_U16BIT diff_enable = 0x10;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_differential[cardnum])
      return API_HARDWARE_NOSUPPORT;

   temp =  vbtGetDiscrete(cardnum,DISREG_DIFF_IO);
   if(chflag)
     temp |= tval[CurrentCardSlot[cardnum]];
   else
     temp &= ~tval[CurrentCardSlot[cardnum]];

   if(diffen)
      temp |= diff_enable;
   else
      temp &= ~diff_enable;

   vbtSetDiscrete(cardnum,DISREG_DIFF_IO,temp);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_RS485_TX_ENABLE
*
*  FUNCTION
*     This routine enables the 8 485-transmit signals as outputs
*     on the QPM-1553 boards.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_RS485_TX_Enable(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT enable,        // (i) discrete flags
   BT_U16BIT mask)          // (i) mask
{
   BT_U16BIT  temp1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_485_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(enable > 0xff || mask > 0xff)
      return API_BAD_DISCRETE;   

   temp1 = vbtGetDiscrete(cardnum,RS845_TX_ENABLE);
   temp1 &= 0xff;  // only 8 RS 485 transmitters.
   
   temp1 = ((temp1 & ~mask) | (enable & mask));

   vbtSetDiscrete(cardnum,RS845_TX_ENABLE,temp1);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_RS485_SET_TX_DATA
*
*  FUNCTION
*     This routine sets/reset the 8 485-transmit signals
*     on the QPM-1553 boards.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_RS485_Set_TX_Data(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT rsdata,        // (i) transmit data pattern
   BT_U16BIT mask)          // (i) mask
{
   BT_U16BIT  temp1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_485_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(rsdata > 0xff || mask > 0xff)
      return API_BAD_DISCRETE;   

   temp1 = vbtGetDiscrete(cardnum,RS485_TX_DATA);
   temp1 &= 0xff;  // only 8 RS 485 transmitters.
   
   temp1 = (temp1 & ~mask) | (rsdata & mask);

   vbtSetDiscrete(cardnum,RS485_TX_DATA,temp1);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_RS485_READ_REGS
*
*  FUNCTION
*     This routine enables the 8 485-Receive signals as inputs
*     on the QPM-1553 boards.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_RS485_ReadRegs(
   BT_UINT cardnum,            // (i) card number
   BT_INT  regval,             // (i) RS 485 register to read
                               //     RS485_TXEN_REG
                               //     RS485_TXDA_REG
                               //     RS485_RXDA_REG
   BT_U16BIT *rsdata)          // (o) RS 485 register data
{
   BT_UINT reg_index[] = {RS845_TX_ENABLE,
                          RS485_TX_DATA,
                          RS485_RX_DATA};

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_485_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(regval > RS485_RX_DATA)
      return API_BAD_PARAM;   
   
      
   *rsdata = vbtGetDiscrete(cardnum,reg_index[regval]);
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetMultipleExtTrig
*
*  FUNCTION
*     This routine select which discrete/PIO/485 channel will act
*     as external triggers.  A single channel can program multiple
*     output triggers and multiple channels can select the same
*     trigger channel.
*
*     The RXMC has multiple out configurations. You must know the 
*     configuration of RXMC-1553 to correctly select triggers.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetMultipleExtTrig(
   BT_UINT cardnum,            // (i) card number
   BT_INT  trigOpt,			   // (i) PIO,DISCRETE,EIA485
   BT_UINT tvalue,             // (i) trigger channel (1 - 12)
   BT_INT  enableFlag)         // (i) ENABLE_TRIG or DISABLE_TRIG
{

   BT_U32BIT trig_addr;
   BT_U16BIT trig_reg;

   BT_U16BIT eia485val[] = {0x8000,0x4000,0x2000,0x1000};
   BT_U16BIT disval[] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800};
   BT_U16BIT pioval[] = {0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800};

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(!boardHasMultipleTriggers[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(CurrentCardSlot[cardnum] == 0)
      trig_addr = RXMC_EXT_TRIG_OUT_CH1;
   else
      trig_addr = RXMC_EXT_TRIG_OUT_CH2;

   trig_reg = vbtGetDiscrete(cardnum,trig_addr);

   switch(rxmc_output_config[cardnum])
   {
      case NO_OUTPUT:
         return API_HARDWARE_NOSUPPORT;
         break;
      case PIO_OPN_GRN:
      case PIO_28V_OPN:
         //This configuration has 12 possible triggers 4 Discrete + 8 PIO
         switch(trigOpt)
         {
            case EIA485:
               return API_HARDWARE_NOSUPPORT;
               break;
            case DISCRETE:
               if( (tvalue < 1) || (tvalue > 4))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                     vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg | disval[tvalue]));
                  else
                     vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg & ~disval[tvalue]));
               break;
            case PIO:
               if((tvalue < 1) || (tvalue > 8))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                      vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg | pioval[tvalue]));
                  else
                      vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg & ~pioval[tvalue]));
               break;
           default:
              return API_HARDWARE_NOSUPPORT;
         }
         break;
      case DIS_OPN_GRN:
      case DIS_28V_OPN:
         //This configuration has 12 possible 4 + 8 Discrete
         switch(trigOpt)
         {
            case EIA485:
               return API_HARDWARE_NOSUPPORT;
               break;
            case DISCRETE:
               if( (tvalue < 1) || (tvalue > 12))
                  return API_HARDWARE_NOSUPPORT;
               else
                   if(enableFlag)
                      vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg | disval[tvalue]));
                  else
                      vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg & ~disval[tvalue]));
               break;
            case PIO:
               return API_HARDWARE_NOSUPPORT;
               break;
           default:
              return API_HARDWARE_NOSUPPORT;
         }
         break;
      case EIA485_OPN_GRN:
      case EIA485_28V_OPN:
         //This configuration has 8 possible triggers 4 EIA485 + 4 Discrete
         switch(trigOpt)
         {
            case EIA485:
               if( (tvalue < 1) || (tvalue > 4))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                      vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg | eia485val[tvalue]));
                  else
                      vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg & ~eia485val[tvalue]));
               break;
            case DISCRETE:
               if( (tvalue < 1) || (tvalue > 4))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                      vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg | disval[tvalue]));
                  else
                      vbtSetDiscrete(cardnum,trig_addr,(BT_U16BIT)(trig_reg & ~disval[tvalue]));
               break;
            case PIO:
               return API_HARDWARE_NOSUPPORT;
               break;
           default:
              return API_HARDWARE_NOSUPPORT;
         }
         break;
      default:
         return API_HARDWARE_NOSUPPORT;
   }

   return API_SUCCESS;

}

