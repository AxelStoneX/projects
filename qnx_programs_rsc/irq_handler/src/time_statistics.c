#include <sys/neutrino.h>
#include <stdlib.h>
#include <math.h>                                                                 //For sqrt
#include <string.h>                                                                // For memset()

#include "time_statistics.h"

void TimeStatisticsInit ( STAT_t *ST )
{
  memset( (void*)ST, 0, sizeof( STAT_t ) );
}

void TimeStatisticsGet ( STAT_t *ST, double measurment )
{
  if(ST->data_counter == 0)
    ST->min_time = measurment;
  ST->data_counter++;
  if(measurment > ST->max_time)
    ST->max_time = measurment;
  if(measurment < ST->min_time)
    ST->min_time = measurment;
  ST->sum_time += measurment;
  ST->sum_sqr_time += measurment*measurment;
  ST->average_time = ST->sum_time/ST->data_counter;
  ST->standart_deviation = sqrt( ST->sum_sqr_time/ST->data_counter -  (ST->average_time)*(ST->average_time) );
}
