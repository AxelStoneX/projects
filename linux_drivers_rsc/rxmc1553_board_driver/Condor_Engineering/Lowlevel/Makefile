# v1.07

#*=============================================================================*
#* FILE:                  Linux Lowlevel Library Makefile
#*=============================================================================*
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
#=============================================================================*
#
# Makefile options
#  Required:
#   KERNEL=24 - if using a Linux kernel 2.4.x
#  Optional:
#   NO_SYSFS=1 - disables support for the SYS file system
#   ARCH= - specify the architecture of the target system
#   32BIT=1 - compiles for 32-bit emulation in a 64-bit system
#   CROSS_COMPILE - specify the toolchain for the target system
#   NO_INSTALL=1 - does not move the library to the /usr/lib(64) directory
#   DEBUG=1 - enables debugging output
#
#==============================================================================*


CC      = ${CROSS_COMPILE}gcc
TARGET  = libceill.so
CFLAGS  = -fPIC -c -g -Wall
SRCS    = mem.c config.c ll_posix.c
OBJS    = mem.o config.o ll_posix.o
TLIBS   = -lc -lrt -lpthread
LFLAGS  = -shared -Wl,-soname -Wl,${TARGET}
LIBDIR  = /usr/lib 


ifeq (${KERNEL}, 24)
 CFLAGS += -DKERNEL_24
endif
ifeq (${NO_SYSFS}, 1)
 CFLAGS += -DNO_SYSFS_SUPPORT
endif
ifeq (${DEBUG}, 1)
 CFLAGS += -DLL_DEBUG
endif

ifeq (${ARCH}, x86_64)
 ifeq (${32BIT}, 1)
  CFLAGS += -D_32BIT -m32 
  LFLAGS += -m32
 else
  CFLAGS += -DLP64 -m64 -mno-red-zone
  LFLAGS += -m64
  LIBDIR  = /usr/lib64
 endif
endif

ifeq (${ARCH}, ppc64)
 ifeq (${32BIT}, 1)
  CFLAGS += -D_32BIT -m32 
  LFLAGS += -m32
 else
  CFLAGS += -DLP64 -m64
  LFLAGS += -m64
  LIBDIR  = /usr/lib64
 endif
endif

ifneq (${CROSS_COMPILE}, )
  NO_INSTALL=1
endif

#CFLAGS += -D_GNU_SOURCE -D_REENTRANT


ifeq ($(NO_INSTALL), 1)
 all:    comp link clean
else
 all:    comp link install clean
endif	

comp:
	${CC} ${CFLAGS} ${SRCS}
link:
	${CC} ${LFLAGS} -o ${TARGET} ${OBJS} ${TLIBS}

install:
	mv ${TARGET} ${LIBDIR}

clean:
	rm -f *.o *~ core
