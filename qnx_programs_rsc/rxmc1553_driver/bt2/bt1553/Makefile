CFLAGS = -c -D_QNXNTO_PCI_X86_ -g2 -w2 -I../Include -I./Source

BT1553_DIR = ./Source
COMMON_DIR = ../Common

SRCS = ${BT1553_DIR}/bc.c\
        ${BT1553_DIR}/bit.c \
        ${BT1553_DIR}/bm.c \
        ${BT1553_DIR}/rt.c \
        ${BT1553_DIR}/ei.c \
        ${BT1553_DIR}/hwsetup.c \
        ${BT1553_DIR}/btdrv.c \
        ${BT1553_DIR}/init.c \
        ${BT1553_DIR}/QNXBoardSetup.c \
        ${BT1553_DIR}/discrete.c \
        ${BT1553_DIR}/time.c \
        ${BT1553_DIR}/CEI_POSIX_INTERRUPT_FUNCTIONS.c \
        ${BT1553_DIR}/dumpmem.c \
        ${BT1553_DIR}/notify.c \
        ${BT1553_DIR}/qnx_irq_setup.c \
        ${COMMON_DIR}/mem_qnx.c

OBJS = bc.o \
       bit.o \
       bm.o \
       rt.o \
       ei.o \
       hwsetup.o \
       btdrv.o \
       init.o \
       QNXBoardSetup.o \
       discrete.o \
       time.o \
       CEI_POSIX_INTERRUPT_FUNCTIONS.o \
       dumpmem.o \
       notify.o \
       qnx_irq_setup.o \
       mem_qnx.o

all : x86 install clean

x86 : compx86 linkx86

compx86:
	qcc -Vgcc_ntox86 -shared $(CFLAGS) $(SRCS)

linkx86:
	qcc -Vgcc_ntox86 -shared -Wl,-soname libbusapi.so -o libbusapi.so $(OBJS) -L/x86/lib -L/x86/usr/lib -lm

install:
	cp libbusapi.so /usr/lib/libbusapi.so

clean:
	rm -f *.o libbusapi.*
