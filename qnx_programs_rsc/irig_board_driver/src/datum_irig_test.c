#ifndef __DATUM_IRIG_TEST_C__
#define __DATUM_IRIG_TEST_C__

#include "datum_irig_api.h"
#define YEAR1 2000
#define YEAR2 2015
#define DAYS 4
#define HOURS 2
#define MINUTES 34
#define SECONDS 45


#endif

int main (void)
{
    int RS;                                         // Code for errors
    uint32_t unix_ms = 25;                          // Binary time to set in seconds
    binary_event_time bin_storage_time;             // Storage structure for time in binary format
    decimal_event_time dec_storage_time;            // Storage structure for time in decimal format
    uint16_t year1 = YEAR1;
    uint16_t year2 = YEAR2;
    uint16_t days = DAYS;
    uint8_t hours = HOURS;
    uint8_t minutes = MINUTES;
    uint8_t seconds = SECONDS;
    
    //------------------------Initialization--------------------------------
    RS = irig_init();
    if(RS == EXIT_FAILURE)
    {
        printf("--->Irig initialization failure in main().\n\n");
        return EXIT_FAILURE;
    }
    
    
    RS = set_binary_irig_time( unix_ms );
    RS = get_binary_irig_time( &bin_storage_time );
    RS = show_binary_time( bin_storage_time );
    sleep(5);
    RS = get_binary_irig_time( &bin_storage_time );
    RS = show_binary_time( bin_storage_time );
    sleep(10);
    RS = get_binary_irig_time( &bin_storage_time );
    RS = show_binary_time( bin_storage_time );
    sleep(15);
    RS = get_binary_irig_time( &bin_storage_time );
    RS = show_binary_time( bin_storage_time );
    printf( "--->Binary Time Check Finished\n\n" );
    
    
    

    /*
    RS = set_decimal_irig_time( year1, days, hours, minutes, seconds );
    RS = get_decimal_irig_time( &dec_storage_time );
    RS = show_decimal_time( dec_storage_time );
    sleep(5);
    RS = get_decimal_irig_time( &dec_storage_time );
    RS = show_decimal_time( dec_storage_time );
    sleep(10);
    RS = get_decimal_irig_time( &dec_storage_time );
    RS = show_decimal_time( dec_storage_time );
    sleep(15);
    RS = get_decimal_irig_time( &dec_storage_time );
    RS = show_decimal_time( dec_storage_time );
    printf( "-->Decimal Time Check Finished\n\n" );
    */
    
    
    
    /*    
    RS = show_year();
    RS = set_year(year2);
    RS = show_year();
    RS = get_year(&year1);
    printf("%u\n", year1);
    */
    
    
    
    
    RS = show_board_info();
    printf( "Board info show checked\n" );
        
}
