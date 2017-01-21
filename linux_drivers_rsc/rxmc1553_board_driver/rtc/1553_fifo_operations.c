#include "1553_interface.h"

//==================================================================================
void BT_AIF_Init(API_INT_FIFO *PIF, MessPL *MPL, P_AIF_FUNC PF, int prio, LWORD cN )
{
    memset( (void*)MPL, 0, sizeof(MessPL) );

    memset( (void*)PIF, 0, sizeof(API_INT_FIFO) );
    PIF->function       = PF;
    PIF->iPriority      = prio;
    PIF->dwMilliseconds = INFINITE;
    PIF->iNotification  = 0;

    // Mutual acceptance, pUser[0] points to MessPL object
    PIF->pUser[MESS_PL_IX] = (void*)MPL;
    MPL->PIF = PIF;
    MPL->CardN = cN;
}

//=======================================================================================
int BT_AIF_SetFilter( API_INT_FIFO *PIF, BUS_PACKAGE *PK, LWORD fType, LWORD fMask )
{   LWORD cN, RT, SA, tr, SN;
    LWORD n;
    cN = PK->card_number;
    RT = PK->remote_terminal;
    SA = PK->sub_address;
    tr = PK->transaction_direction;
    SN = PK->slots_number;
    
    PIF->FilterType = fType;
    for( n=0; n < SN; n++ )
    {   PIF->FilterMask[RT][tr][SA+n] = fMask;
        printf("AIF Filter mask RT=%d tr=%d SA=%d -> 0x%08x\n", RT, tr, SA+n, fMask); }
    
    return RET_OK;
}

//========================================================================================
int BT_AIF_AddFilter( API_INT_FIFO *PIF, BUS_PACKAGE *PK, LWORD fMask )
{   LWORD cN, RT, SA, tr, SN;
    LWORD n;

    cN = PK->card_number;
    RT = PK->remote_terminal;
    SA = PK->sub_address;
    tr = PK-> transaction_direction;
    SN = PK-> slots_number;

    for( n=0; n < SN; n++ )
    {  PIF->FilterMask[RT][tr][SA+n] |= fMask;
       printf("AIF Filter mask RT=%d tr=%d SA=%d -> 0x%08x\n", RT, tr, SA+n, fMask);  }

    return RET_OK;
}

//===========================================================================================
int BT_AIF_SetFilterRTM( API_INT_FIFO *PIF, BUS_PACKAGE *PK )
{   LWORD cN, RT, SA, tr, SN;
    LWORD n;
    LWORD fMask;
    
    cN = PK->card_number;
    RT = PK->remote_terminal;
    SA = PK->sub_address;
    tr = PK-> transaction_direction;
    SN = PK-> slots_number;

    PIF->FilterType = EVENT_RT_MESSAGE;
    for( n=0; n < SN; n++ )
    {  PIF->FilterMask[RT][tr][SA+n] = fMask;
       printf("AIF Filter mask RT=%d tr=%d SA=%d -> 0x%08x\n", RT, tr, SA+n, fMask);  }
    return RET_OK;
}

//=============================================================================================
int BT_AIF_Connect( API_INT_FIFO *PIF )
{
    int status;
    MessPL *MPL;

    status = BT_AIF_Check( PIF );
    if( status != RET_OK )
    {   printf("AIF connect error in BT_AIF_Connect()\n");
        return RET_FAIL; /*..............................*/  }

    MPL = (MessPL*)( PIF->pUser[MESS_PL_IX] );
    
    status = BusTools_RegisterFunction( MPL->CardN, PIF, REGISTER_FUNCTION );
    if( status != API_SUCCESS )
    {   MPL->init = L_OFF;
        printf( "BusTools_RegisterFunction() error in BT_AIF_Connect()\n" );
        return RET_FAIL; /*.................................................*/  }

    MPL->init = L_ON;
    return RET_OK;
}

//==============================================================================================
void BT_AIF_Close( API_INT_FIFO *PIF )
{   int status;
    MessPL *MPL;

    status = BT_AIF_Check( PIF );
    if( status != RET_OK )
    {  printf( "MessPL close error in BT_AIF_Close()\n" );
       return ; /*.......................................*/ }

    MPL = (MessPL*)( PIF->pUser[MESS_PL_IX] );

    if( MPL->init == L_ON )
    {   MPL->init = L_OFF;
        status = BusTools_RegisterFunction( MPL->CardN, PIF, UNREGISTER_FUNCTION );
        if( status != API_SUCCESS )
            printf( "MessPL close error in BT_AIF_Close()\n" );
    }
}

//==============================================================================================
int BT_AIF_Check( API_INT_FIFO *PIF )
{   MessPL *MPL;
 
    if(PIF->pUser[MESS_PL_IX] == NULL)
    {  printf( "Empty AIF in BT_AIF_Check()\n" );
       return (RET_FAIL); /*........................*/ }

    MPL = (MessPL*)(PIF->pUser[MESS_PL_IX]);
    if( MPL->CardN > BT_MAX_cN )
    {  printf( "Bad cN in BT_AIF_Check()\n" );
       return (RET_FAIL); /*..................*/ }
    return (RET_OK);
}
