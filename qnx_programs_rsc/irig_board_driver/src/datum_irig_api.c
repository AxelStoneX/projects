#include "datum_irig_api.h"
/*
*    DATUM(Bancomm) IRIG board BC635PCI driver for QNX 6.
*    Version 1.0.
*    Date: 01.04.2015
*    Creator: Alexander Tsukanov
*    Mail: kongol@live.ru
*/
//---------------------------------------Binary Functions-----------------------------------------------------------

/* Set time in binary format */
int set_binary_irig_time ( uint32_t unix_s )
{
     /* Set time format to binary */
     buffer[0] = DATUM_SET_TIME_FORMAT_CMD;
     buffer[1] = DATUM_TIME_FMT_UNIX;
     write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
     DEV_reg->ack = DATUM_ACK_BIT_CMD;
     while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay(10);
     sleep(2);
     
     /* Set time */
     buffer[0] = DATUM_SET_TIME_CMD;
     buffer[1] = ( unix_s >> 24 ) & 0xFF;
     buffer[2] = ( unix_s >> 16 ) & 0xFF;
     buffer[3] = ( unix_s >> 8 )   & 0xFF;
     buffer[4] = unix_s & 0xFF;
     write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 5 );
     DEV_reg->ack = DATUM_ACK_BIT_CMD;
     while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( 10 );
     sleep( 2 );
     
     return EXIT_SUCCESS;
}

/* Get time in binary format into structure*/
int get_binary_irig_time ( binary_event_time *time_storage )
{
     DEV_reg->timereq = 0;
     ( time_storage->words ).time0 = DEV_reg->time0;
     ( time_storage->words ).time1 = DEV_reg->time1;

     return EXIT_SUCCESS;
}

/* Display current time in binary format */
int show_binary_time ( binary_event_time etime )
{
     double result;
     result = etime.words.time1;
     result += ( etime.bits.u_seconds + etime.bits.n_seconds / 10.0 ) / 1000000.0;
     printf( "Current board time: %f UNIX seconds\n", result );
     
     return EXIT_SUCCESS;
}

//----------------------------------------Decimal Functions--------------------------------------------------------

int set_decimal_irig_time ( uint16_t year, uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds )
{
    /* Set time format to decimal */
    buffer[0] = DATUM_SET_TIME_FORMAT_CMD;
    buffer[1] = DATUM_TIME_FMT_BCD;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( 10 );
    sleep( 5 );
    
    /* Set time via dual port RAM */
    buffer[0] = DATUM_SET_TIME_CMD;
    buffer[1] = (year>>8) & 0xFF;
    buffer[2] = year & 0xFF;
    buffer[3] = (days>>8)  & 0xFF;
    buffer[4] = days & 0xFF;
    buffer[5] = hours;
    buffer[6] = minutes;
    buffer[7] = seconds;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 8 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( 10 );
    sleep( 2 );

    return EXIT_SUCCESS;
}

/* Get time in decimal format into structure */
int get_decimal_irig_time ( decimal_event_time *time_storage )
{
     DEV_reg->timereq = 0;
     ( time_storage->words ).time0 = DEV_reg->time0;
     ( time_storage->words ).time1 = DEV_reg->time1;
     

     return EXIT_SUCCESS;
}

/* Display current time in decimal format */
int show_decimal_time ( decimal_event_time etime )
{
     int n_seconds = etime.bits.n_seconds * 100;
     printf( "Current board time: %ud:%uh:%um:%us:%ums:%dns\n", etime.bits.days, etime.bits.hours, etime.bits.minutes,etime.bits.seconds, etime.bits.u_seconds, n_seconds );
     
     return EXIT_SUCCESS;
}

//------------------------------------------------Year Functions---------------------------------------------------

int set_year(uint16_t year)
{
    buffer[0] = DATUM_SET_YEAR_CMD;
    buffer[1] = (year>>8) & 0xFF;
    buffer[2] = year & 0xFF;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 3 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( TIME_OFFSET );
    sleep( 2 );
   return EXIT_SUCCESS;
}

int get_year( uint16_t *year )
{
    /* Get year stored in dual port RAM */
    buffer[0] = DATUM_GET_DATA_CMD;
    buffer[1] = DATUM_GET_DATA_YEAR;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( TIME_OFFSET );
    sleep( 2 );
    
    read_dpram_data( DP_ram_ofs.output, &buffer[0], 3 );
    *year = (((unsigned char)buffer[1])<<8)|(((unsigned char)buffer[2]));
    
    return EXIT_SUCCESS;
}

int show_year( void )
{
    uint16_t year;
    /* Get year stored in dual port RAM */
    buffer[0] = DATUM_GET_DATA_CMD;
    buffer[1] = DATUM_GET_DATA_YEAR;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( TIME_OFFSET );
    sleep( 2 );
    
    /* Print year from dual port RAM */
    read_dpram_data( DP_ram_ofs.output, &buffer[0], 3 );
    printf( "Current DPRAM year: " );
    year = (((unsigned char)buffer[1])<<8)|(((unsigned char)buffer[2]));
    printf("%u\n", year);
    
    return EXIT_SUCCESS;
}

//-------------------------------------------Board Status Functions--------------------------------------------------

/* Set timing mode for Datum IRIG board */
int set_irig_mode ( int mode )
{
     buffer[0] = DATUM_SET_TIMING_MODE_CMD;
     if( mode == IRIG_DEPENDABLE_MODE )
         buffer[1] = DATUM_MODE_TIMECODE;
     if( mode == (IRIG_AUTONOMOUS_MODE) )
         buffer[1] = DATUM_MODE_FREERUN;
     if( mode == (IRIG_PPS_MODE) )
         buffer[1] = DATUM_MODE_EXT1PPS;
     if( (mode != IRIG_DEPENDABLE_MODE) && (mode != IRIG_AUTONOMOUS_MODE) && (mode != IRIG_PPS_MODE) )
     {
         printf("\n...Wrong card mode. Choose IRIG_DEPENDABLE_MODE, IRIG_AUTONOMOUS_MODE or IRIG_PPS_MODE\n\n");
         return( EXIT_FAILURE );
     }
     write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
     DEV_reg->ack = DATUM_ACK_BIT_CMD;
     while(!(DEV_reg->ack & DATUM_ACK_BIT_REC))
         delay( TIME_OFFSET );
     sleep( 2 );
     return EXIT_SUCCESS;
}

/* Show available additional info about current board status*/
int show_board_info ( void )
{
    int i;
    uint16_t year = 0;
    uint16_t ap_num;
    
    /* Get model of board */
    buffer[0] = DATUM_GET_DATA_CMD;
    buffer[1] = DATUM_GET_DATA_TFPMODEL;                           //DATUM_GET_DATA_TFORMAT;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( TIME_OFFSET );
    sleep( 2 );
    
    /* Print board model */
    read_dpram_data(DP_ram_ofs.output, &buffer[0], 9);
    printf( "Current board model: ");
    for( i=1; i < 9; i++ )
          printf( "%c", buffer[i] );
    printf( "\n" );
    
    /* Get current board operating mode */
    buffer[0] = DATUM_GET_DATA_CMD;
    buffer[1] = DATUM_GET_DATA_MODE;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( TIME_OFFSET );
    sleep( 2 );
    
    /* Print current timing mode */
    read_dpram_data( DP_ram_ofs.output, &buffer[0], 2 );
    printf( "Current timing mode: ");
    if( buffer[1] == 0 )
        printf( "IRIG B Timecode Mode\n" );
    if( buffer[1] == 1 )
        printf( "Free Running Mode\n" );
    if( buffer[1] == 2 )
        printf( "External 1PPS Mode\n" );
    if( buffer[1] != 0 && buffer[1] != 1 && buffer[1] != 2 )
    {
        printf( "Undefined\n" );
        printf( "--->Unable to identify current timing mode in show_board_info()\n" );
        return EXIT_FAILURE;
    }
    
    /* Get current timing format */
    buffer[0] = DATUM_GET_DATA_CMD;
    buffer[1] = DATUM_GET_DATA_TFORMAT;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( TIME_OFFSET );
    sleep( 2 );
    
    /* Print current timing format */
    read_dpram_data( DP_ram_ofs.output, &buffer[0], 2 );
    printf( "Current timing format: " );
    if( buffer[1] == 1 )
        printf( "Binary\n" );
    if( buffer[1] == 0 )
        printf( "Decimal\n" );
    if( buffer[1] != 0 && buffer[1] != 1 )
    {
        printf( "Undefined\n" );
        printf( "--->Error in timing format definition in show_board_info()\n" );
        return EXIT_FAILURE;
    }
    
    /* Get year stored in dual port RAM */
    buffer[0] = DATUM_GET_DATA_CMD;
    buffer[1] = DATUM_GET_DATA_YEAR;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( TIME_OFFSET );
    sleep( 2 );
    
    /* Print year from dual port RAM */
    read_dpram_data( DP_ram_ofs.output, &buffer[0], 3 );
    printf( "Current DPRAM year: " );
    year = (((unsigned char)buffer[1])<<8)|(((unsigned char)buffer[2]));
    printf("%u\n", year);
    
    /* Get current firmware revision data */
    buffer[0] = DATUM_GET_DATA_CMD;
    buffer[1] = DATUM_GET_DATA_FWVER;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( TIME_OFFSET );
    sleep( 2 );
    
    /* Print current firmware revision data */
    read_dpram_data( DP_ram_ofs.output, &buffer[0], 7 );
    year = (((unsigned char)buffer[5])<<8)|((unsigned char)buffer[6]);
    printf( "Current FW version: %u.%u ", buffer[1], buffer[2] );
    printf( "%u/%u/%u\n", buffer[4], buffer[3], year );
      
    return EXIT_SUCCESS;
}

//---------------------------------------------PCI server functions--------------------------------------------------

/* Datum IRIG board initialization  */
int irig_init ( void )
{
    long address;
    int fd;
    long unix_time;
    char *timestr;
    short year;
    int status;

    pci_dev_info inf_F;
    _pci_config_regs PCR;

    inf_F.DeviceId = DATUM_PCI_DEVICE_ID;
    inf_F.VendorId = DATUM_PCI_VENDOR_ID;
    
    /* Attach PCI server*/
    pci_handle = pci_attach( 0 );
    if( pci_handle == (-1) )
    {
        printf( "PCI attach error (check root priveleges) \n" );
        return( EXIT_FAILURE );
    }
    
    /* Find PCI device */
    dev_handle = find_and_attach ( &inf_F );
    if( dev_handle == NULL )
    {
        perror( "\n...Attach error. Device has not been found" );
        printf("\nDeviceID: %x; VendorID: %x\n", inf_F.DeviceId, inf_F.VendorId);
        return( EXIT_FAILURE );
    }    
    reattach_and_fill_info( &inf_F );

    /* Get the device register memory address */
    if( pci_read_config( dev_handle, 0, sizeof( _pci_config_regs ), 1, &PCR ) != PCI_SUCCESS )
    {
        perror( "Can't read pci_config_regs in PCR buffer\n" );
        return( EXIT_FAILURE );
    }
    address = PCR.Base_Address_Regs[0];
    if( address == NULL )
    {
        perror( "Can't read base address 0 from Datum IRIG card" );
        return( EXIT_FAILURE );
    }

    
    /* Map memory address in program space */
    DEV_reg = mmap_device_memory( 0, inf_F.BaseAddressSize[0], (PROT_READ | PROT_WRITE | PROT_NOCACHE), 0, inf_F.PciBaseAddress[0] );
    if( DEV_reg == MAP_FAILED )
    {
        perror( "Can't map physical memory for Datum IRIG card. Error in mmap_device_memory()\n" );
        return( EXIT_FAILURE );
    }
    
    /* Get the dual port ram memory address */
    address = PCR.Base_Address_Regs[1];
    if( address == NULL )
    {
        perror( "Can't read base address 1 from Datum IRIG card" );
        return ( EXIT_FAILURE );
    }
    
    /* Map dual port RAM in program space*/
    DP_ram = mmap_device_memory( 0, inf_F.BaseAddressSize[1], (PROT_READ | PROT_WRITE | PROT_NOCACHE), 0, inf_F.PciBaseAddress[1] );
    if( DP_ram == MAP_FAILED )
    {
        perror( "Can't map Dual Port memory for Datum IRIG card. Error in mmap_device_memory()\n" );
        return ( EXIT_FAILURE );
    }

    /* Setup pointer to dual port ram buffers */
    DP_ram_ofs.input  = DP_ram + *(DP_ram + DATUM_DPRAM_TOP - 1) + *(DP_ram + DATUM_DPRAM_TOP - 2) * 256;
    DP_ram_ofs.output = DP_ram + *(DP_ram + DATUM_DPRAM_TOP - 3) + *(DP_ram + DATUM_DPRAM_TOP - 4) * 256 + 256;
    DP_ram_ofs.year   = DP_ram + *(DP_ram + DATUM_DPRAM_TOP - 7) + *(DP_ram + DATUM_DPRAM_TOP - 8) * 256;

    /* Reset card */
    buffer[0] = DATUM_SOFT_RESET_CMD;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 1 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay(TIME_OFFSET);
    sleep(2);

    /* Set year from operating system */
    unix_time = time( NULL );
    timestr = asctime( gmtime( (time_t*)&unix_time ) );
    buffer[0] = DATUM_SET_YEAR_CMD;
    year = htons( atoi( timestr + 20 ) );
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 1 );
    write_dpram_data( DP_ram_ofs.input + 1, (char*)&year, 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !( DEV_reg->ack & DATUM_ACK_BIT_REC ) ) 
       delay( TIME_OFFSET );
    sleep( 2 );

    return EXIT_SUCCESS;
}

/* Find and attach PCI device with given VendorID and Device ID */
void* find_and_attach(  pci_dev_info* inf_F )
{
	uint32_t flags;                                                 // flags for PCI resourses handle
	uint16_t idx = 0;                                               // the index for device
	void *handle;                                                   // handle for pci_attach_device()

	flags = (PCI_SEARCH_VENDEV | PCI_SHARE);                        // PCI_SHARE is for multiple access	
	handle = pci_attach_device( NULL, flags, idx, inf_F );          // attach pci_driver to the device
	return handle;
}

/* Reattach PCI device with given handle to fill pci_dev_info structure */
void* reattach_and_fill_info( pci_dev_info* inf_F )
{
    uint32_t flags;
    uint16_t idx = 0;
    void *handle;
    
    flags = PCI_INIT_ALL;
    handle = pci_attach_device( dev_handle, flags, idx, inf_F );
    return handle;
}

/* Deinitialization function. Use this to terminate work of IRIG board */
int irig_deinit ( void )
{
    pci_detach( pci_handle );
    return ( EXIT_SUCCESS );
}

//---------------------------------------Dual port RAM functions----------------------------------------------------- 

/* Write into dual port RAM bitwise */
void write_dpram_data( char *addr, char *data, int len )
{
    for( ; len > 0; len-- )
       *(addr++) = *(data++);
}

/* Read from dual port RAM bitwise */
void read_dpram_data( char *addr, char *data, int len )
{
    for( ; len > 0; len-- )
        *( data++ ) = *( addr++ );
}