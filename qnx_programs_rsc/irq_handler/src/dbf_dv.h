#ifndef __OUTPUT_DOUBLE_BUFFERS_H__
#define __OUTPUT_DOUBLE_BUFFERS_H__

#include <stdio.h>        // For FILE*
#include <pthread.h>      // For mutex control

#include "lbm_bs.h"  // Base type and const definitions

// Text output double buffer for intertask communication:
// -> type declarations, externals

#define TDB_OUT_CYCLE 5000000 // TDB output period in us (microseconds) --> 5 s
#define RTC_ISR_CYCLE 500     // RTC cycle period in us

// ===== Text output data buffer =============================================
#define TDB_BUF_SZ  8192               // Buffer size in bytes
#define TDB_MAX_RX (TDB_BUF_SZ - 512)  // Max rx when printb() works

typedef struct // --- Double Buffer data structure ---------------------------
{
  BYTE *RCV, *OUT;     // Receiving / Output buffers pointers
  BYTE  BUF_1[TDB_BUF_SZ];
  BYTE  BUF_2[TDB_BUF_SZ];
  BYTE  DTL, RVS;      // Tran. data are loaded / Reverse state of buffers
  FILE *OF;            // Output file
  pthread_mutex_t mu;  // To control RCV/OUT buffers switch
  LWORD tm, rx;        // Transmission / receiving indexes
  LWORD err;           // vsprintf() error counter
  LWORD lost;          // Lost messages counter
  LWORD lock_err;      // Mutex lock error counter
  LWORD unlock_err;    // Mutex unlock error counter
  LWORD trylock_err;   // Mutex trylock error counter
} DBF_TXT;

extern DBF_TXT DTF;    // Func. task text output double buffer
extern DBF_TXT DTX;    // Exchange task text output double buffer
extern DBF_TXT DTR;    // Regul control task text output double buffer

void TDB_Init( DBF_TXT *dbF );
void TDB_Push( DBF_TXT *dbF );
void TDB_Try_Switch( DBF_TXT *dbF );
void TDB_Show( DBF_TXT *dbF );  // Reader part of double buffer
void TDB_ShowC( DBF_TXT *dbF );  // Reader part of double buffer
void TDB_print( DBF_TXT *dbF, char *format, ... );
void TDB_puts( DBF_TXT *dbF, char *mess );

void Show_Hex( const char *head, SWORD *BUF, SWORD nw );
void Show_Hex_DB( DBF_TXT *dbF, const char *head, SWORD *BUF, SWORD nw );

// ===== Binary file output data buffer =============================================
#define FDB_BUF_SZ  8192               // Buffer size in bytes
#define FDB_MAX_RX (TDB_BUF_SZ - 512)  // Max rx when printb() works
#define FDB_RX_SWT   10                // Minimal data portion for switch
#define FDB_UNDEF  -777
#define FDB_NSZ     64                 // File Name string size

typedef struct // --- Double Buffer data structure ---------------------------
{
  BYTE *RCV, *OUT;     // Receiving / Output buffers pointers
  BYTE  BUF_1[FDB_BUF_SZ];
  BYTE  BUF_2[FDB_BUF_SZ];
  BYTE  DTL, RVS;      // Tran. data are loaded / Reverse state of buffers
  // FILE *OF;         // *** Output file
  pthread_mutex_t mu;  // To control RCV/OUT buffers switch
  LWORD tm, rx;        // Transmission / receiving indexes
  // LWORD err;        // *** vsprintf() error counter
  LWORD sum_p, lost_p; // Summary/Lost packs counters
  LWORD sum_b, lost_b; // Summary/Lost bytes counters
  LWORD sum_fb;        // Written to file bytes summary
  LWORD lock_err;      // Mutex lock error counter
  LWORD unlock_err;    // Mutex unlock error counter
  LWORD trylock_err;   // Mutex trylock error counter
  //--------------------------------------------------
   DBF_TXT *dbT;       // For cyclical text diagnostics
   int   fD;           // Output binary file descriptor
   char *fN;           // File name pointer
   char  fNS[FDB_NSZ]; // File name static buffer
   LWORD  wr_err; 
   int    wr_err_code;
} DBF_BIN;

void FDB_Init( DBF_BIN *dbF, DBF_TXT *p_dbt );  // Set object initial state
int  FDB_Open( DBF_BIN *dbF, char *fNAME );     // Open output file
int  FDB_FileName( DBF_BIN *dbF, char *fNAME ); // Generate open file name
void FDB_Final( DBF_BIN *dbF );
void FDB_Push( DBF_BIN *dbF );
void FDB_Try_Switch( DBF_BIN *dbF );
void FDB_Put( DBF_BIN *dbF, BYTE *Next, LWORD nBytes );
void FDB_Out( DBF_BIN *dbF );  // Reader part of double buffer
void FDB_OutRaw( DBF_BIN *dbF );
void FDB_Show( DBF_BIN *dbF );
void FDB_FinalShow( DBF_BIN *dbF );



#endif // __OUTPUT_DOUBLE_BUFFERS_H__
