#include "1553_interface.h"

//=============================================================================
int BT_ABUF_Init( LWORD cN, LWORD RT )
{  API_RT_ABUF ABF;
   int status;

   ABF.enable_a = 1;
   ABF.enable_b = 1;
   ABF.command  = 0;
   ABF.bit_word = 0;
   ABF.status = 0x0000;
   ABF.inhibit_term_flag = 0;

   status = BusTools_RT_AbufWrite( cN, RT, &ABF );
   if( status != API_SUCCESS )
   {  printf( "BusTools_RT_AbufWrite() error in BT_ABUF_Init\n");
      printf( "status = %d\n", status );
      printf( "card_number = %d\n", cN );
      return RET_FAIL; /*......................................*/ }
   
   return RET_OK;
}

//=============================================================================
int BT_CBUF_Init( BUS_PACKAGE *PK )
{  API_RT_CBUF CBF;
   LWORD cN, RT, SA, tr, SN; 
   LWORD RSZ, sx;
   int status;

   status = BUS_PK_Conv( PK, &cN, &RT, &SA, &tr, &SN );
   RSZ = PK->ring_size;
   if( status != RET_OK )
   {  printf( "BUS_PK_Conv() error in BT_CBUF_Init()\n" );
      return RET_FAIL; /*.................................*/  }

   CBF.legal_wordcount = BT_WCMASK_ALL;  //Enable all possible wc
   for( sx=0; sx < SN; sx++ )
   {  status = BusTools_RT_CbufWrite( cN, RT, SA+sx, tr, RSZ, &CBF );
      printf( "CbufWrite cN=%d RT=%d SA=%d tr=%d RSZ=%d ",
                                            cN, RT, SA+sx, tr, RSZ );
      if( status != API_SUCCESS)
      {   printf("is failed\n");
          return RET_FAIL;        }

      printf("is correct\n" );
      return RET_OK;
   }
}

//=============================================================================
int BT_CBUFBroad_InitOne( BUS_PACKAGE *PK, LWORD mask )
{   API_RT_CBUFBROAD CBF;  //One slot is allocated
    LWORD cN, RT, SA, tr, SN;
    int status, i;

    status = BUS_PK_Conv( PK, &cN, &RT, &SA, &tr, &SN );
    if( status != RET_OK )
    {  printf( "BUS_PK_Conv() error in BT_CBUFBroad_InitOne\n " );
       return RET_FAIL; /*.........................................*/ }
    if( RT != BT_BROAD_ADDR )
    {  printf( "CBUFBroad RT=%d\n", RT );
       return RET_FAIL; /*..........................................*/ }
    if( tr != BT_RECEIVE )
    {  printf( "CBUFBroad tr=%d\n", tr );
       return RET_FAIL; /*..........................................*/ }

     CBF.mbuf_count = 1;    // 1 buffer to be allocated
     // 31 (not 32) elements of CBF.legal_wordcount[i] !!!
     for( i=0; i < CBUFBROAD_MASK_N; i++ )
        CBF.legal_wordcount[i] = mask;

     printf( "CBUFBroad cN=%d SA=%d tr=%d mask=0x%08x\n" ,cN,SA,tr,mask);
     status = BusTools_RT_CbufbroadWrite( cN, SA, tr, &CBF );

     if( status != API_SUCCESS )
     {  printf( " BusTools_RT_CbufbroadWrite() error" ); 
        printf( " in BT_CBUFBroad_InitOne()\n" );
        return RET_FAIL; /*..........................................*/ }

     return RET_OK;
}

//=============================================================================
int BT_MBUF_Init_NoQ( BUS_PACKAGE *PK )
{   API_RT_MBUF_WRITE MBF;
    LWORD cN, RT, SA, tr, SN, sx;
    int status, i;

    status = BUS_PK_Conv( PK, &cN, &RT, &SA, &tr, &SN );
    if( status != RET_OK )
    {   printf("BUS_PK_Conv() error in BT_MBUF_Init_NoQ()\n");
        return RET_FAIL; /*..................................*/ }

    MBF.enable = 0;
    MBF.error_inj_id = 0;

   for( i=0; i < BT_SLOT_SIZE; i++ )
   {  *(&(MBF.mess_data[i]) - 1) = 0xFFFF;
      printf("MBF.mess_data[%d] = %d\n", i, MBF.mess_data[i] );  }

   for( sx=0; sx < SN; sx++ )
   {   printf("MBUF write: cN=%d, RT=%d, SA=%d, tr=%d\n", 
                                     cN, RT, (SA + sx), tr );

       status = BusTools_RT_MessageWrite( cN, RT, (SA+sx), tr, 0, &MBF  );
                                          
       if( status != API_SUCCESS )
       {    printf( ">>>>>BusTools_RT_MessageWrite() error " );
            printf( "in BT_MBUF_Init_NoQ()\n" );
            return RET_FAIL; /*...............................*/ }
   }
   return RET_OK;
}

//=============================================================================
int BT_MBUF_Init( BUS_PACKAGE *PK )
{
    API_RT_MBUF_WRITE MBF;
    LWORD cN, RT, SA, tr, SN, sx;
    int status, i;

    status = BUS_PK_Conv( PK, &cN, &RT, &SA, &tr, &SN );
    if( status != RET_OK )
    {   printf("BUS_PK_Conv() error in BT_MBUF_Init_NoQ()\n");
        return RET_FAIL; /*..................................*/ }

    // End of message interrupts are enabled
    MBF.enable = BT1553_INT_END_OF_MESS;      MBF.error_inj_id = 0;

   for( i=0; i < BT_SLOT_SIZE; i++ )
   {  *(&(MBF.mess_data[i]) - 1) = 0xFFFF;
      printf("MBF.mess_data[%d] = %d\n", i, MBF.mess_data[i] );  }

   for( sx=0; sx < SN; sx++ )
   {   printf("MBUF write: cN=%d, RT=%d, SA=%d, tr=%d\n", 
                                     cN, RT, (SA + sx), tr );

       status = BusTools_RT_MessageWrite( cN, RT, (SA+sx), tr, 0, &MBF  );
                                          
       if( status != API_SUCCESS )
       {    printf( ">>>>>BusTools_RT_MessageWrite() error " );
            printf( "in BT_MBUF_Init_NoQ()\n" );
            return RET_FAIL; /*...............................*/ }
   }
   return RET_OK;
}
    














