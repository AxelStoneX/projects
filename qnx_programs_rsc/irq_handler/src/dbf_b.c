#include <errno.h>        // Error codes + extern int errno;
#include <stdio.h>
#include <string.h>       // strlen()
#include <sys/types.h>    // For low level file operations
#include <sys/stat.h>     // -----------------------------
#include <fcntl.h>        // -----------------------------

#include "dbf_dv.h"
// Double buffer for output to the binary file - implementation

//============================================================================
void FDB_Init( DBF_BIN *dbF, DBF_TXT *p_dbt )  // Set object initial state
{ // Default mutex parameters:PTHREAD_PRIO_INHERIT & PTHREAD_RECURSIVE_DISABLE
  pthread_mutex_init( &( dbF->mu ), NULL );
  dbF->tm = 0;          dbF->rx = 0;
  dbF->sum_p = 0;       dbF->lost_p = 0;	    
  dbF->sum_b = 0;       dbF->lost_b = 0;	     dbF->sum_fb = 0;
  dbF->lock_err = 0;    dbF->unlock_err = 0;     dbF->trylock_err = 0;
  dbF->RCV = &( dbF->BUF_1[0] );   // At first not reverse state
  dbF->OUT = &( dbF->BUF_2[0] );   dbF->RVS = 0;   dbF->DTL = 0;
  // dbF->OF = stdout;   // *** Standart ouput is default now
  dbF->dbT = p_dbt;     dbF->wr_err = 0;         dbF->wr_err_code = 0;
  dbF->fN = NULL;       dbF->fD  = 0;    // Before first return
} // FDB_Init()

//============================================================================
int FDB_Open( DBF_BIN *dbF, char *fNAME )  // Open output file 
{ mode_t MODE;
  int    RS, flags;
  // printf( "\nOpen file %s\n", fNAME );
  RS = FDB_FileName( dbF, fNAME );  if( RS != EOK ) return( RS ); // >>>>> 
  // Write only access, create or replace file if it exists
  flags = O_WRONLY | O_CREAT | O_TRUNC;
  // Set read/write permission for owner, group and all other users
  MODE = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH;

  dbF->fD = open( dbF->fN, flags, MODE );
  if( dbF->fD > 0 )
   { printf( "File %s is open\n", dbF->fN );   return( EOK );   }
  else 
   { dbF->fD = FDB_UNDEF;                      return(errno);   }
} // FDB_Open()

//============================================================================
int FDB_FileName( DBF_BIN *dbF, char *fNAME ) // Generate open file name
{ int nsz = strlen( fNAME );                  // "No enought memory" error
  if( nsz >= FDB_NSZ-5 ) { printf("\nLong file name");  return(ENOMEM);  }
  dbF->fN = &( dbF->fNS[0] );  // Static name buf.
  strcpy( dbF->fN, fNAME );  /* strcat( dbF->fN, ".raw" ); */  // Add prefix to name
  return( EOK );
} // FDB_FileName()

//============================================================================
void FDB_Final( DBF_BIN *dbF )
{ int RS, ix,TERM = 0xEEEECCCC;
  if( dbF->fD != 0 )
   { 
     //for(ix=0;ix<8;ix++) RS = write( dbF->fD, (const void*)(&TERM), 4 ); 
     // FDB_FinalShow( dbF );   // Only when result file is available
     RS = close( dbF->fD );  dbF->fD = FDB_UNDEF;
     if( RS == RET_OK ) printf( "\nFile %s is closed\n", dbF->fN );
     else               perror( "\nBIN dbf close error" );            }
} // FDB_Final()

//============================================================================
void FDB_Push( DBF_BIN *dbF ) // Switch buffers if it'spossible -> do it one 
{ // if( dbF->rx > FDB_RX_SWT ) 
  FDB_Try_Switch(dbF);  }   // time in task cycle

//============================================================================
void FDB_Try_Switch( DBF_BIN *dbF )  // Switch OUT/RCV buffers if it's possible
{ int RST, RSU;
  RST = pthread_mutex_trylock( &(dbF->mu) );  // Data output is terminated ?
  if( RST == EBUSY  ) return;  // >>> OUT buffer transmission is in process

  if( RST == EOK )  // Mutex is taken
   { // TLM Double buffer mutex now is locked by the current task

	 if( dbF->DTL == 0 )   // OUT buffer is already transmitted and empty
      { // Buffers switch is possible
		if( dbF->RVS == 0 )  // --- Not reverse state of buffers
		 { dbF->RCV = &( dbF->BUF_2[0] );   // Set reverse state
		   dbF->OUT = &( dbF->BUF_1[0] );   dbF->RVS = 1;  }
        else                // --- Reverse state of buffers
		 { dbF->RCV = &( dbF->BUF_1[0] );   // Set not reverse state
		   dbF->OUT = &( dbF->BUF_2[0] );   dbF->RVS = 0;  }

		dbF->tm = dbF->rx;   dbF->rx = 0;   dbF->DTL = 1;
      } // if( dbF->DTL == 0 ) ...

	 RSU = pthread_mutex_unlock( &(dbF->mu) );  // Unlock mutex
	 if( RSU != EOK )  dbF->unlock_err++;     // @@@zz - errors processing ?
   }
  else
   { dbF->trylock_err++; }  // @@@zz --> error message ???
} // TDB_Try_Switch()

//============================================================================
void FDB_Put( DBF_BIN *dbF, BYTE *Next, LWORD nBytes )
{ // Data receiving buffer
  BYTE *RCV = dbF->RCV;
  LWORD rx  = dbF->rx;
  
  if( dbF->rx + nBytes < FDB_MAX_RX )
   { memcpy( (void*)(RCV+rx ), (const void*)Next, nBytes );
     dbF->sum_p++;    dbF->sum_b += nBytes;     dbF->rx += nBytes;    }
  else
   { dbF->lost_p++;   dbF->lost_b += nBytes;    } 
} // FDB_Put()

//============================================================================
void FDB_Out( DBF_BIN *dbF )  // Reader part of double buffer
{ int   RS;  // ssize_t is same as _INT32
  pthread_mutex_t *PMU;

  PMU = &(dbF->mu);
  RS = pthread_mutex_lock( PMU );  // Buffers switch shall be disabled
  if( RS != EOK ) { dbF->lock_err++;  return;  }   // @@@zz - processing ?
   
  if( dbF->DTL != 0 )
   { dbF->DTL = 0;     FDB_OutRaw( dbF );      }
   
  RS = pthread_mutex_unlock( PMU );   
  if( RS != EOK ) dbF->unlock_err++;     // @@@zz - errors processing ?
} // DBF_KSO::Out()

//============================================================================
void FDB_OutRaw( DBF_BIN *dbF )
{ int   RS, fD = dbF->fD;
  LWORD tm = dbF->tm;
  
  if( fD == FDB_UNDEF ) return;  // >>> File is not open
  RS = write( fD, (const void*)(dbF->OUT), tm  ); 
  if( RS != RET_FAIL ) { dbF->sum_fb += tm;  }    
  else                 { dbF->wr_err++;   dbF->wr_err_code = errno;  }
} //  FDB_OutRaw() 

//============================================================================
void FDB_Show( DBF_BIN *dbF )
{ DBF_TXT *dbT = dbF->dbT;
  if( dbT == NULL ) return; // >>>>>
  if( dbF->fN == NULL  ) return; // >>>>>
  // TDB_print( dbT, "File '%s' >>>\n", dbF->fN ); 
  TDB_print( dbT, "sum_p =%d : %d,  sum_b =%d : %d, sum_fb =%d, wr_err=%d\n", 
   dbF->sum_p, dbF->lost_p, dbF->sum_b, dbF->lost_b, dbF->sum_fb, dbF->wr_err);                   
} // FDB_Show() 

//============================================================================
void FDB_FinalShow( DBF_BIN *dbF )
{
 if( dbF->fN == NULL ) return;
 printf( "\nFile %s processing:\n", dbF->fN );
 printf( "sum_p =%d : %d,  sum_b =%d : %d, sum_fb =%d, wr_err=%d", 
   dbF->sum_p, dbF->lost_p, dbF->sum_b, dbF->lost_b, dbF->sum_fb, dbF->wr_err);                   
} // FDB_FinalShow()


