#include "1553_interface.h"

int empty_tr = 0;
static BUS_PACKAGE busPK_heap[MAX_TRANSACTION_NUMBER];

//=============================================================================
int buffers_initialization( int bus_number,  int remote_terminal,
                            int sub_address, int word_count,
                            int transaction_direction             )
{
   int status;
   int i;
   API_RT_ABUF       ABF;
   API_RT_CBUF       CBF;
   API_RT_MBUF_WRITE MBF;
   BUS_PACKAGE*      PK;
   
   if( empty_tr > MAX_TRANSACTION_NUMBER )
   {   printf( ">>>>Too many initialized transactions " );
       printf( "in buffers_initialization()\n" );
       return RET_FAIL; /*...........................*/ }
   
   PK = &(busPK_heap[empty_tr]);
   empty_tr++;
   
   
   PK->bus_number = bus_number;
   PK->card_number = bus_to_card_number[bus_number];
   PK->remote_terminal = remote_terminal;
   PK->sub_address = sub_address;
   PK->DATA_BUFFER = NULL;
   PK->transaction_direction = transaction_direction;
   PK->word_count = word_count;
   PK->last_sub_address = PK->sub_address + PK->slots_number - 1;
   PK->slots_number = PK->word_count / BT_SLOT_SIZE;
   
   ABF.enable_a = 1;   ABF.command = 0;   ABF.status = 0x0000;
   ABF.enable_b = 1;   ABF.bit_word = 0;  ABF.inhibit_term_flag = 0;
   
   status = BusTools_RT_AbufWrite( PK->card_number, 
                                   PK->remote_terminal, &ABF );
   if( status != API_SUCCESS )
   {   printf( ">>>>>>BusTools_RT_AbufWrite() error " );
       printf( "in buffers_initialization()\n" );
       return RET_FAIL; /*....................*/  }
       
   CBF.legal_wordcount = 0xFFFFFFFF;
       
   for( i=0; i < PK->slots_number; i++ )
   {   status = BusTools_RT_CbufWrite( PK->card_number,     
                                       PK->remote_terminal,
                                       (PK->sub_address)+i, 
                                       PK->transaction_direction,
                                       1, &CBF );
       if( status != API_SUCCESS )
       {   printf( ">>>>BusTools_RT_CbufWrite() error for " );
           printf( "in buffers_initialization()\n" );
           return RET_FAIL; /*........................*/  }
   }
   
   MBF.enable = 0;
   MBF.error_inj_id = 0;
   for( i=0; i < BT_SLOT_SIZE; i++ )
      MBF.mess_data[i] = 0;
      
   for( i=0; i < PK->slots_number; i++ )
   {   status = BusTools_RT_MessageWrite( PK->card_number,
                                          PK->remote_terminal,
                                          (PK->sub_address)+i,
                                          PK->transaction_direction,
                                          0, &MBF  );
                                          
       if( status != API_SUCCESS )
       {    printf( ">>>>>BusTools_RT_MessageWrite() error " );
            printf( "in buffers_initialization()\n" );
            return RET_OK; /*...............................*/ }          
   }
   
   PK->init = L_ON;
   
   return (empty_tr - 1);
}

//=============================================================================

int buffer_transmit( int transaction_index,
                     ushort_t* DATA_BUFFER )
{
   API_RT_MBUF_WRITE MBF;
   int status;
   LWORD word_count;
   int cp = 0;
   int i;
   int i_max;
   BUS_PACKAGE *PK;
   
   status = check_tr_entry( transaction_index );
   if( status != RET_OK )
   {   printf( ">>>>Attempt to transmit an uninitialized transaction " );
       printf( "in buffer_transamit()\n" );
       return RET_FAIL; /*............................................*/   }
       
   status = check_bus_PK( transaction_index, BT_TRANSMIT );
   if( status != RET_OK )
   {   printf( ">>>>Error in bus_PK data for transaction idx=%d ",
                                                    transaction_index  );
       printf( "in buffer_transmit()\n" );
       return RET_FAIL; /*.............................................*/  }
       
   PK = &(busPK_heap[transaction_index]);
   PK->DATA_BUFFER = DATA_BUFFER;
   
   MBF.enable = 0;
   MBF.error_inj_id = 0;
   
   while( word_count > 0 )
   {   if( word_count > BT_SLOT_SIZE )
       {   i_max = BT_SLOT_SIZE;
           word_count -= BT_SLOT_SIZE;  }
       else
       {   i_max = word_count;
           word_count = 0;    }
           
       for( i=0; i < i_max; i++ )
       {   MBF.mess_data[i] = *( DATA_BUFFER + cp++ );
           cp++;                             }
           
       status = BusTools_RT_MessageWrite( PK->card_number, 
                                          PK->remote_terminal,
                                          PK->sub_address, 
                                          PK->transaction_direction,
                                          0, &MBF );
       if( status != API_SUCCESS )
       {   printf( ">>>>Message write error via BusTools_RT_MessageWrite() " );
           printf( "in buffer_transmit()\n" );
           return( RET_FAIL ); /*........................................*/   }
           
       PK->sub_address++;
       if( PK->sub_address > PK->last_sub_address )
           break;
   }
   
   return( RET_OK );
}

//=============================================================================
int buffer_receive( int   transaction_index, 
                    void* DATA_BUFFER, LWORD buffer_size )
{
    API_RT_MBUF_READ  MBR;
    int status;
    int sx = 0;          // Slot index
    int cp = 0;          // Current position
    int nw;              // Number of words in current slot
    BUS_PACKAGE* PK;
    
    status = check_tr_entry( transaction_index );
    if( status != RET_OK )
    {   printf( ">>>>Attempt to transmit an uninitialized transaction \n" );
        printf( "in buffer_transamit()" );
        return RET_FAIL; /*.............................................*/   }
        
   status = check_bus_PK( transaction_index, BT_RECEIVE );
   if( status != RET_OK )
   {   printf( ">>>>Error in bus_PK data for transaction idx=%d ",
                                                    transaction_index  );
       printf( "in buffer_receive()\n" );
       return RET_FAIL; /*.............................................*/  }
       
    PK = &(busPK_heap[transaction_index]);
    PK->DATA_BUFFER = DATA_BUFFER;

    status = BusTools_RT_MessageRead( PK->card_number, 
                                      PK->remote_terminal,
                                      PK->sub_address, 
                                      PK->transaction_direction,
                                      0,  &MBR  );
                                      
    if( status != API_SUCCESS )
    {   printf( ">>>>Message read error 1 via BusTools_RT_MessageRead " );
        printf( "in buffer_receive()\n" );
        return RET_FAIL; /*.............................................*/  }
        
    
    while( sx < PK->slots_number )
    {   if( sx > 0 )
        {   status = BusTools_RT_MessageRead( PK->card_number,        
                                              PK->remote_terminal,
                                              (PK->sub_address + sx), 
                                              PK->transaction_direction,
                                              0, &MBR  );
            if( status != API_SUCCESS )
            {   printf(">>>>Message read error 2 via BusTools_RT_MessageRead ");
                printf( "in buffer_receive()\n" );
                printf("cN=%d, SA=%d, RT=%d", PK->card_number, 
                                           PK->sub_address, PK->remote_terminal);
                return RET_FAIL; /*...............................................*/ }
                
            nw = (LWORD) MBR.mess_command.wcount;
            if( nw == 0 )
                nw = BT_SLOT_SIZE;  // Value 0 means 32 words in slot
            // Copy received data
            memcpy( (void*)(DATA_BUFFER+cp), (void*)&MBR.mess_data[0], nw*2 );    
            cp += nw;
            sx++;
            
            if( nw < BT_SLOT_SIZE )
                break;
          }
    }
    return( cp );
}