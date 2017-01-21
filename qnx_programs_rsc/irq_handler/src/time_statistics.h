#ifndef __TIME_STATISTICS_H__
#define __TIME_STATISTICS_H__

typedef struct
{
  double max_time;                    //maximal time from all of measurments
  double min_time;                     //minimal time from all of measurments
  double sum_time;                    //summary value of measurments
  double sum_sqr_time;             //summary value of squared measurments
  double average_time;             //average value of measurments
  double standart_deviation;     //standart deviation of all measurments, Gaussian distribution
  int data_counter;                     //amount of all processed measurments
} STAT_t;


/*Time Statistics Initialization function, use this to start or reset statistics logging*/
void TimeStatisticsInit ( STAT_t *ST);

/*Get Time Statistics function. Returns pointer to structure of statistics_t type which contains
a lot of statistics  info about  measurments.*/
void TimeStatisticsGet (STAT_t *ST, double measurment);

#endif

