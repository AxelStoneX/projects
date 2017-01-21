#include "time_counter.h"

void TimeCounterInit ( TMS_t *TM )
{
  TM->clock_coefficient = 1000/ ((double) SYSPAGE_ENTRY(qtime)->cycles_per_sec);
  TM->cycle_counter_1 = 0;
  TM->cycle_counter_2 = 0;
}

void StartTimeCount ( TMS_t *TM)
{
  TM->cycle_counter_1 = ClockCycles();
}

double GetTimeCount ( TMS_t *TM)
{
  double result;
  TM->cycle_counter_2 = ClockCycles();
  result = (double)(TM->cycle_counter_2 - TM->cycle_counter_1);
  result *= TM->clock_coefficient;
  return result;
}
