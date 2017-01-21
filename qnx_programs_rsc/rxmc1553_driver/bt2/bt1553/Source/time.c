/*============================================================================*
 * FILE:                        T I M E. C            
 *============================================================================*
 *
 * COPYRIGHT (C) 1994 - 2010 BY
 *          GE INTELLIGENT PLATFORMS, INC., SANTA BARBARA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY GE
 *          INTELLIGENT PLATFORMS.
 *
 *===========================================================================*
 *
 * FUNCTION:   BusTools/1553-API Library:
 *             This file contains routines to handle time and timetag
 *             operations for the API.
 *             Note that no other modules should have any knowledge or any
 *             understanding of the contents of the BT1553_TIME structure (so
 *             we can change it later if we want to).
 *
 *     Currently, the structure is very simple -- there is a 45-bit value which
 *     consists of a 32-bit LSW and a 16-bit MSW (with the MSBs not used) which 
 *     specify an exact time:
 *     -  The number of microseconds since the channel was started
 *        (giving a total  time limit of only 407 days).
 *
 *     Note that time is created and handled as follows:
 *        1) time is derived from two items:
 *           -  a timetag obtained from the hardware (a 32/45 bit counter
 *              driven by the 50 ns/1 us on-board clock).
 *              > 32-bit 50 ns timer overflows every ~214 seconds (~3.5 minutes)
 *              > 32-bit 1 us timer overflows every 71.58 minutes
 *              > 45-bit 1 us timer overflows every 1.12 years
 *           -  a cummulative elapsed time stored and updated in this
 *              file keeping track of total time since the start of the sim.
 *              > 48-bit 1 us timer overflows every 8.93 years
 *        2) when the board's timetag counter overflows, an interrupt is
 *           generated -- the interrupt calls the update routine in this
 *           module so that we can keep track of the total time elapsed.
 *        3) every message in the hardware is stamped with a timetag -- the
 *           software then converts the elapsed time entry to actual time.
 *     
 * USER ENTRY POINTS: 
 *      BusTools_TimeGetString   - Converts a time value to a printable string
 *      BusTools_TimeGetStringLV - LabView interface
 *      BusTools_TimeTagMode     - Setup time tag parameters & display options
 *      BusTools_TimeTagInit     - Resets elapsed time counters and the
 *                                 timetag counter in the hardware
 *      BusTools_TimeTagRead     - Reads the time tag from the hardware
 *      BusTools_TimeTagWrite    - Writes the hardware time tag
 *      BusTools_SetTimeIncrement- Setups up to use auto-icrement the Time Tag register
 *      BusTools_IRIG_Calibration- Calibrate the IRIG DAC
 *      BusTools_IRIG_Config     - Sets up the IRIG control register initial DAC register value
 *      BusTools_IRIG_SetTime    - Sets the internal IRIG time
 *      BusTools_IRIG_SetBias    - Sets a bias time used to synchronize times
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *      bt_memcpy            - memcpy() replacement which only moves WORDS.
 *      DumpTimeTag          - Debug dump time tag memory helper function.
 *      TimeTagClearFlag     - Called to indicate that there is no need to
 *                             do special wrap-around processing on time
 *                             conversion (sets time_previous=time_current)
 *      TimeTagConvert       - Converts timetag from h/w value to usecs
 *      TimeTagInterrupt     - Counts the timetag overflow interrupts.
 *      TimeTagZeroModule    - Initializes Time Tag module on API cardnum startup.
 *===========================================================================*/

/* $Revision:  6.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/06/1999   Converted time keeping functions to support future IRIG and/or
               GPS timing.V3.03.ajh
  04/20/1999   Added BusTools_TimeGetStringLV() function.V3.03.ajh
  09/01/1999   Added BusTools_SetTimeTag() function prototype.V3.20.ajh
  10/01/1999   Changed bt_memcpy() to move aligned 16-bit words only for the
               IP-1553 on the VXI backplane.  Needs more work.  V3.20.ajh
  11/29/1999   Changed to support 45-bit time tag counter in hardware.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  01/25/2000   Fixed IP-1553 V1-V4 time tag problem (tt was divided by 20) that
               was introduced in V4.00.  Changed TimeGetString to not return a
               full IRIG to M/D string if day is zero.V4.01.ajh
  02/23/2000   Modified the function BusTools_TimeGetString to get the current
               year from the host.  Changed the time tag interrupt and clear
               functions to fix the time tag problem when BusTools_BM_MessageRead
               is called around the time that the tag overflows.V4.01.ajh
  08/19/2000   Changed BusTools_TimeGetString() function to correctly display
               "long" values when compiled in 16-bit mode.V4.11.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  11/30/2000   Fixed Init to zero time tag load register and SW shadow.V4.25.ajh
  01/03/2002   Improve platform and O/S portability.v4.46
  03/15/2002   Add IRIG support.v4.48
  06/05/2002   Add IRIG support for IP-D1553 and VME-1553 v4.52
  02/25/2003   Add IRIG support for QPCI-1553,  QPMC-1553, and PCC-1553
  02/19/2004   Change the time tag read function for newer F/W to read successive buffers
  06/25/2004   Change BusTools_IRIG_SetTime to account day offset and allow for either gmtime
               or localtime functions
  01/02/2006   Update for portibility
  11/19/2007   Added code to disable the IRIG during calibration. In BusTools_IRIG_Calibration.
  11/19/2007   Added code in BusTools_IRIG_SetTime to take time format of dddhhmmss.
  06/25/2009   Move get_48BitHostTimeTag to O/s specific files.

 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "busapi.h" 
#include "apiint.h"
#include "globals.h"

/*---------------------------------------------------------------------------*
 *                     Local Data Base Variables
 *---------------------------------------------------------------------------*/
static int TTDisplay_fmt = {API_TTD_RELM};    // Time Tag display format

static BT1553_TIME time_current [MAX_BTA];    // Time in one microsecond ticks.
static BT1553_TIME time_previous[MAX_BTA];    // Range is 3257+ days.
static BT1553_TIME TTLR_Shadow[MAX_BTA];      // Time Tag Load Register Shadow.
static int         TTInit_type[MAX_BTA];      // Time Tag initialization type
static int         TTMode_Def[MAX_BTA];       // Time Tag operational Mode
static BT_U32BIT   TTPeriod_Def[MAX_BTA];     // External TT sync pulse period (us)
                                              //  or overflow period.
#if defined(__WIN32__)
// Pointer to user function BusTools_TimeTagGet() in DLLname.dll:
static NOMANGLE BT_INT (CCONV *pBusTools_TimeTagGet[MAX_BTA])(
   BT_UINT cardnum,               // (i) card number (0 - based)
   BT1553_TIME * timetag);    // (i) pointer to time tag structure
#endif


/*---------------------------------------------------------------------------*
 *              Internal Prototypes
 *---------------------------------------------------------------------------*/


/****************************************************************************
*
*  PROCEDURE - DumpTimeTag
*
*  FUNCTION
*     Debug dump Time Tag memory helper function.
*
****************************************************************************/

void DumpTimeTag(
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile)       // (i) handle of output file
{
#if 0 //Not used
   char *TTD[3] = {"API_TTD_RELM",
                   "API_TTD_IRIG",
                   "API_TTD_DATE"};
#endif

   char *TTI[4] = {"API_TTI_ZERO",
                   "API_TTI_DAY",
                   "API_TTI_IRIG",
                   "API_TTI_EXT"};

   char *TTM[6] = {"API_TTM_FREE",
                   "API_TTM_RESET",
                   "API_TTM_SYNC",
                   "API_TTM_RELOD",
                   "API_TTM_IRIG",
                   "API_TTM_AUTO"};               


   fprintf(hfMemFile, "Previous Time Tag = %4.4hX.%8.8X, Current = %4.4hX.%8.8X\n"
           "Time Tag Mode = %s, TT Init Type = %s, Period = %d\n\n",
           time_previous[cardnum].topuseconds, time_previous[cardnum].microseconds,
           time_current[cardnum].topuseconds, time_current[cardnum].microseconds,
           TTM[TTMode_Def[cardnum]], TTI[TTInit_type[cardnum]], TTPeriod_Def[cardnum]);

}

/****************************************************************************
*
*  PROCEDURE - TimeTagClearFlag
*
*  FUNCTION
*     This routine is called to indicate that there is no need to do special
*     wrap-around processing on time conversion.  This is not needed by the
*     boards, which incorporate 45-bit HW time tag counters.
*
*     It is called about 1/4 of the way through a timer wrap around,
*     for 32-bit/50 ns HW about 58 seconds after a timer wrap has occured,
*     for 32-bit/1 us HW  about 1074 seconds after a timer wrap has occured.
*
****************************************************************************/
void TimeTagClearFlag(
   BT_UINT   cardnum)      // (i) Card number.
{
   /*******************************************************************
   *  We are far enough into a timer wrap time that all message that
   *   occured before the wrap-around have been processed and are gone.
   *  Get ready for the next wrap-around of the counter.
   *******************************************************************/

   AddTrace(cardnum, NTIME_TAG_CLEARFLAG, 0,
            time_previous[cardnum].topuseconds, time_previous[cardnum].microseconds,
            time_current[cardnum].topuseconds,  time_current[cardnum].microseconds);

   time_previous[cardnum] = time_current[cardnum];
   return;
}

/****************************************************************************
*
*  PROCEDURE - TimeTagConvert
*
*  FUNCTION
*     This routine converts the current 45-bit hardware timetag 
*     to 48-bit microseconds as follows:
*
*            time_actual = time_tag + time_current.
*
*     where time_current is the initial time setup by BusTools_SetTimeTag().
*
****************************************************************************/
void TimeTagConvert(
   BT_UINT          cardnum,      // (i) Card number.
   BT1553_TIME *time_tag,     // (i) 45-bit Hardware value to be updated.
   BT1553_TIME *time_actual)  // (o) Pointer to resulting 48-bit time.
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_U32BIT    microseconds;     // Lower time-tag dword temp.
   BT_U16BIT    topuseconds;      // Upper time-tag word temp.
   BT_U32BIT    micro_save;
   /***********************************************************************
   *  Add supplied hardware timetag to initial time.
   ***********************************************************************/
   micro_save = flips(time_tag->microseconds);
   time_tag->microseconds = micro_save;

   microseconds = time_current[cardnum].microseconds + irig_bias[cardnum] + time_tag->microseconds; 
   topuseconds  = (BT_U16BIT)
                 (time_current[cardnum].topuseconds  + time_tag->topuseconds);

   // Add in the carry if there is one.
   if ( microseconds < time_tag->microseconds)    // Is there a carry?
      topuseconds++;                               // Yes, add it in...

   /*******************************************************************
   *  Return the total time (time_tag + time_current[cardnum]).
   *******************************************************************/

   time_actual->topuseconds = topuseconds;
   time_actual->microseconds = microseconds;
   return;
}

/****************************************************************************
*
*  PROCEDURE - TimeTagInterrupt
*
*  FUNCTION
*     Handle the timetag overflow interrupt queue entry.  
*     This function gets called when the discrete input toggles and causes
*     the TT counter to get loaded from the TT load register, or
*     when the discrete input clears the time tag counter.
****************************************************************************/

BT_U32BIT TimeTagInterrupt(
   BT_UINT   cardnum)      // (i) Card number.
{
   BT_U32BIT TimeToClear;  // Time until TimeTagClearFlag() should be called.

   /*******************************************************************
   *  Save current time in ticks as previous time in ticks.
   *******************************************************************/
   time_previous[cardnum] = time_current[cardnum];

   /*******************************************************************
   *  Increment current time by one tick.
   *******************************************************************/

   /****************************************************************
   * A timetag overflow means that the discrete input has toggled
   *  and caused the time tag counter to either be zero'ed or be
   *  reloaded from the time tag load register.  We need to
   *  compute the new TT load register value and write it to the
   *  HW, or update the software "time_current" value...
   ****************************************************************/
   switch ( TTMode_Def[cardnum] )
   {
   case API_TTM_FREE:  // TT counter free runs
      AddTrace(cardnum, NTIME_TAG_INTERRUPT, 222,
               time_previous[cardnum].topuseconds, time_previous[cardnum].microseconds,
               time_current[cardnum].topuseconds,  time_current[cardnum].microseconds);
      break;
   case API_TTM_RESET: // TT counter reset to zero on external pulse
      time_current[cardnum].microseconds += TTPeriod_Def[cardnum];
      if ( time_current[cardnum].microseconds < TTPeriod_Def[cardnum] ) // Carry?
         time_current[cardnum].topuseconds++;                           // Yes...
      break;
   case API_TTM_SYNC:  // TT counter sync'ed to external pulse train
      TTLR_Shadow[cardnum].microseconds += TTPeriod_Def[cardnum];
      if ( TTLR_Shadow[cardnum].microseconds < TTPeriod_Def[cardnum] ) // Carry?
         TTLR_Shadow[cardnum].topuseconds++;                           // Yes...
      BusTools_TimeTagWrite(cardnum, &TTLR_Shadow[cardnum], 0);
      break;
   case API_TTM_RELOD: // Reload previous value into TT counter
         BusTools_TimeTagWrite(cardnum, &TTLR_Shadow[cardnum], 0);
      break;
   }
   TimeToClear = 0;    // Timer callback for TimeTagClearFlag() not needed.

   AddTrace(cardnum, NTIME_TAG_INTERRUPT, TimeToClear,
            time_previous[cardnum].topuseconds, time_previous[cardnum].microseconds,
            time_current[cardnum].topuseconds,  time_current[cardnum].microseconds);
   return TimeToClear;
}

/****************************************************************************
*
*  PROCEDURE - TimeTagZeroModule
*
*  FUNCTION
*         Initializes Time Tag module on API cardnum startup.
*
****************************************************************************/
void TimeTagZeroModule(
   BT_UINT   cardnum)      // (i) Card number.
{
   time_previous[cardnum].microseconds = 0;   // Base time is zero.
   time_previous[cardnum].topuseconds  = 0;
   time_current[cardnum].microseconds  = 0;
   time_current[cardnum].topuseconds   = 0;
   TTLR_Shadow[cardnum].topuseconds  = 0;         // Time Tag Load Reg Shadow.
   TTLR_Shadow[cardnum].microseconds = 1000000L;  // Time Tag Load Reg Shadow.
   TTInit_type[cardnum]  = API_TTI_ZERO;      // Time Tag initialization type
   TTMode_Def[cardnum]   = API_TTM_FREE;      // Time Tag operational Mode
   TTPeriod_Def[cardnum] = 0L;                // External TT sync pulse period (us)
   sched_tt_clear[cardnum] = 0;               // TimeTagClearFlag not scheduled.
}


/*---------------------------------------------------------------------------*
 *                      EXTERNAL USER ENTRY POINTS
 *---------------------------------------------------------------------------*/
/****************************************************************************
*
*  PROCEDURE - BusTools_TimeGetString
*
*  FUNCTION
*     This routine is used to convert a BusTools time structure into
*     a string suitable for display to a user, containing ASCII days,
*     hours, minutes, seconds, microseconds.
*     The input time is a 48-bit binary value in microseconds.  Since most
*     compilers do not support 48- or 64-bit integer operations, we have to
*     play some games.  A 48-bit value covers a range of about 8.93 years.
*
*     First we divide the 48-bit value by 1,000,000; the remainder is the
*     number of microseconds and the quotent (seconds) fits into 32-bits.
*     Then it is reasonably easy to convert to days, hours, minutes and seconds.
*
*  NOTES:
*     0xFFFFFFFFFFFF divided by 1000000 (0xF4240) = 0x10C6F7A0.
*     0x10C6F7A0     divided by 3600    (0xE10)   = 0x1316B.
*     There are 86,400 (0x15180) seconds in a day.
*     0x10C6F7A0 / 0x15180 = 3257.81 (0xCB9) the range of the counter in days.
*
*  RETURNS
*     Nothing.
*
*  Note: This routine takes about 9.5 us on a 486DX4/100 (not using sprintf).
****************************************************************************/
NOMANGLE void CCONV BusTools_TimeGetString(
   BT1553_TIME * curtime,   // (i) Timer to be converted to string value.
   char        * string)    // (o) Pointer, store resulting time string.
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   long days;          // Number of days in time tag ( 0 to 3257 )
   long hours;         // Number of hours ( 0 to 23 )
   long minutes;       // Number of minutes ( 0 to 59 )
   long seconds;       // Number of seconds ( 0 to 59 )
   unsigned long microseconds;  // Number of microseconds ( 0 to 999999 )

#if !defined(NO_ASSEMBLY)
   BT_U32BIT  local_time[2];  // Temp for curtime, avoids addressing problems
                              //  when using assembly language.

   local_time[0] = curtime->microseconds;
   local_time[1] = curtime->topuseconds;

   /*************************************************************************
   * The following assembly code is compatible with WinTel 32-bit programming 
   * environments!  It is significantly faster than the "C" code...
   **************************************************************************/
   // Copy the input value into a local variable.

   __asm  mov    eax,dword ptr local_time   // Fetch low dword of time value
   __asm  movzx  edx,word  ptr local_time+4 // Fetch high word of time value
   __asm  mov    ebx,1000000                // Divisor is one million
   __asm  div    ebx                        // Split value into sec, usec
   __asm  mov    dword ptr microseconds,edx // Remainder is microseconds
   __asm  xor    edx,edx                    // Zero-extend quotient (seconds, eax)
   __asm  mov    ebx,60                     // Extract minutes from seconds
   __asm  div    ebx                        // Remainder is seconds
   __asm  mov    dword ptr seconds,edx      //  gives seconds as the remainder
   __asm  xor    edx,edx                    // Zero-extend quotient (minutes)
   __asm  div    ebx                        // Dividing by 60
   __asm  mov    dword ptr minutes,edx      //  gives minutes as the remainder.
   __asm  xor    edx,edx                    // Zero-extend quotient (hours)
   __asm  mov    ebx,24                     // Extract days from hours
   __asm  div    ebx                        // Remainder is hours
   __asm  mov    dword ptr hours,edx        //  store remainer as hours
   __asm  mov    dword ptr days,eax         //  and the quotient is days.

#else
   /*********************************************************************
   * If your compiler does not support this code, you will have to build
   * your own using whatever tools your environment provides.
   * When I tried the ldiv() ANSI function it seemed to fail if the sign bit
   * of curtime->microseconds was set (at least on Microsoft/Borland),
   * as well as when the sign bit of curtime->topuseconds was set...ajh
   *********************************************************************/

   CEI_UINT64   big_time;   // 64-bit timer value for the arithmetic

   /* Extract Seconds from Microseconds */
   big_time = curtime->topuseconds;
   big_time = (big_time << 32) | curtime->microseconds;

   microseconds = (unsigned long)(big_time%1000000);  /* 0 to 999999  */
   seconds      = (unsigned long)(big_time/1000000);

   /* Extract Minutes from Seconds */
   minutes = seconds/60;      /* 0 to 0x10C6F7A0/60 */
   seconds = seconds%60;      /* 0 to 59            */

   /* Extract Hours from Minutes */
   hours = minutes/60;        /* 0 to 0x1316B       */
   minutes = minutes%60;      /* 0 to 59            */

   /* Extract Days from Hours */
   days = (hours/24);       /* 1 to 3258          */
   hours = hours%24;        /* 0 to 23            */

#endif  //!defined(NO_ASSEMBLY)

   /* Dispatch on the display format defined */
   switch ( TTDisplay_fmt )
   {
   case API_TTD_RELM:   // Relative to Midnight Format "(ddd)hh:mm:ss.uuuuuu"
                        // Only necessary components are displayed.
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%6.6ld",
                 days, hours, minutes, seconds, microseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6ld",
                 hours, minutes, seconds, microseconds);
      else if ( minutes )
         sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds, microseconds);
      else
         sprintf(string,"%ld.%6.6ld", seconds, microseconds);
      return;

   case API_TTD_IRIG:   // Full IRIG Format "(ddd)hh:mm:ss.uuuuuu"
                        // All components are displayed unless the day is zero
                        // (implies a delta time is being displayed).
      days++;           // increment days to adjust to IRIG julian date 
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%6.6ld",
                 days, hours, minutes, seconds, microseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6d",
                 hours, minutes, seconds, (BT_U32BIT)microseconds);
      else if (minutes )
         sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds,microseconds);
      else
         sprintf(string,"%ld.%6.6ld", seconds, microseconds);
      return;

   case API_TTD_DATE:   // Date Format "(MM/dd)hh:mm:ss.uuuuuu"
      {                 // Convert the IRIG day of the year to month/day.
                        //  IRIG day 1 = Jan 1.  We need the year for
                        //  leap year calculations.
         int m = 0;                 // The first month is January
         static int year = 0;       // We read this from the host.
         int leap;                  // Is this year a leap year?
         int resulting_month = 1;   // Just in case the day of the year should
         int resulting_day = 0;     //  just happen to be zero (which is illegal).
         
         days++;                    // Adjust for the fact there is no day 0.
         // Get the current year for leap year computations:
         if ( year == 0 )
         {
            time_t timer;                     // Time since 1900
            struct tm *tblock;                // Pointer to structure containing year

            timer = time(NULL);               // Get time of day et.al.
            tblock = localtime(&timer);       // Convert date/time to a structure
            year = tblock->tm_year + 1900;    // Get and save the year
         }

         leap = ((year % 4) == 0) ? 1 : 0;    // Is this year a leap year?
                                              // Should work until 3000...
         // Convert day of the year to month:day

         if(days > 366)
            days = 365;

         while ( days > 0 )
         {
            resulting_day   = days;
            resulting_month = m;
            switch(m)
            {
               case 1: days -= 31; break;          // Jan
               case 2: days -= (28 + leap); break; // Feb
               case 3: days -= 31; break;          // March
               case 4: days -= 30; break;          // April
               case 5: days -= 31; break;          // May
               case 6: days -= 30; break;          // June
               case 7: days -= 31; break;          // July
               case 8: days -= 31; break;          // August
               case 9: days -= 30; break;          // Sept
               case 10:days -= 31; break;          // Oct
               case 11:days -= 30; break;          // Nov
               case 12:days -= 31; break;          // Dec
            }
            m++;  // Go to next month
         }
         // Display all components, unless the day is zero
         //   (which implies a delta time display) then display a short version.
         if ( resulting_day )
            sprintf(string,"(%d/%d)%2.2ld:%2.2ld:%2.2ld.%6.6ld", resulting_month,
                    resulting_day, hours, minutes, seconds, microseconds);
         else if ( hours )
            sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6ld",
                    hours, minutes, seconds, microseconds);
         else if ( minutes )
            sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds, microseconds);
         else
            sprintf(string,"%ld.%6.6ld", seconds, microseconds);
      }
   }
}

/*****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagMode
*
*  FUNCTION
*     This routine is called to setup the time tag parameters
*     and conversion options.
*
*   TTDisplay    Display Type:
* -------------- --------------------------------------------------------------
* API_TT_DEFAULT Unchanged from previous call
* API_TTD_RELM   Relative to midnight format (dd-hh:mm:ss.useconds, default)
* API_TTD_IRIG   IRIG Format (ddd-hh:mm:ss.uuuuuu) (All board variants)
* API_TTD_DATE   Date Format (MM/dd-hh:mm:ss.uuuuuu)
*
*    TTRecFmt    Recording Time Format:
* -------------- --------------------------------------------------------------
* API_TT_DEFAULT Unchanged from previous call
* API_TTI_ZERO   Time set to zero when BM Started (All board variants, default)
* API_TTI_DAY    Time of day, relative to midnight when Bus Monitor Started
*                (Host Clock reference)
* API_TTI_IRIG   Time of year (IRIG format) (Host clock reference)
* API_TTI_EXT    External time reference (provided by function BusTools_TimeSet
*                in the user-supplied DLL specified by DLLname)
*
*     TTMode     Time Tag counter operating mode:
* -------------- --------------------------------------------------------------
* API_TT_DEFAULT Unchanged from previous call
* API_TTM_FREE   Free running time tag counter (All board variants, default)
* API_TTM_RESET  Time Tag counter reset to zero on external TTL input
*                discrete active (All board variants).
* API_TTM_SYNC   Sync the Time Tag to the external TTL input.
*                The TTPeriod parameter sets the period of the external TTL
*                input in microseconds.
* API_TTM_RELOD  Time Tag counter is reset to the value previously loaded into
*                the Time Tag Load register (see BusTools_TimeTagWrite).
*
*  RETURNS
*     API_SUCCESS              // no errors.
*     API_HARDWARE_NOSUPPORT   // Function not supported by current hardware
*     API_TIMETAG_BAD_DISPLAY  // Unknown display format
*     API_TIMETAG_BAD_INIT;    // Unknown Time Tag Initialization method
*     API_TIMETAG_BAD_MODE;    // Unknown Time Tag Operating Mode
*     API_TIMETAG_NO_DLL       // DLL containing BusTools_TimeTagGet() could not be loaded
*     API_TIMETAG_NO_FUNCTION  // Error getting address of the BusTools_TimeTagGet() function
*     API_TIMETAG_USER_ERROR   // User function BusTools_TimeTagGet() returned an error
*     API_NO_OS_SUPPORT        // Function not supported by underlying Operating System
*
*****************************************************************************/

#ifdef __BORLANDC__
#pragma argsused
#endif
NOMANGLE BT_INT CCONV BusTools_TimeTagMode(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_INT   TTDisplay,      // (i) time tag display format
   BT_INT   TTInit,         // (i) time tag counter initialization mode
   BT_INT   TTMode,         // (i) time tag timer operation mode
   char     *DLLname,       // (i) name of the DLL containing time function
   BT_U32BIT TTPeriod,      // (i) period of external TTL sync input
   BT_U32BIT lParm1,        // (i) spare parm1
   BT_U32BIT lParm2)        // (i) spare parm2
{
   /***********************************************************************
   *  Check for legal call
   ***********************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;
   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /***********************************************************************
   *  Dispatch on the display format definition specified.  This is the
   *   mode used by BusTools_TimeGetString() to convert time to ASCII.
   * This is a global parameter, global to all cards!
   ***********************************************************************/
   switch ( TTDisplay )
   {
      case API_TT_DEFAULT:   // Leave previous display format unchanged.
         break;
      case API_TTD_RELM:     // Display time relative to midnight this morning
         TTDisplay_fmt = API_TTD_RELM;
         break;
      case API_TTD_IRIG:     // Display time in IRIG format
         TTDisplay_fmt = API_TTD_IRIG;
         break;
      case API_TTD_DATE:     // Display time in month/day format
         TTDisplay_fmt = API_TTD_DATE;
         break;
      default:               // Everything else comes here.
         return API_TIMETAG_BAD_DISPLAY;  // Unknown display format
   }

   /***********************************************************************
   *  Dispatch on the Time Tag Initialization Type definition specified.
   *  This is the method used to initialize the time tag counter.
   ***********************************************************************/
   switch ( TTInit )
   {
      case API_TT_DEFAULT:   // Leave previous TT initialization mode unchanged.
         break;
      case API_TTI_ZERO:     // Zero the TT counter when starting the Bus Monitor
         TTInit_type[cardnum] = API_TTI_ZERO;
         break;
      case API_TTI_DAY:      // Load the TT counter with time of day relative to midnight
         TTInit_type[cardnum] = API_TTI_DAY;
         break;
      case API_TTI_IRIG:     // Load the TT counter with IRIG time from the host clock
         TTInit_type[cardnum] = API_TTI_IRIG;
         break;
      case API_TTI_EXT:      // Load the TT counter with time from the user function
                             //  BusTools_TimeTagGet() in the DLL named "DLLname".
#if defined(_USER_DLL_)
      {
         HMODULE hTTdll;        // Handle to DLLname.dll

         hTTdll = LoadLibrary((LPCTSTR)DLLname);  // Search standard path for user's DLL

         if ( hTTdll == NULL )
            return API_TIMETAG_NO_DLL;
         // Now get the address of the user's time tag initialization function.
         pBusTools_TimeTagGet[MAX_BTA] = (void *)GetProcAddress(hTTdll, "BusTools_TimeTagGet");
         if ( pBusTools_TimeTagGet[MAX_BTA] == NULL )
            return API_TIMETAG_NO_FUNCTION;
         // Got the function address so the mode is valid.
         TTInit_type[cardnum] = API_TTI_EXT;
         break;
      }
#else 
         return API_NO_OS_SUPPORT; // Function not supported by underlying Operating System
#endif //_USER_DLL_
      default:
         return API_TIMETAG_BAD_INIT;  // Unknown Time Tag Initialization method
   }

   /***********************************************************************
   *  Dispatch on the Time Tag Counter Operating Mode specification.
   *  This defines how the Time Tag Counter runs.
   ***********************************************************************/
   channel_status[cardnum].irig_on=0;
   switch ( TTMode )
   {
      case API_TT_DEFAULT:   // Leave previous operating mode unchanged.
         break;

      case API_TTM_FREE:     // Time Tag counter free runs
         TTMode_Def[cardnum] = API_TTM_FREE;  // Remember mode.
         // Disable clear of Time Tag counter by discrete input
         // Disable reload of Time Tag Counter from holding reg by discrete input
         api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0000);
         break;

      case API_TTM_RESET:    // Time Tag counter reset to zero by discrete input
         TTMode_Def[cardnum] = API_TTM_RESET;  // Remember mode.
         // Enable clear of Time Tag counter by discrete input
         // Disable reload of Time Tag Counter from holding reg by discrete input
         api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0001);
         break;
      case API_TTM_SYNC:     // Time Tag counter sync'ed to discrete input
         TTMode_Def[cardnum] = API_TTM_SYNC;  // Remember mode.
         TTPeriod_Def[cardnum] = TTPeriod;    // Remember period of sync input.
         // Disable clear of Time Tag Counter from holding reg by discrete input
         // Enable reload of Time Tag counter by discrete input if supported
         // by the current hardware.  If no HW support, emulate the function
         // in the API software.
        
         api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0001);
         break;

      case API_TTM_RELOD:    // Time Tag counter reset to TT load register by discrete input
         TTMode_Def[cardnum] = API_TTM_RELOD; // Remember mode.
         TTPeriod_Def[cardnum] = TTPeriod;    // Remember period of sync input.
         // Disable clear of Time Tag counter by discrete input
         // Enable reload of Time Tag Counter from holding reg by discrete input
         api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0001);
         break;

      case API_TTM_IRIG: // Time Tag reset to external or internal IRIG time value
         if(!board_has_irig[cardnum])
            return API_HARDWARE_NOSUPPORT; // Does not support IRIG.
         channel_status[cardnum].irig_on=1;
         TTMode_Def[cardnum] = API_TTM_IRIG; // Remember mode.
		 api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_IRIG);
		 break;
      case API_TTM_AUTO: //AUTO Increment Time Tag Counter on external pusle
	     TTMode_Def[cardnum] = API_TTM_AUTO;  // Remember mode.
	     TTPeriod_Def[cardnum] = TTPeriod;    // Remember period of sync input.
		 api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0);  // Clear the register
	     break;
      default:
         return API_TIMETAG_BAD_MODE;  // Unknown Time Tag Operating Mode
   }

   if(lParm2 == 0)
      return BusTools_TimeTagInit(cardnum);
   else
      return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagInit
*
*  FUNCTION
*     This routine resets the elapsed time counters and sets the
*     timetag counter in the hardware to zero, or to the value specified
*     by TTInit_type[cardnum], as set by BusTools_TimeTagMode().
*     Some modes obtain the time from the user-function "BusTools_TimeTagGet"
*     in the DLL specified by the user.
*
* TTInit_type[] Board Type  Time Tag Initializatio Performed
* -------------  --------   --------------------------------
* API_TTI_ZERO     ALL      TT counter set to zero,
*                           sofware base time counters set to zero.
* API_TTI_DAY    PCI-1553   TT counter loaded with host time since midnight,
*                           software base time counters set to zero,
*               All Others  TT counter set to zero,
                            software base time counters set to host time since midnight.
* API_TTI_IRIG   PCI-1553   TT counter loaded with host time since Jan 1,
*                           software base time counters set to zero.
*               All Others  TT counter set to zero,
"                           software base time counters set to host time since Jan 1
* API_TTI_EXT    PCI-1553   TT counter set by user function "BusTools_TimeTagGet",
*                           software base time counters set to zero.
*               All Others  TT counter set to zero,
*                           software base time counters set to value obtained
*                               from the user-function "BusTools_TimeTagGet".
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> cardnum >= MAX_BTA
*     API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_TimeTagInit(
   BT_UINT  cardnum)              // (i) Card number.
{
   /*******************************************************************
   *  Local Variables
   *******************************************************************/
   BT1553_TIME timetag;           // Used to zero the time tag counters.
   int status=API_SUCCESS;
#ifndef NO_HOST_TIME              //
   BT1553_TIME host_time;
#endif //#ifdef NO_HOST_TIME
   /*******************************************************************
   *  Check initial/error conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Initialize the hardware and software time tag components based
   *    on the current initialization mode.
   *******************************************************************/
   switch ( TTInit_type[cardnum] )
   {
   // Initial time set to zero:
   case API_TTI_ZERO:
      /****************************************************************
      *  Reset cummulative time (current and previous) to zero.
      ****************************************************************/
      time_previous[cardnum].microseconds = 0;
      time_previous[cardnum].topuseconds  = 0;
      time_current[cardnum].microseconds  = 0;
      time_current[cardnum].topuseconds   = 0;
      sched_tt_clear[cardnum] = 0; // TimeTagClearFlag not scheduled.

      /****************************************************************
      *  Reset SW and HW timetag load registers to zero.V4.25.ajh
      ****************************************************************/

      timetag.microseconds = 0;
      timetag.topuseconds = 0;
      BusTools_TimeTagWrite(cardnum, &timetag, 1);

      /****************************************************************
      *  Reset hardware Bus Monitor timetag counter to zero.
      ****************************************************************/

      status = API_SUCCESS;
      break;

   // Initial time set to host time since midnight:
   case API_TTI_DAY:
#ifdef NO_HOST_TIME
      status = API_NO_OS_SUPPORT;
#else
      /****************************************************************
      *  Fetch host time since midnight.
      ****************************************************************/
      get_48BitHostTimeTag(API_TTI_DAY, &host_time);
      /****************************************************************
      *  Initialize the hardware and software time tag components.
      ****************************************************************/
      sched_tt_clear[cardnum] = 0; // TimeTagClearFlag not scheduled.
      status = BusTools_TimeTagWrite(cardnum, &host_time, 1);
#endif //NO_HOST_TIME
      break;
      
   // Initial time set to host time since Jan 1:
   case API_TTI_IRIG:
#ifdef NO_HOST_TIME
      status = API_NO_OS_SUPPORT;
#else
      /****************************************************************
      *  Initialize the hardware and software time tag components.
      ****************************************************************/
      sched_tt_clear[cardnum] = 0; // TimeTagClearFlag not scheduled.
      /****************************************************************
      *  Fetch host time since Jan 1.
      ****************************************************************/
      if(TTMode_Def[cardnum] != API_TTM_IRIG)
      {
         get_48BitHostTimeTag(API_TTI_IRIG, &host_time);
         status = BusTools_TimeTagWrite(cardnum, &host_time, 1);
      }
#endif //NO_HOST_TIME
      break;
#ifdef _USER_DLL_
   // Initial time set by user-supplied function:
   case API_TTI_EXT:
      /****************************************************************
      *  Fetch user supplied time value.
      ****************************************************************/
      if ( pBusTools_TimeTagGet[cardnum](cardnum, (BT1553_TIME *)&host_time) )
         return API_TIMETAG_USER_ERROR;
      /****************************************************************
      *  Initialize the hardware and software time tag components.
      ****************************************************************/
      sched_tt_clear[cardnum] = 0; // TimeTagClearFlag not scheduled.
      status = BusTools_TimeTagWrite(cardnum, (BT1553_TIME *) &host_time, 1);
	  break;
#endif //_USER_DLL_
   default:
      return API_NO_OS_SUPPORT;
   }

   if(TTMode_Def[cardnum] == API_TTM_RESET)
   {
      BT1553_TIME ztime;
      ztime.topuseconds = 0x0;
      ztime.microseconds = 0x0;
      vbtWriteTimeTag(cardnum,&ztime);
      vbtWriteTimeTagIncr(cardnum,0x0);                   // set the increment value to 0
      api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_HWL);  // Set auto increment option
   }
   else if(TTMode_Def[cardnum] == API_TTM_AUTO)
   { 
      vbtWriteTimeTagIncr(cardnum,TTPeriod_Def[cardnum]);       // set the increment value
      api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_AUTO);  // Set auto increment option
   }
   else
      vbtWriteTimeTagIncr(cardnum,0);

   return status;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagRead
*
*  FUNCTION
*     This routine reads the current timetag from the hardware.
*
*  RETURNS
*     API_SUCCESS
*     API_BUSTOOLS_BADCARDNUM -> cardnum >= MAX_BTA
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_HARDWARE_NOSUPPORT  -> Does not support reading Time Tag register.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_TimeTagRead(
   BT_UINT cardnum,               // (i) board number
   BT1553_TIME * timetag)     // (o) resulting 48-bit time value
{
   /************************************************
   *  Check initial and error conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   // Get the actual time tag value (45 bits) from the board,
   //  and return it to the caller.
   vbtReadTimeTag(cardnum, (BT_U16BIT *)timetag);

   flip(&timetag->microseconds);
   if(irig_bias[cardnum] == 0)
      return API_SUCCESS;
   else if(irig_bias[cardnum]>0)
      timetag->microseconds += irig_bias[cardnum];
   else
      timetag->microseconds -= irig_bias[cardnum];
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagWrite
*
*  FUNCTION
*     This routine writes the specified value to the timetag load register,
*     and causes the value to be loaded into the time tag counter.
*
*  RETURNS
*     API_SUCCESS
*     API_BUSTOOLS_BADCARDNUM -> cardnum >= MAX_BTA
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_HARDWARE_NOSUPPORT  -> Does not support writing the Time Tag register.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_TimeTagWrite(
   BT_UINT cardnum,           // (i) card number (0 - based)
   BT1553_TIME * timetag, // (i) pointer to time tag structure
   BT_INT            flag)    // (i) flag->0 just load the TT Register,
                              //     flag->1 load the TT Register into the counter
{
   /************************************************
   *  Check initial and error conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   // Save the new value of the Time Tag Load Register since we can't read it.
   TTLR_Shadow[cardnum] = *timetag;

   // Write the value out depending on the hardware support provided:

   if (TTMode_Def[cardnum] == API_TTM_IRIG)
      return API_TIMETAG_WRITE_ERROR;
   // Write the specified value to the 45-bit hardware time tag register.

   vbtWriteTimeTag(cardnum, timetag);

   // If specified, load the TT register into the time tag counter.
   // Be sure to maintain the setting of the HW load TT counter bit!
   if ( flag )
   {
      if ( (TTMode_Def[cardnum] == API_TTM_SYNC) ||
           (TTMode_Def[cardnum] == API_TTM_RELOD)   )
         vbtSetHWRegister(cardnum, HWREG_CONTROL_T_TAG, 0x0003); // SW load, HW load enabled
	  else if (TTMode_Def[cardnum] == API_TTM_RELOD)
         vbtSetHWRegister(cardnum, HWREG_CONTROL_T_TAG, 0); // SW load, HW load enabled		   
      else
         vbtSetHWRegister(cardnum, HWREG_CONTROL_T_TAG, 0x0002); // SW load only.
   }
   return API_SUCCESS;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_Calibration
*
*  FUNCTION
*     This routine calibrates the IRIG DAC
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_Calibration(BT_UINT cardnum, 
                                                BT_INT flag)        // (i) card number
{

   BT_INT    status;

   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT; // Does not support IRIG.

   if(CurrentCardType[cardnum] == PCC1553)
      return API_SUCCESS;
    
   api_writehwreg(cardnum, HWREG_CONTROL_T_TAG,0);  //disable IRIG
   status = vbtIRIGCal(cardnum,flag);
   api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_IRIG);       // enable IRIG

   return status;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_Init
*
*  FUNCTION
*     This routine calibrates the IRIG DAC
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_Config(BT_UINT cardnum,   // (i) card number
                                           BT_UINT intFlag,   // Internal IRIG or External
                                           BT_UINT outFlag)   // Output Internal IRIG    
{
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT; // Does not support IRIG.

   vbtIRIGConfig(cardnum, (BT_U16BIT)(intFlag | outFlag)); 

   vbtIRIGWriteDAC(cardnum,(BT_U16BIT)IRIG_DEFAULT_DAC);  // Set the IRIG DAC to the default value

   return API_SUCCESS;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_Valid
*
*  FUNCTION
*     This routine checks for a valid IRIG signal.
*
*  RETURNS 
*     Error code or Success (valid)
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_Valid(BT_UINT cardnum)    
{

   BT_U16BIT valid;

   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT; // Does not support IRIG.

   vbtIRIGValid(cardnum, &valid);
   
   if(valid)
	   return API_SUCCESS;
   else
	   return API_IRIG_NO_SIGNAL;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_SetBias
*
*  FUNCTION
*     This routine set a bias value to coordinate time between systems
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_SetBias(BT_UINT cardnum,   // (i) card number
                                            BT_INT bias)    
{
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT; // Does not support IRIG.

   irig_bias[cardnum] = bias;

   return API_SUCCESS;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_Settime
*
*  FUNCTION
*     This routine sets the IRIG Time.  Used for Internal IRIG-B mode
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_SetTime(BT_UINT cardnum,   // (i) card number
                                            long timedate,     // (i) timedate to set -1 use system time
                                            BT_U32BIT flag)    // (i) 0 = gmtime 1 = localtime
                                                               //     0x10 = DDD:HH:MM:SS format
{
   BT_U32BIT IRIGTime;
   union
   {
      struct toy
	  {
#ifdef NON_INTEL_BIT_FIELDS
          BT_U32BIT reserved:2;
	      BT_U32BIT hundred_day:2;
	      BT_U32BIT tens_of_day:4;
          BT_U32BIT unit_day   :4;
          BT_U32BIT tens_of_hr :2;
          BT_U32BIT unit_hr    :4;
          BT_U32BIT tens_of_min:3;
          BT_U32BIT unit_min   :4;
          BT_U32BIT tens_of_sec:3;
          BT_U32BIT unit_sec   :4;
#else  //INTEL-Compatible bit field ordering 
	      BT_U32BIT unit_sec   :4;
	      BT_U32BIT tens_of_sec:3;
	      BT_U32BIT unit_min   :4;
	      BT_U32BIT tens_of_min:3;
	      BT_U32BIT unit_hr    :4;
	      BT_U32BIT tens_of_hr :2;
	      BT_U32BIT unit_day   :4;
	      BT_U32BIT tens_of_day:4;
	      BT_U32BIT hundred_day:2;
              BT_U32BIT reserved:2;
#endif  //NON_INTEL_BIT_FIELDS
	  } TOY;
	  BT_U32BIT timeval;
   }timeconvert;

   struct tm *ltime;
   time_t tt;
   BT_UINT yday,ddd,hh,mm,ss;
  
   BT_U16BIT itime_l, itime_m;

   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT; // Does not support IRIG.

   if(flag & 0x10)
   {
      ss = timedate   & 0x0000000ff;
      mm = (timedate  & 0x00000ff00)>>8;
      hh = (timedate  & 0x000ff0000)>>16;
      ddd = (BT_UINT)(timedate & 0xfff00000)>>24; //

      timeconvert.TOY.unit_sec = ss % 10;  //tm_sec [0 - 61] allows for a leap second or two
      timeconvert.TOY.tens_of_sec = ss/10; 
      timeconvert.TOY.unit_min = mm % 10;  //tm_min [0 - 59]
      timeconvert.TOY.tens_of_min = mm/10;
      timeconvert.TOY.unit_hr = hh % 10;  //tm_hour [0 - 23]
      timeconvert.TOY.tens_of_hr = hh/10;
      timeconvert.TOY.unit_day = (((ddd)%100)%10); //tm_yday [0 - 365] allows for leap year
      timeconvert.TOY.tens_of_day = (((ddd)%100)-timeconvert.TOY.unit_day)/10;
      timeconvert.TOY.hundred_day = (ddd)/100;
      timeconvert.TOY.reserved = 0;

   }
   else
   {
      //get the time from the system clock
      if(timedate == -1)
      {
         timedate = (long)time(&tt);
      }

      // convert the the timedate to the irig stuff
      if(flag)
         ltime = localtime((const time_t*)&timedate);
      else
         ltime = gmtime((const time_t *)&timedate);
      timeconvert.TOY.unit_sec = ltime->tm_sec % 10;  //tm_sec [0 - 61] allows for a leap second or two
      timeconvert.TOY.tens_of_sec = ltime->tm_sec/10; 
      timeconvert.TOY.unit_min = ltime->tm_min % 10;  //tm_min [0 - 59]
      timeconvert.TOY.tens_of_min = ltime->tm_min/10;
      timeconvert.TOY.unit_hr = ltime->tm_hour % 10;  //tm_hour [0 - 23]
      timeconvert.TOY.tens_of_hr = ltime->tm_hour/10;
      yday = ltime->tm_yday+1;                        //ltime yday is 0-based; IRIG is 1-based

      timeconvert.TOY.unit_day = (((yday)%100)%10); //tm_yday [0 - 365] allows for leap year
      timeconvert.TOY.tens_of_day = (((yday)%100)-timeconvert.TOY.unit_day)/10;
      timeconvert.TOY.hundred_day = (yday)/100;
      timeconvert.TOY.reserved = 0;
    
   }

   IRIGTime = timeconvert.timeval;  
   itime_l = (BT_U16BIT)(IRIGTime & 0x0000ffff);
   itime_m = (BT_U16BIT)((IRIGTime & 0xffff0000) >> 16); 

   vbtIRIGSetTime(cardnum,itime_l,itime_m);

   return API_SUCCESS;
}
