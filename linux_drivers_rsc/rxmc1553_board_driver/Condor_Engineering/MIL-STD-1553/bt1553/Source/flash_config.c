 /*============================================================================*
 * FILE:                        F L A S H _ C O N F I G . C
 *============================================================================*
 *
 * COPYRIGHT (C) 2010 - 2012 BY
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
 *             This file contains the configuration function for the FLASH and
 *             PLX.
 *
 * USER ENTRY POINTS: 
 *
 *
 *===========================================================================*/

/* $Revision:  8.02 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  12/03/2010   Initial release.
  06/06/2011   Add support for RXMC2-1553 and LPCIe-1553

*/

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#define _GLOBALS_H_

#include <stdio.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"


#define FLASH_WORD_ADDR_LSB  0xa1 //142
#define FLASH_WORD_ADDR_MSB  0xa2 //144
#define FLASH_DATA_WRITE_REG 0xa3 //146
#define FLASH_DATA_READ_REG  0xa4 //148
#define READ_MODE 0xff
#define STATUS_REG 0x70
#define ERASE_SETUP 0x20
#define ERASE_CONFIRM 0xd0
#define WRITE_MODE 0x40
#define DATA_BLOCK_0 0x2000
#define DATA_BLOCK_1 0x3000
#define DATA_BLOCK_2 0x4000
#define DATA_BLOCK_3 0x5000
#define DATA_BLOCK_4 0x6000

#define INVERTBITS(b)   (~(b))
#define REVERSEBITS(b)  (BitReverseTable[b])

static CEI_UCHAR BitReverseTable[256] =
{
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_ReadFlashConfigData
*
*  FUNCTION
*     The function returns the contents of flash Config registers.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_ReadFlashConfigData(BT_UINT cardnum, BT_U16BIT * fdata)
{
   BT_INT status;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   status = vbtReadFlashData(cardnum, fdata);
   return status;
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_1760_DataWrite
*
*  FUNCTION
*     The function writes data into the flash for 1760 start message data
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_1760_DataWrite(BT_UINT cardnum, BT_U16BIT **saData, BT_U32BIT saEnable)
{
   BT_U16BIT dbuf,rdata;
   static int pindx;
   BT_UINT rindx,sa_indx;
   BT_U16BIT edata[2];

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      //some short delay???
      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);

   //Place in erase setup mode
   dbuf = ERASE_SETUP;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   //Write erase block  address
   dbuf = DATA_BLOCK_0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);
   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);  

   //Erase the block
   dbuf = ERASE_CONFIRM;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      //some short delay???
      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);


   pindx = 0x0;

   //now write the data
   for(sa_indx=0; sa_indx<32; sa_indx++)
   {
      for(rindx=0; rindx<32; rindx++)
      { 
         //Place in write mode by writing data 0x40 to FLASH_DATA_REG
         dbuf = WRITE_MODE;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
    
         dbuf = DATA_BLOCK_0 + pindx++;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

         dbuf = 0x0;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

         dbuf = saData[sa_indx][rindx];
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

         do{
            //Place in read status mode
            dbuf = STATUS_REG;
            vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

            //some short delay???
            vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
         } while ((rdata & 0x80) == 0);
      }
   }

   edata[0] = (BT_U16BIT)(saEnable & 0xffff);
   edata[1] = (BT_U16BIT)((saEnable & 0xffff0000)>>16);

   //write out the SA enable register. 
   for(rindx = 0; rindx<2; rindx++)
   {  
      dbuf = WRITE_MODE;  //Place in write mode by writing data 0x40 to FLASH_DATA_REG
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      dbuf = DATA_BLOCK_0 + 0x400 + rindx;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);
      dbuf = 0x0;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
      dbuf = edata[rindx];
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      do{
         //Place in read status mode
         dbuf = STATUS_REG;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

         //some short delay???
         vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
      } while ((rdata & 0x80) == 0);
   }

   return API_SUCCESS;
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_FlashWrite
*
*  FUNCTION
*     The function writes data into the flash data block 0 only
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FlashWrite(BT_UINT cardnum, BT_INT paddr, BT_INT wcount, BT_U16BIT *saData )
{
   BT_U16BIT dbuf,rdata;
   static int rindx;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      //some short delay???
      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);

   //Place in erase setup mode
   dbuf = ERASE_SETUP;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   //Write erase block  address
   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);
   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);  

   //Erase the block
   dbuf = ERASE_CONFIRM;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);
 
   for(rindx=0; rindx<wcount; rindx++)
   { 

      //Place in write mode by writing data 0x40 to FLASH_DATA_REG
      dbuf = WRITE_MODE;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
    
      dbuf = 0 + paddr++;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

      dbuf = 0x0;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

      dbuf = saData[rindx];
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

      do{
         //Place in read status mode
         dbuf = STATUS_REG;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

         vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
      } while ((rdata & 0x80) == 0);
   }

   return API_SUCCESS;
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_1760_DataRead
*
*  FUNCTION
*     The function reads data into the flash for 1760 start message data
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_1760_DataRead(BT_UINT cardnum, BT_U32BIT **saData, BT_U32BIT *saEnable)
{
   BT_U16BIT dbuf,ebuf1,ebuf2,rdata;
   BT_UINT rindx,sa_indx,pindx=0;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   //Place in read status mode
   dbuf = STATUS_REG;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
   //some short delay???
   vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);

   //Place in read mode by writing data 0xff to FLASH_DATA_REG
   dbuf = READ_MODE;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   for(sa_indx=0; sa_indx<32; sa_indx++)
   {
      for(rindx=0; rindx<32; rindx++)
      {    
         dbuf = DATA_BLOCK_0 + pindx++;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

         dbuf = 0x0;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

         dbuf = 0x0;
         vbtReadHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_READ_REG*2,2);
         saData[sa_indx][rindx] = dbuf;
      }
   }

   //read enable LSB
   dbuf = DATA_BLOCK_0 + 0x400;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
   vbtReadHIF(cardnum,(LPSTR)&ebuf1,FLASH_DATA_READ_REG*2,2);

   //read enable MSB
   dbuf = DATA_BLOCK_0 + 0x401;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
   vbtReadHIF(cardnum,(LPSTR)&ebuf2,FLASH_DATA_READ_REG*2,2);
   
   *saEnable = (BT_U32BIT)((BT_U32BIT)ebuf1 | (BT_U32BIT)(ebuf2 << 16));

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_FlashRead
*
*  FUNCTION
*     The function reads data in the flash data block 0 only
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FlashRead(BT_UINT cardnum, BT_INT paddr, BT_INT wcount, BT_U16BIT *saData )
{
   BT_U16BIT dbuf,rdata;
   BT_UINT pindx=0; 
   BT_INT rindx;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   //Place in read status mode
   dbuf = STATUS_REG;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
   vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);

   //Place in read mode by writing data 0xff to FLASH_DATA_REG
   dbuf = READ_MODE;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   for(rindx=0; rindx<wcount; rindx++)
   {    
      dbuf =  pindx++;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

      dbuf = 0x0;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

      dbuf = 0x0;
      vbtReadHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_READ_REG*2,2);
      saData[rindx] = dbuf;
   }

   return API_SUCCESS;
}

NOMANGLE BT_INT CCONV BusTools_LockFlash(cardnum)
{
   BT_INT status;
   BT_U16BIT rdata[2];

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

#define LOCK_BIT 0x200

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;   

   if(CurrentCardType[cardnum] != QVME1553)
      return API_HARDWARE_NOSUPPORT;

   //Read the CSC and ACR
   status = BusTools_FlashRead(cardnum, 0x0, 2, rdata );
   if(status==API_SUCCESS)
   {
      printf("BusTools_LockFlash CSC value = %x\n",rdata[0]);
      printf("BusTools_LockFlash ACR value = %x\n",rdata[1]);

      rdata[1] &= ~(LOCK_BIT);

      status = BusTools_FlashWrite(cardnum, 0x0, 2, rdata );
   }
   return API_SUCCESS;

}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t R e a d S e r i a l N u m b e r
 *===========================================================================*
 *
 * FUNCTION:    reads the serial number on selected boards.
 *
 * DESCRIPTION: This functions read the serial number from flash memory.
 *
 *      It will return:
 *              .API_SUCCESS
 *===========================================================================*/
BT_INT vbtReadSerialNumber(BT_UINT cardnum, BT_U16BIT *serial_number)
{
   BT_U16BIT tdata;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(CurrentCardType[cardnum] == R15EC  || CurrentCardType[cardnum] == RXMC1553 )
   {
      BT_INT time_count=0;
      BT_U8BIT sn_lsb, sn_msb;

      //Read SN LSB

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x142/2] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x144/2] = flipws(0x1e);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = flipws(0x8);

      while(1)
      {
         tdata = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x140];
         if((tdata & 0x8)==0)
            break;
         MSDELAY(1);
         time_count++;
         if(time_count > 5)
            return API_TIMER_ERR;         
      }
      sn_lsb = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x148];
      
      //Read SN MSB
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x142/2] = flipws(0x1);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x144/2] = flipws(0x1e);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = flipws(0x8);

      while(1)
      {
         tdata = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x140];
         if((tdata & 0x8)==0)
            break;
         MSDELAY(1);
         time_count++;
         if(time_count > 5)
            return API_TIMER_ERR;         
      }
      sn_msb = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x148];     
   
      sn_lsb = REVERSEBITS(sn_lsb);
      sn_msb = REVERSEBITS(sn_msb);

      tdata = (CEI_UINT16)sn_lsb | (CEI_UINT16)(sn_msb << 8);
   }
   else if(CurrentCardType[cardnum] == AR15VPX || 
	       CurrentCardType[cardnum] == R15AMC  || 
		   CurrentCardType[cardnum] == RPCIe1553)
   {
       BT_U32BIT indx,addr32,rdata32[4];
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32[0] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32[0] & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);

       for(indx = 0;indx<2;indx++)
       {
          addr32 = 0x80000000 + 0x520000 + indx;
          ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
          MSDELAY(1);
          
          rdata32[indx] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
          rdata32[indx] = little_endian_32(rdata32[indx]);
          rdata32[indx] = REVERSEBITS(rdata32[indx] & 0xff);
       }
#ifdef NON_INTEL_WORD_ORDER
       tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[1]&0xff) + (((BT_U16BIT)rdata32[0]&0xff)<<8));
#else
       tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[0]&0xff) + (((BT_U16BIT)rdata32[1]&0xff)<<8));
#endif
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   } 
   else if(CurrentCardType[cardnum] == R15XMC2)
   {
       BT_U32BIT addr32,rdata32;
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32 = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32 & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);

       addr32 = 0x80000000 + 0x290000;
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
       MSDELAY(1);
          
       rdata32 = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
       rdata32 = little_endian_32(rdata32);
       tdata = (BT_U16BIT)flipws(rdata32);

       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   } 
   else if(CurrentCardType[cardnum] == LPCIE1553)
   {
       BT_U8BIT sn_data[2];
       BT_U16BIT rdata;

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0; 
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x203;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x21e;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
      rdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2];
      sn_data[0] = (BT_U8BIT)(rdata & 0xff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x203;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x21e;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x201; 
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
      rdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2];
      sn_data[1] = (BT_U8BIT)(rdata & 0xff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0;
      
      tdata = (CEI_UINT16)sn_data[0] | (CEI_UINT16)(sn_data[1] << 8);
      
   }
   else
   {
      //clear status
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = flipws(0x50);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata  = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      //read array
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = (BT_U16BIT)flipws(0xff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x140a);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x0838);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3];
      flipw(&tdata); // for PMC on PowerPC
   }
   *serial_number = tdata;
   return API_SUCCESS;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t R e a d F l a s h D a t a
 *===========================================================================*
 *
 * FUNCTION:    Reads all flash config data.
 *
 * DESCRIPTION: This functions read the serial number from flash memory.
 *
 *      It will return:
 *              .API_SUCCESS
 *===========================================================================*/
BT_INT vbtReadFlashData(BT_UINT cardnum, BT_U16BIT *fdata)
{
   BT_INT findx;

   BT_U16BIT tdata;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;


   if(CurrentCardType[cardnum] == R15EC  || CurrentCardType[cardnum] == RXMC1553 )
   {
      BT_INT time_count;

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x144/2] = flipws(0x001f);
      for(findx=0; findx<64; findx++)
      {
         //Read SN LSB
         time_count=0;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x142/2] = flipws(findx);
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = flipws(0x8);

         while(1)
         {
            tdata = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x140];
            if((tdata & 0x8) == 0)
               break;
            MSDELAY(1);
            time_count++;
            if(time_count > 5)
               return API_TIMER_ERR;         
         }
         ((BT_U8BIT *)fdata)[findx] = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x148];
      }
   }
   else if(CurrentCardType[cardnum] == AR15VPX || CurrentCardType[cardnum] == R15AMC || CurrentCardType[cardnum] == RPCIe1553)
   {
       BT_U32BIT indx,addr32,rdata32[4];
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32[0] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32[0] & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);
       for(findx=0; findx < 32; findx++)
       {
          for(indx = 0;indx<2;indx++)
          {
             addr32 = 0x80000000 + 0x5e0000 + findx*2 + indx;
             ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
             MSDELAY(1);
          
             rdata32[indx] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
             rdata32[indx] = little_endian_32(rdata32[indx]);
             rdata32[indx] = REVERSEBITS(rdata32[indx] & 0xff);
          }
#ifdef NON_INTEL_WORD_ORDER
          tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[1]&0xff) + (((BT_U16BIT)rdata32[0]&0xff)<<8));
#else
          tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[0]&0xff) + (((BT_U16BIT)rdata32[1]&0xff)<<8));
#endif
          fdata[findx]=tdata;
      }
      ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   }
   else if(CurrentCardType[cardnum] == R15XMC2)
   {
       BT_U32BIT addr32,rdata32;
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32 = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32 & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);
       for(findx=0; findx < 32; findx++)
       {
          addr32 = 0x80000000 + 0x2f0000 + findx;
          ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
          MSDELAY(1);
          
          rdata32 = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
          rdata32 = little_endian_32(rdata32);
          tdata = (BT_U16BIT)(rdata32 & 0x0000ffff); 
          fdata[findx]= flipws(tdata);
      }
      ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   }
   else if(CurrentCardType[cardnum] == LPCIE1553)
   {
       BT_U8BIT sn_data[2];
       BT_U16BIT rdata;
  
      for(findx=0;findx<32;findx+=2)
      {
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0; 
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x203;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x21f;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200 + findx; 
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
         rdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140];
         sn_data[0] = (BT_U8BIT)(rdata & 0xff);

         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x203;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x21f;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200 + findx + 1; 
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
         rdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140];
         sn_data[1] = (BT_U8BIT)(rdata & 0xff);
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0;
      
         fdata[findx] = (CEI_UINT16)sn_data[0] | (CEI_UINT16)(sn_data[1] << 8);
     }
      
   }
   else
   {

      //clear status
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = flipws(0x50);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata  = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      //read array
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = (BT_U16BIT)flipws(0x00ff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      for(findx = 0; findx<32; findx++)
      {
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws((0x1406 + findx));
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x0839);
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
         do{
            tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
            flipw(&tdata); // for PMC on PowerPC
         }while(tdata != 1);

         fdata[findx] = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3];
         flipw(&fdata[findx]); // for PMC on PowerPC
      }
   }
   return API_SUCCESS;
}


/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t S e t P L X R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into PLX registers.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtSetPLXRegister(BT_UINT   cardnum,      // (i) card number
                       BT_U32BIT regnum,     // (i) host buffer (source)
                       BT_U16BIT regval)   // (o) byte offset within adapter (destination))
{
   *(BT_U16BIT *)(bt_iobase[cardnum]+regnum) = flipws(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t S e t P L X R e g i s t e r 3 2
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into PLX Register.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtSetPLXRegister32(BT_UINT   cardnum,  // (i) card number
                       BT_U32BIT regnum,     // (i) host buffer (source)
                       BT_U32BIT regval)     // (o) byte offset within adapter (destination))
{
   *(BT_U32BIT *)(bt_iobase[cardnum] + regnum) = flips(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t S e t P L X R e g i s t e r 8
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into PLX Register.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtSetPLXRegister8(BT_UINT   cardnum,  // (i) card number
                       BT_U32BIT regnum,     // (i) host buffer (source)
                       BT_U8BIT regval)     // (o) PLX byte offset
{
   *(BT_U8BIT *)(bt_iobase[cardnum] + regnum) = regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t P L X R e g i s t e r 8
 *===========================================================================*
 *
 * FUNCTION:    Return data from PLX Register.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

BT_U8BIT vbtGetPLXRegister8(BT_UINT   cardnum,    // (i) card number
                            BT_U32BIT regnum)     // (i) PLX register address

{
   return( *(BT_U8BIT *)(bt_iobase[cardnum] + regnum));
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t P L X R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

BT_U16BIT vbtGetPLXRegister(
   BT_UINT   cardnum,      // (i) card number
   BT_U32BIT byteOffset)   // (o) byte offset within adapter (destination))
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   regval = *(BT_U16BIT *)(bt_iobase[cardnum]+byteOffset);
   flipw(&regval);
   return regval;
}

#ifdef PLX_DEBUG
/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_PLXWrite
*
*  FUNCTION
*       This procedure reads the specified block of memory and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_PLXWrite(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr,          // (i) BYTE address in BT hardware to begin reading
   BT_U16BIT rData)         // (o) Pointer to user's data
{
   /*******************************************************************
   *  Do error checking
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if((CurrentCardType[cardnum] != QPCI1553) &&
      (CurrentCardType[cardnum] != QPCX1553) &&
      (CurrentCardType[cardnum] != QCP1553) ) 
	  return API_HARDWARE_NOSUPPORT;

   /*******************************************************************
   *  Read specified memory area, anywhere on the card. 
   *******************************************************************/
   if (addr & 0x0001)
      return API_BUSTOOLS_EVENBCOUNT;

   if(addr >= 0x80)
	   return BTD_ERR_BADADDR;

   vbtSetPLXRegister(cardnum, addr,rData);

   return API_SUCCESS;
}


/****************************************************************************
*
*  PROCEDURE NAME -   BusTools_PLXRead
*
*  FUNCTION
*       This procedure reads the specified block of memory and
*       returns it to the caller.
*
*  RETURNS
*       API_SUCCESS -> success
*       API_BUSTOOLS_NOTINITED -> bustools not inited for this card
*       API_BUSTOOLS_BADCARDNUM -> illegal card number
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_PLXRead(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr,          // (i) BYTE address in BT hardware to begin reading
   BT_U16BIT *rData)         // (o) Pointer to user's data
{
   /*******************************************************************
   *  Do error checking
   *******************************************************************/
   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if((CurrentCardType[cardnum] != QPCI1553) &&
      (CurrentCardType[cardnum] != QPCX1553) && 
      (CurrentCardType[cardnum] != QCP1553))
	  return API_HARDWARE_NOSUPPORT;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;


   /*******************************************************************
   *  Read specified memory area, anywhere on the card.  "buff" can
   *  only point to a 64K buffer in 16-bit land.
   *******************************************************************/
   if (addr & 0x0001)
      return API_BUSTOOLS_EVENBCOUNT;

   if(addr >= 0x80)
	   return BTD_ERR_BADADDR;

   *rData = vbtGetPLXRegister(cardnum, addr);

   return API_SUCCESS;
}

NOMANGLE BT_INT CCONV BusTools_PLX_View(cardnum)
{
   BT_U16BIT pData;
   BT_U32BIT addr;
   char cmd[2];
   char saddr[16],sdata[16];

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if((CurrentCardType[cardnum] != QPCI1553) &&
      (CurrentCardType[cardnum] != QPCX1553) &&
      (CurrentCardType[cardnum] != QCP1553))
      return API_HARDWARE_NOSUPPORT;
     
   printf("Enter w xxxx yy for write; xxxx is addr and yy is the data\n");
   printf("Enter r xxxx nn for read;  xxxx is addr and nn is the word cnt\n");

   do{
      scanf("%s",cmd);
	  if(strcmp(cmd,"q")==0)
         break;
	  scanf("%s%s",saddr,sdata);
	  addr = strtol(saddr,(char**)NULL,16);
      pData = (BT_U16BIT)strtol(sdata,(char **)NULL,16);

	  printf("cmd = %s addr = %lx data/cnt = %x\n",cmd,addr,pData);

	  if(strcmp(cmd,"w")==0)
	  {
         BusTools_PLXWrite(cardnum,addr,pData);
	  }
	  else if(strcmp(cmd,"r")==0)
	  {
		 int i, cnt;
		 cnt = pData;

         for(i=0;i<cnt;i++)
		 {
		    BusTools_PLXRead(cardnum,addr,&pData);
		    printf("%lx - %8x\n",addr,pData);
			addr+=2;
		 }
	  }
   }while(cmd != "q" || cmd != "Q");
   return API_SUCCESS;
}


NOMANGLE BT_INT CCONV BusTools_PLX_Dump(cardnum)
{
   int i;
   BT_U32BIT * plx_reg;

   if( (CurrentCardType[cardnum] != QPCI1553) &&
       (CurrentCardType[cardnum] != QPCX1553) &&
       (CurrentCardType[cardnum] != QCP1553) )
      return 9876;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

       
   plx_reg = (BT_U32BIT *)bt_iobase[cardnum];
   for(i=0;i<200;i++)
      printf("PLX REG[%x] = %lx\n",i,plx_reg[i]);

   return 0;
}

#endif //PLX_DEBUG
