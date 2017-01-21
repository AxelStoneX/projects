#include <errno.h>        // Error codes + extern int errno;
#include <stdio.h>
#include <stdarg.h>       // for va_list, va_arg and so on

#include "dbf_dv.h"
// Double buffer for intertask data exchanges - implementation

//============================================================================
void TDB_Init( DBF_TXT *dbF )
{ // Default mutex parameters:PTHREAD_PRIO_INHERIT & PTHREAD_RECURSIVE_DISABLE
  pthread_mutex_init( &( dbF->mu ), NULL );
  dbF->tm = 0;          dbF->rx = 0;
  dbF->lost = 0;	    dbF->trylock_err = 0;
  dbF->lock_err = 0;    dbF->unlock_err  = 0;     dbF->err = 0;
  dbF->RCV = &( dbF->BUF_1[0] );   // At first not reverse state
  dbF->OUT = &( dbF->BUF_2[0] );   dbF->RVS = 0;   dbF->DTL = 0;

  dbF->OF = stdout;   // Standart ouput is default now
} // TDB_Init()

//============================================================================
void TDB_Push( DBF_TXT *dbF )  // @@@_ZZ - do it one tyme in task cycle
{ if( dbF->rx > 0 ) TDB_Try_Switch(dbF);  }  // Switch buffers if it'spossible

//============================================================================
void TDB_Try_Switch( DBF_TXT *dbF )  // Switch OUT/RCV buffers if it's possible
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
void TDB_Show( DBF_TXT *dbF )  // Reader part of double buffer
{ LWORD ix = 0, nx = 0, tm = dbF->tm;
  pthread_mutex_t *pmu = &(dbF->mu);
  BYTE *OUT = dbF->OUT;
  FILE *OF  = dbF->OF;
  int   RS;

  RS = pthread_mutex_lock( pmu );  // Buffers switch shall be disabled
  if( RS != EOK ) { dbF->lock_err++;  return;  }  //@@@zz - errors processing ?
   
  if( dbF->DTL != 0 )  // Data is really loaded in the OUT part of buffer
   { // Seach cycle for all character strings in OUT part of buffer,
	 while( nx < tm )  // print all found strings to the specified file.
      {                // Printing of the empty line is possible
		if( *( OUT + nx ) == '\0' )
		 { fprintf( OF, "%s", ( OUT + ix ) );   nx += 1;   ix = nx;   }
		else
		 { nx++;   }
	  } // while()
     dbF->DTL = 0;
   } // if( dbF->DTL != 0 )
  
  RS = pthread_mutex_unlock( pmu );  // Unlock mutex
  if( RS != EOK )  dbF->unlock_err++;     // @@@zz - errors processing ?
} // TDB_Show()

//============================================================================
void TDB_ShowC( DBF_TXT *dbF )  // Reader part of double buffer
{ LWORD ix = 0, nx = 0, tm = dbF->tm;
  pthread_mutex_t *pmu = &(dbF->mu);
  BYTE *OUT = dbF->OUT;
  FILE *OF  = dbF->OF;
  int   RS, RS1;

  RS = pthread_mutex_lock( pmu );  // Buffers switch shall be disabled
  if( RS != EOK ) { dbF->lock_err++;  return;  }  //@@@zz - errors processing ?
   
  if( dbF->DTL != 0 )  // Data is really loaded in the OUT part of buffer
   { // Seach cycle for all character strings in OUT part of buffer,
	 while( nx < tm )  // print all found strings to the specified file.
      {                // Printing of the empty line is possible
		if( *( OUT + nx ) == '\0' )
		 { fprintf( OF, "%s", ( OUT + ix ) );   nx += 1;   ix = nx;   }
		else if( *( OUT + nx ) == 0x07 )
          { fprintf( OF, "\n\n>>>>>>>>>>>>\n\n" );   nx += 1;  }
		else
		 { nx++;   }
	  } // while()
     dbF->DTL = 0;
   } // if( dbF->DTL != 0 )
  
  RS = pthread_mutex_unlock( pmu );  // Unlock mutex
  if( RS != EOK )  dbF->unlock_err++;     // @@@zz - errors processing ?
} // TDB_ShowC()

//============================================================================
void TDB_print( DBF_TXT *dbF, char *format, ... )
{ // Writer part of double buffer
  LWORD rx = dbF->rx;
  va_list val;  // Arg. list
  char *out;
  int NS;  // number of written symbols
  // Data receiving buffer
  if( rx < TDB_MAX_RX )
	 out = (char*)( dbF->RCV + rx );  // next empty byte
  else
   { dbF->lost++;   return;   }   // buf. overflow, message is lost

  va_start( val, format );  // format string is the last fixed parameter
  NS = vsprintf( out, format, val );
  va_end( val );

  if( NS > 0 )   dbF->rx += (NS+1);
  else         { dbF->lost++;   dbF->err++;  }
} // TDB_print()

//============================================================================
void TDB_puts( DBF_TXT *dbF, char *mess )
{ // Write a text message to double buffer
  LWORD rx = dbF->rx;
  int NS;  // number of written symbols
  char *out;
  // Data receiving buffer
  if( rx < TDB_MAX_RX )
	out = (char*)( dbF->RCV + rx );  // next empty byte
  else
    { dbF->lost++;   return;   }     // buf. overflow, message is lost

  NS = sprintf( out, "%s", mess );
  if( NS > 0 )   dbF->rx += (NS+1);
  else         { dbF->lost++;   dbF->err++;  }
} // TDB_puts()

//=============================================================================
void Show_Hex( const char *head, SWORD *BUF, SWORD nw )
{ SWORD ix, ln = 1, ll = 8, pct = ll;  // ll - line length, pct - param. cnt.
  printf( "\n%s", head );
  for( ix=0; ix < nw; ix++ )
   { if( pct >= ll ) { printf( "\n%2d > ", ln );  ln++;  pct=0;  } // New line
	 printf( " %04x", *(BUF+ix) );                       pct++;       }
  printf( "\n" );
} // Show_Hex()

//=============================================================================
void Show_Hex_DB( DBF_TXT *dbF, const char *head, SWORD *BUF, SWORD nw )
{ SWORD ix, ln = 1, ll = 8, pct = ll;  // ll - line length, pct - param. cnt.
  TDB_print( dbF, "%s", head );
  for( ix=0; ix < nw; ix++ )
   { if( pct >= ll ) { TDB_print( dbF, "\n%2d > ", ln );  ln++;  pct=0;  }
	 TDB_print( dbF, " 0x%04x", *(BUF+ix) );                       pct++;     }
  TDB_print( dbF, "\n" );
} // Show_Hex_DB()

