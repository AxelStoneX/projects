/*===========================================================================*
 * LOCAL ENTRY POINT:    v b t B o a r d A c c e s s S e t u p . c
 *===========================================================================*
 *
 * FUNCTION:    Setup specified adapter for read/write access.
 *
 * DESCRIPTION: Initialize the page access data elements and map the board
 *              into the host address space..
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/

/*---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#define _BT_GLOBALS_  /* This module allocates the memory for the driver      */
#include "btdrv.h"
#include "lowlevel.h"

CEI_INT qnx_GetCardID(CEI_INT);
CEI_INT mapCondorPCIAddress(CEI_INT);

BT_INT vbtBoardAccessSetup(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT phys_addr)
{
   BT_INT     status;
   char      *hostaddr;        // Mapped address of the board in host space.

 //  BT_U32BIT mapLength;

   /*********************************************************************
   *  First verify that this is a board type we can handle.
   *********************************************************************/
   if ( (CurrentCardType[cardnum] != PCI1553)   &&
        (CurrentCardType[cardnum] != PMC1553)   &&
        (CurrentCardType[cardnum] != QPMC1553)  &&
        (CurrentCardType[cardnum] != QCP1553)   &&
        (CurrentCardType[cardnum] != QPCI1553)  &&
        (CurrentCardType[cardnum] != QPCX1553)  &&
        (CurrentCardType[cardnum] != R15EC)     &&
        (CurrentCardType[cardnum] != RXMC1553)  &&
        (CurrentCardType[cardnum] != RPCIe1553) &&
        (CurrentCardType[cardnum] != Q1041553P))
      return BTD_ERR_BADBOARDTYPE;    // Unknown board type

   /*********************************************************************
   *  Now map the board address(es) and the I/O address.
   *********************************************************************/
   if( (status = vbtMapBoardAddresses( api_device[cardnum], &bt_devmap[api_device[cardnum]])) != BTD_OK)
   {
      return status;
   }

   if(bt_devmap[api_device[cardnum]].memSections == 1)
   {
      hostaddr = (char *)bt_devmap[api_device[cardnum]].memHostBase[0];
   }
   else if (bt_devmap[api_device[cardnum]].memSections == 2)
   {
      hostaddr = (char *)bt_devmap[api_device[cardnum]].memHostBase[1];
      bt_iobase[cardnum] = (BT_U32BIT)bt_devmap[api_device[cardnum]].memHostBase[0];
   }
   else if (bt_devmap[api_device[cardnum]].memSections == 3)
   {
      hostaddr = (char *)bt_devmap[api_device[cardnum]].memHostBase[2];
      bt_iobase[cardnum] = (BT_U32BIT)bt_devmap[api_device[cardnum]].memHostBase[0];
   }
   else
     return -1;   
   /*********************************************************************
   *  Setup the access parameters for the PCI-1553 or PMC-1553.
   *********************************************************************/
   if ( (CurrentCardType[cardnum] == PCI1553) ||
        (CurrentCardType[cardnum] == PMC1553)    )
   {
      return vbtPageAccessSetupPCI(cardnum, hostaddr);
   }
   /*********************************************************************
   *  Setup the access parameters for the QPMC-1553.
   *********************************************************************/
   if (CurrentCardType[cardnum] == QPMC1553 || CurrentCardType[cardnum] == RPCIe1553 )
   {
      return vbtPageAccessSetupQPMC(cardnum, hostaddr);
   }
   /*********************************************************************
   *  Setup the access parameters for the QCP-1553.
   *********************************************************************/
   if (CurrentCardType[cardnum] == QCP1553)
   {
      return vbtPageAccessSetupQCP(cardnum, hostaddr);
   }
   /*********************************************************************
   *  Setup the access parameters for the R15-EC.
   *********************************************************************/
   if (CurrentCardType[cardnum] == R15EC)
   {
      return vbtPageAccessSetupR15EC(cardnum, hostaddr);
   }
   /*********************************************************************
   *  Setup the access parameters for the RPCIe-1553.
   *********************************************************************/
   if (CurrentCardType[cardnum] == RXMC1553)
   {
      return vbtPageAccessSetupRXMC(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the QPCI-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == (unsigned)QPCI1553 || CurrentCardType[cardnum] == (unsigned)QPCX1553)
   {
      return vbtPageAccessSetupQPCI(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the Q104-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == (unsigned)Q1041553 || CurrentCardType[cardnum] == (unsigned)Q1041553P)
   {
      return vbtPageAccessSetupQ104(cardnum, hostaddr);
   }
   return -1;
}

/*===========================================================================*
 *  ENTRY POINT:     B u s T o o l s _ A P I _ O p e n C h a n n e l 
 *===========================================================================*
 *
 * FUNCTION:    Open 1553 device channel for use.
 *
 * DESCRIPTION: This function assigns the next available cardnum and assigns it 
 *              to the channel.  Then it starts the initialization process for 
 *              that channel.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/

NOMANGLE BT_INT CCONV BusTools_API_OpenChannel(
  BT_UINT   *chnd,       // (o) card number (0 - 12) (device number in 32-bit land)
  BT_UINT   mode,        // (i) operational mode
  BT_INT    devid,       // (i) linear base address of board/carrier memory area or WinRT device number
  BT_UINT   channel)     // (i) channel 1 - 4.
{
   BT_UINT status, cardnum;
//   BT_UINT card_id[16];

   if(devid < 0 || devid >= MAX_BTA)
      return API_BAD_DEVICE_ID;

   if((status = mapCondorPCIAddress(1)) != 0)
   {
      printf(" mapCondorPCIAddress status = %d\n",status);
      return BTD_ERR_BADADDRMAP;
   } 

   //clear the assigned cardnum table on initial start
   if(assigned_cards[MAX_BTA] != 0xbeef)
   {
       assigned_cards[MAX_BTA] = 0xbeef;
       for(cardnum = 0;cardnum<MAX_BTA;cardnum++)
          assigned_cards[cardnum] = 0;
   }
   
   if((status = vbtOpen1553Channel(chnd,mode,devid,channel))!= 0)
   {
      return status;
   }
   
   cardnum = *chnd;
   CurrentPlatform[cardnum] = PLATFORM_PC;            // Platform is PC.
   CurrentMemMap[cardnum]   = CARRIER_MAP_DEFAULT;    // No carrier mapping
   CurrentCardSlot[cardnum] = channel;                // Specified channel
   CurrentCarrier[cardnum]  = NATIVE;                 // No Carrier.
   CurrentCardType[cardnum] = qnx_GetCardID(cardnum); // Set card type
   if(CurrentCardType[cardnum] == IPD1553)            // Card type is IP-D1553.
      return 4567;

   status = API_Init(cardnum, (BT_U32BIT)devid, (BT_UINT)channel, &mode);
   if(status)
   {
      bt_inuse[cardnum] = 0;
   }
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_FindDevice
*
*  FUNCTION
*       This function checks the windows registery for for th einstance of the
*       specificed device.  If found it returns the device number otherwise -1.
*
*  RETURNS
*       Device number or -1 on fail.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FindDevice(
  BT_INT   device_type,  // (i) card number (0 - 15)
  BT_INT   instance)     // (i) device instance (1 - 16)
{
   BT_INT i,dev_cnt;
   BT_UINT card_id[MAX_BTA];

   if(mapCondorPCIAddress(0) != 0)
   {
      return BTD_ERR_BADADDRMAP;
   } 

   dev_cnt=0;
   get_dev_id(card_id);

   for(i=0;i<MAX_BTA;i++)
   {
      if(card_id[i] == device_type)
      {
         dev_cnt++;
         if(dev_cnt == instance)
            break;
      }
   }
   if(i==MAX_BTA)
      return -1;
   else
      return i; 
} 

/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_ListDevices
*
*  FUNCTION
*       This function checks the windows registery for all installed Condor
*       Engineernig boards and returns a list of those devices.
*
*  RETURNS
*       API_SUCCESS
*       BTD_ERR_BADADDRMAP
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_ListDevices(DeviceList *dlist)     // (i) channel 1 - 4.
{
   BT_INT product_index;
   BT_UINT card_id[MAX_BTA];

   if(mapCondorPCIAddress(0))
   {
      return BTD_ERR_BADADDRMAP;
   }

   dlist->num_devices=0; 
   get_dev_id(card_id);
  
   for(product_index=0;product_index<MAX_BTA;product_index++)
   {
      switch(card_id[product_index])
      {               
         case IPD1553:
            strcpy((dlist->name[product_index]),"IP-D1553 PCI");
            dlist->device_name[product_index] = IPD1553;           // Card type is IP-D1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++; 
            break;
         case PCI1553:
            strcpy(dlist->name[product_index],"PCI-1553");
            dlist->device_name[product_index] = PCI1553;           // Card type PCI-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
         case QPMC1553:
            strcpy((dlist->name[product_index]),"QPMC-1553");
            dlist->device_name[product_index] = QPMC1553;          // Card type QPMC-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
         case QPCI1553:
            strcpy(dlist->name[product_index],"QPCI-1553"); 
            dlist->device_name[product_index] = QPCI1553;          // Card type QPCI-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
         case QPCX1553:
            strcpy(dlist->name[product_index],"QPCX-1553"); 
            dlist->device_name[product_index] = QPCX1553;          // Card type QPCX-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
         case QCP1553:
            strcpy(dlist->name[product_index],"QcP-1553"); 
            dlist->device_name[product_index] = QCP1553;          // Card type QcP-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
         case RPCIe1553:
            strcpy(dlist->name[product_index],"RPCIe-1553"); 
            dlist->device_name[product_index] = RPCIe1553;          // Card type RPCIe-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
         case RXMC1553:
            strcpy(dlist->name[product_index],"RXMC-1553"); 
            dlist->device_name[product_index] = RXMC1553;          // Card type RXMC-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
         case R15EC:
            strcpy(dlist->name[product_index],"R15-EC"); 
            dlist->device_name[product_index] = R15EC;          // Card type R15-EC.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
         case ISA1553:         // Caller specified ISA-1553 board.
            strcpy((dlist->name[product_index]),"ISA-1553");
            dlist->device_name[product_index] = ISA1553;           // Card type ISA-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
        case Q1041553:     // Caller specified Q104-1553 board.
            strcpy((dlist->name[product_index]),"Q104-1553");
            dlist->device_name[product_index] = Q1041553;          // Card type Q104-1553.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
        case Q1041553P:
            strcpy((dlist->name[product_index]),"Q104-1553P");
            dlist->device_name[product_index] = Q1041553P;          // Card type Q104-1553P.
            dlist->device_num[product_index] = product_index;
            dlist->num_devices++;
            break;
        default:
            strcpy((dlist->name[product_index]),"-");
            dlist->device_name[product_index] = 0;   
            dlist->device_num[product_index] = -1;
      }
   } 
   return API_SUCCESS;              
}


/****************************************************************************
*
*  PROCEDURE - get_48BitHostTimeTag()
*
*  FUNCTION
*     This routine reads the host clock and returns an API-formatted initial
*     time tag value as a 64-bit integer in microseconds.  We wait for the
*     clock to change to make sure we get as accurate a time value as possible.
*
****************************************************************************/
void get_48BitHostTimeTag(BT_INT mode, BT1553_TIME *host_time)  //__uint64 *rtn_time)
{
	union bigtime
	{
         CEI_UINT64 rtn_time;
	   BT1553_TIME rtn_val;
	}bt;
	
   time_t   now;             // Current time, modify to get Time Reference Point.
   struct   tm start;        // Formatted "now" current time, modified for TRP.
   time_t   then=0;          // Time Reference Point, Midnight or Jan 1.
   struct   timeb ts;        // Current time, use to get milliseconds.

   unsigned last_milli;      // This is the time we use to wait for time to change.
   union{
      CEI_UINT64 diff;       // This is the difference time we return in microseconds,
      BT_U32BIT tt[2];
   }utime;                   //  either time since Midnight or time since Jan 1.
   switch (mode)
   {
   case API_TTI_DAY:  // Initial time relative to midnight
      time(&now);               // Time in seconds since midnight Jan 1, 1970 GMT
      start = *localtime(&now); // Local time in seconds, min, hour, day, month
      start.tm_sec  = 0;        // Local
      start.tm_min  = 0;        //       time
      start.tm_hour = 0;        //            at midnight
      then = mktime(&start);    // local midnight to time in seconds since Jan 1, 1970
      break;

   case API_TTI_IRIG: // Initial time relative to Jan 1, this year
      time(&now);               // Time in seconds since midnight Jan 1, 1970 GMT
      start = *localtime(&now); // Local
      start.tm_sec  = 0;        //   time
      start.tm_min  = 0;        //     at
      start.tm_hour = 0;        //       Jan 1
      start.tm_mday = 0;        //         midnight (This is day 1)
      start.tm_mon  = 0;        //           this
      start.tm_yday = 0;        //             year
      then = mktime(&start);    // local Jan 1 to time in seconds since Jan 1, 1970
                                // We do IRIG time, which makes Jan 1 "day 1" */
      break;
   }

   ftime(&ts);               // Current time
   last_milli = ts.millitm;  //  save milliseconds   

   while (1)                 // Wait for milliseconds to chnage
   {
      ftime(&ts);               // Current time
      last_milli = ts.millitm;  //  save milliseconds	  
      break;              // Break at exact time tick change
   }
   utime.diff = ts.time-then;       // We only want the time since Midnight or Jan 1
   utime.diff *= 1000000;          // Number of Seconds converted to microseconds
   utime.diff += ts.millitm * 1000; //  plus number of Milliseconds converted to microseconds.

   if(mode==API_TTI_IRIG)
      bt.rtn_time = (utime.diff-86400000000ll);        // Pass it back to the caller.
   else
      bt.rtn_time = utime.diff;

   *host_time = bt.rtn_val; 
   return;
}
