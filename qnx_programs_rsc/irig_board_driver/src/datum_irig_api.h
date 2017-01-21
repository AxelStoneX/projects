/*
*    DATUM(Bancomm) IRIG board BC635PCI driver for QNX 6.
*    Version 1.0.
*    Date: 01.04.2015
*    Creator: Alexander Tsukanov
*    Mail: kongol@live.ru
*/
/*-------------------------------------------------------------------------------------------*/
#ifndef __DATUM_IRIG_API_H_
#define __DATUM_IRIG_API_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/pci_serv.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <hw/pci.h>
#include <hw/pci_devices.h>
#include <errno.h>

/*--------------------------------------Types Definition-------------------------------------*/

typedef union binary_event_time binary_event_time;
typedef union decimal_event_time decimal_event_time;
typedef union binary_strobe_time binary_strobe_time;
typedef union decimal_strobe_time decimal_strobe_time;
typedef struct _pci_config_regs _pci_config_regs;
typedef struct pci_dev_info pci_dev_info;

/*-----------------------------------Registers Definitions-----------------------------------*/
/* PCI data */
#define DATUM_PCI_VENDOR_ID   0x12E2
#define DATUM_PCI_DEVICE_ID   0x4013
#define DATUM_DPRAM_TOP       0x800

/* Device register control field bit definition */   // CONTROL register
#define DATUM_CONTROL_BIT_LOCKEN     0x01
#define DATUM_CONTROL_BIT_EVSOURCE   0x02
#define DATUM_CONTROL_BIT_EVSENSE    0x04
#define DATUM_CONTROL_BIT_EVENTEN    0x08
#define DATUM_CONTROL_BIT_STREN      0x10
#define DATUM_CONTROL_BIT_STRMODE    0x20
#define DATUM_CONTROL_BIT_FREQSEL0   0x40
#define DATUM_CONTROL_BIT_FREQSEL1   0x80

/* Device register interrupt mask and status field bit definitions */    // MASK and INTSTAT registers
#define DATUM_INT_BIT_EVENT          0x01
#define DATUM_INT_BIT_PERIOD         0x02
#define DATUM_INT_BIT_STROBE         0x04
#define DATUM_INT_BIT_1PPS           0x08
#define DATUM_INT_BIT_ALL            ( DATUM_INT_BIT_EVENT|DATUM_INT_BIT_PERIOD|DATUM_INT_BIT_STROBE|DATUM_INT_BIT_1PPS )

/* Time data status bit fields definitions */                  // status bits found in TIME0 and EVENT0
#define DATUM_STATUS_BIT_FLY   0x01
#define DATUM_STATUS_BIT_TOFS  0x02
#define DATUM_STATUS_BIT_FOFS  0x04

/* Device register acknowledge field bit definitions */        // ACK register contents
#define DATUM_ACK_BIT_REC      0x01
#define DATUM_ACK_BIT_CMD      0x80

/* Dual port RAM commands */                         // A lot of RAM commands
#define DATUM_SET_TIMING_MODE_CMD   0x10             // Set TFP timing mode
#   define DATUM_MODE_TIMECODE      0x00
#   define DATUM_MODE_FREERUN       0x01
#   define DATUM_MODE_EXT1PPS       0x02
#   define DATUM_MODE_RTC           0x03

#define DATUM_SET_TIME_FORMAT_CMD   0x11             // Set time register format
#   define DATUM_TIME_FMT_BCD       0x00
#   define DATUM_TIME_FMT_UNIX      0x01

#define DATUM_SET_TIME_CMD          0x12             // Set major time

#define DATUM_SET_YEAR_CMD          0x13             // Set year

#define DATUM_SET_PERIODIC_CMD      0x14             // Set periodic output
#   define DATUM_NO_SYNC_PPS        0x00
#   define DATUM_SYNC_PPS           0x01

#define DATUM_SET_TC_FORMAT_CMD     0x15             // Set input time code format
#   define DATUM_TIME_CODE_IRIGA    0x41
#   define DATUM_TIME_CODE_IRIGB    0x42
#   define DATUM_TIME_CODE_1344     0x49

#define DATUM_SET_TC_MOD_CMD        0x16             // Set input time code format (IRIG B by default)
#   define DATUM_MODE_AMP           0x4D
#   define DATUM_MODE_PULSE         0x44

#define DATUM_SET_TIMING_OFFSET_CMD 0X17             // Set propagation delay compensation

#define DATUM_GET_DATA_CMD          0x19             // Request TFP data
#   define DATUM_GET_DATA_MODE      0x10                 // Timing Mode
#   define DATUM_GET_DATA_TFORMAT   0x11                 // Timing Format
#   define DATUM_GET_DATA_YEAR      0x13                 // Current Year
#   define DATUM_GET_DATA_PER       0x14                 // Periodic Output
#   define DATUM_GET_DATA_TCFMT     0x15                 // Time Code Format
#   define DATUM_GET_DATA_TCMOD     0x16                 // Time Code Modulation
#   define DATUM_GET_DATA_TOFF      0x17                 // Timing Offset
#   define DATUM_GET_DATA_UTCINFO   0x18                 // UTC Info Control

#   define DATUM_GET_DATA_TCOUTFMT  0x1B                 // Time Code Output Format
#   define DATUM_GET_DATA_TCGENOFF  0x1C                 // Generator Time Offset
#   define DATUM_GET_DATA_LOCTMOFF  0x1D                 // Local Time Offset
#   define DATUM_GET_DATA_LEAPSEC   0x1E                 // Leap Second Setting

#   define DATUM_GET_DATA_FWVER     0x1F                 // TFP Firmware Version
#   define DATUM_GET_DATA_CLKSRC    0x20                 // Clock Source
#   define DATUM_GET_DATA_JAMSC     0x21                 // Jam Sync Control
#   define DATUM_GET_DATA_OSCCTL    0x23                 // Oscillator Disciplining Control
#   define DATUM_GET_DATA_DAVAL     0x24                 // D A Value (damn electricity)

#   define DATUM_GET_DATA_BATTSTAT  0x26                 // Battery Status
#   define DATUM_GET_DATA_CLKVAL    0x29                 // Clock Value Offset
#   define DATUM_GET_DATA_DTFW      0x4F                 // FW Version
#   define DATUM_GET_DATA_ASMB      0xF4                 // Assembly Number
#   define DATUM_GET_DATA_HWFAB     0xF5                 // Hardware Revision
#   define DATUM_GET_DATA_TFPMODEL  0xF6                 // TFP Model
#   define DATUM_GET_DATA_SERIAL    0xFE                 // Serial Number

#define DATUM_SOFT_RESET_CMD        0x1A               // Software Reset. Registers untouched

#define DATUM_SET_TC_OUT_FMT_CMD    0x1B               // Set time code output format

#define DATUM_SET_GEN_TM_OFF_CMD    0x1C               // Set generator time offset
#   define DATUM_HALF_NO            0x00
#   define DATUM_HALF_YES           0x01

#define DATUM_SET_LOC_TM_OFF_CMD    0x1D               // Set local time offset

#define DATUM_SET_LEAP_SEC_CMD      0x1E               // Set leap second event

#define DATUM_SEL_CLOCK_CMD         0x20               // Set clock source ("I" by default)
#   define DATUM_SEL_CLOCK_INT      0x49                  // Internal source
#   define DATUM_SEL_CLOCK_EXT      0x45                  // External Source

#define DATUM_JAM_SYNC_ENABLE_CMD   0x21               // Control Jam-Sync
#   define DATUM_JAMSYNC_DIS        0x00
#   define DATUM_JAMSYNC_EN         0x01

#define DATUM_FORCEJAMSYNC_CMD      0x22               // Perform one force Jam-Sync

#define DATUM_SET_DAC_CMD           0x24               // Load D/A Converter

#define DATUM_SET_DISC_GAIN_CMD     0x25               // Set disciplining gain

#define DATUM_SET_LOC_TM_FLG_CMD    0x40               // Observe IEEE 1344 local time flag

#define DATUM_REQ_DAYLT_FLG_CMD     0x41               // Request IEEE 1344 daylight saving
#   define DATUM_LOCAL_ENABLE       0x08

#define DATUM_SET_YR_INC_FLG_CMD    0x42               // Year auto increment flag

#define DATUM_REQ_DTFW_NUM_CMD      0x4F               // Request PCI firmware part number
#   define DATUM_ID_DTFW            "DT6000"

#define DATUM_REQ_ASSEMB_CMD        0xF4               // Request Assembly Part Number
#   define DATUM_OLD_ASM_PART       12043
#   define DATUM_NEW_ASM_PART       12083

#define DATUM_REQ_HW_FAB_CMD        0xF5               // Request hardware fab part number
#   define DATUM_OLD_FAB_PART       12042
#   define DATUM_NEW_FAB_PART       12083

#define DATUM_REQ_TFP_MODEL_CMD     0xF6               // Request TFP Model Identefication
#   define DATUM_ID_IRIG            "BC635PCI"
#   define DATUM_ID_GPS             "BC637PCI"

#define DATUM_REQ_SER_NIM_CMD       0xFE               // Request TFP serial number

/*-------------------------------Structures Definitions------------------------------------------*/

union binary_event_time
{
    struct
    {
        unsigned     u_seconds   : 20;
        unsigned     n_seconds   : 4;
        unsigned     status      : 4;
        unsigned     unused      : 4;
        unsigned     seconds     : 32;
    } bits;
    struct
    {
        unsigned long time0;
        unsigned long time1;
    } words;
};

union decimal_event_time
{
     struct
     {
        unsigned     u_seconds   : 20;
        unsigned     n_seconds   : 4;
        unsigned     status      : 4;
        unsigned     day8        : 1;
        unsigned     unused      : 3;
        unsigned     seconds     : 8;
        unsigned     minutes     : 8;
        unsigned     hours       : 8;
        unsigned     days        : 8;
     } bits;
     
     struct
     {
        unsigned     long time0;
        unsigned     long time1;
     } words;
};

struct
{
     unsigned long timereq;       // Time request
     unsigned long eventreq;      // Event request
     unsigned long unlock;        // Release capture lockout
     unsigned long r1;            // Reserved1
     unsigned long control;       // CONTROL register
     unsigned long ack;           // Acknowledge register
     unsigned long mask;          // Interruption mask register
     unsigned long intstat;       // Interrupt Status
     unsigned long minstrb;       // Minor strobe time
     unsigned long majstrb;       // Major strobe time;
     unsigned long r2;            // Reserved 2;
     unsigned long r3;            // Reserved 3;
     unsigned long time0;         // Minor time holding register
     unsigned long time1;         // Major time holding register
     unsigned long event0;        // Minor event holding register
     unsigned long event1;        // Major event holding register   
} *DEV_reg;

struct
{
     char *input;                 // Input area
     char *output;                // Output area
     char *GPS;                   // GPS packet area
     char *year;                  // Year area
} DP_ram_ofs;

char *DP_ram;                     // Pointer to Dual Port RAM address

char buffer[10];                  // Buffer for RAM commands

int pci_handle;                   // Handle for PCI device initialization

void* dev_handle;                 // Handle for PCI device usage

/*------------------------------------Internal Functions Prototypes------------------------------------------------------- */

/* Internal functions. Never use this without API internal structure knowledge */
void write_dpram_data( char *addr, char *data, int len );
void* find_and_attach(  pci_dev_info* inf_F );
void pci_config_show ( _pci_config_regs *PC );
void* reattach_and_fill_info( pci_dev_info* inf_F );
void read_dpram_data( char *addr, char *data, int len );

/*----------------------------------------Constants Definition---------------------------------------------------------*/

#define IRIG_DEPENDABLE_MODE 0
#define IRIG_AUTONOMOUS_MODE 1
#define IRIG_PPS_MODE        2
#define TIME_OFFSET          5    /* Controls speed of card work and initializing. Too small values
                                     can cause program to work incorrect. Depends on hardware */

/*---------------------------------------Functions Prototypes-----------------------------------------------------------*/

/* Initialize IRIG board. Use this before any other function. */
int irig_init ( void );

/* Get current time in binary format. Data loads in time_storage structure */
int get_binary_irig_time ( binary_event_time * time_storage );

/* Set current time in binary format. Data is loaded from new_time structure. UNIX seconds required */
int set_binary_irig_time ( uint32_t unix_s);

/* Get current time in decimal format. Data loades in time_storage structure */ 
int get_decimal_irig_time ( decimal_event_time* time_storage );

/* Set current time in decimal format. Data is loaded from new_time structure */
int set_decimal_irig_time ( uint16_t year, uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds );

/* Show current time in binary format (seconds:microseconds:nanoseconds) */
int show_binary_time ( binary_event_time etime );

/* Show current time in decimal format (days:hours:minutes:seconds:microseconds:nanoseconds) */
int show_decimal_time ( decimal_event_time etime );

/* Set IRIG mode. Available modes:
IRIG_DEPENDABLE_MODE
IRIG_AUTONOMOUS_MODE
IRIG_PPS_MODE */
int set_irig_mode (int mode);

/* Set year on IRIG board*/
int set_year(uint16_t year);

/* Show current year */
int show_year ( void );

/* Get current year */
int get_year ( uint16_t* year );

/* Show allavailable information about current board status, including current operating modes */
int show_board_info ( void );

/* Deinitialize IRIG board. Use this after any other function to clean up memory */
int irig_deinit ( void );



#endif
