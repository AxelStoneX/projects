/*===========================================================================*
 * FILE:                     T S T _ A L L . C
 *===========================================================================*
 *
 * COPYRIGHT (C) 1995 - 2010 BY
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
 *============================================================================*
 *
 * FUNCTION:    Demonstration of BusTools/1553-API routines. This console 
 *              application shows how to setup interrupts on a RT messages.
 *              This program uses BusTools_RegisterFunction. 
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  6/11/2001    initial release
*/

/* Defines for IRIG-B
 Select one of these options for IRIG-B timinig.  If you select IRIG_B_EXTERNAL
 *you need an external IRIG-B signal.  If you select neither then this code
 *defaults to using the internal timer starting at 0. */

//#define IRIG_B_INTERNAL
//#define IRIG_B_EXTERNAL

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include "busapi.h"

int BusController_Init(int,int);
int BusMonitor_Init(int);
int RemoteTerminal_Init(int);
void setupThe_BC_IntFIFO(int);
void setupThe_BM_IntFIFO(int);
void setupThe_RT_IntFIFO(int);

API_INT_FIFO  sIntFIFO1;     // Thread FIFO structure for BC
API_INT_FIFO  sIntFIFO2;     // Thread FIFO structure for RT
API_INT_FIFO  sIntFIFO3;     // Thread FIFO structure for BM

int main(int argc, char **argv)                     
{
   int status,channel;
   unsigned pwFlag,cardnum,cardnum1,device;
   int i;
   time_t tt;
   DeviceList dlist;
   cardnum = 0;

   if(argc >= 2)
      device = atoi(argv[1]);
   else
      device = 0;

   if(argc >= 3)
      channel = atoi(argv[2])-1;
   else
      channel = 0;

   if(argc == 4)
       pwFlag= atoi(argv[3]);
   else
      pwFlag = 3;

   printf("Starting Test for Device %d.  Hit Enter to stop\n\n",device);
   status = BusTools_API_Close(cardnum);

    printf("Initializing Test\n");
/* These initializations are for the the older BusTools_API_InitExtended function */

/* Select the intialization for your board type */
//   status = BusTools_API_Init(cardnum,dwMemAddr[cardnum],wIoAddr[cardnum],&pwFlag);
//   status = BusTools_API_InitExtended(cardnum,0x1000000,0xc3c0,&pwFlag,PLATFORM_USER,QVME1553,NATIVE_32,CHANNEL_1,(BT_UINT)"BTVXIMAP.DLL");
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,ISA1553,NATIVE,SLOT_A,CARRIER_MAP_DEFAULT);
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,PCC1553,NATIVE,SLOT_A,CARRIER_MAP_DEFAULT);
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,QPMC1553,NATIVE,CHANNEL_1,CARRIER_MAP_DEFAULT);
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,Q1041553P,NATIVE,CHANNEL_2,CARRIER_MAP_DEFAULT);
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,QPCI1553,NATIVE,CHANNEL_2,CARRIER_MAP_DEFAULT);
//   status = BusTools_API_Init(cardnum,1,0x160,&pwFlag);
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,QPMC1553,NATIVE,CHANNEL_1,CARRIER_MAP_DEFAULT);
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,IP1553MF,IP1553_ISA,SLOT_A,CARRIER_MAP_DEFAULT);
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,IPD1553,IP1553_PCI,SLOT_A_CH1,CARRIER_MAP_LARGE);
//   status = BusTools_API_InitExtended(cardnum,0,0,&pwFlag,PLATFORM_PC,IP1553,IP1553_pci,SLOT_A,CARRIER_MAP_DEFAULT);
//   status = BusTools_API_InitExtended(cardnum,2,0,&pwFlag,PLATFORM_PC,IP1553,IP1553_ISA,SLOT_A,CARRIER_MAP_DEFAULT);
//   printf("BusTools_API_InitExtended status = %d\n",status);

   status = BusTools_ListDevices(&dlist);
   for(i=0;i<dlist.num_devices;i++)
      printf("%s -- %x -- %d\n",dlist.name[i],dlist.device_name[i],dlist.device_num[i]); 
 

   printf("Initialize Channel %d on Device %d with flag %d\n",channel,device,pwFlag);

   status = BusTools_API_OpenChannel(&cardnum,pwFlag,device,channel);
   printf("BusTools_API_InitExtended status = %d\n",status);

   status = BusTools_SetInternalBus(cardnum,1);                  // Set up internal bus
   printf("BusTools_SetInternalBus status = %d\n",status);

#if defined (IRIG_B_EXTERNAL)
   status = BusTools_IRIG_Config(cardnum,IRIG_EXTERNAL,IRIG_OUT_DISABLE);
   printf("BusTools_IRIG_Config status IRIG_INTERNAL IRIG_OUT_ENABLE = %d\n",status);

   status = BusTools_TimeTagMode( 0, API_TTD_IRIG, API_TTI_IRIG, API_TTM_IRIG, NULL, 0, 0, 0 );
   printf("BusTools_TimeTagMode status = %d\n",status);

   status = BusTools_IRIG_Calibration(cardnum,1);
   printf("BusTools_IRIG_Calibration status = %d\n",status); 

   MSDELAY(2000); //Wait for IRIG-B

   status = BusTools_IRIG_Valid(cardnum);
   if(status == API_IRIG_NO_SIGNAL)
      printf("API_IRIG_NO_SIGNAL\n");
   else
      printf("BusTools_IRIG_Valid status = %d\n",status);
#elif defined(IRIG_B_INTERNAL)
   status = BusTools_IRIG_Config(cardnum,IRIG_INTERNAL,IRIG_OUT_DISABLE);
   printf("BusTools_IRIG_Config status IRIG_INTERNAL IRIG_OUT_ENABLE = %d\n",status);

   BusTools_IRIG_SetTime(cardnum,time(&tt),1);
   printf("BusTools_IRIG_SetTime status = %d\n",status);

   status = BusTools_TimeTagMode( 0, API_TTD_IRIG, API_TTI_IRIG, API_TTM_IRIG, NULL, 0, 0, 0 );
   printf("BusTools_TimeTagMode status = %d\n",status);
 
   MSDELAY(2000); //wait for IRIG-B
#else //IRIG_B_INTERNAL
   status = BusTools_TimeTagMode( cardnum, API_TTD_IRIG, API_TTI_ZERO, API_TTM_FREE, NULL, 0, 0, 0 );
   printf("BusTools_TimeTagMode status = %d\n",status);
#endif //IRIG-B
   status = BusMonitor_Init(cardnum);
   status = BusController_Init(cardnum,0);
   status = RemoteTerminal_Init(cardnum);
 
/* You can select one or all of these interrupt options by uncommenting the 3 lines below */
   setupThe_BC_IntFIFO(cardnum); //Set up BC interrupts using BusTools_RegisterFunction
//   setupThe_BM_IntFIFO(cardnum); //Set up BM interrupts using BusTools_RegisterFunction
//   setupThe_RT_IntFIFO(cardnum); //Set up RT interrupts using BusTools_RegisterFunction
   printf("\nHit enter to start 1553 running\n");
   getchar();

   status = BusTools_BM_StartStop(cardnum,1);
   printf("BusTools_BM_StartStop status = %d\n",status);

   //status = BusTools_RT_StartStop(cardnum,1);
   //printf("BusTools_RT_StartStop status = %d\n",status);

   status = BusTools_BC_StartStop(cardnum,1);
   printf("BusTools_BC_StartStop status = %d\n",status);

   while(getc(stdin) != '\n');

   printf("main: closing API\n");

   status = BusTools_BC_StartStop(cardnum,0);
   printf("BusTools_BC_StartStop status = %d\n",status);

   status = BusTools_BM_StartStop(cardnum,0);
   printf("BusTools_BM_StartStop status = %d\n",status);

   status = BusTools_RT_StartStop(cardnum,0);
   printf("BusTools_RT_StartStop status = %d\n",status);

   status = BusTools_RegisterFunction(cardnum,&sIntFIFO1,0);
   printf("BusTools_RegisterFunction status = %d\n",status);

   status = BusTools_RegisterFunction(cardnum,&sIntFIFO2,0);
   printf("BusTools_RegisterFunction status = %d\n",status);

   status = BusTools_RegisterFunction(cardnum,&sIntFIFO3,0);
   printf("BusTools_RegisterFunction status = %d\n",status);

   status = BusTools_API_Close(cardnum);
   printf("BusTools_API_Close status = %d\n",status);
}

int BusController_Init(int cardnum,int sa_index)
{
   int status, messno;
   API_BC_MBUF bcmessage;

   status = BusTools_BC_Init(cardnum,0,BT1553_INT_END_OF_MESS,0,20,20,1000000,1);
   printf("BusTools_BC_Init status = %d\n",status);

   status = BusTools_BC_MessageAlloc(cardnum,50);
   printf("BusTools_BC_MessageAlloc status = %d\n",status); 

   messno = 0;
   memset((char*)&bcmessage,0,sizeof(bcmessage));    
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.control |= BC_CONTROL_MFRAME_BEG;
   bcmessage.mess_command1.rtaddr   = 2;
   bcmessage.mess_command1.subaddr  = 2;
   bcmessage.mess_command1.wcount   = 2;
   bcmessage.mess_command1.tran_rec = 0;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 8 microsecond inter-message gap.
   bcmessage.data[0][0] = 1;
   bcmessage.data[0][1] = 1;

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage);
   printf("BusTools_BC_MessageWrite status = %d\n",status);
    
   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 3;
   bcmessage.mess_command1.subaddr  = 3;
   bcmessage.mess_command1.wcount   = 3;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 8 microsecond inter-message gap.
 
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage);
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 4;
   bcmessage.mess_command1.subaddr  = 4;
   bcmessage.mess_command1.wcount   = 4;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 8 microsecond inter-message gap.

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 5;
   bcmessage.mess_command1.subaddr  = 5;
   bcmessage.mess_command1.wcount   = 5;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 8 microsecond inter-message gap.

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 2;
   bcmessage.mess_command1.subaddr  = 8;
   bcmessage.mess_command1.wcount   = 8;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 8 microsecond inter-message gap.

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 1;
   bcmessage.mess_command1.subaddr  = 12;
   bcmessage.mess_command1.wcount   = 6;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 8 microsecond inter-message gap.

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   //printf("BusTools_BC_MessageWrite status = %d\n",status);

   /* Mode Code */
   messno++;
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 1;
   bcmessage.mess_command1.subaddr  = 0;
   bcmessage.mess_command1.wcount   = 18;  
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 12 microsecond inter-message gap.
 
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);


   /* RT - RT Message */
   messno++;
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_RTRTFORMAT;
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.control |= BC_CONTROL_MFRAME_END;
   bcmessage.mess_command1.rtaddr   = 1;
   bcmessage.mess_command1.subaddr  = 10;
   bcmessage.mess_command1.wcount   = 6;
   bcmessage.mess_command1.tran_rec = 0;

   bcmessage.mess_command2.rtaddr   = 3;
   bcmessage.mess_command2.subaddr  = 6;
   bcmessage.mess_command2.wcount   = 6;
   bcmessage.mess_command2.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 12 microsecond inter-message gap.
 
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;
   bcmessage.messno = messno;
   bcmessage.messno_next = 0x0;
   bcmessage.messno_branch = 0;
   bcmessage.control |= BC_CONTROL_MFRAME_END;
   bcmessage.control = BC_CONTROL_NOP;        // Do it again and again.
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage);
   printf("BusTools_BC_MessageWrite status = %d\n",status);

#ifdef ERROR_INJECTION
   {
      API_EIBUF ebuf;
      int i;

      ebuf.buftype = EI_BC_REC;

      ebuf.error[0].etype = EI_BITCOUNT;
      ebuf.error[0].edata = 16;

      for ( i = 1; i < EI_COUNT; i++ )
      {
	     ebuf.error[i].etype = EI_NONE;
	     ebuf.error[i].edata = 0;
      }

      status = BusTools_EI_EbufWrite(cardnum, 1, &ebuf);
   }
#endif

   return status; 
} 

/****************************************************************************
*
*  Function:  bc_intFunction
*
*  Description:
*     This routine is the user callback function invoke when the Board detects
*     a end-of-message condition. This function read the data in the RT message
*     and print message data to the screen.
*
****************************************************************************/
BT_INT  _stdcall bc_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO)
{   
   API_BC_MBUF bcmessage;
   int       bufnum;
   static int sa_index;
   int       i,j;
   int       status;
   int       wcount;
   BT_INT tail, rtrt,mc;
   BT_UINT messno;

   tail = sIntFIFO->tail_index;
   while(tail != sIntFIFO->head_index)
   {
	   if(sIntFIFO->FilterType == EVENT_BC_MESSAGE)
	   {
		  messno = sIntFIFO->fifo[tail].bufferID;        
          status = BusTools_BC_MessageRead(cardnum,messno,&bcmessage);
          if (status)
             return status;
          rtrt=0;
          mc=0;
          if(bcmessage.status & BT1553_INT_RT_RT_FORMAT)
          {
             printf("RT<->RT (%d) \n",messno);
             rtrt=1;
          }
          else if(bcmessage.status & BT1553_INT_MODE_CODE)
          {
             printf("Mode Code --  (%d) \n", messno);
             mc=1;
          }
          else if (bcmessage.mess_command1.tran_rec)
             printf("BC**RT->BC (%d)  \n", messno);
          else
             printf("BC->RT (%d)  \n", messno);
 
          printf("BC**RT-%d  \n",bcmessage.mess_command1.rtaddr);
          printf("BC**SA-%d  \n",bcmessage.mess_command1.subaddr);
          printf("BC**WC-%d  \n",bcmessage.mess_command1.wcount);
          if(rtrt)
          {   
             printf("BC**RT-%d  \n",bcmessage.mess_command2.rtaddr);
             printf("BC**SA-%d  \n",bcmessage.mess_command2.subaddr);
             printf("BC**WC-%d  \n",bcmessage.mess_command2.wcount);
          }
        
          printf("BC**status-1 0x%04x\n",*((WORD*)(&bcmessage.mess_status1)));
          if(rtrt)
             printf("BC**status-2 0x%04x\n",*((WORD*)(&bcmessage.mess_status2)));
          
          printf("BC**int_stat-0x%08lx\n",*((DWORD*)(&bcmessage.status)));

          wcount = bcmessage.mess_command1.wcount;
          if (wcount == 0)
             wcount = 32;           // Zero word count means 32 words.

          if(mc)
            if(wcount >=16)
               wcount = 1;

          j = wcount;               // See if this msg shorted than previous
    
          if (bcmessage.control & BC_CONTROL_BUFFERA)
             bufnum = 0;            // Display data from first BC buffer
          else
             bufnum = 1;            // Display data from second BC buffer

          for ( i = 0; i < j; i++ ) // Display all of the data words
          {
             // If this is an RT->BC message, and there is a Message Error
             //  or No Response, than xxxx the data (since the RT did not
             //    send any data words).
             if ( i >= wcount )
             {
                printf("    ");     // Data does not apply to this msg
             }
             else if ( bcmessage.mess_command1.tran_rec &&
               (bcmessage.mess_status1.me || (bcmessage.status&BT1553_INT_NO_RESP) ) )
             {
                printf("xxxx ");      // No data was transfered because of error
             }
             else
             {
                printf("BC**%04x ",bcmessage.data[bufnum][i]);
             }
          }
          printf("\n");
       }
            
       tail++;
       tail &= sIntFIFO->mask_index;
       sIntFIFO->tail_index = tail;
   }

   return 0;
}

/****************************************************************************
*
*  Function:  bm_intFunction
*
*  Description:
*     This routine is the user callback function invoke when the Board detects
*     a end-of-message condition. This function read the data in the RT message
*     and print message data to the screen.
*
****************************************************************************/
BT_INT  _stdcall bm_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO)
{   
   API_BM_MBUF mbuf;
   int       status;
   BT_INT tail;
   BT_UINT messno;
   char      outbuf[100];

   tail = sIntFIFO->tail_index;
   while(tail != sIntFIFO->head_index)
   {
	   if(sIntFIFO->FilterType & EVENT_BM_MESSAGE)
	   {
          messno = sIntFIFO->fifo[tail].bufferID;
          status = BusTools_BM_MessageRead(cardnum,messno,&mbuf);// read the BM message data
          if (status)
             return status;

          BusTools_TimeGetString(&mbuf.time, outbuf);
          printf("BM>>>>> time tag = %s\n",outbuf);
          if (mbuf.command1.tran_rec)
             printf("BM>>>>> RT->BC (%d)  \n", messno);
          else
             printf("BM>>>>> BC->RT (%d)  \n", messno);
          printf("BM>>>>> RT-%d  \n",mbuf.command1.rtaddr);
          printf("BM>>>>> SA-%d  \n",mbuf.command1.subaddr);
          printf("BM>>>>> WC%d  \n",mbuf.command1.wcount);
          printf("BM>>>>> 0x%08lx\n",mbuf.int_status);
	   }
       tail++;
       tail &= sIntFIFO->mask_index;
       sIntFIFO->tail_index = tail;
   }
   return 0;
}

/****************************************************************************
*
*  Function:  rt_intFunction
*
*  Description:
*     This routine is the user callback function invoke when the Board detects
*     a end-of-message condition. This function read the data in the RT message
*     and print message data to the screen.
*
****************************************************************************/
BT_INT _stdcall rt_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO)
{
   int status;
   API_RT_MBUF_READ rt_mbuf;
   BT_INT tail;
   BT_UINT messno;
   BT_UINT rt_addr,sub_addr,trans;
   static int sflg = 0;
   char      outbuf[100];
   BT1553_TIME tt;
   int wcount,i,mc;

   mc = 0;
   tail = sIntFIFO->tail_index;
   while(tail != sIntFIFO->head_index)
   {
	   if(sIntFIFO->FilterType == EVENT_RT_MESSAGE)
	   {
	      messno   = sIntFIFO->fifo[tail].bufferID;
          rt_addr  = sIntFIFO->fifo[tail].rtaddress;        // RT address
          trans    = sIntFIFO->fifo[tail].transrec;         // Transmit/Receive
          sub_addr = sIntFIFO->fifo[tail].subaddress;       // Subaddress number

	      status = BusTools_RT_MessageRead( cardnum, rt_addr, sub_addr, trans, messno, &rt_mbuf);
            
          if (status)
             return status;

          BusTools_TimeGetString(&rt_mbuf.time, outbuf);
          printf("tag time = %s\n",outbuf);

		  status = BusTools_TimeTagRead(cardnum,&tt);
          BusTools_TimeGetString(&tt, outbuf);
		  printf("tag time read = %s\n",outbuf);
          if(rt_mbuf.status & BT1553_INT_RT_RT_FORMAT)
             printf("RT<->RT (%d) \n",messno);
          else if(rt_mbuf.status & BT1553_INT_MODE_CODE)
          {
             printf("RT--MC  (%d) \n", messno);
             mc = 1;
          }
          else if (rt_mbuf.mess_command.tran_rec)
             printf("RT->BC (%d) \n", messno);
          else
             printf("BC->RT (%d)  \n", messno);
          printf("RT**RT-%d  \n",rt_mbuf.mess_command.rtaddr);
          printf("RT**SA-%d  \n",rt_mbuf.mess_command.subaddr);
          printf("RT**WC-%d  \n",rt_mbuf.mess_command.wcount);
          printf("RT**mstat-0x%04x\n",*((WORD*)(&rt_mbuf.mess_status)));
          printf("RT**int_stat-0x%08lx\n",rt_mbuf.status);
          if (rt_mbuf.mess_command.wcount == 0)
             wcount = 32;           // Zero word count means 32 words.
          else
             wcount = rt_mbuf.mess_command.wcount ;               // See if this msg shorted than previous

          if(mc)
            if(wcount >=16)
               wcount = 1;

          for ( i = 0; i < wcount; i++ ) // Display all of the data words
          {
             // If this is an RT->BC message, and there is a Message Error
             //  or No Response, than xxxx the data (since the RT did not
             //    send any data words).
         
             printf("RT**%04x ",rt_mbuf.mess_data[i]);
		  }
          printf("\n");         
	  if(sflg)
          {
	    status = BusTools_RT_MessageWriteStatusWord(cardnum, rt_addr,sub_addr,trans,messno, API_1553_STAT_TF, 1);
          }
	  else
          {
            status = BusTools_RT_MessageWriteStatusWord(cardnum, rt_addr,sub_addr,trans,messno, API_1553_STAT_TF, 0);
          }
	  sflg^=1;
      }
       tail++;
       tail &= sIntFIFO->mask_index;
       sIntFIFO->tail_index = tail;
   }
   return 0;
}

int RemoteTerminal_Init(int cardnum)
{
   int status;
   int RT_MBUF_COUNT;
   API_RT_CBUF cbuf;       // RT Control Buffer Legal Word Count Mask (2 words)
   API_RT_MBUF_WRITE mbuf;
   API_RT_ABUF abuf;
   int         tr;
   int         subaddress;
   int i,j,ii;
   int rt_addr[] = {1,2,3,4,5};

   RT_MBUF_COUNT = 4;

   status = BusTools_RT_Init(cardnum,0);
   printf("BusTools_RT_Init status = %d\n",status);

   /**********************************************************************
   *  Output RT control buffers
   *   Enable or disable all word counts and mode codes as specified by
   *    the caller's arguements.
   **********************************************************************/

   cbuf.legal_wordcount = 0xffffffff;  // legailize all word counts
   for ( i = 0; i < 5; i++)
   {
      for ( tr = 0; tr < 2; tr++ )
      {
         for ( subaddress = 0; subaddress < 32; subaddress++ )
         {
            status = BusTools_RT_CbufWrite(cardnum,rt_addr[i],subaddress,tr,
			                            RT_MBUF_COUNT,&cbuf);
         }
      }
   }

   /*******************************************************************
   *  Fill in message buffers for receive and transmit sub-units.
   *******************************************************************/

   mbuf.enable = BT1553_INT_END_OF_MESS;   // Interrupt enables on Broadcast Message
   mbuf.error_inj_id = 0;

   // For all subaddresses...
   for (ii = 0; ii<5; ii++)
   {  
      for (i=0; i<32; i++)
      {
         // Create data buffer data
         for (j=0; j<32; j++)
            mbuf.mess_data[j] = (BT_U16BIT)(0xF000 + i);
         // For each data buffer...
         for (j=0; j<RT_MBUF_COUNT; j++)
         {
            status = BusTools_RT_MessageWrite(cardnum,rt_addr[ii],i,0,j,&mbuf);
            status = BusTools_RT_MessageWrite(cardnum,rt_addr[ii],i,1,j,&mbuf);
         }
      }
   }

   /************************************************
   *  Setup RT address control block
   ************************************************/

   abuf.enable_a          = 1;    // Enable Bus A...Note that the API bits are
   abuf.enable_b          = 1;    // Enable Bus B... reversed from the HW bits.
   abuf.inhibit_term_flag = 0;
   abuf.status            = 0x0000;
   abuf.command           = 0;
   abuf.bit_word          = 0;

   for(i=0;i<5;i++)
   {
      status = BusTools_RT_AbufWrite(cardnum,rt_addr[i],&abuf);
      printf("BusTools_RT_AbufWrite status = %d\n",status);
   }

#ifdef auto_incr
   status = BusTools_RT_AutoIncrMessageData(cardnum,2,2,1,0,1,1,0,1);          
   status = BusTools_RT_AutoIncrMessageData(cardnum,3,3,0,0,1,1,0,1);
   status = BusTools_RT_AutoIncrMessageData(cardnum,4,4,3,0,1,1,0,1);
   status = BusTools_RT_AutoIncrMessageData(cardnum,5,5,4,0,1,1,0,1);
#endif
   
   return status;
}

int BusMonitor_Init(int cardnum)
{
   int status;
   BT_UINT actual;

   status = BusTools_BM_Init(cardnum,1,1);
   printf("BusTools_BM_Init status = %d\n",status);

   status = BusTools_BM_MessageAlloc(cardnum, 300, &actual, 0xffffffff ); //interrupt on END_OF_MESS
   printf("BusTools_BM_MessageAlloc status = %d - actual =%d\n",status,actual);

   return status;
}

/****************************************************************************
*
*  Function:  setupThe_RT_IntFIFO1
*
*  Description:
*     This routine setup the interrupt FIFO with the value needed for a 
*     interrupt on a RT message.  The interrupt is set for only the end-of
*     message.  
*
*     Any errors detected are reported to the user.  The user
*     has the option of terminating the test by striking any key.
*
****************************************************************************/
void setupThe_RT_IntFIFO(int cardnum)
{
   int rt,tr,sa,status;
   // Setup the FIFO structure for this board.
   memset(&sIntFIFO2, 0, sizeof(sIntFIFO2));
   sIntFIFO2.function     = rt_intFunction;
   sIntFIFO2.iPriority    = 2;
   sIntFIFO2.dwMilliseconds = INFINITE;
   sIntFIFO2.iNotification  = 0;
   sIntFIFO2.FilterType     = EVENT_RT_MESSAGE;

   for ( rt=0; rt < 32; rt++ )
      for (tr = 0; tr < 2; tr++ )
         for (sa = 0; sa < 32; sa++ )
            sIntFIFO2.FilterMask[rt][tr][sa] =  0xffffffff;  // Disable all Word Counts on all RT/SA/TR combos
		 
   // Call the register function to register and start the thread.
   status = BusTools_RegisterFunction(cardnum, &sIntFIFO2, 1);
   printf("BusTools_RegisterFunction status = %d\n",status);
}

/****************************************************************************
*
*  Function:  setupThe_BC_IntFIFO
*
*  Description:
*     This routine setup the interrupt FIFO with the value needed for a 
*     interrupt on a BC message.  The interrupt is set for only the end-of
*     message.  
*
*     Any errors detected are reported to the user.  The user
*     has the option of terminating the test by striking any key.
*
****************************************************************************/
void setupThe_BC_IntFIFO(int cardnum)
{
   int rt,tr,sa,status;
   // Setup the FIFO structure for this board.
   memset(&sIntFIFO1, 0, sizeof(sIntFIFO1));
   sIntFIFO1.function     = bc_intFunction;
   sIntFIFO1.iPriority    = 64;
   sIntFIFO1.dwMilliseconds = INFINITE;
   sIntFIFO1.iNotification  = 0;
   sIntFIFO1.FilterType     = EVENT_BC_MESSAGE;

   for ( rt=0; rt < 32; rt++ )
      for (tr = 0; tr < 2; tr++ )
         for (sa = 0; sa < 32; sa++ )
            sIntFIFO1.FilterMask[rt][tr][sa] =  0xffffffff;  // Enable all Word Counts on all RT/SA/TR combos

/* Use this if to get notified only on No response, late response of message error 
   sIntFIFO1.EventInit=USE_INTERRUPT_MASK;   
   for ( rt=0; rt < 32; rt++ )
      for (tr = 0; tr < 2; tr++ )
         for (sa = 0; sa < 32; sa++ )
            sIntFIFO1.EventMask[rt][tr][sa] = BT1553_INT_NO_RESP | BT1553_INT_ME_BIT //interrupt on ME and no-rsp
                                              | BT1553_INT_LATE_RESP; // interrupt on Late-response
*/
   // Call the register function to register and start the thread.
   status = BusTools_RegisterFunction(cardnum, &sIntFIFO1, 1);
   printf("BusTools_RegisterFunction status = %d\n",status);
}

/****************************************************************************
*
*  Function:  setupThe_BM_IntFIFO 
*
*  Description:
*     This routine setup the interrupt FIFO with the value needed for a 
*     interrupt on a BC message.  The interrupt is set for only the end-of
*     message.  
*
*     Any errors detected are reported to the user.  The user
*     has the option of terminating the test by striking any key.
*
****************************************************************************/
void setupThe_BM_IntFIFO(int cardnum)
{
   int rt,tr,sa,status;
   // Setup the FIFO structure for this board.
   memset(&sIntFIFO3, 0, sizeof(sIntFIFO3));
   sIntFIFO3.function     = bm_intFunction;
   sIntFIFO3.iPriority    = THREAD_PRIORITY_NORMAL;
   sIntFIFO3.dwMilliseconds = INFINITE;
   sIntFIFO3.iNotification  = 0;
   sIntFIFO3.FilterType     = EVENT_BM_MESSAGE;

   for ( rt=0; rt < 32; rt++ )
      for (tr = 0; tr < 2; tr++ )
         for (sa = 0; sa < 32; sa++ )
            sIntFIFO3.FilterMask[rt][tr][sa] =  0xffffffff;  // Enable all Word Counts on all RT/SA/TR combos

/********* Interrupt only on message error ***********************************
   sIntFIFO3.EventInit=USE_INTERRUPT_MASK;   
   for ( rt=0; rt < 32; rt++ )
      for (tr = 0; tr < 2; tr++ )
         for (sa = 0; sa < 32; sa++ )
            sIntFIFO3.EventMask[rt][tr][sa] = BT1553_INT_ME_BIT;  // Enable all Word Counts on all RT/SA/TR combos
*****************************************************************************/
   // Call the register function to register and start the thread.
   status = BusTools_RegisterFunction(cardnum, &sIntFIFO3, 1);
   printf("BusTools_RegisterFunction status = %d\n",status);
}
