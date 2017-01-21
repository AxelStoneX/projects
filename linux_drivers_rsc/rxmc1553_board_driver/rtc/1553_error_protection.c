#include "1553_interface.h"

//=============================================================================
int check_bus_repeating_entry( LWORD bus_number, int dev, LWORD channel )
{   int i;

    for( i=0; i < MAX_CARD_NUMBER; i++ )
    if( bus_ent[i].init )
    {   if( bus_number == bus_ent[i].bus_number )
        {   printf( ">>>>Repeating bus=%d \n", bus_number );
            printf( "in check_repeating_entry()" );
            return( RET_FAIL ); /*.........................*/  }
            
        if( dev == bus_ent[i].dev && channel == bus_ent[i].channel )
        {   printf( ">>>>Repeating dev=%d chan=%d ", dev, channel );
            printf( "in check_repeating_entry()\n" );
            return( RET_FAIL ); /*...................................*/  }      
    }
    return RET_OK;
}

//=============================================================================
int check_tr_entry( int transaction_index )
{   if( transaction_index > (empty_tr - 1) || transaction_index < 0 )
    {   printf( ">>>>Uninitialized transaction detected at index=%d",
                                                      transaction_index );
        printf( " in check_tr_entry()\n" );
        return RET_FAIL; /*.............................*/ }
        
    return RET_OK;
}

//=============================================================================
int check_bus_PK( int transaction_index, int transaction_direction )
{   if( busPK_heap[transaction_index].init != L_ON )
    {   printf( "Uninitialized bus_PK #%d detected ", transaction_index );
        printf( "in check_bus_PK()\n" );
        printf( "Info: busPK_heap[%d].init = %d\n", transaction_index, busPK_heap[transaction_index].init);
        return RET_FAIL; /*..............................................*/ }
    
    // START OF DEBUG
    printf( "-----------------------------------------\n" );    
    printf( "------Buffer had been checked succesfully\n" );
    printf( "------Info: busPK_heap[%d].init = %d\n", transaction_index, busPK_heap[transaction_index].init);
    printf( "-----------------------------------------\n" );
    // END OF DEBUG

    if( busPK_heap[transaction_index].transaction_direction 
                                                 != transaction_direction )
    {   printf( "Transaction directions don't match " );
        printf( "in check_bus_PK()\n" );
        return RET_FAIL; /*...............................................*/}
        
    return RET_OK;
}
