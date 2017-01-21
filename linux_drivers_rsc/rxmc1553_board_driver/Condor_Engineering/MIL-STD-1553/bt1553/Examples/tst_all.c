/*===========================================================================*
 * FILE:                     T S T _ A L L . C
 *===========================================================================*
 *
 * COPYRIGHT (C) 1995 - 2012 BY
 *          GE INTELLIGENT PLATFORMS, INC., GOLETA, CALIFORNIA
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
 *
 *===========================================================================*/

/* $Revision:  2.00 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  6/11/2001     initial release
  11/18/2008    merged x86 and PPC versions. added support for QPM-1553. added
                 BusTools_GetRevision. bch
  10/22/2012   	added status checking, configuration options, and support for
                 UCA32 boards. bch
*/



/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "busapi.h"


/*  CONFIGURATION(required): select one option for the interrupt mode: "software interrupts", "hardware interrupts", or both  */
#define BT_PWFLAG  API_SW_INTERRUPT
//#define BT_PWFLAG  API_HW_ONLY_INT
//#define BT_PWFLAG  API_HW_INTERRUPT

/*  CONFIGURATION(required): select the minor frame rate for the BC  */
#define MINOR_FRAME_RATE  1000000

/*  CONFIGURATION(optional): select which function to monitor traffic using BusTools_RegisterFunction  */
#define CONFIG_BM_INTFIFO  // setup BM interrupts
//#define CONFIG_RT_INTFIFO  // setup RT interrupts
//#define CONFIG_BC_INTFIFO  // setup BC interrupts

/*  CONFIGURATION(optional): select one option for IRIG-B timing:  IRIG_B_EXTERNAL will require an
 *   external IRIG-B signal, while IRIG_B_INTERNAL will use the internal IRIG.  */
//#define IRIG_B_EXTERNAL
//#define IRIG_B_INTERNAL

/*  CONFIGURATION(optional): increment RT data words  */
//#define ENABLE_RT_AUTO_INCREMENT

/*  set the TEST_HWINT compiler define to compile with hardware interrupts  */
#ifdef TEST_HWINT
#undef BT_PWFLAG
#define BT_PWFLAG  API_HW_ONLY_INT
#endif

// prototypes
BT_INT init_BM(BT_UINT cardnum);
BT_INT init_RT(BT_UINT cardnum);
BT_INT init_BC(BT_UINT cardnum, BT_INT sa_index);
BT_INT _stdcall bm_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO);
BT_INT _stdcall rt_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO);
BT_INT _stdcall bc_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO);
BT_INT setupThe_BM_IntFIFO(BT_UINT cardnum);
BT_INT setupThe_RT_IntFIFO(BT_UINT cardnum);
BT_INT setupThe_BC_IntFIFO(BT_UINT cardnum);

// globals
API_INT_FIFO  sIntFIFO_BC;
API_INT_FIFO  sIntFIFO_RT;
API_INT_FIFO  sIntFIFO_BM;


int main(int argc, char **argv) {
   BT_INT i, status=0, build=0, channel=0, time_ns=0;
   BT_UINT cardnum=0, device=0, ucode_rev=0, api_rev=0, pwFlag=BT_PWFLAG;
   float wrev, lrev;
   DeviceList dlist;

   if(argc >= 2)
     device = atoi(argv[1]);
   if(argc >= 3)
     channel = atoi(argv[2]);

   printf("Starting test for device %d.  Hit Enter to stop\n\n", device);
   BusTools_API_Close(cardnum);

   if((status = BusTools_ListDevices(&dlist)) != API_SUCCESS)
     printf("Fail: BusTools_ListDevices status = %d\n", status);
   else {
     for(i=0; i<dlist.num_devices; i++)
       printf("Board Type %s ID 0x%x is device %d\n",dlist.name[i],dlist.device_name[i],dlist.device_num[i]);
   } 

   printf("Initializing device\n");
   if((status = BusTools_API_OpenChannel(&cardnum, pwFlag, device, channel)) != API_SUCCESS) {
     printf("Fail: BusTools_API_OpenChannel status (%d, %d) = %d\n", device, channel, status);
     return -1;
   }

   if((status = BusTools_GetRevision(cardnum, &ucode_rev, &api_rev)) != API_SUCCESS)
     printf("Fail: BusTools_GetRevision status (%d, %d) = %d\n", device, channel, status);
   else
     printf("  Microcode revision = 0x%X, API revision = %d\n",ucode_rev, api_rev);

   if((status = BusTools_GetFWRevision(cardnum, &wrev, &lrev, &build)) != API_SUCCESS)
     printf("Fail: BusTools_GetFWRevision status (%d, %d) = %d\n", device, channel, status);
   else
     printf("  WCS revision = %.2f, LPU revision = %.2f\n",wrev, lrev);

   /*  select either: 0 = external bus, 1 = internal bus  */
   if((status = BusTools_SetInternalBus(cardnum,1)) != API_SUCCESS)
     printf("Fail: BusTools_SetInternalBus status (%d, %d) = %d\n", device, channel, status);

  #ifdef LYNXOS_VME
   if((status = BusTools_VME_Reset(cardnum, 1)) != API_SUCCESS)
     printf("Fail: BusTools_VME_Reset status (%d, %d) = %d\n", device, channel, status);
  #endif


   // check if board is UCA32
   {
     BT_U16BIT csc=0, acr=0;
     if((status = BusTools_GetCSCRegs(cardnum, &csc, &acr)) != API_SUCCESS)
       printf("Fail: BusTools_GetCSCRegs status for cardnum %d = %d\n", cardnum, status);
     else if(csc & 0x4000)
       time_ns = API_TT_NANO;
   }


  #if defined(IRIG_B_EXTERNAL)
   if((status = BusTools_IRIG_Config(cardnum, IRIG_EXTERNAL, IRIG_OUT_DISABLE)) != API_SUCCESS)
     printf("Fail: BusTools_IRIG_Config status (%d, %d) = %d\n", device, channel, status);

   if((status = BusTools_TimeTagMode(cardnum, API_TTD_IRIG|time_ns, API_TTI_IRIG, API_TTM_IRIG, NULL, 0, 0, 0 )) != API_SUCCESS)
     printf("Fail: BusTools_TimeTagMode status (%d, %d) = %d\n", device, channel, status);

   if((status = BusTools_IRIG_Calibration(cardnum, 1)) != API_SUCCESS);
     printf("Fail: BusTools_IRIG_Calibration status (%d, %d) = %d\n", device, channel, status);

   usleep(2000000); //Wait for IRIG-B

   status = BusTools_IRIG_Valid(cardnum);
   if(status == API_IRIG_NO_SIGNAL)
      printf("Failed to detected IRIG signal\n");
   else if(status != API_SUCCESS)
     printf("Fail: BusTools_IRIG_Valid status (%d, %d) = %d\n", device, channel, status);
  #elif defined(IRIG_B_INTERNAL)
   if((status = BusTools_IRIG_Config(cardnum, IRIG_INTERNAL, IRIG_OUT_DISABLE)) != API_SUCCESS)
     printf("Fail: BusTools_IRIG_Config status (%d, %d) = %d\n", device, channel, status);

   {
     time_t tt;
     if((BusTools_IRIG_SetTime(cardnum, time(&tt), 1)) != API_SUCCESS);
       printf("Fail: BusTools_IRIG_SetTime status (%d, %d) = %d\n", device, channel, status);
   }

   if((status = BusTools_TimeTagMode(cardnum, API_TTD_IRIG|time_ns, API_TTI_IRIG, API_TTM_IRIG, NULL, 0, 0, 0 )) != API_SUCCESS)
     printf("Fail: BusTools_TimeTagMode status (%d, %d) = %d\n", device, channel, status);

   usleep(2000000); //wait for IRIG-B
  #else
   if((status = BusTools_TimeTagMode(cardnum, API_TTD_DATE|time_ns, API_TTI_ZERO, API_TTM_FREE, NULL, 0, 0, 0 )) != API_SUCCESS)
     printf("Fail: BusTools_TimeTagMode status (%d, %d) = %d\n", device, channel, status);
  #endif

   /*  initialize the BM  */
   if((status = init_BM(cardnum)) != API_SUCCESS)
     return -1;

   /*  initialize the RT  */
   if((status = init_RT(cardnum)) != API_SUCCESS)
     return -1;

   /*  initialize the BC  */
   if((status = init_BC(cardnum, 0)) != API_SUCCESS)
     return -1;

  #ifdef CONFIG_BM_INTFIFO
   if(setupThe_BM_IntFIFO(cardnum) != API_SUCCESS)
     return -1;
  #endif

  #ifdef CONFIG_RT_INTFIFO
   if(setupThe_RT_IntFIFO(cardnum) != API_SUCCESS)
     return -1;
  #endif

  #ifdef CONFIG_BC_INTFIFO
   if(setupThe_BC_IntFIFO(cardnum) != API_SUCCESS)
     return -1;
  #endif

   /*  start the BM  */
   if((status = BusTools_BM_StartStop(cardnum, 1)) != API_SUCCESS) {
     printf("Fail: BusTools_BM_StartStop status (%d, %d) = %d\n", device, channel, status);
     BusTools_API_Close(cardnum);
     return -1;
   }

   /*  start the RT  */
   if((status = BusTools_RT_StartStop(cardnum, 1)) != API_SUCCESS) {
     printf("Fail: BusTools_RT_StartStop status (%d, %d) = %d\n", device, channel, status);
     BusTools_API_Close(cardnum);
     return -1;
   }

   /*  start the BC  */
   if((status = BusTools_BC_StartStop(cardnum, 1)) != API_SUCCESS) {
     printf("Fail: BusTools_BC_StartStop status (%d, %d) = %d\n", device, channel, status);
     BusTools_API_Close(cardnum);
     return -1;
   }

   /*  wait for user to hit enter  */
   while(getc(stdin) != '\n');

   printf("main: closing device %d\n", device);

   if((status = BusTools_BC_StartStop(cardnum,0)) != API_SUCCESS)
     printf("Fail: BusTools_BC_StartStop status (%d, %d) = %d\n", device, channel, status);

   if((status = BusTools_BM_StartStop(cardnum,0)) != API_SUCCESS)
     printf("Fail: BusTools_BM_StartStop status (%d, %d) = %d\n", device, channel, status);

   if((status = BusTools_RT_StartStop(cardnum,0)) != API_SUCCESS)
     printf("Fail: BusTools_RT_StartStop status (%d, %d) = %d\n", device, channel, status);

  #ifdef CONFIG_BC_INTFIFO
   if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_BC, 0)) != API_SUCCESS)
     printf("Fail: BusTools_RegisterFunction status (BC, %d, %d) = %d\n", device, channel, status);
  #endif

  #ifdef CONFIG_RT_INTFIFO
   if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_RT, 0)) != API_SUCCESS)
     printf("Fail: BusTools_RegisterFunction status (RT, %d, %d) = %d\n", device, channel, status);
  #endif

  #ifdef CONFIG_BM_INTFIFO
   if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_BM, 0)) != API_SUCCESS)
     printf("Fail: BusTools_RegisterFunction status (BM, %d, %d) = %d\n", device, channel, status);
  #endif

   if((status = BusTools_API_Close(cardnum)) != API_SUCCESS)
     printf("Fail: BusTools_API_Close status (%d, %d) = %d\n", device, channel, status);

   return 0;
}


BT_INT init_BM(BT_UINT cardnum) {
  BT_INT status=0;
  BT_UINT actual=0;

  if((status = BusTools_BM_Init(cardnum, 1, 1)) != API_SUCCESS) {
    printf("Fail: BusTools_BM_Init status (%d), status = %d\n", cardnum, status);
    return status;
  }

  if((status = BusTools_BM_MessageAlloc(cardnum, 300, &actual, 0xFFFFFFFF)) != API_SUCCESS) {
    printf("Fail: BusTools_BM_MessageAlloc (%d), actual = %d, status = %d\n", cardnum, actual, status);
    return status;
  }

  return API_SUCCESS;
}


BT_INT init_RT(BT_UINT cardnum) {
  BT_INT status=0, i, rt=0, sa=0, tr=0;
  API_RT_CBUF cbuf;
  API_RT_MBUF_WRITE mbuf;
  API_RT_ABUF abuf;


  if((status = BusTools_RT_Init(cardnum, 0)) != API_SUCCESS) {
    printf("Fail: BusTools_RT_Init (%d), status = %d\n", cardnum, status);
    return status;
  }

  cbuf.legal_wordcount = 0xFFFFFFFF;
  for(rt = 1; rt <= 5; rt++) {
    for(tr = 0; tr < 2; tr++) {
      for(sa = 0; sa < 31; sa++) {
        if((status = BusTools_RT_CbufWrite(cardnum, rt, sa, tr, 4, &cbuf)) != API_SUCCESS) {
          printf("Fail: BusTools_RT_CbufWrite (%d), RT = %d, SA = %d, TR = %d, status = %d\n", cardnum, rt, sa, tr, status);
          return status;
        }
      }
    }
  }

  memset(&mbuf, 0, sizeof(mbuf));
  /*  Interrupt enables on Broadcast Message  */
  mbuf.enable = BT1553_INT_END_OF_MESS;

  for(rt = 1; rt <= 5; rt++) {
    for(sa = 0; sa < 31; sa++) {
      for(i = 0; i < 32; i++)
        mbuf.mess_data[i] = (BT_U16BIT)(0xF000 + i);
      for(i = 0; i < 4; i++) {
        if((status = BusTools_RT_MessageWrite(cardnum, rt, sa, 0, i, &mbuf)) != API_SUCCESS) {
          printf("Fail: BusTools_RT_MessageWrite (%d), RT = %d, SA = %d, msg = %d, status = %d\n", cardnum, rt, sa, i, status);
          return status;
        }
        if((status = BusTools_RT_MessageWrite(cardnum, rt, sa, 1, i, &mbuf)) != API_SUCCESS) {
          printf("Fail: BusTools_RT_MessageWrite (%d), RT = %d, SA = %d, msg = %d, status = %d\n", cardnum, rt, sa, i, status);
          return status;
        }
      }
    }
  }

  memset(&abuf, 0, sizeof(abuf));
  abuf.enable_a = 1;
  abuf.enable_b = 1;
  for(rt = 1; rt <= 5; rt++) {
    if((status = BusTools_RT_AbufWrite(cardnum, rt, &abuf)) != API_SUCCESS) {
      printf("Fail: BusTools_RT_MessageWrite (%d), RT = %d, status = %d\n", cardnum, rt, status);
      return status;
    }
  }

#ifdef ENABLE_RT_AUTO_INCREMENT
  if((status = BusTools_RT_AutoIncrMessageData(cardnum, 2, 2, 1, 0, 1, 1, 0, 1)) != API_SUCCESS)
    printf("Fail: BusTools_RT_AutoIncrMessageData (%d), RT = 2, SA = 2, status = %d\n", cardnum, status);
  if((status = BusTools_RT_AutoIncrMessageData(cardnum, 3, 3, 0, 0, 1, 1, 0, 1)) != API_SUCCESS)
    printf("Fail: BusTools_RT_AutoIncrMessageData (%d), RT = 3, SA = 3, status = %d\n", cardnum, status);
  if((status = BusTools_RT_AutoIncrMessageData(cardnum, 4, 4, 3, 0, 1, 1, 0, 1)) != API_SUCCESS)
    printf("Fail: BusTools_RT_AutoIncrMessageData (%d), RT = 4, SA = 4, status = %d\n", cardnum, status);
  if((status = BusTools_RT_AutoIncrMessageData(cardnum, 5, 5, 4, 0, 1, 1, 0, 1)) != API_SUCCESS)
    printf("Fail: BusTools_RT_AutoIncrMessageData (%d), RT = 5, SA = 5, status = %d\n", cardnum, status);
#endif

   return API_SUCCESS;
}


BT_INT init_BC(BT_UINT cardnum, BT_INT sa_index) {
  BT_INT status=0, messno=0;
  API_BC_MBUF bcmessage;

  if((status = BusTools_BC_Init(cardnum, 0, BT1553_INT_END_OF_MESS, 0, 20, 20, MINOR_FRAME_RATE, 1)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_Init status (%d) = %d\n", cardnum, status);
     return status;
  }

  if((status = BusTools_BC_MessageAlloc(cardnum, 10)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageAlloc status (%d) = %d\n", cardnum, status);
    return status;
  }

  messno = 0;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = (BT_U16BIT)(messno + 1);
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CONTROL_CHANNELA | BC_CONTROL_MFRAME_BEG | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 2;
  bcmessage.mess_command1.subaddr  = 2;
  bcmessage.mess_command1.wcount   = 2;
  bcmessage.mess_command1.tran_rec = 0;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  bcmessage.data[0][0] = 1;
  bcmessage.data[0][1] = 1;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = (BT_U16BIT)(messno + 1);
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CONTROL_CHANNELA | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 3;
  bcmessage.mess_command1.subaddr  = 3;
  bcmessage.mess_command1.wcount   = 3;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = (BT_U16BIT)(messno + 1);
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CONTROL_CHANNELA | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 4;
  bcmessage.mess_command1.subaddr  = 4;
  bcmessage.mess_command1.wcount   = 4;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = (BT_U16BIT)(messno + 1);
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CONTROL_CHANNELA | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 5;
  bcmessage.mess_command1.subaddr  = 5;
  bcmessage.mess_command1.wcount   = 5;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = (BT_U16BIT)(messno + 1);
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CONTROL_CHANNELA | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 2;
  bcmessage.mess_command1.subaddr  = 8;
  bcmessage.mess_command1.wcount   = 8;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = (BT_U16BIT)(messno + 1);
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CONTROL_CHANNELA | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 1;
  bcmessage.mess_command1.subaddr  = 12;
  bcmessage.mess_command1.wcount   = 6;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

  /* Mode Code */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = (BT_U16BIT)(messno + 1);
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CONTROL_CHANNELA | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 1;
  bcmessage.mess_command1.subaddr  = 0;
  bcmessage.mess_command1.wcount   = 18;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

  /* RT - RT Message */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = (BT_U16BIT)(messno + 1);
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CONTROL_CHANNELA | BC_CONTROL_RTRTFORMAT | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 1;
  bcmessage.mess_command1.subaddr  = 10;
  bcmessage.mess_command1.wcount   = 6;
  bcmessage.mess_command1.tran_rec = 0;
  bcmessage.mess_command2.rtaddr   = 3;
  bcmessage.mess_command2.subaddr  = 6;
  bcmessage.mess_command2.wcount   = 6;
  bcmessage.mess_command2.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 200;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

  messno++;
  bcmessage.messno = messno;
  bcmessage.messno_next = 0x0;
  bcmessage.messno_branch = 0;
  bcmessage.control = BC_CONTROL_MFRAME_END;
//   bcmessage.control = BC_CONTROL_NOP;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    printf("Fail: BusTools_BC_MessageWrite status (%d) = %d\n", cardnum, status);
    return status;
  }

 #ifdef ERROR_INJECTION
  {
    API_EIBUF ebuf;
    int i;

    ebuf.buftype = EI_BC_REC;
    ebuf.error[0].etype = EI_BITCOUNT;
    ebuf.error[0].edata = 16;
    for(i=1;i<EI_COUNT;i++) {
      ebuf.error[i].etype = EI_NONE;
      ebuf.error[i].edata = 0;
    }

    if((status = BusTools_EI_EbufWrite(cardnum, 1, &ebuf)) != API_SUCCESS)
      printf("Fail: BusTools_EI_EbufWrite status (%d) = %d\n", cardnum, status);
  }
 #endif

  return API_SUCCESS;
}


BT_INT _stdcall bm_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO) {
  API_BM_MBUF mbuf;
  BT_INT status=0, tail=0;
  char outbuf[100];

  tail = sIntFIFO->tail_index;
  while(tail != sIntFIFO->head_index) {
    if(sIntFIFO->FilterType & EVENT_BM_MESSAGE) {
      if((status = BusTools_BM_MessageRead(cardnum, sIntFIFO->fifo[tail].bufferID, &mbuf)) != API_SUCCESS) {
        printf("Fail: BusTools_BM_MessageRead status (%d) = %d\n", cardnum, status);
        return status;
      }
      if(mbuf.command1.tran_rec)
        printf("BM:  RT->BC\n");
      else
        printf("BM:  BC->RT\n");
      printf("BM:  RT(%d), SA(%d), WC(%d)\n", mbuf.command1.rtaddr, mbuf.command1.subaddr, mbuf.command1.wcount);
      printf("BM:  int_stat (0x%08x)\n", mbuf.int_status);

      BusTools_TimeGetString(&mbuf.time, outbuf);
      printf("BM:  time tag = %s\n\n", outbuf);
    }

    tail++;
    tail &= sIntFIFO->mask_index;
    sIntFIFO->tail_index = tail;
  }

  return API_SUCCESS;
}


BT_INT _stdcall rt_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO) {
  BT_INT status=0, tail=0, wcount=0, i=0, mc=0;
  BT_UINT messno=0, rt=0, sa=0, tr=0;
  API_RT_MBUF_READ rt_mbuf;
  char outbuf[100];

  tail = sIntFIFO->tail_index;
  while(tail != sIntFIFO->head_index) {
    if(sIntFIFO->FilterType == EVENT_RT_MESSAGE) {
      messno = sIntFIFO->fifo[tail].bufferID;
      rt = sIntFIFO->fifo[tail].rtaddress;
      sa = sIntFIFO->fifo[tail].subaddress;
      tr = sIntFIFO->fifo[tail].transrec;
      memset(&rt_mbuf, 0, sizeof(rt_mbuf));
      if((status = BusTools_RT_MessageRead(cardnum, rt, sa, tr, messno, &rt_mbuf)) != API_SUCCESS) {
        printf("Fail: BusTools_RT_MessageRead status (%d) = %d\n", cardnum, status);
        return status;
      }

      if(rt_mbuf.status & BT1553_INT_RT_RT_FORMAT)
        printf("RT:  RT<->RT\n");
      else if(rt_mbuf.status & BT1553_INT_MODE_CODE) {
        printf("RT:  Mode Code\n");
        mc = 1;
      }
      else if (rt_mbuf.mess_command.tran_rec)
        printf("RT:  RT->BC\n");
      else
        printf("RT:  BC->RT\n");

      printf("RT:  RT(%d), SA(%d), WC(%d), msg status(0x%04x), int status(0x%08x)\n", rt_mbuf.mess_command.rtaddr, rt_mbuf.mess_command.subaddr, rt_mbuf.mess_command.wcount, *((BT_U16BIT*)&rt_mbuf.mess_status), rt_mbuf.status);

      wcount = rt_mbuf.mess_command.wcount;
      if(mc) {
        if(wcount >= 16)
          wcount = 1;
      }
      else if(wcount == 0)
        wcount = 32;
      printf("RT:  DATA - ");
      for(i = 0; i < wcount; i++)
        printf("0x%04x ", rt_mbuf.mess_data[i]);
      printf("\n");

      BusTools_TimeGetString(&rt_mbuf.time, outbuf);
      printf("RT:  time tag = %s\n\n", outbuf);
    }

    tail++;
    tail &= sIntFIFO->mask_index;
    sIntFIFO->tail_index = tail;
  }

  return API_SUCCESS;
}


BT_INT _stdcall bc_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO) {
  API_BC_MBUF bcmessage;
  BT_INT i, bufnum=0, status=0, wcount=0, tail=0, rtrt=0, mc=0;
  BT_UINT messno=0;
  char outbuf[100];

  tail = sIntFIFO->tail_index;
  while(tail != sIntFIFO->head_index) {
    if(sIntFIFO->FilterType == EVENT_BC_MESSAGE) {
      messno = sIntFIFO->fifo[tail].bufferID;
      if((status = BusTools_BC_MessageRead(cardnum, messno, &bcmessage)) != API_SUCCESS) {
        printf("Fail: BusTools_BC_MessageRead status (%d) = %d\n", cardnum, status);
        return status;
      }
      mc=rtrt=0;
      if(bcmessage.status & BT1553_INT_RT_RT_FORMAT) {
        printf("BC:  RT<->RT\n");
        rtrt=1;
      }
      else if(bcmessage.status & BT1553_INT_MODE_CODE) {
        printf("BC:  Mode Code\n");
        mc=1;
      }
      else if(bcmessage.mess_command1.tran_rec)
        printf("BC:  RT->BC\n");
      else
        printf("BC:  BC->RT\n");

      printf("BC:  RT(%d), SA(%d), WC(%d)\n", bcmessage.mess_command1.rtaddr, bcmessage.mess_command1.subaddr, bcmessage.mess_command1.wcount);
      if(rtrt)
        printf("BC:  RT(%d), SA(%d), WC(%d)\n", bcmessage.mess_command2.rtaddr, bcmessage.mess_command2.subaddr, bcmessage.mess_command2.wcount);

      printf("BC:  status-1 (0x%04x)\n", *((BT_U16BIT*)&bcmessage.mess_status1));
      if(rtrt)
        printf("BC:  status-2 (0x%04x)\n", *((BT_U16BIT*)&bcmessage.mess_status2));

      printf("BC:  int_stat (0x%08x)\n", bcmessage.status);

      if(bcmessage.control & BC_CONTROL_BUFFERA)
        bufnum = 0;
      else
        bufnum = 1;

      wcount = bcmessage.mess_command1.wcount;
      if(wcount == 0)
        wcount = 32;
      if(mc) {
        if(wcount >= 16)
          wcount = 1;
      }

      printf("BC:  data - ");
      // if an error then display no data (xxxx)
      for(i = 0; i < wcount; i++) {
        if(i >= wcount)
          printf("    ");
        else if(bcmessage.mess_command1.tran_rec && (bcmessage.mess_status1.me || (bcmessage.status & BT1553_INT_NO_RESP)))
          printf("xxxx ");
        else
          printf("0x%04x ", bcmessage.data[bufnum][i]);
      }

      BusTools_TimeGetString(&bcmessage.time_tag, outbuf);
      printf("\nBC:  time tag = %s\n\n", outbuf);

      tail++;
      tail &= sIntFIFO->mask_index;
      sIntFIFO->tail_index = tail;
    }
  }

  return API_SUCCESS;
}


BT_INT setupThe_BM_IntFIFO(BT_UINT cardnum) {
  BT_INT rt=0, tr=0, sa=0, status=0;

  memset(&sIntFIFO_BM, 0, sizeof(sIntFIFO_BM));
  sIntFIFO_BM.function       = bm_intFunction;
  sIntFIFO_BM.iPriority      = THREAD_PRIORITY_CRITICAL;
  sIntFIFO_BM.dwMilliseconds = INFINITE;
  sIntFIFO_BM.FilterType     = EVENT_BM_MESSAGE;

  for(rt = 0; rt < 32; rt++) {
    for(tr = 0; tr < 2; tr++) {
      for(sa = 0; sa < 32; sa++)
        sIntFIFO_BM.FilterMask[rt][tr][sa] =  0xFFFFFFFF;
    }
  }

  if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_BM, 1)) != API_SUCCESS) {
    printf("Fail: BusTools_RegisterFunction(BM), cardnum = %d, status = %d\n", cardnum, status);
    return status;
  }

  return API_SUCCESS;
}


BT_INT setupThe_RT_IntFIFO(BT_UINT cardnum) {
  BT_INT rt=0, tr=0, sa=0, status=0;

  memset(&sIntFIFO_RT, 0, sizeof(sIntFIFO_RT));
  sIntFIFO_RT.function       = rt_intFunction;
  sIntFIFO_RT.iPriority      = THREAD_PRIORITY_CRITICAL;
  sIntFIFO_RT.dwMilliseconds = INFINITE;
  sIntFIFO_RT.FilterType     = EVENT_RT_MESSAGE;

  for(rt = 0; rt < 32; rt++) {
    for(tr = 0; tr < 2; tr++) {
      for(sa = 0; sa < 32; sa++)
        sIntFIFO_RT.FilterMask[rt][tr][sa] =  0xFFFFFFFF;
    }
  }

  if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_RT, 1)) != API_SUCCESS) {
    printf("Fail: BusTools_RegisterFunction(RT), cardnum = %d, status = %d\n", cardnum, status);
    return status;
  }

  return API_SUCCESS;
}


BT_INT setupThe_BC_IntFIFO(BT_UINT cardnum) {
  BT_INT rt=0, tr=0, sa=0, status=0;

  memset(&sIntFIFO_BC, 0, sizeof(sIntFIFO_BC));
  sIntFIFO_BC.function       = bc_intFunction;
  sIntFIFO_BC.iPriority      = THREAD_PRIORITY_CRITICAL;
  sIntFIFO_BC.dwMilliseconds = INFINITE;
  sIntFIFO_BC.FilterType     = EVENT_BC_MESSAGE;

  for(rt = 0; rt < 32; rt++) {
    for(tr = 0; tr < 2; tr++) {
      for(sa = 0; sa < 32; sa++)
        sIntFIFO_BC.FilterMask[rt][tr][sa] =  0xFFFFFFFF;
    }
  }

  if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_BC, 1)) != API_SUCCESS) {
    printf("Fail: BusTools_RegisterFunction(BC), cardnum = %d, status = %d\n", cardnum, status);
    return status;
  }

  return API_SUCCESS;
}


