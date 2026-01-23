PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = vitaplus

C_OBJS =  \
	main.o \
	src/syspatch.o \
		
OBJS = \
	$(C_OBJS) imports.o

all: $(TARGET).prx

INCDIR = include rebootex
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic -fshort-wchar -mno-check-zero-division

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = 
LDFLAGS =  -nostartfiles
LIBS = -larkctrl -lpspsystemctrl_kernel -lpspkermit_driver

PSP_FW_VERSION = 660

include $(PSPSDK)/lib/build.mak
