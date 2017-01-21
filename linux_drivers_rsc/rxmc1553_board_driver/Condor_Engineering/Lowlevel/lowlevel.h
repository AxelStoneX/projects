/*============================================================================*
 * FILE:                      L O W L E V E L . H
 *============================================================================*
 *
 * COPYRIGHT (C) 1997 - 2012 BY
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
 *===========================================================================*
 *
 * FUNCTION:    Header file for low level device driver access.  This file
 *              defines the error codes returned from the memory mapping
 *              functions.
 *
 *===========================================================================*/

/* $Revision:  5.21 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  12/23/1997   Created file from busapi.h to support CEI-220 program loader.ajh.
  01/05/1998   Added function prototypes to make this a reusable component.
  12/30/1999   Added vbtGetPCIRevision() function.V3.30.ajh
  01/18/2000   Added support for the ISA-1553.V4.00.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  10/03/2001   Modify for improved initialization. V4.43 rhc
  01/07/2002   Added support for Quad-PMC and Dual Channel IP V4.46 rhc
  02/15/2002   Added support for modular API. v4.48
  02/19/2004   Add support for BusTools_API_OpenChannel
  02/19/2004   Add support for mapping devices only once
  04/06/2005   modified for new Linux lowlevel. bch
  01/19/2006   modified for 64-bit support. bch
  02/20/2006   added common data types. bch
  05/25/2006   added vbtConfigInterrupt. bch
  06/15/2007   added vbtInterruptMode. added vbtWaitForInterrupt. modified
                vbtGetInterrupt. bch
  08/07/2007   added CEI_TYPES_H check. bch 
  11/18/2008   added "cei_types.h" and removed duplicate typedefs. bch
  03/26/2009   removed CEI_* types. bch
  10/11/2011   added vbtInterruptWait. bch
  07/31/2012   added variables and packed DEVMAP_T and DEVINFO_T. modified 
                MAX_DEVICES. bch
 */

#ifndef _LOWLEVEL_H_
#define _LOWLEVEL_H_

#include "cei_types.h"

// Define the max number of physical devices we support
#define MAX_DEVICES   16

/**********************************************************************
*  Error return codes from LOWLEVEL routines
**********************************************************************/
#define BTD_OK                0     // success
#define BTD_ERR_BADADDR       4     // invalid address
#define BTD_BAD_DEVICE_ID     11
#define BTD_ERR_BADADDRMAP    12    // bad initial mapping of address
#define BTD_BAD_HW_INTERRUPT  25
#define BTD_NO_DRV_MOD        30    // No Driver Module found
#define BTD_IOCTL_SET_REG     32
#define BTD_IOCTL_REG_SIZE    33
#define BTD_IOCTL_GET_REG     34
#define BTD_BAD_SIZE          35
#define BTD_NO_HASH_ENTRY     38
#define BTD_ERR_NOWINRT       50    // WinRT driver not loaded/started
#define BTD_ERR_BADREGISTER   51    // WinRT parameters don't match registry
#define BTD_ERR_BADOPEN       52    // WinRT device open failed
#define BTD_UNKNOWN_BUS       53    // Bus is not PCI, ISA or VME
#define BTD_BAD_LL_VERSION    54    // Unsupported lowlevel driver installed
#define BTD_BAD_INT_EVENT     55    // Unable to create interrupt event
#define BTD_ISR_SETUP_ERROR   56    // Error setting up the ISR driver
#define BTD_CREATE_ISR_THREAD 57    // Error creating the ISR thread
#define BTD_NO_REGIONS_TO_MAP 58

/*
#define BTD_BAD_DEVICE_NUM    59
#define BTD_DRV_READ_FAIL     60
#define BTD_DRV_WRITE_FAIL    61
#define BTD_DRV_IOCTL_FAIL    62
#define BTD_SYSFS_FAIL        63
#define BTD_SYSFS_ATTR_FAIL   64
#define BTD_TOO_MANY_RGNS     65    // invalid hardware memory region count
*/
/**********************************************************************
*  Lowlevel address mapping structure
**********************************************************************/
#define MAX_MEMORY   6     /* Number of supported memory regions */
#define MAX_PORTS    2     /* Number of supported IO regions     */
typedef enum _BUS_TYPE {    /* Host bus types supported           */
    BUS_INTERNAL,
    BUS_ISA,
    BUS_PCI,
    BUS_VME,
    BUS_PCMCIA,
    BUS_OTHER
} BUS_TYPE;           // These are the bus types supported by lowlevel

#pragma pack(push, 2)  // alignment to a 2 byte boundary
// Structure for communicating memory mapping information about a device.
typedef struct _DEVMAP_T {
   CEI_INT       busType;                    // One of BUS_TYPE.
   CEI_INT       interruptNumber;            // Interrupt number.
   CEI_INT       memSections;                // Number of memory regions defined.
   CEI_INT       flagMapToHost[MAX_MEMORY];  // Set to map region into host space.
   CEI_CHAR*     memHostBase[MAX_MEMORY];    // Base address of region in host space.
   CEI_ULONG     memStartPhy[MAX_MEMORY];    // Physical base address of region.
   CEI_UINT      memLengthBytes[MAX_MEMORY]; // Length of region in bytes.
   CEI_INT       portSections;               // Number of I/O port regions.
   CEI_ULONG     portStart[MAX_PORTS];       // I/O Address of first byte.
   CEI_UINT      portLength[MAX_PORTS];      // Number of bytes in region.
   CEI_INT       llDriverVersion;            // Low Level driver version.
   CEI_INT       KernelDriverVersion;        // Kernel driver version.
   CEI_INT       hKernelDriver;              // Handle to the kernel driver.
   CEI_UINT      VendorID;                   // Vendor ID if PCI card
   CEI_UINT      DeviceID;                   // Device ID if PCI card
   CEI_INT       device;                     // This the device
   CEI_INT       use_count;                  // Number of user channels
  #ifdef _USE_BM_DMA
   CEI_UINT*     vaddr;
   CEI_UINT64    laddr;
  #endif
   CEI_INT       use_channel_map;
   CEI_INT       mapping;      
} DEVMAP_T, *PDEVMAP_T;

typedef struct _DEVINFO_T {
   CEI_INT      busType;                    // the bus type.
   CEI_INT      nchan;                      // number of channels
   CEI_INT      irig;                       // IRIG option 0=no 1=yes.
   CEI_INT      mode;                       // mode 0=single 1=multi.
   CEI_INT      memSections;                // number of PCI memory sections
   CEI_ULONG    base_address;               // base address
   CEI_UINT     VendorID;                   // Vendor ID if PCI card
   CEI_UINT     DeviceID;                   // Device ID if PCI card
   CEI_UINT16   host_interface;             // host interface
} DEVINFO_T, *PDEVINFO_T;
#pragma pack(pop)  // reset alignment

// prototypes
// device addressing for PCI, ISA, and PCMCIA
CEI_INT vbtMapBoardAddresses(CEI_UINT device, // Device to open (0 - 9)
                             PDEVMAP_T pMap);  // Pointer to region mapping structure
CEI_VOID vbtFreeBoardAddresses(PDEVMAP_T pMap);  // Pointer to region mapping structure

// reads PCI config space
CEI_INT vbtGetPCIConfigRegister(CEI_UINT device,  // Device to open (0 - 9)
                                CEI_UINT offset,  // Offset of PCI config register
                                CEI_UINT length,  // Number of bytes to read (1,2,4)
                                CEI_VOID* value);     // Returned value of the register

// retreives device information
CEI_INT vbtGetDevInfo(CEI_UINT device, CEI_CHAR* devInfo, CEI_VOID* pData);

// handles hardware interrupts
CEI_INT vbtGetInterrupt(CEI_UINT device);
CEI_INT vbtConfigInterrupt(CEI_UINT device, CEI_INT signal, CEI_INT pid, CEI_INT val);
CEI_INT vbtInterruptMode(CEI_UINT device, CEI_INT mode);
CEI_INT vbtInterruptWait(CEI_UINT device, CEI_INT mode, CEI_INT val);
CEI_INT vbtWaitForInterrupt(CEI_UINT device);

// device addressing for VME boards
CEI_INT vbtMapIoAddress(CEI_ULONG base_address,
                        CEI_CHAR** addr);     // for VME carriers under VxWorks 
CEI_INT vbtMapBoardAddress(CEI_UINT cardnum,         // Card Number to open
                           CEI_UINT device,          // Device to open (0 - 9)
                           CEI_CHAR** addr,          // Computed host address
                           CEI_VOID* MemorySize,         // Byte size of region mapped
                           CEI_INT addressing_mode); // For VME boars only.
CEI_VOID vbtFreeBoardAddress(CEI_UINT device);
CEI_INT vbtMapBoardAddress(CEI_UINT device, CEI_UINT map_len, CEI_CHAR** addr, CEI_VOID* junk, CEI_INT junk2);

#endif  // _LOWLEVEL_H_
