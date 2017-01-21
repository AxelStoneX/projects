#include "datum_irig_api.h"

int irig_init ( void )
{
    unsigned busnum;
    unsigned devfuncnum;
    //char irq;
    //char pin;
    long address;
    int fd;
    long unix_time;
    char *timestr;
    short year;
    int status;
    
    /* Find PCI device */
    status = __CA_PCI_find_device( DATUM_PCI_DEVICE_ID, DATUM_PCI_VENDOR_ID, 0, &busnum, &devfuncnum );
    if( status != PCI_SUCCESS )
    {
        perror( "\n...Can't find Datum IRIG card \n\n" );
        return( EXIT_FAILURE );
    }

    /* Get the device register memory address */
    status = __CA_PCI_read_config_dword( busnum, devfuncnum, offsetof(struct _pci_config_regs, Base_Address_Regs[0]), 1, (char*)&address);
    if( status != PCI_SUCCESS )
    {
       perror( "Can't read base address 0 from Datum IRIG card" );
       return( EXIT_FAILURE );
    }

    /* Map memory address in program space */
    fd = shm_open( "Physical", O_RDWR, 0777 );
    if( fd == -1 )
    {
        perror( "Can't open physical memory for Datum IRIG card" );
        return ( EXIT_FAILURE );
    }
    DEV_reg = mmap( 0, __PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PCI_MEM_ADDR( address ) );
    if( DEV_reg == (void*) -1 )
    {
        perror( "Can't map physical memory for Datum IRIG card" );
        return ( EXIT_FAILURE );
    }

    /* Get the dual port ram memory address */
    status = _CA_PCI_Read_Config_DWord( busnum, devfuncnum, offsetof(struct _pci_config_regs, Base_Address_Regs[1]), 1, (char*)&address );
    if( status != PCI_SUCCESS )
    {
        perror( "Can't read base address 1 from Datun IRIG card" );
        return( EXIT_FAILURE );
    }
    
    /* Map DPRAM memory address into program space */
    DP_ram = mmap( 0, __PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PCI_MEM_ADDR( address ) );
    if ( DP_ram == (void*) -1 )
    {
        perror( "Can't map physical dual port ram memory for Datum IRIG card" );
        return( EXIT_FAILURE );
    }

    /* Setup pointer to dual port ram buffers */
    DP_ram_ofs.input  = DP_ram + *(DP_ram + DATUM_DPRAM_TOP - 1) + *(DP_ram + DATUM_DPRAM_TOP - 2) * 256;
    DP_ram_ofs.output = DP_ram + *(DP_ram + DATUM_DPRAM_TOP - 3) + *(DP_ram + DATUM_DPRAM_TOP - 4) * 256;
    DP_ram_ofs.year   = DP_ram + *(DP_ram + DATUM_DPRAM_TOP - 7) + *(DP_ram + DATUM_DPRAM_TOP - 8) * 256;

    /* Reset card */
    buffer[0] = DATUM_SOFT_RESET_CMD;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 1 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay(10);
    sleep(2);

    	/* Set year */
    unix_time = time( NULL );
    timestr = asctime( gmtime( (time_t*)&unix_time ) );
    buffer[0] = DATUM_SET_YEAR_CMD;
    year = htons( atoi( timestr + 20 ) );
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 1 );
    write_dpram_data( DP_ram_ofs.input + 1, (char*)&year, 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !( DEV_reg->ack & DATUM_ACK_BIT_REC ) ) 
       delay( 10 );
    sleep( 2 );

    return EXIT_SUCCESS;
}
 
void write_dpram_data( char *addr, char *data, int len )
{
    for( ; len > 0; len-- )
       *(addr++) = *(data++);
}

int set_irig_mode ( int mode )
{
     buffer[0] = DATUM_SET_TIMING_MODE_CMD;
     if( mode == IRIG_DEPENDABLE_MODE )
         buffer[1] = DATUM_MODE_TIMECODE;
     if( mode == (IRIG_AUTONOMOUS_MODE) )
         buffer[1] = DATUM_MODE_FREERUN;
     if( mode == (IRIG_PPS_MODE) )
         buffer[1] = DATUM_MODE_EXT1PPS;
     if( mode != (IRIG_DEPENDABLE_MODE | IRIG_AUTONOMOUS_MODE | IRIG_PPS_MODE) )
     {
         printf("\n...Wrong card mode. Choose IRIG_DEPENDABLE_MODE, IRIG_AUTONOMOUS_MODE or IRIG_PPS_MODE\n\n");
         return( EXIT_FAILURE );
     }
     write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
     DEV_reg->ack = DATUM_ACK_BIT_CMD;
     while(!(DEV_reg->ack & DATUM_ACK_BIT_REC))
         delay( 10 );
     sleep( 2 );
     return EXIT_SUCCESS;
}

int get_binary_irig_time ( binary_event_time *time_storage )
{
     DEV_reg->timereq = 0;
     ( time_storage->words ).time0 = DEV_reg->time0;
     ( time_storage->words ).time1 = DEV_reg->time1;

     return EXIT_SUCCESS;
}

int get_decimal_irig_time ( decimal_event_time *time_storage )
{
     DEV_reg->timereq = 0;
     ( time_storage->words ).time0 = DEV_reg->time0;
     ( time_storage->words ).time1 = DEV_reg->time1;

     return EXIT_SUCCESS;
}

int show_binary_time ( binary_event_time time )
{
     printf( "Current board time: %t %d s:%d ms:%d ns", time.words.time0, time.bits.u_seconds, time.bits.n_seconds );
     return EXIT_SUCCESS;
}

int show_decimal_time ( decimal_event_time time )
{
     printf( "Current board time: %t %d d: %d h:%d m: %d s: %d ms: %d ns", time.bits.days, time.bits.hours, time.bits.minutes, time.bits.u_seconds, time.bits.n_seconds );
     return EXIT_SUCCESS;
}

int set_binary_irig_time ( uint32_t nanoseconds )
{
     buffer[0] = DATUM_SET_TIME_FORMAT_CMD;
     buffer[1] = DATUM_TIME_FMT_UNIX;
     write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
     DEV_reg->ack = DATUM_ACK_BIT_CMD;
     while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay(10);
     sleep(2);

     buffer[0] = DATUM_SET_TIME_CMD;
     buffer[1] = nanoseconds;                           //????
     write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 5 );
     DEV_reg->ack = DATUM_ACK_BIT_CMD;
     while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( 10 );
     sleep( 2 );
     
     return EXIT_SUCCESS;
}

int set_decimal_irig_time ( uint16_t year, uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds )
{

    buffer[0] = DATUM_SET_TIME_FORMAT_CMD;
    buffer[1] = DATUM_TIME_FMT_BCD;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 2 );
    DEV_reg->ack = DATUM_ACK_BIT_CMD;
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( 10 );
    sleep( 2 );

    buffer[0] = DATUM_SET_TIME_CMD;
    buffer[1] = year;
    buffer[3] = days;
    buffer[5] = hours;
    buffer[6] = minutes;
    buffer[7] = seconds;
    write_dpram_data( DP_ram_ofs.input + 0, &buffer[0], 7 );
    while( !(DEV_reg->ack & DATUM_ACK_BIT_REC) )
       delay( 10 );
    sleep( 2 );

    return EXIT_SUCCESS;
}



