#include "1553_interface.h"

BUS_PACKAGE  busPK_heap[MAX_TRANSACTION_NUMBER];
int          empty_tr = 0;                        // Number of next empty transaction in heap

//=============================================================================
int standart_transaction_initialization( int bus_number,  int remote_terminal,
                                         int sub_address, int word_count,
                                         int transaction_direction             )
{
   int status;
   int transaction_index;
   BUS_PACKAGE* PK;
   
   if( empty_tr > MAX_TRANSACTION_NUMBER )
   {   printf( ">>>>Too many initialized transactions " );
       printf( "in transaction_initialization()\n" );
       return RET_FAIL; /*...........................*/ }
   
   transaction_index  =  Bus_PK_Init( bus_number, remote_terminal,
                                      sub_address, word_count, 
                                      transaction_direction );

   status = tr_idx_to_BUS_PK_conv( transaction_index, PK );
   if( status != RET_OK )
   {   printf( "tr_idx_to_BUS_PK_conv() error in transaction_initialization\n" );
       return RET_FAIL; /*.....................................................*/ }
   
   BT_ABUF_Init( PK->card_number, PK->remote_terminal );
   if( status != RET_OK )
   {   printf( "BT_ABUF_Init() error in transaction_initialization\n" );
       return RET_FAIL; /*...............................................*/ }

   BT_CBUF_Init( PK );
   if( status != RET_OK )
   {   printf( "BT_CBUF_Init() error in transaction_initialization\n" );
       return RET_FAIL; /*...............................................*/ }

   BT_MBUF_Init_NoQ( PK );
   if( status != RET_OK )
   {   printf( "BT_MBUF_Init_MoQ() error in transaction_initialization\n" );
       return RET_FAIL; /*...............................................*/ }

   // START OF DEBUG
   printf( "-----------------------------------------\n" );
   printf( "------Buffers had been initialized\n" );
   printf( "------busPK_heap[%d].init = %d\n", empty_tr-1, busPK_heap[empty_tr-1].init );
   printf( "------PK->bus_number = %d\n", PK->bus_number );
   printf( "------PK->card_number = %d\n", bus_to_card_number[bus_number] );
   printf( "------PK->remote_terminal = %d\n", remote_terminal );
   printf( "------PK->sub_address = %d\n", sub_address );
   printf( "------PK->transaction_direction = %d\n", transaction_direction );
   printf( "------PK->word_count = %d\n", word_count);
   printf( "------PK->last_sub_address = %d\n", PK->sub_address + PK->slots_number - 1 );
   printf( "------PK->slots_number = %d\n", PK->word_count / BT_SLOT_SIZE);
   printf( "------PK->init = %d\n", PK->init );
   printf( "-----------------------------------------\n" );
   // END OF DEBUG
   
   return transaction_index;
}

//=============================================================================

int buffer_transmit( int transaction_index, SWORD* DATA_BUFFER, SWORD buffer_size )
{
   int status;
   int cp;
   int i;
   int i_max;

   API_RT_MBUF_WRITE MBF;
   BUS_PACKAGE *PK;
   LWORD card_number, remote_terminal, sub_address;
   LWORD word_count, last_sub_address, transaction_direction;

   cp = 0;
   
   status = check_tr_entry( transaction_index );
   if( status != RET_OK )
   {   printf( ">>>>Attempt to transmit an uninitialized transaction " );
       printf( "in buffer_transmit()\n" );
       return RET_FAIL; /*............................................*/   }
       
   status = check_bus_PK( transaction_index, BT_TRANSMIT );
   if( status != RET_OK )
   {   printf( ">>>>Error in bus_PK data for transaction idx=%d ",
                                                    transaction_index  );
       printf( "in buffer_transmit()\n" );
       return RET_FAIL; /*.............................................*/  }
   
   //Reading transaction parameters from busPK
   PK = &(busPK_heap[transaction_index]);
   card_number = PK->card_number;
   remote_terminal = PK->remote_terminal;
   sub_address = PK->sub_address;
   word_count = PK->word_count;
   last_sub_address = PK->last_sub_address;
   transaction_direction = PK->transaction_direction;
                                                      
   //Checking buffer size
   if( buffer_size < (sizeof(DATA_BUFFER[0]) * word_count) )
   {  printf( ">>>>Buffer is too small for transaction idx=%d ", 
                                                   transaction_index );
      printf( "in buffer_transmit()\n" );
      printf( "buffer_size = %d\n sizeof(DATA_BUFFER[0]) * PK->word_count = %d",
                                                   buffer_size, sizeof(DATA_BUFFER[0]) * word_count );
      return RET_FAIL; /*..............................................*/ }

   //No interrupts enebled
   MBF.enable = 0;
   MBF.error_inj_id = 0;
  
   while( word_count > 0 )
   {   if( word_count > BT_SLOT_SIZE )
       {   i_max = BT_SLOT_SIZE;
           word_count -= BT_SLOT_SIZE;  }
       else
       {   i_max = word_count;
           word_count = 0;      }
           
       for( i=0; i < i_max; i++ )
       {   
           *(&(MBF.mess_data[i]) - 1) = *( DATA_BUFFER + cp );
           printf( "(TRANSMIT) DATA_BUFFER[%d] = MBF.mess_data[%d] = %04x\n", cp, i, MBF.mess_data[i] );
           cp++;      
       }
           
       status = BusTools_RT_MessageWrite( card_number, 
                                          remote_terminal,
                                          sub_address, 
                                          transaction_direction,
                                          0, &MBF );
       if( status != API_SUCCESS )
       {   printf( ">>>>Message write error via BusTools_RT_MessageWrite() " );
           printf( "in buffer_transmit()\n" );
           return( RET_FAIL ); /*........................................*/   }
           
       sub_address++;
       if( sub_address > PK->last_sub_address )
           break;
   }
   
   return( RET_OK );
}

//=============================================================================
int buffer_receive( int transaction_index, SWORD* DATA_BUFFER, SWORD buffer_size )
{
    int status;
    int sx = 0;          // Slot index
    int cp = 0;          // Current position
    int nw;              // Number of words in current slot
    int i;               // Auxiliary index

    API_RT_MBUF_READ MBR;
    BUS_PACKAGE* PK;
    LWORD card_number, remote_terminal, sub_address, slots_number;
    LWORD word_count, last_sub_address, transaction_direction;
    
    //Checking if transaction had been initialized
    status = check_tr_entry( transaction_index );
    if( status != RET_OK )
    {   printf( ">>>>Attempt to receive an uninitialized transaction \n" );
        printf( "in buffer_receive()" );
        return RET_FAIL; /*.............................................*/   }
   
   //Checking correctness of transaction initialization
   status = check_bus_PK( transaction_index, BT_RECEIVE );
   if( status != RET_OK )
   {   printf( ">>>>Error in bus_PK data for transaction idx=%d ",
                                                    transaction_index  );
       printf( "in buffer_receive()\n" );
       return RET_FAIL; /*.............................................*/  }
    
    //Reading transaction parameters from busPK   
    PK = &(busPK_heap[transaction_index]);
    card_number = PK->card_number;
    remote_terminal = PK->remote_terminal;
    sub_address = PK->sub_address;
    word_count = PK->word_count;
    last_sub_address = PK->last_sub_address;
    transaction_direction = PK->transaction_direction;
    slots_number = PK->slots_number;

    if( buffer_size < (sizeof(DATA_BUFFER[0]) * PK->word_count) )
    {  printf( ">>>>Buffer is too small for transaction idx=%d ",transaction_index );
       printf( "in buffer_receive()\n" );
       printf( "buffer_size = %d, sizeof(DATA_BUFFER[0]) * PK->word_count = %d\n", 
                                          buffer_size, sizeof(DATA_BUFFER[0]) * PK->word_count );
       return RET_FAIL; /*..............................................*/ }
        
    //START OF DEBUG
    printf( "-----------------------------------------\n" );
    printf( "------Start of cycle in buffer_receive()\n");
    printf( "-----------------------------------------\n" );
    //END OF DEBUG  
    
    while( sx < slots_number )
    {   
       //START OF DEBUG
       printf( "-----------------------------------------\n" );
       printf("Info from buffer_receive(): card_number=%d, rt=%d, sa+sx=%d, tr=%d\n",
                 card_number, remote_terminal, sub_address + sx, transaction_direction );
       printf( "-----------------------------------------\n" );
       //END OF DEBUG  
       
       status = BusTools_RT_MessageRead( card_number,        
                                         remote_terminal,
                                         sub_address + sx, 
                                         transaction_direction,
                                         0, &MBR  );
       if( status != API_SUCCESS )
       {   printf(">>>>Message read error 2 via BusTools_RT_MessageRead ");
           printf( "in buffer_receive()\n" );
           printf("cN=%d, SA=%d, RT=%d\n", card_number, 
                                           sub_address, remote_terminal);
           return RET_FAIL; /*...............................................*/ }
                
       nw = (LWORD) MBR.mess_command.wcount;

       if( nw == 0 )
           nw = BT_SLOT_SIZE;  // Value 0 means 32 words in slot

       //START OF DEBUG - 32 here
       printf( "-----------------------------------------\n" );
       printf( "------Info: nw = %d\n", nw );
       printf( "-----------------------------------------\n" );
       //END OF DEBUG

       //START OF DEBUG
       for (i = 0; i < nw; i++ )
            printf( "(RECEIVE) DATA_BUFFER[%d] = MBR.mess_data[%d] = %04x\n", cp+i, i, MBR.mess_data[i] );
       //END OF DEBUG
       memcpy( (void*)(DATA_BUFFER+cp), (void*) (&(MBR.mess_data[0]) - 1), nw*2 );
       cp += nw;
       sx++;
            
       if( nw < BT_SLOT_SIZE )
             break;
    }

    //START OF DEBUG
    printf( "-----------------------------------------\n" );
    printf( "------End of cycle in buffer_receive()\n");
    printf( "-----------------------------------------\n" );
    //END OF DEBUG  

    return( cp );
}

//==============================================================================
int Bus_PK_Init ( int bus_number, int remote_terminal, 
                  int sub_address, int word_count, 
                  int transaction_direction )
{  
   BUS_PACKAGE*  PK;

   PK = &(busPK_heap[empty_tr]);
   empty_tr++;
   
   
   PK->bus_number = (LWORD) bus_number;
   PK->card_number = (LWORD) bus_to_card_number[bus_number];
   PK->remote_terminal = (LWORD) remote_terminal;
   PK->sub_address = (LWORD) sub_address;
   PK->transaction_direction = (LWORD) transaction_direction;
   PK->word_count = (LWORD) word_count;
   if (PK->word_count % BT_SLOT_SIZE != 0)                                         
	PK->slots_number = (LWORD)((PK->word_count / BT_SLOT_SIZE) + 1);           
   else                                                                            
	PK->slots_number = (LWORD) (PK->word_count / BT_SLOT_SIZE);                
   PK->last_sub_address = (LWORD) (PK->sub_address + PK->slots_number - 1);
   PK->ring_size = BT_DEFAULT_RSZ;

   PK->init = L_ON;


   return (empty_tr-1);
}

//=============================================================================
int BUS_PK_Conv( BUS_PACKAGE *PK, LWORD *cN, LWORD *RT, LWORD *SA, LWORD *tr, LWORD *SN )
{
    int status;
    if (PK == NULL)
    {   printf("Null BUS_PK pointer in BUS_PK_Conv()\n");
        return RET_FAIL; /*.............................*/ }

    if (PK->init != L_ON)
    {   printf("Uninitialized BUS_PK in BUS_PK_Conv\n");
        return RET_FAIL; /*.............................*/ }

    *cN = PK->card_number;
    *RT = PK->remote_terminal;
    *SA = PK->sub_address;
    *tr = PK->transaction_direction;
    *SN = PK->slots_number;

    return RET_OK;
}

//=============================================================================
int transaction_index_convert( int transaction_index, LWORD *cN, 
                               LWORD *RT, LWORD *SA, LWORD *tr, LWORD *SN )
{
   int status;
   BUS_PACKAGE *PK;
   
   status = check_tr_entry( transaction_index );
   if( status != RET_OK )
   {   printf("check_tr_entry() fail in transaction_index_convert()\n");
       return RET_FAIL; /*..........................................*/ }

   PK = &(busPK_heap[transaction_index]);
   if (PK->init != L_ON)
   {   printf("Uninitialized BUS_PK in transaction_index_convert\n");
       return RET_FAIL; /*..........................................*/ }

    *cN = PK->card_number;
    *RT = PK->remote_terminal;
    *SA = PK->sub_address;
    *tr = PK->transaction_direction;
    *SN = PK->slots_number;

   return RET_OK;
}

//===============================================================================
int tr_idx_to_BUS_PK_conv( int transaction_index, BUS_PACKAGE* PK )
{
   int status;

   status = check_tr_entry( transaction_index );
   if( status != RET_OK )
   {   printf("check_tr_entry() fail in tr_idx_to_BUS_PK_conv()\n");
       return RET_FAIL; /*..........................................*/ }

   PK = &(busPK_heap[transaction_index]);
   if (PK->init != L_ON)
   {   printf("Uninitialized BUS_PK in tr_idx_to_BUS_PK_conv\n");
       return RET_FAIL; /*..........................................*/ }

   return RET_OK;
}










