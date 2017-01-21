# v1.06

#*============================================================================*
#* FILE:                  Linux BusTools/1553 API Library Makefile
#*============================================================================*
#
# COPYRIGHT (C) 2006 - 2011 BY
#          GE INTELLIGENT PLATFORMS, INC., GOLETA, CALIFORNIA
#          ALL RIGHTS RESERVED.
#
#          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
#          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
#          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
#          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
#          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
#          SOFTWARE IS HEREBY TRANSFERRED.
#
#          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
#          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY GE 
#          INTELLIGENT PLATFORMS.
#
#============================================================================*
#
# Makefile options
#  Required:
#   none
#  Optional:
#   ARCH= - specify the architecture
#   32BIT=1 - builds 32-bit examples to execute on a 64-bit system
#   NO_INSTALL=1 - does not move the library to the /usr/lib(64) directory
#   CROSS_COMPILE= - specify the toolchain compiler for the target system
#   NO_WAITQUEUE=1 - build the library to not use a "wait queue" for detecting
#                    hardware interrupts, but instead use a POSIX signal. this
#                    is required for kernel 2.4.X
#
#============================================================================*


CC     = ${CROSS_COMPILE}gcc
TARGET = libbusapi.so
CFLAGS = -fPIC -c -I../Include -I./Source -g
LFLAGS = -shared -Wl,-soname -Wl,${TARGET}
TLIBS  = -lc -lpthread -lrt -lceill
LIBDIR = /usr/lib


ifeq (${ARCH}, x86)
 CFLAGS += -D_LINUX_X86_
endif
ifeq (${ARCH}, x86_64)
 CFLAGS += -D_LINUX_X86_
 ifeq (${32BIT}, 1)
  CFLAGS += -m32 
  LFLAGS += -m32
 else
  CFLAGS += -DLP64 -D_64_BIT_ -m64 -mno-red-zone
  LFLAGS += -m64
  LIBDIR  = /usr/lib64
 endif
endif
ifeq (${ARCH}, ppc)
 CFLAGS += -D_LINUX_PPC_
endif
ifeq (${ARCH}, ppc64)
 CFLAGS += -D_LINUX_PPC_
 ifeq (${32BIT}, 1)
  CFLAGS += -m32 
  LFLAGS += -m32
  TLIBS =
 else
  CFLAGS += -DLP64 -D_64_BIT_ -m64
  LFLAGS += -m64
  LIBDIR  = /usr/lib64
 endif
endif

ifneq (${CROSS_COMPILE}, )
  NO_INSTALL=1
endif
ifeq (${NO_WAITQUEUE}, 1)
  CFLAGS += -DPOSIX_SIGNAL 
endif

# CFLAGS += -DBT1553_LL_DEBUG
# CFLAGS += -D_REENTRANT
# CFLAGS += -D_GNU_SOURCE

SRCS = bc.c\
       bit.c \
       bm.c \
       rt.c \
       ei.c \
       hwsetup.c \
       btdrv.c \
       init.c \
       time.c \
       discrete.c \
       dumpmem.c \
       notify.c \
       playback.c \
       flash_config.c \
       irq_setup.c \
       linuxBoardSetup.c

OBJS += bc.o \
        bit.o \
        bm.o \
        rt.o \
        ei.o \
        hwsetup.o \
        btdrv.o \
        init.o \
        time.o \
        discrete.o \
        dumpmem.o \
        notify.o \
        playback.o \
	flash_config.o \
        irq_setup.o \
        linuxBoardSetup.o

ifeq ($(NO_INSTALL), 1)
 LFLAGS += -L../../../Lowlevel -Wl,-rpath=../../../Lowlevel
 all:  comp link clean
else
 all:  comp link install clean
endif	

comp:
	$(CC) $(CFLAGS) $(SRCS)

link:
	${CC} ${LFLAGS} -o ${TARGET} ${OBJS} ${TLIBS}

install:
	mv $(TARGET) ${LIBDIR}

clean:
	rm -f *.o *~ core
