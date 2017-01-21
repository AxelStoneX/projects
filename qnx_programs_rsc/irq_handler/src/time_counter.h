#ifndef __TIMER_COUNTER_H___
#define __TIMER_COUNTER_H___

#include <sys/syspage.h>
#include <sys/neutrino.h>
#include <inttypes.h>

typedef struct
{ 
  double clock_coefficient;
  uint64_t cycle_counter_1;
  uint64_t cycle_counter_2;
} TMS_t; // Time measurement Set



//Initialization of time-estimating mechanism. Use this once per program before counting
void TimeCounterInit ( TMS_t *TM );

//Start Time Counting function. Use this to start mesuarment of time
void StartTimeCount ( TMS_t *TM );

//Get Time Counter function. Returns measured time in milliseconds
double GetTimeCount ( TMS_t *TM );


#endif // __TIMER_COUNTER_H___
