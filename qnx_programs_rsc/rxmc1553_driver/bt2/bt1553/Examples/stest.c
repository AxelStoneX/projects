#include <stdio.h>
#include "busapi.h"
#include "apiint.h"

int main(int argc, char** argv)
{
    printf("sizeof BC_MESSAGE = %d\n",sizeof(BC_MESSAGE));
    printf("sizeof BC_CBUF = %d\n",sizeof(BC_CBUF));
    printf("sizeof BC_DBLOCK  = %d\n",sizeof(BC_DBLOCK));
    printf("sizeof BM_CBUF  = %d\n",sizeof(BM_CBUF));
    printf("sizeof BM_MBUF = %d\n",sizeof(BM_MBUF));
    printf("sizeof BM_FBUF = %d\n",sizeof(BM_FBUF));
    printf("sizeof EI_MESSAGE = %d\n",sizeof(EI_MESSAGE));
    printf("sizeof RT_ABUF_ENTRY = %d\n",sizeof(RT_ABUF_ENTRY));
    printf("sizeof RT_ABUF = %d\n",sizeof(RT_ABUF));
    printf("sizeof RT_CBUF = %d\n",sizeof(RT_CBUF));
    printf("sizeof RT_CBUFBROAD  = %d\n",sizeof(RT_CBUFBROAD));
    printf("sizeof RT_FBUF = %d\n",sizeof(RT_FBUF));
    printf("sizeof RT_MBUF_HW  = %d\n",sizeof(RT_MBUF_HW));
    printf("sizeof RT_MBUF_API  = %d\n",sizeof(RT_MBUF_API));
    printf("sizeof RT_MBUF = %d\n",sizeof(RT_MBUF));
    printf("sizeof BT1553_TIME = %d\n",sizeof(BT1553_TIME));
    printf("sizeof unsigned  = %d\n",sizeof(unsigned));
    printf("sizeof short  = %d\n",sizeof(short));
    printf("sizeof long  = %d\n",sizeof(long));
    printf("sizeof long long  = %d\n",sizeof(long long));
}


