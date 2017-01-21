/****************************************************************************
*
*  PROCEDURE - get_48BitHostTimeTag()
*
*  FUNCTION
*     This routine reads the host clock and returns an API-formatted initial
*     time tag value as a 64-bit integer in microseconds.  We wait for the
*     clock to change to make sure we get as accurate a time value as possible.
*
****************************************************************************/
void get_48BitHostTimeTag(BT_INT mode, BT1553_TIME *host_time)  //__uint64 *rtn_time)
{
	union bigtime
	{
         CEI_UINT64 rtn_time;
	   BT1553_TIME rtn_val;
	}bt;
	
   time_t   now;             // Current time, modify to get Time Reference Point.
   struct   tm start;        // Formatted "now" current time, modified for TRP.
   time_t   then=0;          // Time Reference Point, Midnight or Jan 1.
   struct   timeb ts;        // Current time, use to get milliseconds.

   unsigned last_milli;      // This is the time we use to wait for time to change.
   union{
      CEI_UINT64 diff;       // This is the difference time we return in microseconds,
      BT_U32BIT tt[2];
   }utime;                   //  either time since Midnight or time since Jan 1.
   switch (mode)
   {
   case API_TTI_DAY:  // Initial time relative to midnight
      time(&now);               // Time in seconds since midnight Jan 1, 1970 GMT
      start = *localtime(&now); // Local time in seconds, min, hour, day, month
      start.tm_sec  = 0;        // Local
      start.tm_min  = 0;        //       time
      start.tm_hour = 0;        //            at midnight
      then = mktime(&start);    // local midnight to time in seconds since Jan 1, 1970
      break;

   case API_TTI_IRIG: // Initial time relative to Jan 1, this year
      time(&now);               // Time in seconds since midnight Jan 1, 1970 GMT
      start = *localtime(&now); // Local
      start.tm_sec  = 0;        //   time
      start.tm_min  = 0;        //     at
      start.tm_hour = 0;        //       Jan 1
      start.tm_mday = 0;        //         midnight (This is day 1)
      start.tm_mon  = 0;        //           this
      start.tm_yday = 0;        //             year
      then = mktime(&start);    // local Jan 1 to time in seconds since Jan 1, 1970
                                // We do IRIG time, which makes Jan 1 "day 1" */
      break;
   }

   ftime(&ts);               // Current time
   last_milli = ts.millitm;  //  save milliseconds   

   while (1)                 // Wait for milliseconds to chnage
   {
      ftime(&ts);               // Current time
      last_milli = ts.millitm;  //  save milliseconds	  
      break;              // Break at exact time tick change
   }
   utime.diff = ts.time-then;       // We only want the time since Midnight or Jan 1
   utime.diff *= 1000000;          // Number of Seconds converted to microseconds
   utime.diff += ts.millitm * 1000; //  plus number of Milliseconds converted to microseconds.

   if(mode==API_TTI_IRIG)
      bt.rtn_time = (utime.diff-86400000000);        // Pass it back to the caller.
   else
      bt.rtn_time = utime.diff;

   *host_time = bt.rtn_val; 
   return;
}