#include "1553_interface.h"

LWORD       bus_to_card_number[BT_MAX_BUS];
BUS_ENTRY   bus_ent[MAX_CARD_NUMBER];

//=============================================================================
int init_all_cards( void )
{
   int status;
   int i;
   int dev[MAX_DEVICE_NUMBER];
   int dev_counter = 0;
   LWORD card_number;
   LWORD mode = API_B_MODE | API_SW_INTERRUPT;
   
   printf( "Starting all cards initialization:\n" );
   for( i=0; i < MAX_DEVICE_NUMBER; i++ )
   {
       dev[i] = BusTools_FindDevice( QCP1553, 1 + i );
       if(dev[i] >= 0)
       {  printf( "Found BusTools-1553 dev[%d]=%d\n", i, dev[i] );
          dev_counter++; /*.................................*/ }
       if( dev[i] <= RET_FAIL )
          printf( "QCP1553 card #%d not found in init_all_cards()>>>>>>\n", i);
   }
   
   if( dev_counter <= 0 )
   {   printf( "Not a single BusTools-1553 card was found " );
       printf( "in init_all_cards()>>>>\n" );
       return RET_FAIL; /*...............................*/ }
   
   memset( (void*) &bus_to_card_number, -1, sizeof(LWORD) * BT_MAX_BUS );   //!!!
   memset( (void*) &bus_ent, -1, sizeof(BUS_ENTRY) * MAX_CARD_NUMBER );     //!!!
   
   status = init_single_card( 0, mode, dev[0], CHANNEL_1 );
   if( status != RET_OK )
   {   printf(">>>>>First card initialization error in init_all_cards()\n");
       return( RET_FAIL );  }
   
   status = init_single_card( 1, mode, dev[0], CHANNEL_2 );
   if( status != RET_OK )
   {   printf(">>>>>First card initialization error in init_all_cards()\n");
       return( RET_FAIL );  }
       
   return RET_OK;
}

//=============================================================================
int init_single_card( LWORD bus_number, LWORD mode, int dev, LWORD channel )
{
   LWORD card_number;
   int status;
   
   status = check_bus_repeating_entry( bus_number, dev, channel );
   if( status != RET_OK )
   {   printf( ">>>>Attempt to initialize card repeatively " );
       printf( " in init_single_card()\n" );
       printf( "bus_number=%d device=%d channel=%d", 
                              bus_number, dev, channel );
       return( RET_FAIL ); /*................................*/ }
   
   if( bus_number >= MAX_BUS_NUMBER ) 
   {   printf( ">>>>>Too big bus_number argument in init_single_card()\n" );
       return RET_FAIL;   /*...........................................*/   }
   if( channel >= MAX_CHANNEL_NUMBER ) 
   {   printf( ">>>>>Too big channel number in init_single_card()\n" );
       return RET_FAIL;   /*...........................................*/   }
   if( dev >= MAX_DEVICE_NUMBER  )
   {   printf( ">>>>>Too big device number in init_single_card()\n" );
       return RET_FAIL;   /*...........................................*/   }
       
   status = BusTools_API_OpenChannel( &card_number, mode, dev, channel );
   if( status != API_SUCCESS )
   {   printf( ">>>>Unable to open channel via BusTools_API_OpenChannel()" );
       printf( " in init_single_card()\n" );
       return RET_FAIL;   /*............................................*/   }
       
   if( card_number >= MAX_CARD_NUMBER )
   {   printf( ">>>>Too big card_number from BusTools_API_OpenChannel" );
       printf( " in init_single_card()\n" );
       return RET_FAIL; /*..............................................*/ }
       
   printf( "Device attached: bus=%d : dev=%d : chan=%d\n", 
                                 bus_number, dev, channel );

   status = register_single_card( bus_number, card_number, dev, channel );
 
   status = BusTools_BM_Init( card_number, 1, 1 );
   if( status != API_SUCCESS )
   {   printf( ">>>>>BusTools_BM_Init() error in init_single_card()\n" );
       return RET_FAIL; /*............................................*/ }
       
   status = BusTools_SetInternalBus( card_number, EXTERNAL_BUS );
   if( status != API_SUCCESS )
   {   printf( ">>>>>BusTools_SetInternalBus() error in init_single_card()\n" );
       return RET_FAIL; /*............................................*/        }
       
   status = BusTools_SetBroadcast( card_number, 1 );
   if( status != API_SUCCESS )
   {   printf( ">>>>>BusTools_SetBroadcast() error in init_single_card()\n" );
       return RET_FAIL; /*............................................*/      }
       
   status = BusTools_SetSa31( card_number, 1 );
   if( status != API_SUCCESS )
   {   printf( ">>>>>BusTools_SetSa31() error in init_single_card()\n" );
       return RET_FAIL; /*............................................*/  }    
       
   status = BusTools_RT_Init( card_number, 0 );
   if( status != API_SUCCESS )
   {   printf( ">>>>>BusTools_SetSa31() error in init_single_card()\n" );
       return RET_FAIL; /*............................................*/  }
       
   return RET_OK;
   
}

//=============================================================================
int register_single_card (LWORD bus_number, LWORD card_number, int dev, LWORD channel)
{
   bus_to_card_number[bus_number]  = card_number;
   bus_ent[card_number].dev        = dev;
   bus_ent[card_number].bus_number = bus_number;
   bus_ent[card_number].channel    = channel;
   bus_ent[card_number].init       = L_ON;
   return RET_OK;
}

//==============================================================================
int start_all_rt( void )
{
   int status, ix;
   LWORD card_number;
   for(ix=0; ix < MAX_BUS_NUMBER; ix++)
   {   card_number = bus_to_card_number[ix];
       if( card_number != -1 )
       {   status = BusTools_RT_StartStop( card_number, 1 );
           if( status != API_SUCCESS )
           {   printf( ">>>>>BusTools_RT_StartStop() error in start_all_rt()\n" );
               printf( "card_number = %d, bus_to_card_number[%d]\n",
                                          card_number, bus_to_card_number[ix]);
               return RET_FAIL; /*...........................................*/   }
           printf( "RT on card number = %d started\n", card_number );
       }
   }
   return RET_OK;
}

//==============================================================================
int stop_all_rt( void )
{
   int status, ix;
   LWORD card_number;
   for(ix=0; ix < MAX_BUS_NUMBER; ix++)
   {   card_number = bus_to_card_number[ix];
       if( card_number != -1 )
       {   status = BusTools_RT_StartStop( card_number, 0 );
           if( status != API_SUCCESS )
           {   printf( ">>>>>BusTools_RT_StartStop() error in start_all_rt()\n" );
               printf( "card_number = %d; bus_to_card_number[%d]\n",
                                          card_number, bus_to_card_number[ix]);
               return RET_FAIL; /*...........................................*/   }
           printf( "RT on card number = %d started\n", card_number );
       }
   }
   return RET_OK;
}

//================================================================================
void close_all_cards( void )
{
   int status, ix;
   LWORD card_number;
   for(ix=0; ix < MAX_BUS_NUMBER; ix++)
   {   card_number = bus_to_card_number[ix];
       if( card_number != -1 )
       {   status = BusTools_RT_StartStop( card_number, 0 );
           status = BusTools_API_Close( card_number );
           printf( "card number = %d closed\n", card_number );
       }
   }
}

