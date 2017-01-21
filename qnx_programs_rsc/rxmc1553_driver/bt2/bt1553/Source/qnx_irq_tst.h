#ifndef __QNX_IRQ_TESTING__
#define __QNX_IRQ_TESTING__

// typedef uint32_t LWORD;
// typedef int64_t Q_INT;

#include "lbm_bs.h"

#define ECT_SIZE 16

typedef struct _evcnt
{
  int IRQ_on, IRQ_off;
  int TaskCC[8];
} EV_CNT;

extern EV_CNT ECT[ECT_SIZE];

extern int BT_IRQ_on, BT_IRQ_off;
extern pthread_mutex_t muIQ;  // @@@QQ1 --> mutex for Interrupt Queue control

void Event_CNT_Init( int ix );

#endif // __QNX_IRQ_TESTING__


