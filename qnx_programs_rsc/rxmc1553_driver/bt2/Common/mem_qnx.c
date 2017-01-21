/*============================================================================*
 * FILE:                     mem_qnx . C
 *============================================================================*
 *
 * COPYRIGHT (C) 1997 - 2010 BY
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
 * FUNCTION:   BusTools API Linux interface functions.
 *
 *             This module provides the interface between BusTools
 *             and Condor Engineering's Linux PCI products.
 *
 *             This module performs the actual I/O and memory-mapping functions
 *             needed to support operation in a LINUX environment.
 *
 *             Since this file is used by multiple products (CEI-220 and BusAPI)
 *             use care when performing modifications.
 *
 * EXTERNAL ENTRY POINTS:
 *    vbtFreeBoardAddresses   Free any system resources which were obtained in
 *                            the mapping of the phyical board address.
 *
 *    vbtMapBoardAddresses    Looks up the physical base address of the board
 *                            in the registery then creates a pointer which is
 *                            valid in the current 32-bit environment and may
 *                            be used to directly access the memory-mapped RAM
 *                            on the board.
 *
 *    vbtFreeBoardAddress     Free any system resources which were obtained in
 *                            the mapping of the phyical board address.
 *
 *    vbtMapBoardAddress      Looks up the physical base address of the board
 *                            in the registery then creates a pointer which is
 *                            valid in the current 32-bit environment and may
 *                            be used to directly access the memory-mapped RAM
 *                            on the board.
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------

 */

/*---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <hw/inout.h>
#include "target_defines.h"
#include "lowlevel.h"
#define SYSTEM_ERROR 5
#define SYSTEM_OK 0
#define MAX_BTA 16
#define PCI_SET_REGION      0x13c61
#define PCI_GET_REGION_MEM  0x13c62
#define PCI_GET_REGION_SIZE 0x13c63
#define PCI_GET_INTERRUPT   0x13c64
#define PCI_SET_FUNCTION    0x13c65
#define PCI_SET_PID         0x13c67
#define PCI_GET_PCIREGION_BYTE  0x13c6a
#define PCI_GET_PCIREGION_WORD  0x13c6b
#define PCI_GET_PCIREGION_DWORD 0x13c6c
#define ISA_SET_IRQ 0x13c6a
#define BTD_ERR_BADADDRMAP 12
#define BTD_NO_SUPPORT 24
#define BTD_NO_DRV_MOD 30
#define BTD_IOCTL_DEV_ERR 31
#define BTD_IOCTL_SET_REG 32
#define BTD_IOCTL_REG_SIZE 33
#define BTD_IOCTL_GET_REG 34
#define BTD_BAD_SIZE 35
#define BTD_BAD_PROC_ID 36

#define MAX_MEMORY_REGIONS 6

#undef DEBUG 
#define QDEBUG 1
#define NO_DEBUG 0

#ifdef DEBUG
#define dprint(a,b) printf(a,b);
#else
#define dprint(a,b) 
#endif

#include <hw/pci.h>
#include <hw/pci_devices.h>
static struct pci_dev_info pciinf[8];
static int pcishdl;
static void* pcihdl[8];
static int  pci_init = 0;

typedef struct pciregion_state 
{
   int busNumber;
   int deviceNumber;
   int functionNumber;
   char	irq;                        /* Interrupt Request Level */
   unsigned long size[MAX_MEMORY_REGIONS];
   unsigned long membase[MAX_MEMORY_REGIONS];
   int bustype;                     /* ISA = 1 PCI = 2 */
   short CondorDeviceID;

} pciregion_state;

static pciregion_state pci_state[MAX_BTA];

static UINT cardNumberIndex[] = {0xf,0xf,0xf,0xf,   /* this is the card number table               */
                                 0xf,0xf,0xf,0xf,   /* Its indexed to return the device            */ 
                                 0xf,0xf,0xf,0xf,   /* number by card number                       */
                                 0xf,0xf,0xf,0xf};

static UINT cardID[]           = {0xf,0xf,0xf,0xf,   /* This is the cardID table                   */
                                  0xf,0xf,0xf,0xf,   /* it list the card type (ID)                 */
                                  0xf,0xf,0xf,0xf,   /* by cardnum.                                */
                                  0xf,0xf,0xf,0xf};

static UINT dev_chan[]         ={0x0,0x0,0x0,0x0,   /* this is the channel number table           */
                                 0x0,0x0,0x0,0x0,   /* it list the number of channels per device  */
                                 0x0,0x0,0x0,0x0,   /* its indexed by device number               */ 
                                 0x0,0x0,0x0,0x0};

static UINT dev_id[]          = {0xf,0xf,0xf,0xf,    /* This is the device table. it list the      */
                                 0xf,0xf,0xf,0xf,    /* device installed.  It indexed by device    */
                                 0xf,0xf,0xf,0xf,    /* number.                                    */
                                 0xf,0xf,0xf,0xf};	

static UINT QdevID[] = {0x110,0x120,0x160,0x170,0x180,0x0,0x210,0,0x230,0,0x260,0x250};
static UINT  devID[] = {0x40,0x80,0xa0,0x100};

int btype,nchan,ci; 
unsigned short hif[MAX_BTA];

UINT qnx_GetCardID(UINT cardnum )
{
   return cardID[cardnum];
}

UINT qnx_GetHIF(int dev)
{
   return hif[dev];
}

UINT qnx_GetDevChan(UINT device)
{
   return dev_chan[device];
}

UINT qnx_GetDevID(UINT device)
{
   return dev_id[device];
}

UINT qnx_GetDevIRQ(UINT device)
{
   return pci_state[device].irq;
}

UINT get_device(UINT cardnum)
{
   return cardNumberIndex[cardnum];
}

void get_cardID(UINT *cndx)
{
   int i;

   for(i=0;i<MAX_BTA;i++)
      cndx[i] = cardID[i];
}

void get_dev_chan(UINT *dchan)
{
   int i;
   for(i=0;i<MAX_BTA;i++)
      dchan[i] = dev_chan[i];
}

void get_dev_id(UINT *devid)
{
   int i;
   for(i=0;i<MAX_BTA;i++)
      devid[i] = dev_id[i];
}

CEI_INT mapCondorPCIAddress(CEI_INT debug) 
{
   int unit,mem_reg,mem_addr_reg=0,chndx=0;
   unsigned short host_interface;
   unsigned short * hbase;
 	
   if(pci_init)
      return(0);

   dprint("mapCondorPCIAddress debug = %d\n",debug);
   pcishdl = pci_attach(0);
   if(pcishdl == -1)
   {
      // Unable to find PCI Server
      return -1;
   }
   dprint("mapCondorPCIAddress Found PCI server %d\n",debug);

   for( unit = 0; unit < MAX_BTA; unit++ ) 
   {
      memset( &pciinf[unit], 0, sizeof( pciinf[unit] ) );
      /*
       * Scan for an installed PCI device of the proper type
       */
      pciinf[unit].VendorId = 0x13c6; // PCI vendor id of Condor Engineering
      pcihdl[unit] = pci_attach_device( NULL, PCI_SEARCH_VEND | PCI_INIT_ALL, unit, &pciinf[unit] );
      if( pcihdl[unit] == NULL ) 
      {
         // Unable to locate a 1553 PCI adapter
         if(unit == 0)
         {
            dprint("mapCondorPCIAddres pcihdl[%d] == NULL\n",unit);
            pci_detach( pcishdl );
            return -1;
         }
         break;
      }
      dprint("mapCondorPCIAddres found device %d\n",unit);
      pci_state[unit].busNumber = pciinf[unit].BusNumber;
      pci_state[unit].deviceNumber = pciinf[unit].DeviceId;
      pci_state[unit].CondorDeviceID = pciinf[unit].DeviceId;
      pci_state[unit].functionNumber = pciinf[unit].DevFunc;
      pci_state[unit].bustype = 2;
      dprint("Device ID = %x\n",pci_state[unit].CondorDeviceID);
      for(mem_reg = 0;mem_reg<MAX_MEMORY_REGIONS; mem_reg++)
      {
         dprint("mapCondorPCIAddress region = %d\n",mem_reg);
         pci_state[unit].membase[mem_reg] = pciinf[unit].CpuBaseAddress[mem_reg];
         dprint("mapCondorPCIAddress mem_base = %x\n",pciinf[unit].CpuBaseAddress[mem_reg]);
         pci_state[unit].size[mem_reg] = pciinf[unit].BaseAddressSize[mem_reg];
         dprint("mapCondorPCIAddress Size = %x\n",pci_state[unit].size[mem_reg]);
         if (pci_state[unit].size[mem_reg] >= 0x800000)
         {
            dprint("mapCondorPCIAddress memory region = %d\n",mem_reg);
            mem_addr_reg = mem_reg;
         }
      }
      pci_state[unit].irq = pciinf[unit].Irq;
      dprint("mapCondorPCIAddress IRQ = %x\n",pci_state[unit].irq);

      hbase = (unsigned short *)mmap_device_memory(NULL,2, PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, pci_state[unit].membase[mem_addr_reg]);
      hif[unit] =hbase[0];
      munmap(hbase,2);
      host_interface = hif[unit];
      dprint("mapCondorPCIAddress host interface = %x\n", hif[unit]);
      if((host_interface & 0x000f) != 0)
      {
         dprint("1553 Qboard host interface = %x\n",host_interface);
         btype = (host_interface & 0x3e)>>1;
         nchan = (host_interface & 0x7c0)>>6;
         dev_chan[unit]=nchan;
         dev_id[unit] = QdevID[btype-1];
         if(pci_state[unit].CondorDeviceID == 0x1556) 
            dev_id[unit] = 0x220;

         for(ci=0;ci<nchan;ci++)
         {
            cardID[chndx] = dev_id[unit];
            cardNumberIndex[chndx] = unit;
            chndx++;
         }
         dprint("btype = %x\n",btype);
         dprint("nchan = %d\n",nchan);
         dprint("dev_id = %x\n", dev_id[unit]);
      } 
      else
      {
         dprint("1553 Board host interface = %x\n",host_interface);
         btype = (host_interface & 0xc000)>>14;
         nchan = (host_interface & 0x3000)>>12;
         if(nchan > 1)
            nchan = 2;
         dev_chan[unit]=nchan;
         dev_id[unit] = devID[btype];
         for(ci=0;ci<nchan;ci++)
         {
            cardID[chndx] = devID[btype];
            cardNumberIndex[chndx] = unit;
            chndx++;
         }
         dprint("btype = %x\n",btype);
         dprint("nchan = %d\n",nchan);
         dprint("dev_id = %x\n", dev_id[unit]);
      }
   }
	
   pci_init = 1;
   return(0);
}
	

/*---------------------------------------------------------------------------*
 *          LOCAL ROUTINES, NOT REFERENCED OUTSIDE OF THIS MODULE
 *---------------------------------------------------------------------------*/

unsigned memorySize;

/****************************************************************
*
*  PROCEDURE - vbtMapBoardAddress(DWORD base_address,
*                                 DWORD map_length, 
*                                 char  ** addr,
*                                 DWORD *junk,
*                                 int   junk2)
*
*  PARAMETERS
*     BT_U32BIT  base_address      - Card physical base address.
*     BT_U32BIT  map_length        - Size of region to map in bytes.
*     char       ** addr           - Returned address of board.
*     DWORD      *junk             - not used (compatibility)
*     int        junk2             - not used (compatibility)
*
*  RETURNS
*     BTD_OK          - Mapped address with no error
****************************************************************/
int vbtMapBoardAddress(unsigned  cardNum,
                       unsigned  map_len,
                       char   **addr,
		               void  *junk,
                       int    junk2)
{
	unsigned long membase;
	unsigned long size;
	mapCondorPCIAddress(QDEBUG);
	size = pciinf[cardNum].BaseAddressSize[0];
	membase = pciinf[cardNum].CpuBaseAddress[0];
	if(size > 0) {
		addr[0] = (char *) mmap_device_memory(NULL, size, PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, membase);
		return SYSTEM_OK;
		}
	return BTD_BAD_SIZE;
}

/****************************************************************************
*
*  PROCEDURE - vbtMapBoardAddresses
*
*  FUNCTION
*     This routine looks up the physical base addresses of the board in ioctl, 
*     then creates pointers which are valid in the current
*     32-bit environment and may be used to directly access the memory-mapped
*     RAM on the board.  
*     Only memory regions specified by setting "memMapToHost" are mapped.  If
*     any such regions are mapped, call vbtFreeBoardAddress() to free the region
*     maps and close the handle.
*     If no regions were specified to be mapped, the handle is closed and it is
*     not necessary to call vbtFreeBoardAddress().
*
*  PARAMETERS
*     See below.
*
*  RETURNS
*     BTD_OK          - Mapped address with no error
*     SYSTEM_ERROR    - any errors
****************************************************************************/
int vbtMapBoardAddresses( unsigned device, // Device to open (0 - 9)
                          PDEVMAP_T pMap)   // Pointer to region mapping structure
{

   unsigned            lpbase;        // Region base address in host memory.
   int                 status = 0;    // Returned status.
   int                 RegionsMapped; // Number of regions mapped during this call.
   int                 bustype;
   unsigned            i;             // Loop counter.
   int                 region_count;
   int                 board_regions;
   unsigned long       membase;
   unsigned long       size;

   if(pMap->use_count>0)
   {
      pMap->use_count++;
      return 0;
   }

   mapCondorPCIAddress(QDEBUG);
   pMap->busType = pci_state[device].bustype;  /* set bus type 1=isa 2=pci */
    
   // Extract all of the memory region addresses and sizes, and map
   //  those regions requested by the caller.
   RegionsMapped  = 0;                 // No regions have been mapped.

   // Clear the found region_count and struct
   region_count=0;
   for ( i = 0; i < MAX_MEMORY; i++ )
   {
      pMap->memStartPhy[i]    = 0;    // Physical address is null.
      pMap->memLengthBytes[i] = 0;    // Length is null.
      pMap->memHostBase[i]    = NULL; // Host address is null.
      pMap->memSections       = 0;
   }

// find the used memory regions they are not necessarily contiguous
   for ( i = 0; i < MAX_MEMORY; i++ )
   {
      size = pci_state[device].size[i];      
      if (size > 0)
      {
         // Get region memory base address.
         membase = pci_state[device].membase[i];
         pMap->memStartPhy[region_count]    = membase;
         pMap->memLengthBytes[region_count] = size;
         // If the caller wants this region mapped, map it.
         status = BTD_OK;

         lpbase = (unsigned)mmap_device_memory(NULL, size, PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, membase);
         if(lpbase == -1)
         {
            status = BTD_ERR_BADADDRMAP + 5000;
         }
         if ( status == BTD_OK )
         {  // Successfully mapped region.
            RegionsMapped++;
            pMap->flagMapToHost[region_count] = 1;
            lpbase |= pMap->memStartPhy[region_count] & 0x00000FFF;
            pMap->memHostBase[region_count] = (char *)lpbase;
         }
         else
         {  // Region did not map.
            pMap->flagMapToHost[region_count] = 0;
            pMap->memHostBase[region_count] = NULL;
            break;
         }
         region_count++;
      }
   }
   pMap->memSections = RegionsMapped;
   pMap->use_count++;
   return status;
}

/****************************************************************
*
*  PROCEDURE - vbtFreeBoardAddresses
*
*  FUNCTION
*     This routine frees any system resources which were obtained
*     in the mapping of the phyical board address.
*
*  PARAMETERS
*     See below.
*
*  RETURNS
*     Tothing
****************************************************************/
void vbtFreeBoardAddresses(PDEVMAP_T pMap)
{
   int i;

   if(--pMap->use_count != 0) // Do not unmap it there are other users
      return;

   for(i=0;i<6;i++)
   {
      if(pMap->flagMapToHost[i])
      {
         munmap(pMap->memHostBase[i],pMap->memLengthBytes[i]);
         pMap->flagMapToHost[i] = 0;
      }
   }    
}

/****************************************************************
*
*  PROCEDURE - vbtFreeBoardAddress
*
*  FUNCTION
*     This routine frees any system resources which were obtained
*     in the mapping of the phyical board address.
*
*  PARAMETERS
*     See below.
*
*  RETURNS
*     Tothing
****************************************************************/
void vbtFreeBoardAddress(UINT cardnum)
{
 
}
 
/****************************************************************
*
*  PROCEDURE - vbtOpen1553Channel
*
*  FUNCTION
*     This function open a 1553 Channel and return a handle to 
*     the channel.
*
*  PARAMETERS
*     BT_UNIT  *chnd   -- Handle to the channel use for cardnum.
*     BT_UINT  mode    -- Operation mode
*     BT_INT   devid   -- device ID
*     BT_UINT  channel -- Channel
*
*  RETURNS
*     BTD_OK
****************************************************************/

int vbtOpen1553Channel( UINT   *chnd,       /* (o) card number (0 - 12) (device number in 32-bit land) */
                        UINT   mode,        /* (i) operational mode                                    */
                        int    devid,       /* (i) device ID                                           */
                        UINT   channel)     /* (i) channel                                             */
{
   int chndx, card_cnt;

   card_cnt=0;

   if((devid < 0) || (devid > MAX_BTA))
      return BTD_BAD_DEVICE_ID;
   
   if(dev_chan[devid] == 0) 
      return BTD_BAD_DEVICE_ID;
   
   if(channel > dev_chan[devid] - 1)
      return BTD_CHAN_NOT_PRESENT;

   for(chndx=0;chndx<devid;chndx++)
   {
      card_cnt += dev_chan[chndx];
   }
   card_cnt+=channel;
   *chnd=card_cnt;

   return BTD_OK;      
}

/****************************************************************************
*
*  PROCEDURE - vbtGetPCIConfigRegister 
*
*  FUNCTION
*       This function reads the dat in a PCI configuration register.
*
*  PARAMETERS
*       cardnum - Card number
*       offset  - Offset into the configuration register
*       length  - length in bytes of data 1, 2, or 4
*       value   - returned data value
*
*  RETURNS
*       status.
*
****************************************************************************/
int vbtGetPCIConfigRegister(UINT cardnum,
                            UINT offset,
                            UINT length,
                            UINT *value)
{
	mapCondorPCIAddress(NO_DEBUG);
	switch(length) {
	case 1: {
		uint8_t data;
		if( pci_read_config( pcihdl[cardnum], offset, 1, sizeof(data), &data ) != 0)
            return BTD_IOCTL_GET_REG + 2000;
		*value = (UINT) data;
		break;
		}
	case 2: {
		uint16_t data;
		if( pci_read_config( pcihdl[cardnum], offset, 1, sizeof(data), &data ) != 0)
            return BTD_IOCTL_GET_REG + 3000;
		*value = (UINT) data;
		break;
		}
	case 4:
	default: {
		uint32_t data;
		if( pci_read_config( pcihdl[cardnum], offset, 1, sizeof(data), &data ) != 0)
            return BTD_IOCTL_GET_REG + 4000;
		*value = (UINT) data;
		break;
		}
		} /* switch */
    return BTD_OK;

}

/****************************************************************************
*
*  PROCEDURE - vbtGetPCIDeviceID
*
*  FUNCTION
*       This function return the device ID from the PCI Configuration register.
*
*  PARAMETERS
*       cardnum    - Card number
*       ceiDevID   - returned data value
*
*  RETURNS
*       status.
*
****************************************************************************/
CEI_INT32 vbtGetPCIDeviceID(CEI_UINT32 cardnum, CEI_UINT32 *ceiDevID)
{
	uint16_t data;
	mapCondorPCIAddress(QDEBUG);
	if( pci_read_config( pcihdl[cardnum], 2, 1, sizeof(data), &data ) != 0)
        return BTD_IOCTL_DEV_ERR;
	*ceiDevID = (CEI_UINT) data;
	return BTD_OK;
}

int vbtGetInterrupt(unsigned int cardnum)
{
   	mapCondorPCIAddress(NO_DEBUG);
	return( pciinf[cardnum].Irq );
}

