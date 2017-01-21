#include "1553_interface.h"

//=============================================================================
void BT_TTag_Diff( BT1553_TIME *tG1, BT1553_TIME *tG2, Q_INT *df )
{   QWORD us1, us2;
    Q_INT ms1, ms2, dif;

    us1 = tG1->topuseconds;
    us1 = us1 << 32;
    us1 = us1 | tG1->microseconds;

    us2 = tG2->topuseconds;
    us2 = us2 << 32;
    us2 = us2 | tG2->microseconds;

    ms1 = (Q_INT) us1;
    ms2 = (Q_INT) us2;
    dif = ms1 - ms2;
    *df = dif;
}

//=============================================================================
void BT_TTag_Conv( BT1553_TIME *ttG, Q_INT *ms )
{   QWORD us1;  // Convert microseconds in 16 + 32 bits struct --> 64 bit int
    us1 = ttG->topuseconds;
    us1 = us1 << 32;
    us1 = us1 | ttG->microseconds;
    *ms = (Q_INT) us1;
}


